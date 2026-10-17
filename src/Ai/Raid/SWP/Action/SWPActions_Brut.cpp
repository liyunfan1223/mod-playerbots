/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SWPEncounter_Brut.h"
#include "SWPShared.h"
#include <cmath>

using namespace SwpHelpers;
using namespace EncounterHelpers;

bool BrutallusTanksPositionAndSwapAction::Execute(Event event)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    if (AI_VALUE(Unit*, "current target") != brutallus)
        return Attack(brutallus);

    Player* mainTank = GetGroupMainTank(bot);
    Player* assistTank = GetGroupAssistTank(bot, 0);

    // If either tank is dead, just bail and fall back to standard tank logic. You're screwed
    // anyway unless Brutallus is almost dead.
    if (!mainTank || !assistTank)
        return false;

    Aura* mainTankAura = mainTank->GetAura(Id(SwpSpells::SPELL_METEOR_SLASH));
    Aura* assistTankAura = assistTank->GetAura(Id(SwpSpells::SPELL_METEOR_SLASH));

    if (mainTank == bot)
    {
        if (brutallus->GetVictim() != bot && !mainTankAura &&
            ((assistTankAura && assistTankAura->GetStackAmount() >= METEOR_SLASH_SWAP_STACKS) ||
             !assistTankAura))
        {
            return botAI->DoSpecificAction("taunt spell", event, true);
        }

        if (_mainTankInitialPositionReached)
            return false;

        Position const& position = BRUTALLUS_MAIN_TANK_POSITION;
        constexpr float arrivalDist = 2.0f;

        if (bot->GetExactDist2d(position) <= arrivalDist)
        {
            _mainTankInitialPositionReached = true;
            return false;
        }

        if (brutallus->GetVictim() != bot || !bot->IsWithinMeleeRange(brutallus))
            return false;

        constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
        if (IsWaitingForLastMove(priority))
            return false;

        float moveX;
        float moveY;
        bool backwards;
        if (!GetPathStepToPosition(bot, position, arrivalDist, brutallus, moveX, moveY, backwards))
            return false;

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            priority, true, backwards);
    }
    else if (assistTank == bot)
    {
        if (brutallus->GetVictim() != bot && !assistTankAura &&
            mainTankAura && mainTankAura->GetStackAmount() >= METEOR_SLASH_SWAP_STACKS)
        {
            return botAI->DoSpecificAction("taunt spell", event, true);
        }

        float const mainTankAngle = GetBrutallusMainTankAngle(brutallus, mainTank);
        float const assistTankAngle = Position::NormalizeOrientation(
            mainTankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET);

        Position const position = GetBrutallusPositionAtAngle(
            bot, brutallus, assistTankAngle, BRUTALLUS_TANK_POSITION_RADIUS);

        constexpr float arrivalDist = 2.0f;
        constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
        if (IsWaitingForLastMove(priority))
            return false;

        float moveX;
        float moveY;
        bool backwards;
        if (!GetPathStepToPosition(bot, position, arrivalDist, brutallus, moveX, moveY, backwards))
            return false;

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            priority, true, backwards);
    }

    return false;
}

bool BrutallusPositionMeleeAtRearCenterAction::Execute(Event /*event*/)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    Player* assistTank = GetGroupAssistTank(bot, 0);

    uint8 meleeIndex = 0;
    if (!TryGetBrutallusAssignedPositionIndex(bot, meleeIndex))
        return false;

    Position position;
    if (!TryGetBrutallusMeleePosition(brutallus, mainTank, assistTank, meleeIndex, position))
        return false;

    constexpr float arrivalDist = 1.0f;
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetPathStepToPosition(bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        priority, true, false);
}

