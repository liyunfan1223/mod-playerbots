/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Kalec.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include <algorithm>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace SwpHelpers
{

// Note: Kalecgos's CombatReach is 10.5f, and Sathrovarr's CombatReach is 4.0f (4.5f in PR 28010).
// Note: Kalecgos remains on player threat lists for the duration of the encounter, even for
// players in the Spectral Realm and after he turns friendly after Sathrovarr is killed.

namespace
{

std::mutex kalecgosStateMutex;
std::unordered_map<uint32, KalecgosEncounterState> kalecgosStates;

void ClearExpiredActiveRift(KalecgosEncounterState& state, uint32 now)
{
    if (!state.activeRiftOpenedMs)
        return;

    if (getMSTimeDiff(state.activeRiftOpenedMs, now) <= SPECTRAL_RIFT_ACTIVE_WINDOW_MS)
        return;

    state.activeRiftOpenedMs = 0;
    state.activeRiftGroup = KALECGOS_INVALID_GROUP;
    state.blastedPlayerGuid = ObjectGuid::Empty;
    state.activeRiftOutgoingTankGuid = ObjectGuid::Empty;
}

uint8 GetAssignedGroup(KalecgosEncounterState const& state, ObjectGuid playerGuid)
{
    auto const assignment = state.playerToGroup.find(playerGuid);
    return assignment != state.playerToGroup.end() ? assignment->second : KALECGOS_INVALID_GROUP;
}

KalecgosEncounterState& GetPreparedEncounterState(Player* player)
{
    KalecgosEncounterState& state = KalecgosState(player->GetInstanceId());
    if (!state.encounterStartMs)
        state.encounterStartMs = getMSTime();

    ClearExpiredActiveRift(state, getMSTime());
    EnsureKalecgosRaidAssignments(player);
    return state;
}

bool CanReachPortalBeforeExpiry(Player* bot)
{
    Aura* exhaustion = bot->GetAura(Id(SwpSpells::SPELL_SPECTRAL_EXHAUSTION));
    if (!exhaustion)
        return true;

    return exhaustion->GetDuration() <= SPECTRAL_RIFT_ENTRY_WINDOW_MS;
}

bool IsPortalEligibleCandidate(Player* bot)
{
    if (!bot->IsAlive() || bot->GetMapId() != SWP_MAP_ID || !GET_PLAYERBOT_AI(bot))
        return false;

    return !IsInSpectralRealm(bot) && CanReachPortalBeforeExpiry(bot);
}

void AnnounceTankTransition(
    PlayerbotAI* botAI, std::string const& textId, std::string const& defaultText,
    std::map<std::string, std::string> const& placeholders)
{
    std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
        textId, defaultText, placeholders);

    if (botAI)
        botAI->SayToRaid(text);
}

std::array<ObjectGuid, KALECGOS_TANK_COUNT> GetExpectedTankAssignmentGuids(Player* player)
{
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };

    Group* group = player->GetGroup();
    if (!group)
        return tankGuids;

    ObjectGuid const mainTankGuid = PlayerbotAI::GetMainTankGuid(group);

    Player* mainTank = nullptr;
    std::vector<Player*> assistTanks;
    std::vector<Player*> otherTanks;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;

        if (member->GetGUID() == mainTankGuid)
        {
            mainTank = member;
            continue;
        }

        if (!PlayerbotAI::IsTank(member))
            continue;

        if (group->IsAssistant(member->GetGUID()))
            assistTanks.push_back(member);
        else
            otherTanks.push_back(member);
    }

    assistTanks.insert(assistTanks.end(), otherTanks.begin(), otherTanks.end());

    auto const assignSlot = [&tankGuids](uint8 slot, Player* tank)
    {
        if (tank && tank->GetMapId() == SWP_MAP_ID)
            tankGuids[slot] = tank->GetGUID();
    };

    assignSlot(0, mainTank);
    for (uint8 index = 0; index < KALECGOS_TANK_COUNT - 1 && index < assistTanks.size(); ++index)
        assignSlot(index + 1, assistTanks[index]);

    return tankGuids;
}

