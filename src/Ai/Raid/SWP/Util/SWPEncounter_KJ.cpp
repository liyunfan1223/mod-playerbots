/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_KJ.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SWPShared.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>
#include <mutex>

namespace SwpHelpers
{

// Note: Kil'jaeden's CombatReach is 15.0f

namespace
{

bool IsDragonGroupTarget(Player* bot, Player* member)
{
    return member && member != bot && member->IsAlive() &&
        member->GetMapId() == SWP_MAP_ID && !PlayerbotAI::IsTank(member);
}

uint32 GetDragonAppliedAuraSpell(uint32 spellId)
{
    switch (spellId)
    {
        case Id(SwpSpells::SPELL_DRAGON_BREATH_HASTE):
            return Id(SwpSpells::SPELL_DRAGON_BREATH_HASTE);
        case Id(SwpSpells::SPELL_DRAGON_BREATH_REVITALIZE):
            return Id(SwpSpells::SPELL_DRAGON_BREATH_REVITALIZE);
        default:
            return 0;
    }
}

bool HasAuraFromDragon(Player* member, uint32 spellId)
{
    uint32 const auraSpellId = GetDragonAppliedAuraSpell(spellId);
    return auraSpellId && member && member->HasAura(auraSpellId);
}

float GetRangedSlotAngle(uint8 slotIndex)
{
    Position position;
    if (!TryGetKiljaedenRangedSlotPosition(slotIndex, position))
        return 0.0f;

    return Position::NormalizeOrientation(std::atan2(
        position.GetPositionY() - SUNWELL_CENTER_POSITION.GetPositionY(),
        position.GetPositionX() - SUNWELL_CENTER_POSITION.GetPositionX()));
}

bool IsRangedSlotSafeFromArmageddons(
    Position const& position, std::vector<KiljaedenArmageddon> const& armageddons)
{
    for (KiljaedenArmageddon const& armageddon : armageddons)
    {
        if (position.GetExactDist2d(armageddon.destination) < armageddon.safeDistance)
            return false;
    }

    return true;
}

float GetNearestArmageddonDistance(
    Position const& position, std::vector<KiljaedenArmageddon> const& armageddons)
{
    float nearestDistance = std::numeric_limits<float>::max();

    for (KiljaedenArmageddon const& armageddon : armageddons)
    {
        nearestDistance = std::min(
            nearestDistance, position.GetExactDist2d(armageddon.destination));
    }

    return nearestDistance;
}

bool ShouldRebuildKiljaedenAssignments(uint32& lastRebuildMs, uint32 intervalMs)
{
    uint32 const now = getMSTime();
    if (lastRebuildMs && getMSTimeDiff(lastRebuildMs, now) < intervalMs)
        return false;

    lastRebuildMs = now;
    return true;
}

std::mutex kiljaedenStateMutex;
std::unordered_map<uint32, KiljaedenEncounterState> kiljaedenStates;

} // end anonymous namespace

GuidVector FindKiljaedenHandGuids(Player* bot)
{
    GuidVector guids;

    std::list<Creature*> creatures;
    bot->GetCreatureListWithEntryInGrid(
        creatures, Id(SwpNpcs::NPC_HAND_OF_THE_DECEIVER), HAND_SEARCH_RADIUS);

    for (Creature* creature : creatures)
    {
        if (creature && creature->IsAlive() && creature->IsInCombat())
            guids.push_back(creature->GetGUID());
    }

    std::sort(guids.begin(), guids.end());

    return guids;
}

std::vector<Unit*> GetKiljaedenHands(PlayerbotAI* botAI)
{
    std::vector<Unit*> hands;

    for (auto const& guid :
         botAI->GetAiObjectContext()->GetValue<GuidVector>("kiljaeden hands")->RefGet())
    {
        Unit* hand = botAI->GetUnit(guid);
        if (hand && hand->IsAlive())
            hands.push_back(hand);
    }

    return hands;
}

bool IsKiljaedenHandControlClaimed(Unit* hand)
{
    KiljaedenEncounterState* state = FindKiljaedenState(hand->GetInstanceId());
    if (!state)
        return false;

    auto const claimItr = state->handControlClaims.find(hand->GetGUID());
    if (claimItr == state->handControlClaims.end())
        return false;

    if (claimItr->second > getMSTime())
        return true;

    state->handControlClaims.erase(claimItr);
    return false;
}

void ClaimKiljaedenHandControl(Unit* hand)
{
    KiljaedenState(hand->GetInstanceId()).handControlClaims[hand->GetGUID()] =
        getMSTime() + HAND_CONTROL_CLAIM_MS;
}

void AddKiljaedenArmageddon(
    uint32 instanceId, Position const& destination, uint32 durationMs, float safeDistance)
{
    if (!durationMs || safeDistance <= 0.0f)
        return;

    uint32 const now = getMSTime();
    PruneExpiredKiljaedenArmageddons(instanceId);

    KiljaedenArmageddon armageddon;
    armageddon.destination = destination;
    armageddon.expireMs = now + durationMs;
    armageddon.safeDistance = safeDistance;
    KiljaedenState(instanceId).armageddons.push_back(armageddon);
}

bool TryGetKiljaedenNearestArmageddon(Player* bot, KiljaedenArmageddon& armageddon)
{
    PruneExpiredKiljaedenArmageddons(bot->GetInstanceId());
    KiljaedenEncounterState const* state = FindKiljaedenState(bot->GetInstanceId());
    if (!state)
        return false;

    bool foundArmageddon = false;
    float bestDistance = std::numeric_limits<float>::max();

    for (KiljaedenArmageddon const& candidate : state->armageddons)
    {
        float const distance = bot->GetExactDist2d(candidate.destination);
        if (distance >= candidate.safeDistance)
            continue;

        if (distance < bestDistance)
        {
            armageddon = candidate;
            bestDistance = distance;
            foundArmageddon = true;
        }
    }

    return foundArmageddon;
}

void PruneExpiredKiljaedenArmageddons(uint32 instanceId)
{
    KiljaedenEncounterState* state = FindKiljaedenState(instanceId);
    if (!state)
        return;

    uint32 const now = getMSTime();
    std::vector<KiljaedenArmageddon>& armageddons = state->armageddons;
    armageddons.erase(std::remove_if(armageddons.begin(), armageddons.end(),
        [now](KiljaedenArmageddon const& armageddon) {
            return !armageddon.expireMs || armageddon.expireMs <= now;
        }), armageddons.end());
}

bool TryGetKiljaedenRangedSlotPosition(uint8 slotIndex, Position& position)
{
    if (slotIndex >= KILJAEDEN_TOTAL_RANGED_SLOT_COUNT)
        return false;

    float radius = KILJAEDEN_OUTER_RANGED_RADIUS;
    uint8 localSlotIndex = slotIndex;
    uint8 slotCount = KILJAEDEN_OUTER_RANGED_SLOT_COUNT;

    if (slotIndex < KILJAEDEN_INNER_RANGED_SLOT_COUNT)
    {
        radius = KILJAEDEN_INNER_RANGED_RADIUS;
        slotCount = KILJAEDEN_INNER_RANGED_SLOT_COUNT;
    }
    else
    {
        localSlotIndex -= KILJAEDEN_INNER_RANGED_SLOT_COUNT;
    }

    float const angleOffset = GetCenteredArcSlotAngleOffset(localSlotIndex, slotCount, M_PI);
    float const angle = Position::NormalizeOrientation(
        KILJAEDEN_RANGED_ARC_ORIENTATION + angleOffset);

    Position const& center = SUNWELL_CENTER_POSITION;
    float const positionX = center.GetPositionX() + std::cos(angle) * radius;
    float const positionY = center.GetPositionY() + std::sin(angle) * radius;

    position = Position{ positionX, positionY, center.GetPositionZ() };
    return true;
}

void EnsureKiljaedenRangedAssignments(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    KiljaedenEncounterState& state = KiljaedenState(bot->GetInstanceId());
    if (!ShouldRebuildKiljaedenAssignments(
            state.rangedAssignmentRebuildMs, KILJAEDEN_RANGED_ASSIGNMENT_REBUILD_INTERVAL_MS))
    {
        return;
    }

    auto& assignments = state.rangedAssignments;

    std::vector<ObjectGuid> invalidAssignments;
    for (auto const& assignment : assignments)
    {
        bool found = false;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetGUID() != assignment.first)
                continue;

            found = member->GetMapId() == SWP_MAP_ID && GET_PLAYERBOT_AI(member) &&
                PlayerbotAI::IsRanged(member);

            break;
        }