bool BrutallusPositionMeleeAtRearCenterAction::TryGetBrutallusMeleePosition(
    Unit* brutallus, Player* mainTank, Player* assistTank, uint8 meleeIndex, Position& position)
{
    if (meleeIndex >= BRUTALLUS_TOTAL_MELEE_POSITIONS)
        return false;

    float meleeRadius = 0.0f;
    uint8 localMeleeIndex = meleeIndex;
    uint8 maxMeleeSlots = 0;
    for (auto const& meleeRingLayout : BRUTALLUS_MELEE_RING_LAYOUTS)
    {
        if (localMeleeIndex < meleeRingLayout.slotCount)
        {
            meleeRadius = meleeRingLayout.radius;
            maxMeleeSlots = meleeRingLayout.slotCount;
            break;
        }

        localMeleeIndex -= meleeRingLayout.slotCount;
    }

    float const mainTankAngle = GetBrutallusMainTankAngle(brutallus, mainTank);

    float midpointAngle;
    if (!mainTank || !assistTank)
    {
        midpointAngle = Position::NormalizeOrientation(
            mainTankAngle + BRUTALLUS_ASSIST_TANK_ANGLE_OFFSET / 2.0f);
    }
    else
    {
        float const midpointX =
            (mainTank->GetPositionX() + assistTank->GetPositionX()) / 2.0f;
        float const midpointY =
            (mainTank->GetPositionY() + assistTank->GetPositionY()) / 2.0f;

        if (brutallus->GetExactDist2d(midpointX, midpointY) <= 0.1f)
        {
            float const assistTankAngle =
                GetBrutallusAssistTankAngle(brutallus, assistTank, mainTankAngle);

            float assistAngleDelta =
                Position::NormalizeOrientation(assistTankAngle - mainTankAngle);
            if (assistAngleDelta > static_cast<float>(M_PI))
                assistAngleDelta -= 2.0f * static_cast<float>(M_PI);

            midpointAngle = Position::NormalizeOrientation(
                mainTankAngle + assistAngleDelta / 2.0f);
        }
        else
        {
            midpointAngle = Position::NormalizeOrientation(std::atan2(
                midpointY - brutallus->GetPositionY(),
                midpointX - brutallus->GetPositionX()));
        }
    }

    float const baseAngle = Position::NormalizeOrientation(midpointAngle + M_PI);
    float const angleOffset = GetCenteredArcSlotAngleOffset(
        localMeleeIndex, maxMeleeSlots, BRUTALLUS_SHARED_SAFE_MELEE_ARC_WIDTH);

    float const angle = Position::NormalizeOrientation(baseAngle + angleOffset);
    position = GetBrutallusPositionAtAngle(bot, brutallus, angle, meleeRadius);
    return true;
}

bool BrutallusPositionRangedInTwoGroupsAction::Execute(Event /*event*/)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    Player* assistTank = GetGroupAssistTank(bot, 0);

    ObjectGuid const guid = bot->GetGUID();
    uint8 rangedIndex = 0;
    if (!TryGetBrutallusAssignedPositionIndex(bot, rangedIndex))
        return false;

    auto& burnStates = BrutallusState(bot->GetInstanceId()).rangedBurnStates;

    auto const burnStateItr = burnStates.find(guid);
    BrutallusRangedBurnState burnState = BrutallusRangedBurnState::None;
    if (burnStateItr != burnStates.end())
        burnState = burnStateItr->second;

    if (burnState == BrutallusRangedBurnState::MovingToInnerLane)
    {
        ReleaseBrutallusBurnPad(bot);
        burnStates.erase(guid);
        burnState = BrutallusRangedBurnState::None;
    }
    else if (burnState == BrutallusRangedBurnState::TraversingInnerLane ||
        burnState == BrutallusRangedBurnState::MovingToBurnPosition ||
        burnState == BrutallusRangedBurnState::AtBurnPosition)
    {
        burnState = BrutallusRangedBurnState::MovingToOuterLane;
        burnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::MovingToOuterLane)
    {
        float const currentAngle = Position::NormalizeOrientation(std::atan2(
            bot->GetPositionY() - brutallus->GetPositionY(),
            bot->GetPositionX() - brutallus->GetPositionX()));

        Position const position = GetBrutallusPositionAtAngle(
            bot, brutallus, currentAngle, BRUTALLUS_OUTER_LANE_RADIUS);

        if (bot->GetExactDist2d(position) <= 1.0f)
        {
            burnStates[guid] = BrutallusRangedBurnState::TraversingOuterLane;
            return false;
        }

        return StepToPosition(position, 1.0f);
    }

    if (burnState == BrutallusRangedBurnState::TraversingOuterLane)
    {
        Position returnTargetPosition;
        if (!TryGetBrutallusRangedPosition(
                bot, brutallus, mainTank, assistTank, rangedIndex,
                BRUTALLUS_OUTER_LANE_RADIUS, returnTargetPosition))
        {
            return false;
        }

        Position position;
        if (!TryGetBrutallusLaneTraversalPosition(
                bot, brutallus, returnTargetPosition.GetPositionX(),
                returnTargetPosition.GetPositionY(), BRUTALLUS_OUTER_LANE_RADIUS,
                bot->GetPositionX(), bot->GetPositionY(), position))
        {
            return false;
        }

        if (bot->GetExactDist2d(returnTargetPosition) <= 1.0f)
            burnStates[guid] = BrutallusRangedBurnState::ReturningToNormalPosition;

        if (bot->GetExactDist2d(position) <= 1.0f)
            return false;

        return MoveTo(
            SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (burnState == BrutallusRangedBurnState::ReturningToNormalPosition)
    {
        Position position;
        if (!TryGetBrutallusRangedPosition(
                bot, brutallus, mainTank, assistTank, rangedIndex,
                BRUTALLUS_NORMAL_RANGED_RADIUS, position))
        {
            return false;
        }

        if (bot->GetExactDist2d(position) <= 1.0f)
        {
            ReleaseBrutallusBurnPad(bot);
            burnStates.erase(guid);
            return false;
        }

        return StepToPosition(position, 1.0f);
    }

    Position position;
    if (!TryGetBrutallusRangedPosition(
            bot, brutallus, mainTank, assistTank, rangedIndex,
            BRUTALLUS_NORMAL_RANGED_RADIUS, position))
    {
        return false;
    }

    return StepToPosition(position, 1.0f);
}

bool BrutallusPositionRangedInTwoGroupsAction::StepToPosition(
    Position const& position, float arrivalDist)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetPathStepToPosition(bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        priority, true, false);
}