std::array<ObjectGuid, KALECGOS_TANK_COUNT> BuildInitialTankPortalRotationGuids(
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& tankAssignmentGuids)
{
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> rotationGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };

    uint8 nextIndex = 0;

    auto const appendGuid = [&](ObjectGuid guid)
    {
        if (guid == ObjectGuid::Empty || nextIndex >= KALECGOS_TANK_COUNT)
            return;

        if (std::find(rotationGuids.begin(), rotationGuids.end(), guid) == rotationGuids.end())
            rotationGuids[nextIndex++] = guid;
    };

    appendGuid(tankAssignmentGuids[2]);
    appendGuid(tankAssignmentGuids[1]);
    appendGuid(tankAssignmentGuids[0]);

    return rotationGuids;
}

bool HasKalecgosTankAssignment(
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& tankAssignmentGuids, ObjectGuid guid)
{
    if (guid == ObjectGuid::Empty)
        return false;

    return std::find(tankAssignmentGuids.begin(), tankAssignmentGuids.end(), guid) !=
        tankAssignmentGuids.end();
}

std::array<ObjectGuid, KALECGOS_TANK_COUNT> RebuildTankPortalRotationGuids(
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& existingRotationGuids,
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& tankAssignmentGuids)
{
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> rotationGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };

    uint8 nextIndex = 0;

    auto const appendGuid = [&](ObjectGuid guid)
    {
        if (!HasKalecgosTankAssignment(tankAssignmentGuids, guid) ||
            nextIndex >= KALECGOS_TANK_COUNT)
        {
            return;
        }

        if (std::find(rotationGuids.begin(), rotationGuids.end(), guid) == rotationGuids.end())
            rotationGuids[nextIndex++] = guid;
    };

    for (ObjectGuid guid : existingRotationGuids)
        appendGuid(guid);

    for (ObjectGuid guid : BuildInitialTankPortalRotationGuids(tankAssignmentGuids))
        appendGuid(guid);

    return rotationGuids;
}

Player* ResolveSurfaceTank(Group* group, ObjectGuid guid)
{
    if (!group || guid == ObjectGuid::Empty)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetGUID() != guid)
            continue;

        if (!member->IsAlive() || member->GetMapId() != SWP_MAP_ID || IsInSpectralRealm(member))
            return nullptr;

        return member;
    }

    return nullptr;
}

Player* GetFirstResolvedSurfaceTank(
    Group* group, std::array<ObjectGuid, KALECGOS_TANK_COUNT> const& orderedGuids,
    ObjectGuid firstExcludedGuid = ObjectGuid::Empty,
    ObjectGuid secondExcludedGuid = ObjectGuid::Empty)
{
    for (ObjectGuid guid : orderedGuids)
    {
        if (guid == firstExcludedGuid || guid == secondExcludedGuid)
            continue;

        if (Player* tank = ResolveSurfaceTank(group, guid))
            return tank;
    }

    return nullptr;
}

Player* GetNextSurfaceTankForPortal(
    Group* group, KalecgosEncounterState const& state,
    ObjectGuid firstExcludedGuid = ObjectGuid::Empty,
    ObjectGuid secondExcludedGuid = ObjectGuid::Empty)
{
    if (Player* tank = GetFirstResolvedSurfaceTank(
            group, state.tankPortalRotationGuids, firstExcludedGuid, secondExcludedGuid))
    {
        return tank;
    }

    return GetFirstResolvedSurfaceTank(
        group, state.tankAssignmentGuids, firstExcludedGuid, secondExcludedGuid);
}

// The next tank in the rotation is the surface tank that has been out of the Spectral Realm
// longest.
Player* GetReplacementSurfaceTank(
    Group* group, KalecgosEncounterState const& state, ObjectGuid departingGuid,
    ObjectGuid excludedGuid = ObjectGuid::Empty)
{
    if (Player* replacement = GetFirstResolvedSurfaceTank(
            group, state.tankPortalRotationGuids, departingGuid, excludedGuid))
    {
        return replacement;
    }

    return GetFirstResolvedSurfaceTank(
        group, state.tankAssignmentGuids, departingGuid, excludedGuid);
}