        if (!found)
            invalidAssignments.push_back(assignment.first);
    }

    for (ObjectGuid const& guid : invalidAssignments)
        assignments.erase(guid);

    std::array<bool, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> usedSlots = {};
    for (auto const& assignment : assignments)
    {
        if (assignment.second < KILJAEDEN_TOTAL_RANGED_SLOT_COUNT)
            usedSlots[assignment.second] = true;
    }

    auto const assignNextOpenSlot = [&](Player* member)
    {
        for (uint8 slotIndex = 0; slotIndex < KILJAEDEN_TOTAL_RANGED_SLOT_COUNT;
             ++slotIndex)
        {
            if (usedSlots[slotIndex])
                continue;

            assignments[member->GetGUID()] = slotIndex;
            usedSlots[slotIndex] = true;
            return;
        }
    };

    std::vector<Player*> healers;
    std::vector<Player*> rangedDamage;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SWP_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        if (assignments.find(member->GetGUID()) != assignments.end())
            continue;

        if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else
            rangedDamage.push_back(member);
    }

    auto const sortByGuid = [](std::vector<Player*>& members)
    {
        std::sort(members.begin(), members.end(),
            [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); });
    };

    sortByGuid(healers);
    sortByGuid(rangedDamage);

    for (Player* member : healers)
        assignNextOpenSlot(member);

    for (Player* member : rangedDamage)
        assignNextOpenSlot(member);
}

