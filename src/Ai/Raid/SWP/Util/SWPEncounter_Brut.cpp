/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Brut.h"
#include "Playerbots.h"
#include "SWPShared.h"
#include <algorithm>
#include <cmath>
#include <mutex>
#include <vector>

namespace SwpHelpers
{

// Note: Brutallus's CombatReach is 18.0f

namespace
{

std::mutex brutallusStateMutex;
std::unordered_map<uint32, BrutallusEncounterState> brutallusStates;

bool IsBurnPadActive(BrutallusEncounterState const& state, ObjectGuid ownerGuid)
{
    auto const burnStateItr = state.rangedBurnStates.find(ownerGuid);
    return burnStateItr != state.rangedBurnStates.end() &&
        burnStateItr->second != BrutallusRangedBurnState::None;
}

bool TryGetBurnPadIndex(
    BrutallusEncounterState& state, Player* bot, uint8 rangedIndex, uint8& padIndex)
{
    auto& assignments = state.rangedBurnPadAssignments;
    for (auto itr = assignments.begin(); itr != assignments.end();)
    {
        if (itr->first != bot->GetGUID() && !IsBurnPadActive(state, itr->first))
        {
            itr = assignments.erase(itr);
            continue;
        }

        ++itr;
    }

    auto const existingItr = assignments.find(bot->GetGUID());
    if (existingItr != assignments.end() &&
        existingItr->second < BRUTALLUS_TOTAL_BURN_PADS)
    {
        padIndex = existingItr->second;
        return true;
    }

    std::array<bool, BRUTALLUS_TOTAL_BURN_PADS> usedPads = {};
    for (auto const& assignment : assignments)
    {
        if (assignment.second < BRUTALLUS_TOTAL_BURN_PADS)
            usedPads[assignment.second] = true;
    }

    static_assert(BRUTALLUS_TOTAL_BURN_PADS == 8,
        "Burn pad order tables and angle offsets assume exactly 8 pads");

    static constexpr std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP>
        mainGroupPriority = { 0, 1, 2, 3 };
    static constexpr std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP>
        mainGroupOverflow = { 4, 5, 6, 7 };
    static constexpr std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP>
        assistGroupPriority = { 7, 6, 5, 4 };
    static constexpr std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP>
        assistGroupOverflow = { 3, 2, 1, 0 };

    auto const assignFromOrder = [&](std::array<uint8, BRUTALLUS_BURN_PADS_PER_GROUP> const& order)
    {
        for (uint8 candidate : order)
        {
            if (usedPads[candidate])
                continue;

            assignments[bot->GetGUID()] = candidate;
            padIndex = candidate;
            return true;
        }
        return false;
    };

    if (rangedIndex % 2 == 0)
        return assignFromOrder(mainGroupPriority) || assignFromOrder(mainGroupOverflow);

    return assignFromOrder(assistGroupPriority) || assignFromOrder(assistGroupOverflow);
}

bool ShouldRebuildAssignments(uint32& lastRebuildMs)
{
    uint32 const now = getMSTime();
    if (lastRebuildMs &&
        getMSTimeDiff(lastRebuildMs, now) < BRUTALLUS_ASSIGNMENT_REBUILD_INTERVAL_MS)
    {
        return false;
    }

    lastRebuildMs = now;
    return true;
}

void PruneAssignments(
    std::unordered_map<ObjectGuid, uint8>& assignments,
    std::vector<ObjectGuid> const& eligibleGuids)
{
    for (auto itr = assignments.begin(); itr != assignments.end();)
    {
        if (std::find(eligibleGuids.begin(), eligibleGuids.end(), itr->first) ==
            eligibleGuids.end())
        {
            itr = assignments.erase(itr);
            continue;
        }

        ++itr;
    }
}

void EnsureRangedAssignments(Group* group, BrutallusEncounterState& state)
{
    if (!ShouldRebuildAssignments(state.rangedAssignmentRebuildMs))
        return;

    auto& assignments = state.rangedAssignments;

    std::vector<ObjectGuid> eligibleGuids;
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

        eligibleGuids.push_back(member->GetGUID());

        if (assignments.find(member->GetGUID()) != assignments.end())
            continue;

        if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else
            rangedDamage.push_back(member);
    }

    PruneAssignments(assignments, eligibleGuids);

    std::array<bool, BRUTALLUS_TOTAL_RANGED_POSITIONS> usedPositions = {};
    for (auto const& assignment : assignments)
    {
        if (assignment.second < BRUTALLUS_TOTAL_RANGED_POSITIONS)
            usedPositions[assignment.second] = true;
    }