Player* GetSurfaceTankAfterCurrentHandOff(Group* group, KalecgosEncounterState const& state)
{
    ObjectGuid const currentTankGuid = state.currentTankGuid;
    if (currentTankGuid == ObjectGuid::Empty)
        return nullptr;

    if (currentTankGuid != state.activeRiftOutgoingTankGuid &&
        currentTankGuid != state.blastedPlayerGuid)
    {
        return nullptr;
    }

    return GetReplacementSurfaceTank(group, state, currentTankGuid);
}

Player* GetKalecgosCurrentVictimTank(
    Player* player, Group* group, KalecgosEncounterState const& state)
{
    Unit* kalecgos = nullptr;

    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
    {
        AiObjectContext* context = botAI->GetAiObjectContext();
        kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    }

    constexpr float searchRadius = 200.0f;
    if (!kalecgos)
        kalecgos = player->FindNearestCreature(Id(SwpNpcs::NPC_KALECGOS_DRAGON), searchRadius);

    if (kalecgos)
    {
        Unit* victim = kalecgos->GetVictim();
        if (victim && victim->IsPlayer())
        {
            Player* currentVictim = victim->ToPlayer();
            if (HasKalecgosTankAssignment(state.tankAssignmentGuids, currentVictim->GetGUID()))
                return currentVictim;
        }
    }

    return GetFirstResolvedSurfaceTank(group, state.tankAssignmentGuids);
}

Player* ResolveKalecgosDesignatedTank(
    Player* player, Group* group, KalecgosEncounterState const& state)
{
    if (Player* tank = ResolveSurfaceTank(group, state.currentTankGuid))
    {
        if (Player* replacementTank = GetSurfaceTankAfterCurrentHandOff(group, state))
            return replacementTank;

        return tank;
    }

    return GetKalecgosCurrentVictimTank(player, group, state);
}

Player* SelectOutgoingTankForRift(Group* group, KalecgosEncounterState const& state)
{
    if (!state.activeRiftOpenedMs ||
        HasKalecgosTankAssignment(state.tankAssignmentGuids, state.blastedPlayerGuid))
    {
        return nullptr;
    }

    uint8 surfaceTankCount = 0;
    for (ObjectGuid guid : state.tankAssignmentGuids)
    {
        if (ResolveSurfaceTank(group, guid))
            ++surfaceTankCount;
    }
    if (surfaceTankCount <= 1)
        return nullptr;

    if (Player* nextTank = GetNextSurfaceTankForPortal(group, state);
        nextTank && !IsExhausted(nextTank))
    {
        return nextTank;
    }

    return nullptr;
}

void AssignKalecgosTankTargetsForActiveRift(
    Player* player, Group* group, KalecgosEncounterState& state)
{
    Player* currentTank = GetKalecgosCurrentVictimTank(player, group, state);
    Player* outgoingTank = SelectOutgoingTankForRift(group, state);

    state.activeRiftOutgoingTankGuid = outgoingTank ?
        outgoingTank->GetGUID() : ObjectGuid::Empty;

    if (!currentTank)
    {
        currentTank = GetNextSurfaceTankForPortal(
            group, state, state.activeRiftOutgoingTankGuid);
    }

    state.currentTankGuid = currentTank ? currentTank->GetGUID() : ObjectGuid::Empty;
}

void AdvanceKalecgosTankPortalRotation(KalecgosEncounterState& state, ObjectGuid tankGuid)
{
    if (!HasKalecgosTankAssignment(state.tankAssignmentGuids, tankGuid))
        return;

    std::array<ObjectGuid, KALECGOS_TANK_COUNT> rotationGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };

    uint8 nextIndex = 0;

    for (ObjectGuid guid : state.tankPortalRotationGuids)
    {
        if (guid == ObjectGuid::Empty || guid == tankGuid)
            continue;

        rotationGuids[nextIndex++] = guid;
    }

    if (nextIndex < KALECGOS_TANK_COUNT)
        rotationGuids[nextIndex] = tankGuid;

    state.tankPortalRotationGuids = RebuildTankPortalRotationGuids(
        rotationGuids, state.tankAssignmentGuids);
}