void EnsureKiljaedenRangedArmageddonAssignments(Player* bot)
{
    struct CandidateSlotScore
    {
        uint8 slotIndex = 0;
        bool sameRow = false;
        float angleDistance = 0.0f;
        uint8 occupancy = 0;
        float armageddonDistance = 0.0f;
    };

    uint32 const instanceId = bot->GetInstanceId();
    PruneExpiredKiljaedenArmageddons(instanceId);

    KiljaedenEncounterState* foundState = FindKiljaedenState(instanceId);
    if (!foundState)
        return;

    KiljaedenEncounterState& state = *foundState;

    Group* group = bot->GetGroup();
    if (state.armageddons.empty() || !group)
    {
        state.rangedArmageddonAssignments.clear();
        return;
    }

    // For bots to return to their normal positions once Armageddons stop.
    if (!ShouldRebuildKiljaedenAssignments(
            state.rangedArmageddonRebuildMs, ARMAGEDDON_ASSIGNMENT_REBUILD_INTERVAL_MS))
    {
        return;
    }

    auto const& armageddons = state.armageddons;
    auto const& canonicalAssignments = state.rangedAssignments;

    std::vector<KiljaedenRangedBotAssignment> rangedBots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SWP_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        auto const assignmentItr = canonicalAssignments.find(member->GetGUID());
        if (assignmentItr == canonicalAssignments.end() ||
            assignmentItr->second >= KILJAEDEN_TOTAL_RANGED_SLOT_COUNT)
        {
            continue;
        }

        rangedBots.push_back({ member->GetGUID(), assignmentItr->second });
    }

    std::sort(rangedBots.begin(), rangedBots.end(),
        [](KiljaedenRangedBotAssignment const& left, KiljaedenRangedBotAssignment const& right)
        {
            if (left.slotIndex != right.slotIndex)
                return left.slotIndex < right.slotIndex;

            return left.guid < right.guid;
        });

    std::array<bool, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> safeSlots = {};
    std::array<float, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> slotAngles = {};
    std::array<float, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> nearestArmageddonDistances = {};

    for (uint8 slotIndex = 0; slotIndex < KILJAEDEN_TOTAL_RANGED_SLOT_COUNT; ++slotIndex)
    {
        Position slotPosition;
        if (!TryGetKiljaedenRangedSlotPosition(slotIndex, slotPosition))
            continue;

        safeSlots[slotIndex] = IsRangedSlotSafeFromArmageddons(slotPosition, armageddons);
        slotAngles[slotIndex] = GetRangedSlotAngle(slotIndex);
        nearestArmageddonDistances[slotIndex] =
            GetNearestArmageddonDistance(slotPosition, armageddons);
    }

    std::array<uint8, KILJAEDEN_TOTAL_RANGED_SLOT_COUNT> plannedOccupancy = {};
    auto& tempAssignments = state.rangedArmageddonAssignments;
    tempAssignments.clear();

    auto const getCandidateScore =
        [&](KiljaedenRangedBotAssignment const& rangedBot, uint8 candidateSlotIndex)
    {
        CandidateSlotScore score;
        score.slotIndex = candidateSlotIndex;
        score.sameRow =
            (candidateSlotIndex < KILJAEDEN_INNER_RANGED_SLOT_COUNT) ==
            (rangedBot.slotIndex < KILJAEDEN_INNER_RANGED_SLOT_COUNT);

        float angleDistance = Position::NormalizeOrientation(
            slotAngles[candidateSlotIndex] - slotAngles[rangedBot.slotIndex]);
        if (angleDistance > static_cast<float>(M_PI))
            angleDistance -= 2.0f * static_cast<float>(M_PI);

        score.angleDistance = std::fabs(angleDistance);
        score.occupancy = plannedOccupancy[candidateSlotIndex];
        score.armageddonDistance = nearestArmageddonDistances[candidateSlotIndex];
        return score;
    };

    auto const shouldTakeCandidate =
        [&](CandidateSlotScore const& candidate, CandidateSlotScore const& best, bool bestFound)
    {
        if (!bestFound)
            return true;

        if (candidate.sameRow != best.sameRow)
            return candidate.sameRow;

        if (candidate.angleDistance != best.angleDistance)
            return candidate.angleDistance < best.angleDistance;

        if (candidate.occupancy != best.occupancy)
            return candidate.occupancy < best.occupancy;

        if (candidate.armageddonDistance != best.armageddonDistance)
            return candidate.armageddonDistance > best.armageddonDistance;

        return candidate.slotIndex < best.slotIndex;
    };

    std::vector<KiljaedenRangedBotAssignment> displacedBots;
    for (KiljaedenRangedBotAssignment const& rangedBot : rangedBots)
    {
        if (!safeSlots[rangedBot.slotIndex])
        {
            displacedBots.push_back(rangedBot);
            continue;
        }

        tempAssignments[rangedBot.guid] = rangedBot.slotIndex;
        ++plannedOccupancy[rangedBot.slotIndex];
    }

    for (KiljaedenRangedBotAssignment const& rangedBot : displacedBots)
    {
        bool bestFound = false;
        CandidateSlotScore bestCandidate;
        bestCandidate.slotIndex = rangedBot.slotIndex;

        for (uint8 candidateSlotIndex = 0;
             candidateSlotIndex < KILJAEDEN_TOTAL_RANGED_SLOT_COUNT; ++candidateSlotIndex)
        {
            if (!safeSlots[candidateSlotIndex] ||
                plannedOccupancy[candidateSlotIndex] >= KILJAEDEN_MAX_BOTS_PER_RANGED_SLOT)
            {
                continue;
            }

            const CandidateSlotScore candidate = getCandidateScore(rangedBot, candidateSlotIndex);
            if (!shouldTakeCandidate(candidate, bestCandidate, bestFound))
                continue;

            bestFound = true;
            bestCandidate = candidate;
        }

        tempAssignments[rangedBot.guid] = bestCandidate.slotIndex;
        if (bestFound)
            ++plannedOccupancy[bestCandidate.slotIndex];
    }
}