    auto const assignNextOpenSlot = [&](Player* member)
    {
        for (uint8 slotIndex = 0; slotIndex < BRUTALLUS_TOTAL_RANGED_POSITIONS; ++slotIndex)
        {
            if (usedPositions[slotIndex])
                continue;

            assignments[member->GetGUID()] = slotIndex;
            usedPositions[slotIndex] = true;
            return;
        }

        assignments[member->GetGUID()] =
            static_cast<uint8>(assignments.size() % BRUTALLUS_TOTAL_RANGED_POSITIONS);
    };

    for (Player* member : healers)
        assignNextOpenSlot(member);

    for (Player* member : rangedDamage)
        assignNextOpenSlot(member);
}

void EnsureMeleeAssignments(Group* group, BrutallusEncounterState& state)
{
    if (!ShouldRebuildAssignments(state.meleeAssignmentRebuildMs))
        return;

    auto& assignments = state.meleeAssignments;

    std::vector<ObjectGuid> eligibleGuids;
    std::vector<Player*> unassigned;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SWP_MAP_ID || !GET_PLAYERBOT_AI(member))
            continue;

        if (!PlayerbotAI::IsMelee(member) || PlayerbotAI::IsMainTank(member) ||
            PlayerbotAI::IsAssistTankOfIndex(member, 0, true))
        {
            continue;
        }

        eligibleGuids.push_back(member->GetGUID());

        if (assignments.find(member->GetGUID()) == assignments.end())
            unassigned.push_back(member);
    }

    PruneAssignments(assignments, eligibleGuids);

    std::array<bool, BRUTALLUS_TOTAL_MELEE_POSITIONS> usedPositions = {};
    for (auto const& assignment : assignments)
    {
        if (assignment.second < BRUTALLUS_TOTAL_MELEE_POSITIONS)
            usedPositions[assignment.second] = true;
    }

    auto const assignNextOpenSlot = [&](Player* member)
    {
        for (uint8 slotIndex = 0; slotIndex < BRUTALLUS_TOTAL_MELEE_POSITIONS; ++slotIndex)
        {
            if (usedPositions[slotIndex])
                continue;

            assignments[member->GetGUID()] = slotIndex;
            usedPositions[slotIndex] = true;
            return;
        }

        assignments[member->GetGUID()] =
            static_cast<uint8>(assignments.size() % BRUTALLUS_TOTAL_MELEE_POSITIONS);
    };

    for (Player* member : unassigned)
        assignNextOpenSlot(member);
}

float GetTankAngle(Unit* brutallus, Player* tank, float fallbackAngle)
{
    if (!brutallus || !tank)
        return Position::NormalizeOrientation(fallbackAngle);

    return Position::NormalizeOrientation(std::atan2(
        tank->GetPositionY() - brutallus->GetPositionY(),
        tank->GetPositionX() - brutallus->GetPositionX()));
}

float GetDefaultMainTankAngle(Unit* brutallus)
{
    if (!brutallus)
        return 0.0f;

    return Position::NormalizeOrientation(std::atan2(
        BRUTALLUS_MAIN_TANK_POSITION.GetPositionY() - brutallus->GetPositionY(),
        BRUTALLUS_MAIN_TANK_POSITION.GetPositionX() - brutallus->GetPositionX()));
}

} // end anonymous namespace

float GetBrutallusMainTankAngle(Unit* brutallus, Player* mainTank)
{
    return GetTankAngle(brutallus, mainTank, GetDefaultMainTankAngle(brutallus));
}

float GetBrutallusAssistTankAngle(Unit* brutallus, Player* assistTank, float mainTankAngle)
{
    return GetTankAngle(brutallus, assistTank, Position::NormalizeOrientation(
        mainTankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET));
}

Position GetBrutallusPositionAtAngle(Player* bot, Unit* brutallus, float angle, float radius)
{
    float const x = brutallus->GetPositionX() + std::cos(angle) * radius;
    float const y = brutallus->GetPositionY() + std::sin(angle) * radius;
    return { x, y, bot->GetPositionZ() };
}

bool TryGetBrutallusAssignedPositionIndex(Player* bot, uint8& positionIndex)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    bool const isRanged = PlayerbotAI::IsRanged(bot);
    auto& state = BrutallusState(bot->GetInstanceId());

    if (isRanged)
        EnsureRangedAssignments(group, state);
    else
        EnsureMeleeAssignments(group, state);

    auto& assignments = isRanged ? state.rangedAssignments : state.meleeAssignments;
    auto const assignmentItr = assignments.find(bot->GetGUID());
    if (assignmentItr == assignments.end())
        return false;

    positionIndex = assignmentItr->second;
    return true;
}