bool GroupHasEligibleEntrant(Group* group, KalecgosEncounterState const& state, uint8 groupIndex)
{
    if (!group || groupIndex >= KALECGOS_GROUP_COUNT)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || GetAssignedGroup(state, member->GetGUID()) != groupIndex)
            continue;

        if (IsPortalEligibleCandidate(member) && state.blastedPlayerGuid != member->GetGUID())
            return true;
    }

    return false;
}

uint8 GetNextAvailablePortalGroup(Group* group, KalecgosEncounterState const& state)
{
    for (uint8 groupIndex = 0; groupIndex < KALECGOS_GROUP_COUNT; ++groupIndex)
    {
        if (GroupHasEligibleEntrant(group, state, groupIndex))
            return groupIndex;
    }

    return KALECGOS_INVALID_GROUP;
}

uint8 ResolveActivePortalGroup(Group* group, KalecgosEncounterState const& state)
{
    if (state.blastedPlayerGuid == ObjectGuid::Empty)
        return KALECGOS_INVALID_GROUP;

    uint8 const blastedGroup = GetAssignedGroup(state, state.blastedPlayerGuid);
    if (blastedGroup != KALECGOS_INVALID_GROUP &&
        GroupHasEligibleEntrant(group, state, blastedGroup))
    {
        return blastedGroup;
    }

    return GetNextAvailablePortalGroup(group, state);
}

void AssignPlayerToGroup(
    KalecgosEncounterState& state, std::array<size_t, KALECGOS_GROUP_COUNT>& groupSizes,
    std::array<bool, KALECGOS_GROUP_COUNT>& groupHasDecurser,
    Player* member, uint8 groupIndex)
{
    if (!member || groupIndex >= KALECGOS_GROUP_COUNT)
        return;

    state.playerToGroup[member->GetGUID()] = groupIndex;
    ++groupSizes[groupIndex];

    if (GET_PLAYERBOT_AI(member))
        groupHasDecurser[groupIndex] = groupHasDecurser[groupIndex] || IsKalecgosDecurser(member);
}

uint8 GetLeastFilledGroup(
    std::array<size_t, KALECGOS_GROUP_COUNT> const& groupSizes,
    std::array<bool, KALECGOS_GROUP_COUNT> const* requiredFlags = nullptr,
    bool preferMissingFlag = false)
{
    uint8 bestGroup = KALECGOS_INVALID_GROUP;
    size_t smallestSize = std::numeric_limits<size_t>::max();

    for (uint8 groupIndex = 0; groupIndex < KALECGOS_GROUP_COUNT; ++groupIndex)
    {
        if (requiredFlags && preferMissingFlag && (*requiredFlags)[groupIndex])
            continue;

        if (groupSizes[groupIndex] < smallestSize)
        {
            bestGroup = groupIndex;
            smallestSize = groupSizes[groupIndex];
        }
    }

    if (bestGroup != KALECGOS_INVALID_GROUP || !requiredFlags || !preferMissingFlag)
        return bestGroup;

    return GetLeastFilledGroup(groupSizes);
}

} // end anonymous namespace

bool IsExhausted(Player* bot)
{
    return bot->HasAura(Id(SwpSpells::SPELL_SPECTRAL_EXHAUSTION));
}

bool IsInSpectralRealm(Player* bot)
{
    return bot->HasAura(Id(SwpSpells::SPELL_SPECTRAL_REALM));
}

bool IsKalecgosDecurser(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
        case CLASS_SHAMAN:
            break;
        case CLASS_DRUID:
        {
            if (!PlayerbotAI::IsRanged(bot))
                return false;
            break;
        }
        default:
            return false;
    }

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    return botAI->HasStrategy("cure", BOT_STATE_COMBAT);
}