bool IsKiljaedenCastingDarknessOfAThousandSouls(Unit* kiljaeden)
{
    return kiljaeden && kiljaeden->HasUnitState(UNIT_STATE_CASTING) &&
        kiljaeden->FindCurrentSpellBySpellId(Id(SwpSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS));
}

GuidVector FindKiljaedenDragonOrbGuids(Player* bot)
{
    GuidVector guids;
    guids.reserve(KILJAEDEN_DRAGON_ORB_ENTRIES.size());

    for (uint32 const orbEntry : KILJAEDEN_DRAGON_ORB_ENTRIES)
    {
        if (GameObject* orb =
                bot->FindNearestGameObject(orbEntry, DRAGON_ORB_SEARCH_RADIUS, true))
        {
            guids.push_back(orb->GetGUID());
        }
    }

    return guids;
}

Player* GetKiljaedenDragonOrbUser(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != SWP_MAP_ID ||
            !GET_PLAYERBOT_AI(member))
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member;
    }

    return nullptr;
}

bool HasUsedKiljaedenDragonOrb(Player* bot)
{
    KiljaedenEncounterState const* state = FindKiljaedenState(bot->GetInstanceId());
    return state && state->dragonOrbUsers.contains(bot->GetGUID());
}

bool HasKiljaedenDragonAura(Player* bot)
{
    return bot->HasAura(Id(SwpSpells::SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT));
}

Unit* GetKiljaedenControlledDragon(Player* bot)
{
    Unit* dragon = bot->GetCharm();
    if (!dragon || !dragon->IsAlive() ||
        dragon->GetEntry() != Id(SwpNpcs::NPC_POWER_OF_THE_BLUE_FLIGHT))
    {
        return nullptr;
    }

    return dragon;
}