bool TryGetBrutallusRangedPosition(
    Player* bot, Unit* brutallus, Player* mainTank, Player* assistTank,
    uint8 rangedIndex, float radius, Position& position)
{
    if (!brutallus || rangedIndex >= BRUTALLUS_TOTAL_RANGED_POSITIONS)
        return false;

    bool const isMainTankGroup = rangedIndex % 2 == 0;
    uint8 const arcPositionIndex =
        static_cast<uint8>((rangedIndex / 2) % BRUTALLUS_RANGED_POSITIONS_PER_GROUP);

    float const mainTankAngle = GetBrutallusMainTankAngle(brutallus, mainTank);
    float const assistTankAngle =
        GetBrutallusAssistTankAngle(brutallus, assistTank, mainTankAngle);

    float const tankAngle = isMainTankGroup ? mainTankAngle : assistTankAngle;
    float const angleOffset = GetCenteredArcSlotAngleOffset(
        arcPositionIndex, BRUTALLUS_RANGED_POSITIONS_PER_GROUP,
        BRUTALLUS_RANGED_GROUP_ARC_WIDTH);

    float const angle = Position::NormalizeOrientation(tankAngle + angleOffset);

    position = GetBrutallusPositionAtAngle(bot, brutallus, angle, radius);
    return true;
}

bool TryGetBrutallusBurnPadPosition(
    Player* bot, Unit* brutallus, Player* mainTank,
    uint8 rangedIndex, float radius, Position& position)
{
    if (!brutallus || rangedIndex >= BRUTALLUS_TOTAL_RANGED_POSITIONS)
        return false;

    uint8 padIndex = 0;
    if (!TryGetBurnPadIndex(BrutallusState(bot->GetInstanceId()), bot, rangedIndex, padIndex))
        return false;

    constexpr float degreeToRadian = M_PI / 180.0f;
    static constexpr std::array burnPadAngleOffsets = {
        70.0f * degreeToRadian,
        83.3f * degreeToRadian,
        96.7f * degreeToRadian,
        110.0f * degreeToRadian,
        130.0f * degreeToRadian,
        143.3f * degreeToRadian,
        156.7f * degreeToRadian,
        170.0f * degreeToRadian
    };

    float const mainTankAngle = GetBrutallusMainTankAngle(brutallus, mainTank);
    float const angle = Position::NormalizeOrientation(
        mainTankAngle + burnPadAngleOffsets[padIndex]);

    position = GetBrutallusPositionAtAngle(bot, brutallus, angle, radius);
    return true;
}

bool TryGetBrutallusLaneTraversalPosition(
    Player* bot, Unit* brutallus, float targetX, float targetY, float radius,
    float currentX, float currentY, Position& position)
{
    if (!brutallus)
        return false;

    float const targetAngle = Position::NormalizeOrientation(std::atan2(
        targetY - brutallus->GetPositionY(), targetX - brutallus->GetPositionX()));

    float const currentAngle = Position::NormalizeOrientation(
        std::atan2(currentY - brutallus->GetPositionY(), currentX - brutallus->GetPositionX()));

    float remainingAngle = Position::NormalizeOrientation(targetAngle - currentAngle);
    if (remainingAngle > static_cast<float>(M_PI))
        remainingAngle -= 2.0f * static_cast<float>(M_PI);

    constexpr float stepDistance = 3.0f;
    float const stepRatio = stepDistance / (2.0f * radius);
    float const clampedStepRatio = std::clamp(stepRatio, 0.0f, 1.0f);
    float const stepAngle = 2.0f * std::asin(clampedStepRatio);
    float nextAngle = targetAngle;

    if (std::fabs(remainingAngle) > stepAngle)
    {
        nextAngle = Position::NormalizeOrientation(
            currentAngle + std::copysign(stepAngle, remainingAngle));
    }

    position = GetBrutallusPositionAtAngle(bot, brutallus, nextAngle, radius);
    return true;
}

bool ReleaseBrutallusBurnPad(Player* bot)
{
    BrutallusEncounterState* state = FindBrutallusState(bot->GetInstanceId());
    if (!state)
        return false;

    return state->rangedBurnPadAssignments.erase(bot->GetGUID()) > 0;
}

bool HasBrutallusBurn(Player* bot)
{
    return bot->HasAura(Id(SwpSpells::SPELL_BURN));
}

// Shared encounter state

BrutallusEncounterState& BrutallusState(uint32 instanceId)
{
    std::lock_guard lock(brutallusStateMutex);
    return brutallusStates[instanceId];
}

BrutallusEncounterState* FindBrutallusState(uint32 instanceId)
{
    std::lock_guard lock(brutallusStateMutex);
    auto it = brutallusStates.find(instanceId);
    return it != brutallusStates.end() ? &it->second : nullptr;
}

bool BrutallusResetInstance(uint32 instanceId)
{
    std::lock_guard lock(brutallusStateMutex);
    return brutallusStates.erase(instanceId) > 0;
}

}