void EnsureKalecgosRaidAssignments(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
        return;

    KalecgosEncounterState& state = KalecgosState(player->GetInstanceId());
    std::vector<Player*> botMembers;
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> const expectedTankAssignmentGuids =
        GetExpectedTankAssignmentGuids(player);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SWP_MAP_ID)
            continue;

        if (GET_PLAYERBOT_AI(member) && !PlayerbotAI::IsTank(member))
            botMembers.push_back(member);
    }

    bool needsRebuild = state.playerToGroup.size() != botMembers.size() ||
        state.tankAssignmentGuids != expectedTankAssignmentGuids;
    if (!needsRebuild)
    {
        for (Player* member : botMembers)
        {
            if (state.playerToGroup.find(member->GetGUID()) == state.playerToGroup.end())
            {
                needsRebuild = true;
                break;
            }
        }
    }

    if (!needsRebuild)
        return;

    state.playerToGroup.clear();
    state.tankAssignmentGuids = expectedTankAssignmentGuids;
    state.tankPortalRotationGuids = RebuildTankPortalRotationGuids(
        state.tankPortalRotationGuids, state.tankAssignmentGuids);

    if (!HasKalecgosTankAssignment(state.tankAssignmentGuids, state.currentTankGuid))
    {
        if (Player* fallbackTank = GetKalecgosCurrentVictimTank(player, group, state))
            state.currentTankGuid = fallbackTank->GetGUID();
        else
            state.currentTankGuid = ObjectGuid::Empty;
    }

    if (!HasKalecgosTankAssignment(state.tankAssignmentGuids, state.activeRiftOutgoingTankGuid))
        state.activeRiftOutgoingTankGuid = ObjectGuid::Empty;

    std::array<size_t, KALECGOS_GROUP_COUNT> groupSizes = { 0, 0, 0, 0 };
    std::array<bool, KALECGOS_GROUP_COUNT> groupHasDecurser = { false, false, false, false };

    std::vector<Player*> decursers;
    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    std::vector<Player*> meleeDps;
    std::vector<Player*> others;

    for (Player* member : botMembers)
    {
        if (state.playerToGroup.find(member->GetGUID()) != state.playerToGroup.end())
            continue;

        if (IsKalecgosDecurser(member))
            decursers.push_back(member);
        else if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else if (PlayerbotAI::IsRangedDps(member))
            rangedDps.push_back(member);
        else if (PlayerbotAI::IsMelee(member) && PlayerbotAI::IsDps(member))
            meleeDps.push_back(member);
        else
            others.push_back(member);
    }

    for (Player* decurser : decursers)
    {
        uint8 groupIndex = GetLeastFilledGroup(groupSizes, &groupHasDecurser, true);
        AssignPlayerToGroup(state, groupSizes, groupHasDecurser, decurser, groupIndex);
    }

    for (Player* healer : healers)
        AssignPlayerToGroup(
            state, groupSizes, groupHasDecurser, healer, GetLeastFilledGroup(groupSizes));

    for (Player* ranged : rangedDps)
        AssignPlayerToGroup(
            state, groupSizes, groupHasDecurser, ranged, GetLeastFilledGroup(groupSizes));

    for (Player* melee : meleeDps)
        AssignPlayerToGroup(
            state, groupSizes, groupHasDecurser, melee, GetLeastFilledGroup(groupSizes));

    for (Player* other : others)
        AssignPlayerToGroup(
            state, groupSizes, groupHasDecurser, other, GetLeastFilledGroup(groupSizes));

    if (state.activeRiftGroup == KALECGOS_INVALID_GROUP)
        state.activeRiftGroup = ResolveActivePortalGroup(group, state);
}

// Read-only companion to GetKalecgosDesignatedTank below.
Player* FindKalecgosDesignatedTank(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
        return nullptr;

    KalecgosEncounterState* state = FindKalecgosState(player->GetInstanceId());
    if (!state)
        return nullptr;

    return ResolveKalecgosDesignatedTank(player, group, *state);
}

Player* GetKalecgosDesignatedTank(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
        return nullptr;

    KalecgosEncounterState& state = GetPreparedEncounterState(player);
    Player* const tank = ResolveKalecgosDesignatedTank(player, group, state);

    if (!ResolveSurfaceTank(group, state.currentTankGuid))
        state.currentTankGuid = tank ? tank->GetGUID() : ObjectGuid::Empty;

    return tank;
}

ObjectGuid FindKalecgosSpectralRiftGuid(Player* bot)
{
    GameObject* rift = bot->FindNearestGameObject(
        Id(SwpObjects::GO_SPECTRAL_RIFT), SPECTRAL_RIFT_SEARCH_RADIUS, true);

    return rift ? rift->GetGUID() : ObjectGuid::Empty;
}