bool CastKiljaedenDragonSpell(Unit* dragon, uint32 spellId)
{
    if (!dragon || dragon->HasSpellCooldown(spellId))
        return false;

    dragon->CastSpell(dragon, spellId, true);
    dragon->AddSpellCooldown(spellId, 0, GetManualCastCooldown(spellId));

    // The engine records no global cooldown for a triggered cast, so hold the dragon's other
    // abilities here. Without it, Haste and Revitalize go out on consecutive ticks.
    if (uint32 const globalCooldownMs = GetManualCastGlobalCooldown(spellId))
    {
        for (uint32 otherSpellId : KILJAEDEN_DRAGON_SPELLS)
        {
            if (otherSpellId != spellId && !dragon->HasSpellCooldown(otherSpellId))
                dragon->AddSpellCooldown(otherSpellId, 0, globalCooldownMs);
        }
    }

    return true;
}

Player* FindBestKiljaedenDragonClusterTarget(Player* bot, Unit* dragon, uint32 spellId)
{
    if (!dragon)
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* bestTarget = nullptr;
    uint32 bestClusterSize = 0;
    uint32 bestTotalClusterSize = 0;
    float bestDistanceToDragon = 0.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* candidate = ref->GetSource();
        if (!IsDragonGroupTarget(bot, candidate) || HasAuraFromDragon(candidate, spellId))
            continue;

        uint32 clusterSize = 0;
        uint32 totalClusterSize = 0;
        for (GroupReference* otherRef = group->GetFirstMember();
             otherRef; otherRef = otherRef->next())
        {
            Player* other = otherRef->GetSource();
            if (!IsDragonGroupTarget(bot, other) ||
                candidate->GetExactDist2d(other) > KILJAEDEN_DRAGON_CLUSTER_RADIUS)
            {
                continue;
            }

            ++totalClusterSize;
            if (!HasAuraFromDragon(other, spellId))
                ++clusterSize;
        }

        if (clusterSize < KILJAEDEN_DRAGON_MIN_CLUSTER_SIZE)
            continue;

        float const distanceToDragon = dragon->GetExactDist2d(candidate);

        auto const isBetter = [&]() -> bool
        {
            if (!bestTarget)
                return true;
            if (clusterSize != bestClusterSize)
                return clusterSize > bestClusterSize;
            if (totalClusterSize != bestTotalClusterSize)
                return totalClusterSize > bestTotalClusterSize;
            if (distanceToDragon != bestDistanceToDragon)
                return distanceToDragon < bestDistanceToDragon;
            return candidate->GetGUID() < bestTarget->GetGUID();
        };

        if (isBetter())
        {
            bestTarget = candidate;
            bestClusterSize = clusterSize;
            bestTotalClusterSize = totalClusterSize;
            bestDistanceToDragon = distanceToDragon;
        }
    }

    return bestTarget;
}

Player* FindClosestKiljaedenDragonTarget(Player* bot, Unit* dragon, uint32 spellId)
{
    if (!dragon)
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* closestTarget = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!IsDragonGroupTarget(bot, member) || HasAuraFromDragon(member, spellId))
            continue;

        float const distance = dragon->GetExactDist2d(member);
        if (distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

bool HasStaleRootFlag(Player* bot)
{
    // This helper ensures no genuine roots are cleared because one of the unit states below is
    // always present during a genuine root via Unit::SetRooted.
    return bot->IsRooted() &&
        !bot->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_LOGOUT_TIMER);
}

bool ReleaseStaleRootFlag(Player* bot)
{
    if (!HasStaleRootFlag(bot))
        return false;

    bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_ROOT);
    bot->SendMovementFlagUpdate();
    return true;
}

// Shared encounter state

KiljaedenEncounterState& KiljaedenState(uint32 instanceId)
{
    std::lock_guard lock(kiljaedenStateMutex);
    return kiljaedenStates[instanceId];
}

KiljaedenEncounterState* FindKiljaedenState(uint32 instanceId)
{
    std::lock_guard lock(kiljaedenStateMutex);
    auto it = kiljaedenStates.find(instanceId);
    return it != kiljaedenStates.end() ? &it->second : nullptr;
}

bool KiljaedenResetInstance(uint32 instanceId)
{
    std::lock_guard lock(kiljaedenStateMutex);
    return kiljaedenStates.erase(instanceId) > 0;
}

}