bool BrutallusIsolateBurnAction::Execute(Event /*event*/)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return false;

    if (RemoveBurnWithCooldown())
        return true;

    if (PlayerbotAI::IsMelee(bot))
        return false;

    ObjectGuid const guid = bot->GetGUID();
    Player* mainTank = GetGroupMainTank(bot);
    Player* assistTank = GetGroupAssistTank(bot, 0);
    uint8 rangedIndex = 0;

    if (!TryGetBrutallusAssignedPositionIndex(bot, rangedIndex))
        return false;

    auto& burnStates = BrutallusState(bot->GetInstanceId()).rangedBurnStates;

    auto const burnStateItr = burnStates.find(guid);
    BrutallusRangedBurnState burnState = BrutallusRangedBurnState::None;
    if (burnStateItr != burnStates.end())
        burnState = burnStateItr->second;

    if (burnState == BrutallusRangedBurnState::MovingToOuterLane ||
        burnState == BrutallusRangedBurnState::TraversingOuterLane ||
        burnState == BrutallusRangedBurnState::ReturningToNormalPosition)
    {
        burnState = BrutallusRangedBurnState::MovingToInnerLane;
        burnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::None)
    {
        burnState = BrutallusRangedBurnState::MovingToInnerLane;
        burnStates[guid] = burnState;
    }

    if (burnState == BrutallusRangedBurnState::MovingToInnerLane)
    {
        Position position;
        if (!TryGetBrutallusRangedPosition(
                bot, brutallus, mainTank, assistTank, rangedIndex,
                BRUTALLUS_INNER_LANE_RADIUS, position))
        {
            return false;
        }

        if (bot->GetExactDist2d(position) <= 1.0f)
        {
            burnStates[guid] = BrutallusRangedBurnState::TraversingInnerLane;
            return false;
        }

        return StepToPosition(position, 1.0f);
    }

    if (burnState == BrutallusRangedBurnState::TraversingInnerLane)
    {
        Position padIngressPosition;
        if (!TryGetBrutallusBurnPadPosition(
                bot, brutallus, mainTank, rangedIndex,
                BRUTALLUS_INNER_LANE_RADIUS, padIngressPosition))
        {
            return false;
        }

        Position position;
        if (!TryGetBrutallusLaneTraversalPosition(
                bot, brutallus, padIngressPosition.GetPositionX(),
                padIngressPosition.GetPositionY(), BRUTALLUS_INNER_LANE_RADIUS,
                bot->GetPositionX(), bot->GetPositionY(), position))
        {
            return false;
        }

        if (bot->GetExactDist2d(padIngressPosition) <= 1.0f)
            burnStates[guid] = BrutallusRangedBurnState::MovingToBurnPosition;

        if (bot->GetExactDist2d(position) <= 1.0f)
            return false;

        return MoveTo(
            SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    Position position;
    if (!TryGetBrutallusBurnPadPosition(
            bot, brutallus, mainTank, rangedIndex, BRUTALLUS_BURN_PAD_RADIUS, position))
    {
        return false;
    }

    if (bot->GetExactDist2d(position) <= 1.0f)
    {
        burnStates[guid] = BrutallusRangedBurnState::AtBurnPosition;
        return false;
    }

    return StepToPosition(position, 1.0f);
}

bool BrutallusIsolateBurnAction::RemoveBurnWithCooldown()
{
    uint32 const spellId = GetSelfImmunitySpell(bot);
    return spellId && botAI->CanCastSpell(spellId, bot) && botAI->CastSpell(spellId, bot);
}

bool BrutallusIsolateBurnAction::StepToPosition(Position const& position, float arrivalDist)
{
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetPathStepToPosition(bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        priority, true, false);
}