bool ShouldEnterKalecgosPortal(Player* bot)
{
    if (!IsPortalEligibleCandidate(bot))
        return false;

    KalecgosEncounterState& state = GetPreparedEncounterState(bot);
    if (!state.activeRiftOpenedMs)
        return false;

    if (HasKalecgosTankAssignment(state.tankAssignmentGuids, bot->GetGUID()))
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        return ResolveSurfaceTank(group, state.activeRiftOutgoingTankGuid) == bot &&
            state.blastedPlayerGuid != bot->GetGUID();
    }

    if (state.activeRiftGroup == KALECGOS_INVALID_GROUP ||
        GetAssignedGroup(state, bot->GetGUID()) != state.activeRiftGroup)
    {
        return false;
    }

    return state.blastedPlayerGuid != bot->GetGUID();
}

void RecordSpectralBlastTarget(Player* player, PlayerbotAI* announcerAI)
{
    Group* group = player->GetGroup();
    if (!group)
        return;

    KalecgosEncounterState& state = GetPreparedEncounterState(player);
    uint32 const now = getMSTime();

    state.activeRiftOpenedMs = now;
    state.blastedPlayerGuid = player->GetGUID();
    state.activeRiftGroup = ResolveActivePortalGroup(group, state);
    AssignKalecgosTankTargetsForActiveRift(player, group, state);

    Player* currentTank = GetSurfaceTankAfterCurrentHandOff(group, state);
    if (!currentTank)
        currentTank = ResolveSurfaceTank(group, state.currentTankGuid);

    if (HasKalecgosTankAssignment(state.tankAssignmentGuids, player->GetGUID()))
    {
        if (!currentTank)
            return;

        AnnounceTankTransition(
            announcerAI, "kalecgos_tank_sent_to_spectral_realm",
            "Tank %tank has been sent to the Spectral Realm. The active Kalecgos tank is %current.",
            {
                {"%tank", player->GetName()},
                {"%current", currentTank->GetName()}
            });

        return;
    }

    if (Player* outgoingTank = ResolveSurfaceTank(group, state.activeRiftOutgoingTankGuid);
        outgoingTank && currentTank)
    {
        AnnounceTankTransition(
            announcerAI, "kalecgos_tank_should_enter_spectral_realm",
            "Tank %tank should enter the Spectral Realm. The active Kalecgos tank is %current.",
            {
                {"%tank", outgoingTank->GetName()},
                {"%current", currentTank->GetName()}
            });
    }
}

void RecordSpectralRealmEnter(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
        return;

    KalecgosEncounterState& state = GetPreparedEncounterState(player);
    ObjectGuid const guid = player->GetGUID();
    bool const wasCurrentTank = state.currentTankGuid == guid;
    Player* replacementTank = nullptr;

    if (wasCurrentTank)
    {
        replacementTank =
            GetReplacementSurfaceTank(group, state, guid, state.activeRiftOutgoingTankGuid);
    }

    if (state.activeRiftOpenedMs && state.activeRiftGroup == KALECGOS_INVALID_GROUP)
        state.activeRiftGroup = ResolveActivePortalGroup(group, state);

    if (HasKalecgosTankAssignment(state.tankAssignmentGuids, player->GetGUID()))
    {
        AdvanceKalecgosTankPortalRotation(state, guid);

        if (state.activeRiftOutgoingTankGuid == guid)
            state.activeRiftOutgoingTankGuid = ObjectGuid::Empty;

        if (wasCurrentTank)
        {
            state.currentTankGuid =
                replacementTank ? replacementTank->GetGUID() : ObjectGuid::Empty;
        }
    }
}

// Shared encounter state

KalecgosEncounterState& KalecgosState(uint32 instanceId)
{
    std::lock_guard lock(kalecgosStateMutex);
    return kalecgosStates[instanceId];
}

KalecgosEncounterState* FindKalecgosState(uint32 instanceId)
{
    std::lock_guard lock(kalecgosStateMutex);
    auto it = kalecgosStates.find(instanceId);
    return it != kalecgosStates.end() ? &it->second : nullptr;
}

bool KalecgosResetInstance(uint32 instanceId)
{
    std::lock_guard lock(kalecgosStateMutex);
    return kalecgosStates.erase(instanceId) > 0;
}

}
