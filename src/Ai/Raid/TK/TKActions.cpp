/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKActions.h"
#include "AiFactory.h"
#include "EncounterHelpers.h"
#include "ItemPackets.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "Playerbots.h"
#include "StatsWeightCalculator.h"
#include "TKHelpers.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <limits>

using namespace TkHelpers;
using namespace EncounterHelpers;

// General

bool TempestKeepResetEncounterStatesAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetInstanceId();

    bool reset = false;
    reset |= isAlarInPhase2.erase(instanceId) > 0;
    reset |= lastRebirthState.erase(instanceId) > 0;
    reset |= voidReaverArcaneOrbs.erase(instanceId) > 0;
    reset |= advisorDpsWaitTimer.erase(instanceId) > 0;

    return reset;
}

// This is needed in the event of a wipe during Kael's Gravity Lapse phase.
bool TempestKeepClearStaleFallingFlagAction::Execute(Event /*event*/)
{
    // Clear the flag only once the bot is back on the floor, or it would be cleared mid-descent.
    float const floorZ = bot->GetMapHeight(
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), true, MAX_FALL_DISTANCE);
    if (floorZ <= INVALID_HEIGHT || bot->GetPositionZ() - floorZ > 1.0f)
        return false;

    bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR);
    if (!bot->IsRooted())
        bot->SendMovementFlagUpdate();

    return true;
}

bool TempestKeepTankPositionAction::MoveToTankPosition(
    Unit* target, Position const& position, float tolerance, bool shouldAttack)
{
    if (shouldAttack && AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    if (target->GetVictim() != bot || !bot->IsWithinMeleeRange(target))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, tolerance, target, moveX, moveY, backwards))
        return false;

    return MoveTo(
        TK_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Trash

bool CrimsonHandCenturionCastPolymorphAction::Execute(Event /*event*/)
{
    Unit* target = GetCenturionCastingArcaneFlurry(botAI);
    if (!target)
        return false;

    bot->CastStop();

    if (!botAI->CanCastSpell("polymorph", target))
        return false;

    return botAI->CastSpell("polymorph", target);
}

// Al'ar <Phoenix God>

bool AlarMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (!bot->HasAura(Id(TkSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", alar) && botAI->CastSpell("steady shot", alar);
}

bool AlarBossTanksMoveBetweenPlatformsAction::Execute(Event event)
{
    bool const isFirstAlarTank = IsFirstAlarTank(bot);
    if (!isFirstAlarTank && !IsSecondAlarTank(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (AI_VALUE(Unit*, "current target") != alar)
        return Attack(alar);

    int8 alarPlatformIndex = GetAlarPlatformIndex(alar);
    int8 tankPlatformIndex; // Determine which platform the tank goes to based on Al'ar's platform.
    if (isFirstAlarTank)
    {
        tankPlatformIndex =
            (alarPlatformIndex == PLATFORM_0_IDX || alarPlatformIndex == PLATFORM_3_IDX) ?
            PLATFORM_0_IDX : PLATFORM_2_IDX;
    }
    else // isSecondAlarTank
    {
        tankPlatformIndex =
            (alarPlatformIndex == PLATFORM_0_IDX || alarPlatformIndex == PLATFORM_1_IDX) ?
            PLATFORM_1_IDX : PLATFORM_3_IDX;
    }

    Position const& target = ALAR_TANK_PLATFORM_POSITIONS[tankPlatformIndex];
    if (bot->GetExactDist2d(target) <= 2.0f)
    {
        if (alar->GetVictim() != bot)
            return botAI->DoSpecificAction("taunt spell", event, true);

        return false;
    }

    return MoveTo(
        TK_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool AlarMeleeDpsMoveBetweenPlatformsAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsMelee(bot) || IsFirstAlarTank(bot) || IsSecondAlarTank(bot) ||
        IsPrimaryEmberTank(bot))
    {
        return false;
    }

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || alar->GetHealthPct() <= 5.0f)
        return false;

    int8 platformIndex = GetAlarPlatformIndex(alar);
    if (platformIndex == LOCATION_NONE)
        return false;

    Position const& target = ALAR_MELEE_DPS_PLATFORM_POSITIONS[platformIndex];

    if (bot->GetExactDist2d(target) <= 2.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool AlarRangedAndEmberTankMoveUnderPlatformsAction::Execute(Event /*event*/)
{
    bool const isRanged = PlayerbotAI::IsRanged(bot);
    bool const isEmberTank = IsPrimaryEmberTank(bot);
    if (!isRanged && !isEmberTank)
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    int8 platformIndex = GetAlarPlatformIndex(alar);
    if (platformIndex == LOCATION_NONE)
        return false;

    Position const& position = ALAR_GROUND_POSITIONS[platformIndex];

    float distFromTarget = 0.0f;
    if (isRanged)
        distFromTarget = 8.0f;
    else if (isEmberTank && !AI_VALUE2(Unit*, "find target", "ember of al'ar"))
        distFromTarget = 20.0f;
    else
        return false;

    if (bot->GetExactDist2d(position) <= distFromTarget)
        return false;

    return MoveInside(
        TK_MAP_ID, position.GetPositionX(), position.GetPositionY(),
        position.GetPositionZ(), distFromTarget, MovementPriority::MOVEMENT_COMBAT);
}

bool AlarAssistTanksPickUpEmbersAction::Execute(Event event)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (!IsAlarInPhase2(alar->GetInstanceId()))
        return HandlePhase1Embers(alar);

    return HandlePhase2Embers(event);
}

// Embers will be tanked by only the second assist tank in Phase 1.
bool AlarAssistTanksPickUpEmbersAction::HandlePhase1Embers(Unit* alar)
{
    if (!IsPrimaryEmberTank(bot))
        return false;

    Unit* ember = AI_VALUE2(Unit*, "find target", "ember of al'ar");
    if (!ember)
        return false;

    if (AI_VALUE(Unit*, "current target") != ember)
        return Attack(ember);

    if (!bot->IsWithinMeleeRange(ember))
    {
        return MoveTo(
            TK_MAP_ID, ember->GetPositionX(), ember->GetPositionY(), ember->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (ember->GetVictim() != bot)
        return false;

    int8 platformIndex = GetAlarPlatformIndex(alar);
    if (platformIndex == LOCATION_NONE)
        return false;

    Position const& position = ALAR_GROUND_POSITIONS[platformIndex];
    Position const& center = ALAR_POINT_MIDDLE;

    float dx = center.GetPositionX() - position.GetPositionX();
    float dy = center.GetPositionY() - position.GetPositionY();
    float distToCenter = position.GetExactDist2d(center);

    constexpr float moveDist = 26.0f;
    float targetX = position.GetPositionX() + (dx / distToCenter) * moveDist;
    float targetY = position.GetPositionY() + (dy / distToCenter) * moveDist;

    if (bot->GetExactDist2d(targetX, targetY) <= 1.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, targetX, targetY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

// One Ember will be tanked by the second assist tank in Phase 2, and the other by
// the main tank or first assist tank (whichever is not tanking Al'ar; see helpers for more info).
bool AlarAssistTanksPickUpEmbersAction::HandlePhase2Embers(Event const& event)
{
    auto const& [firstEmber, secondEmber] = GetTargetUnitPair(botAI, Id(TkNpcs::NPC_EMBER_OF_ALAR));

    Unit* ember = nullptr;
    if (IsPrimaryEmberTank(bot))
        ember = firstEmber;
    else if (GetSecondaryEmberTank(bot) == bot)
        ember = secondEmber;

    if (!ember)
        return false;

    if (AI_VALUE(Unit*, "current target") != ember)
        return Attack(ember);

    if (ember->GetVictim() != bot)
        return botAI->DoSpecificAction("taunt spell", event, true);

    // The Embers' Rebirth has a range of 15 yards (16.5y damage radius), but more space is
    // needed since this movement is tank-to-player, not Ember.
    constexpr float safeDistance = 20.0f;
    if (!GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return false;

    return MoveFromGroup(safeDistance);
}

bool AlarAssignNonTankTargetAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    if (PlayerbotAI::IsRanged(bot))
    {
        auto const& [firstEmber, secondEmber] =
            GetTargetUnitPair(botAI, Id(TkNpcs::NPC_EMBER_OF_ALAR));

        Unit* ember = firstEmber;
        if (!ember && secondEmber)
            ember = secondEmber;

        if (ember)
        {
            target = ember;
            constexpr float safeDistance = 20.0f;
            float const currentDistance = bot->GetExactDist2d(ember);
            if (currentDistance < safeDistance)
            {
                bot->CastStop();
                return MoveAway(ember, safeDistance - currentDistance);
            }
        }
    }

    if (!target)
        target = AI_VALUE2(Unit*, "find target", "al'ar");

    if (!target || AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}

// During Flame Quills
bool AlarJumpFromPlatformAction::Execute(Event /*event*/)
{
    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
    {
        Position const ground = GetClosestGroundPosition(bot->GetPosition());
        bot->CastStop();
        return JumpTo(
            TK_MAP_ID, ground.GetPositionX(), ground.GetPositionY(), ground.GetPositionZ(),
            MovementPriority::MOVEMENT_FORCED);
    }

    if (IsFirstAlarTank(bot))
    {
        return MoveTo(
            TK_MAP_ID, ALAR_SW_RAMP_BASE.GetPositionX(), ALAR_SW_RAMP_BASE.GetPositionY(),
            ALAR_SW_RAMP_BASE.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (IsSecondAlarTank(bot))
    {
        return MoveTo(
            TK_MAP_ID, ALAR_SE_RAMP_BASE.GetPositionX(), ALAR_SE_RAMP_BASE.GetPositionY(),
            ALAR_SE_RAMP_BASE.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (PlayerbotAI::IsTank(bot))
    {
        return MoveTo(
            TK_MAP_ID, ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY(),
            ALAR_POINT_MIDDLE.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (PlayerbotAI::IsMelee(bot)) // Melee dps
    {
        return MoveTo(
            TK_MAP_ID, ALAR_ROOM_S_CENTER.GetPositionX(), ALAR_ROOM_S_CENTER.GetPositionY(),
            ALAR_ROOM_S_CENTER.GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    // Ranged
    constexpr float distFromPos = 10.0f;
    return MoveInside(
        TK_MAP_ID, ALAR_POINT_MIDDLE.GetPositionX(), ALAR_POINT_MIDDLE.GetPositionY(),
        ALAR_POINT_MIDDLE.GetPositionZ(), distFromPos, MovementPriority::MOVEMENT_FORCED);
}

bool AlarMoveAwayFromRebirthAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    // Ranged/tanks wait until Al'ar actually "dies" to activate this P1->P2 transition action.
    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
    {
        Creature* alarCreature = alar->ToCreature();
        if (!alarCreature || alarCreature->GetReactState() != REACT_PASSIVE)
            return false;
    }

    // On the other hand, melee dps jumps off at 5% HP because TBC hates them.
    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
    {
        Position const ground = GetClosestGroundPosition(bot->GetPosition());
        bot->CastStop();
        return JumpTo(
            TK_MAP_ID, ground.GetPositionX(), ground.GetPositionY(), ground.GetPositionZ(),
            MovementPriority::MOVEMENT_FORCED);
    }

    // Al'ar stays at its platform for the first 8 seconds of the pretend-death and is only then
    // moved to the middle for Rebirth, so move away from the room center, not Al'ar.
    constexpr float safeDistance = 35.0f;
    if (bot->GetExactDist2d(ALAR_ROOM_CENTER) >= safeDistance)
        return false;

    float const angle = ALAR_ROOM_CENTER.GetAngle(bot);
    float const targetX = ALAR_ROOM_CENTER.GetPositionX() + std::cos(angle) * safeDistance;
    float const targetY = ALAR_ROOM_CENTER.GetPositionY() + std::sin(angle) * safeDistance;

    return MoveTo(
        TK_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool AlarSwapTanksOnBossAction::Execute(Event event)
{
    if (!IsFirstAlarTank(bot) && !IsSecondAlarTank(bot))
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (GetSecondaryEmberTank(bot) == bot)
        return false;

    if (AI_VALUE(Unit*, "current target") != alar)
        return Attack(alar);

    if (alar->GetVictim() == bot)
        return false;

    return botAI->DoSpecificAction("taunt spell", event, true);
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    return AvoidFlamePatch() || HandleDiveBomb(alar);
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::AvoidFlamePatch()
{
    std::vector<Unit*> const flamePatches = GetFlamePatches(botAI);

    constexpr float hazardRadius = 8.0f;

    for (Unit* flamePatch : flamePatches)
    {
        if (bot->GetExactDist2d(flamePatch) < hazardRadius)
        {
            Position safestPos = FindSafestNearbyPosition(flamePatches, hazardRadius);
            bot->CastStop();
            return MoveTo(
                TK_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                safestPos.GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

// Al'ar is the one fight in TK with multiple ground AoEs active at once, so where to run has to be
// specifically weighed, rather than just "run away to a generally sensible position," which is
// essentially what FleePosition() does. This action searches rings going outward from the bot, with
// the first candidate clear of every patch that is reachable without crossing a patch being the
// one selected (with a fallback to the nearest safe spot, regardless of whether the path is safe).
Position AlarAvoidFlamePatchesAndDiveBombsAction::FindSafestNearbyPosition(
    std::vector<Unit*> const& flamePatches, float hazardRadius)
{
    constexpr float searchStep = M_PI / 12.0f;
    constexpr float minDistance = 2.0f;
    constexpr float distanceStep = 1.0f;
    constexpr uint8 numAngles = 24;
    constexpr uint8 numDistSteps = 28;

    Position const botPos = bot->GetPosition();
    Position fallback;
    bool haveFallback = false;

    for (uint8 i = 0; i <= numDistSteps; ++i)
    {
        float const distance = minDistance + i * distanceStep;
        for (uint8 j = 0; j < numAngles; ++j)
        {
            float const angle = j * searchStep;
            Position const testPos(
                botPos.GetPositionX() + distance * std::cos(angle),
                botPos.GetPositionY() + distance * std::sin(angle),
                botPos.GetPositionZ());

            auto const inPatch = [&](Unit* flamePatch)
            {
                return flamePatch->GetExactDist2d(
                    testPos.GetPositionX(), testPos.GetPositionY()) < hazardRadius;
            };

            if (std::any_of(flamePatches.begin(), flamePatches.end(), inPatch))
                continue;

            if (IsPathSafe(botPos, testPos, flamePatches, hazardRadius))
                return testPos;

            if (!haveFallback)
            {
                fallback = testPos;
                haveFallback = true;
            }
        }
    }

    return haveFallback ? fallback : botPos;
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::IsPathSafe(
    Position const& start, Position const& end, std::vector<Unit*> const& flamePatches,
    float hazardRadius)
{
    constexpr uint8 numChecks = 10;
    float const dx = end.GetPositionX() - start.GetPositionX();
    float const dy = end.GetPositionY() - start.GetPositionY();

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        float const ratio = static_cast<float>(i) / numChecks;
        float const checkX = start.GetPositionX() + dx * ratio;
        float const checkY = start.GetPositionY() + dy * ratio;

        for (Unit* flamePatch : flamePatches)
        {
            if (flamePatch->GetExactDist2d(checkX, checkY) < hazardRadius)
                return false;
        }
    }

    return true;
}

bool AlarAvoidFlamePatchesAndDiveBombsAction::HandleDiveBomb(Unit* alar)
{
    // Avoidance after Dive Bomb, before reapperance
    if (alar->HasAura(Id(TkSpells::SPELL_MODEL_INVISIBILITY)) ||
        alar->FindCurrentSpellBySpellId(Id(TkSpells::SPELL_REBIRTH_DIVE)))
    {
        constexpr float safeDistance = 25.0f;
        float const currentDistance = bot->GetExactDist2d(alar);
        if (currentDistance >= safeDistance)
            return false;

        bot->CastStop();
        return MoveAway(alar, safeDistance - currentDistance);
    }

    if (GetAlarCurrentLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX &&
        GetAlarDestinationLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX)
    {
        return false;
    }

    // Avoidance during Dive Bomb sequence
    constexpr float safeDistance = 15.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    if (!nearestPlayer)
        return false;

    float const currentDistance = bot->GetExactDist2d(nearestPlayer);
    return MoveAway(nearestPlayer, safeDistance - currentDistance);
}

bool AlarManagePhaseTrackerAction::Execute(Event /*event*/)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    uint32 const instanceId = alar->GetInstanceId();
    bool const rebirthActive = alar->FindCurrentSpellBySpellId(Id(TkSpells::SPELL_REBIRTH_PHASE2));

    if (!IsAlarInPhase2(instanceId) && lastRebirthState[instanceId] && !rebirthActive)
    {
        isAlarInPhase2[instanceId] = true;
        return true;
    }

    lastRebirthState[instanceId] = rebirthActive;

    return false;
}

// Void Reaver

// Pounding is part of why the spot is fixed, though tanking is needed in any case.
bool VoidReaverTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    constexpr float tolerance = 2.0f;
    return MoveToTankPosition(voidReaver, VOID_REAVER_TANK_POSITION, tolerance);
}

bool VoidReaverUseAggroDumpAbilityAction::Execute(Event /*event*/)
{
    static constexpr std::array spells = {
        "divine shield",
        "divine protection",
        "fade",
        "feign death",
        "ice block",
        "soulshatter",
        "vanish",
    };

    for (char const* spell : spells)
    {
        if (botAI->CanCastSpell(spell, bot) && botAI->CastSpell(spell, bot))
            return true;
    }

    return false;
}

bool VoidReaverRangedBackOffAndSpreadAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    constexpr float minDistFromBoss = 38.5f; // 22.0f GetDistance2d()
    constexpr uint32 minInterval = 0;
    if (bot->GetExactDist2d(voidReaver) < minDistFromBoss)
        return FleePosition(voidReaver->GetPosition(), minDistFromBoss, minInterval);

    if (voidReaver->GetHealthPct() > 90.0f)
        return false;

    // Maintain small spread after pull to discourage clumping from avoiding orbs.
    constexpr float minDistFromPlayer = 3.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, minDistFromPlayer);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), minDistFromPlayer);
}

bool VoidReaverAvoidArcaneOrbAction::Execute(Event /*event*/)
{
    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver)
        return false;

    std::vector<Position> const activeOrbs = GetActiveArcaneOrbs(bot->GetInstanceId());
    if (!IsNearArcaneOrb(bot, activeOrbs, ARCANE_ORB_SAFE_DISTANCE))
        return false;

    constexpr float searchStep = M_PI / 12.0f;
    constexpr float minSearchDist = 1.0f;
    constexpr float searchDistStep = 1.0f;
    constexpr float minDistFromBoss = 35.5f;
    constexpr float maxDistFromBoss = 43.5f;
    constexpr uint8 numAngles = 24;
    constexpr uint8 numDistSteps = 39;
    constexpr float safeDistanceSq = ARCANE_ORB_SAFE_DISTANCE * ARCANE_ORB_SAFE_DISTANCE;

    std::vector<Position> bestCandidates;
    float bestMoveDist = std::numeric_limits<float>::max();

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    for (uint8 i = 0; i <= numDistSteps; ++i)
    {
        float const dist = minSearchDist + i * searchDistStep;
        if (dist > bestMoveDist)
            break;

        for (uint8 j = 0; j < numAngles; ++j)
        {
            float const angle = j * searchStep;
            float const x = botX + dist * std::cos(angle);
            float const y = botY + dist * std::sin(angle);

            float const distFromBoss = voidReaver->GetExactDist2d(x, y);
            if (distFromBoss < minDistFromBoss || distFromBoss > maxDistFromBoss)
                continue;

            bool safeFromOrbs = true;
            for (auto const& orbPos : activeOrbs)
            {
                float const dx = x - orbPos.GetPositionX();
                float const dy = y - orbPos.GetPositionY();
                if (dx * dx + dy * dy < safeDistanceSq)
                {
                    safeFromOrbs = false;
                    break;
                }
            }

            if (safeFromOrbs && dist <= bestMoveDist)
            {
                if (dist < bestMoveDist)
                {
                    bestCandidates.clear();
                    bestMoveDist = dist;
                }
                bestCandidates.push_back(Position(x, y, bot->GetPositionZ()));
            }
        }
    }

    bot->CastStop();

    if (!bestCandidates.empty())
    {
        Position const& chosen = bestCandidates[urand(0, bestCandidates.size() - 1)];
        return MoveTo(
            TK_MAP_ID, chosen.GetPositionX(), chosen.GetPositionY(), chosen.GetPositionZ(),
            false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    constexpr uint32 minInterval = 0;
    return FleePosition(activeOrbs[0], ARCANE_ORB_SAFE_DISTANCE, minInterval);
}

// High Astromancer Solarian

bool HighAstromancerSolarianMainTankPickUpBossAction::Execute(Event /*event*/)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    return AI_VALUE(Unit*, "current target") != astromancer && Attack(astromancer);
}

bool HighAstromancerSolarianMoveAwayFromGroupAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 15.0f;
    if (!GetNearestPlayerInRadius(bot, safeDistance))
        return false;

    bot->CastStop();
    return MoveFromGroup(safeDistance);
}

bool HighAstromancerSolarianTargetSolariumPriestsAction::Execute(Event /*event*/)
{
    auto const& priestsPair = GetTargetUnitPair(botAI, Id(TkNpcs::NPC_SOLARIUM_PRIEST));
    if (!priestsPair.first)
        return false;

    if (PlayerbotAI::IsRanged(bot) && !AI_VALUE2(Unit*, "find target", "solarium agent"))
    {
        return AI_VALUE(Unit*, "current target") != priestsPair.first &&
            Attack(priestsPair.first);
    }

    // Split melee into two groups, one on each Solarium Priest.
    Unit* targetPriest = AssignSolariumPriestsToMeleeBots(priestsPair, GetMeleeBots());
    if (!targetPriest)
        return false;

    return AI_VALUE(Unit*, "current target") != targetPriest && Attack(targetPriest);
}

std::vector<Player*> HighAstromancerSolarianTargetSolariumPriestsAction::GetMeleeBots()
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    std::vector<Player*> meleeMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != TK_MAP_ID ||
            !GET_PLAYERBOT_AI(member))
        {
            continue;
        }

        if (PlayerbotAI::IsMelee(member) && !PlayerbotAI::IsMainTank(member))
            meleeMembers.push_back(member);
    }

    return meleeMembers;
}

Unit* HighAstromancerSolarianTargetSolariumPriestsAction::AssignSolariumPriestsToMeleeBots(
    std::pair<Unit*, Unit*> const& priestsPair, std::vector<Player*> const& meleeMembers)
{
    if (!priestsPair.first || meleeMembers.empty())
        return nullptr;

    auto it = std::find(meleeMembers.begin(), meleeMembers.end(), bot);
    if (it == meleeMembers.end())
        return nullptr;

    size_t botIndex = std::distance(meleeMembers.begin(), it);
    size_t totalMelee = meleeMembers.size();

    if (totalMelee == 1)
        return priestsPair.first;

    size_t split = totalMelee / 2;

    if (botIndex < split)
        return priestsPair.first;

    return priestsPair.second;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool KaelthasSunstriderKiteThaladredAction::Execute(Event /*event*/)
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred)
        return false;

    constexpr float safeDistance = 20.0f;
    float const currentDistance = bot->GetExactDist2d(thaladred);
    if (currentDistance >= safeDistance)
        return false;

    bot->CastStop();
    return MoveAway(thaladred, safeDistance - currentDistance);
}

// Misdirect order: (1) Capernian, (2) Telonicus, (3) Capernian (again for good measure)
bool KaelthasSunstriderMisdirectAdvisorsToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            member->GetMapId() == TK_MAP_ID && GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

        if (hunters.size() >= 3)
            break;
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* advisor = nullptr;
    Player* tank = nullptr;
    if (hunterIndex == 0 || hunterIndex == 2)
    {
        advisor = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        tank = GetCapernianTank(bot);
    }
    else if (hunterIndex == 1)
    {
        advisor = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
        tank = GetGroupAssistTank(bot, 0);
    }

    if (!IsAdvisorActive(advisor) || advisor->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return false;

    if (!tank || !tank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(TkSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", advisor) && botAI->CastSpell("steady shot", advisor);
}

bool KaelthasSunstriderMeleeTanksPositionAdvisorsAction::Execute(Event /*event*/)
{
    Unit* advisor = nullptr;
    Position position;
    if (PlayerbotAI::IsMainTank(bot))
    {
        advisor = AI_VALUE2(Unit*, "find target", "lord sanguinar");
        position = SANGUINAR_TANK_POSITION;
    }
    else // PlayerbotAI::IsAssistTankOfIndex(bot, 0, true)
    {
        advisor = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
        position = TELONICUS_TANK_POSITION;
    }

    if (!advisor)
        return false;

    constexpr float tolerance = 2.0f;
    return MoveToTankPosition(advisor, position, tolerance);
}

bool KaelthasSunstriderWarlockTankPositionCapernianAction::Execute(Event /*event*/)
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    char const* const searingPain = "searing pain";

    if (AI_VALUE(Unit*, "current target") != capernian &&
        botAI->CanCastSpell(searingPain, capernian) && botAI->CastSpell(searingPain, capernian))
    {
        return true;
    }

    if (capernian->GetVictim() == bot)
    {
        constexpr float minDistance = 28.0f;
        float const currentDist = bot->GetDistance2d(capernian);
        if ((currentDist < minDistance) && MoveAway(capernian, minDistance - currentDist))
            return true;
    }

    return botAI->CanCastSpell(searingPain, capernian) &&
        botAI->CastSpell(searingPain, capernian);
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::Execute(Event /*event*/)
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (phase == PHASE_NONE)
        return false;

    if (PlayerbotAI::IsRanged(bot) && capernian->GetVictim() != bot)
        return RangedBotsDisperse(phase, capernian);

    if (PlayerbotAI::IsMelee(bot) && phase == PHASE_SINGLE_ADVISOR)
        return MeleeStayBackFromCapernian(capernian);

    return false;
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::RangedBotsDisperse(
    uint32 phase, Unit* capernian)
{
    if (phase == PHASE_ALL_ADVISORS)
    {
        if (AI_VALUE2(Unit*, "find target", "thaladred the darkener"))
            return false;

        constexpr float safeDistance = 6.0f;
        Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
        if (!nearestPlayer)
            return false;

        return FleePosition(nearestPlayer->GetPosition(), safeDistance);
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != TK_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    bool const isHeal = PlayerbotAI::IsHeal(bot);
    std::vector<Player*> const& ring = isHeal ? healers : rangedDps;

    auto const findIt = std::find(ring.begin(), ring.end(), bot);
    if (findIt == ring.end())
        return false;

    // The spread is a 90-degree arc for healers and a 120-degree arc for ranged DPS.
    // Capernian's CombatReach is 4.5y + standard player CombatReach (without Bloodlust) is 1.5y.
    float const arcSpan = isHeal ? M_PI / 2.0f : 2.0f * M_PI / 3.0f;
    float const radius = isHeal ? 42.0f : 34.0f; // 36 and 28 yards of actual distance
    constexpr float arcCenter = 2.9f;
    float const arcStart = arcCenter - arcSpan / 2.0f;

    size_t const count = ring.size();
    size_t const botIndex = std::distance(ring.begin(), findIt);
    float const angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    float targetX = capernian->GetPositionX() + radius * std::cos(angle);
    float targetY = capernian->GetPositionY() + radius * std::sin(angle);
    float targetZ = bot->GetPositionZ();

    if (bot->GetExactDist2d(targetX, targetY) <= 1.0f)
        return false;

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            targetX, targetY, targetZ))
    {
        return false;
    }

    bot->CastStop();
    return MoveTo(
        TK_MAP_ID, targetX, targetY, targetZ, false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction::MeleeStayBackFromCapernian(
    Unit* capernian)
{
    constexpr float safeDistance = 50.0f; // There's no need for melee to be anywhere nearby.
    float const currentDistance = bot->GetExactDist2d(capernian);
    if (currentDistance >= safeDistance)
        return true;

    bot->CastStop();
    return MoveAway(capernian, safeDistance - currentDistance);
}

bool KaelthasSunstriderHandleAdvisorRolesInPhase3Action::Execute(Event /*event*/)
{
    Position position;
    if (PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
        position = ADVISOR_HEAL_POSITION;
    else if (PlayerbotAI::IsMainTank(bot))
        position = SANGUINAR_WAITING_POSITION;
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        position = TELONICUS_WAITING_POSITION;
    else // IsCapernianTank(bot)
        position = CAPERNIAN_WAITING_POSITION;

    if (bot->GetExactDist2d(position) <= 2.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KaelthasSunstriderReequipGearAction::Execute(Event event)
{
    return botAI->DoSpecificAction("equip upgrade", event, true);
}

bool KaelthasSunstriderAssignAdvisorDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (phase == PHASE_NONE)
        return false;

    bool const isPhase3 = phase == PHASE_ALL_ADVISORS;
    bool const isActiveCapernianTank = isPhase3 && IsCapernianTank(bot);

    Unit* target = nullptr;

    // Target priority 1: Thaladred, except Capernian tank during all advisors phase
    if (!isActiveCapernianTank)
    {
        Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
        if (IsAdvisorActive(thaladred))
        {
            target = thaladred;
            if (isPhase3 && MarkTargetWithSkull(bot, thaladred))
                return true;
        }
    }

    // Target priority 2: Capernian for ranged only (excluding debuff Hunter)
    if (!target && PlayerbotAI::IsRangedDps(bot) && !IsSanguinarDebuffHunter(bot))
    {
        Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        if (IsAdvisorActive(capernian))
        {
            target = capernian;
            if (isPhase3 && MarkTargetWithCross(bot, capernian))
                return true;
        }
    }

    // Target priority 3: Sanguinar (debuff Hunter and melee move here after Thaladred)
    if (!target)
    {
        Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
        if (IsAdvisorActive(sanguinar))
        {
            target = sanguinar;
            if (isPhase3 && MarkTargetWithSkull(bot, sanguinar))
                return true;
        }
    }

    // Target priority 4: Telonicus; initialized as nullptr for melee positioning below
    Unit* telonicus = nullptr;
    if (!target)
    {
        telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
        if (IsAdvisorActive(telonicus))
        {
            target = telonicus;
            if (isPhase3 && MarkTargetWithSkull(bot, telonicus))
                return true;
        }
    }

    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    if (target != telonicus || telonicus->GetVictim() == bot)
        return false;

    // Melee DPS need to stay at max-ish melee range behind Telonicus to avoid bombs.
    return MeleeDpsPositionOutsideBombRange(telonicus);
}

bool KaelthasSunstriderAssignAdvisorDpsPriorityAction::MeleeDpsPositionOutsideBombRange(
    Unit* telonicus)
{
    if (!PlayerbotAI::IsMelee(bot) || !PlayerbotAI::IsDps(bot))
        return false;

    float const desiredDist = bot->GetMeleeRange(telonicus) - 0.5f;
    float const behindAngle = Position::NormalizeOrientation(telonicus->GetOrientation() + M_PI);
    float const targetX = telonicus->GetPositionX() + desiredDist * std::cos(behindAngle);
    float const targetY = telonicus->GetPositionY() + desiredDist * std::sin(behindAngle);

    if (bot->GetExactDist2d(targetX, targetY) <= 0.25f)
        return false;

    return MoveTo(
        TK_MAP_ID, targetX, targetY, telonicus->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KaelthasSunstriderManageAdvisorDpsTimerAction::Execute(Event /*event*/)
{
    static constexpr std::array advisorNames = {
        "grand astromancer capernian", "master engineer telonicus", "lord sanguinar", };

    bool advisorAtFullHp = false;
    for (char const* name : advisorNames)
    {
        Unit* advisor = AI_VALUE2(Unit*, "find target", name);
        if (!advisor)
            continue;

        if (advisor->GetHealth() == advisor->GetMaxHealth() &&
            !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        {
            advisorAtFullHp = true;
            break;
        }
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const instanceId = kaelthas->GetInstanceId();

    if (advisorAtFullHp)
    {
        advisorDpsWaitTimer[instanceId] = ADVISOR_DPS_WAIT_NOT_STARTED;
        return false;
    }

    auto it = advisorDpsWaitTimer.find(instanceId);
    if (it == advisorDpsWaitTimer.end() || it->second != ADVISOR_DPS_WAIT_NOT_STARTED)
        return false;

    it->second = getMSTime();
    return true;
}

bool KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* axe = GetLegendaryWeapon(botAI, Id(TkNpcs::NPC_DEVASTATION));
    Unit* mace = GetLegendaryWeapon(botAI, Id(TkNpcs::NPC_COSMIC_INFUSER));
    Unit* dagger = GetLegendaryWeapon(botAI, Id(TkNpcs::NPC_INFINITY_BLADES));
    Unit* sword = GetLegendaryWeapon(botAI, Id(TkNpcs::NPC_WARP_SLICER));

    bool const isTank = PlayerbotAI::IsTank(bot);
    bool const isRangedDps = PlayerbotAI::IsRangedDps(bot);
    bool const isMeleeDps = PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot);

    // Priority 0: Everybody other than the main tank needs to stay away from the axe.
    // But for assist tanks, move away only after getting aggro on the mace, dagger, or sword
    // The variable return allows a failure to MoveAway not to exit the function.
    bool didAvoidDevastation = false;
    if (axe && HandleDevastationAvoidance(axe, mace, dagger, sword, isTank, isMeleeDps))
        didAvoidDevastation = true;

    if (isTank)
        return didAvoidDevastation;

    // Melee dps obviously has to stand near a weapon to hit it, so any weapon too close to the axe
    // is skipped temporarily.
    constexpr float safeDistance = 12.0f;
    auto const isTooCloseToAxe = [&](Unit* candidate)
    {
        return isMeleeDps && axe && candidate->GetExactDist2d(axe) <= safeDistance;
    };

    struct WeaponPriority
    {
        TkNpcs entry;
        // The axe is marked with a cross instead of skull as it is outside the full priority
        // chain (only ranged dps attacks it).
        bool markWithCross;
        bool rangedDpsOnly;
    };

    static constexpr std::array weaponPriorities = {
        WeaponPriority{ TkNpcs::NPC_STAFF_OF_DISINTEGRATION, false, false },
        WeaponPriority{ TkNpcs::NPC_COSMIC_INFUSER,          false, false },
        WeaponPriority{ TkNpcs::NPC_NETHERSTRAND_LONGBOW,    false, false },
        WeaponPriority{ TkNpcs::NPC_DEVASTATION,             true,  true  },
        WeaponPriority{ TkNpcs::NPC_INFINITY_BLADES,         false, false },
        WeaponPriority{ TkNpcs::NPC_WARP_SLICER,             false, false },
        WeaponPriority{ TkNpcs::NPC_PHASESHIFT_BULWARK,      false, false },
    };

    Unit* target = nullptr;
    bool markClaimed = false;
    bool skippedForAxe = false;
    for (WeaponPriority const& weapon : weaponPriorities)
    {
        if (weapon.rangedDpsOnly && !isRangedDps)
            continue;

        Unit* candidate = GetLegendaryWeapon(botAI, Id(weapon.entry));
        if (!candidate)
            continue;

        if (!markClaimed)
        {
            markClaimed = true;
            if (weapon.markWithCross ?
                    MarkTargetWithCross(bot, candidate) : MarkTargetWithSkull(bot, candidate))
            {
                return true;
            }
        }

        if (isTooCloseToAxe(candidate))
        {
            skippedForAxe = true;
            continue;
        }

        target = candidate;
        break;
    }

    if (!target)
    {
        if (!skippedForAxe)
            return didAvoidDevastation;

        // If every weapon still standing is inside the axe's reach, then melee should hold and
        // attack nothing rather than end up in Whirlwind range.
        bot->AttackStop();
        bot->InterruptSpell(CURRENT_MELEE_SPELL);
        bot->CastStop();
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetSelection(ObjectGuid());
        return true;
    }

    return didAvoidDevastation || (AI_VALUE(Unit*, "current target") != target && Attack(target));
}

bool KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction::HandleDevastationAvoidance(
    Unit* axe, Unit* mace, Unit* dagger, Unit* sword, bool const isTank, bool const isMeleeDps)
{
    bool const hasAggroFromWeapon =
        (mace && mace->GetVictim() == bot) ||
        (dagger && dagger->GetVictim() == bot) ||
        (sword && sword->GetVictim() == bot);

    bool result = false;

    if (!isTank || hasAggroFromWeapon)
    {
        float const safeDistance = isTank ? 17.0f : 12.0f;
        float const currentDistance = bot->GetExactDist2d(axe);
        if (currentDistance < safeDistance)
            result = MoveAway(axe, safeDistance - currentDistance);
    }

    if (isMeleeDps && AI_VALUE(Unit*, "current target") == axe)
    {
        // Just in case melee ends up on the axe despite the priority list leaving them off
        bot->AttackStop();
        bot->InterruptSpell(CURRENT_MELEE_SPELL);
        bot->CastStop();
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetSelection(ObjectGuid());
    }

    return result;
}

bool KaelthasSunstriderMoveDevastationAwayAction::Execute(Event /*event*/)
{
    Unit* axe = GetLegendaryWeapon(botAI, Id(TkNpcs::NPC_DEVASTATION));
    if (!axe)
        return false;

    if (AI_VALUE(Unit*, "current target") != axe)
        return Attack(axe);

    if (MarkTargetWithCross(bot, axe))
        return true;

    if (axe->GetVictim() != bot || !bot->IsWithinMeleeRange(axe))
        return false;

    // Devastation's CombatReach is 0y (so it is set to DEFAULT_COMBAT_REACH, or 1.5y).
    constexpr float safeDistance = 15.0f;
    if (!GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return false;

    return MoveFromGroup(safeDistance);
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::Execute(Event /*event*/)
{
    static constexpr std::array weapons = {
        WeaponInfo{ TkNpcs::NPC_NETHERSTRAND_LONGBOW, TkItems::ITEM_NETHERSTRAND_LONGBOW },
        WeaponInfo{ TkNpcs::NPC_COSMIC_INFUSER, TkItems::ITEM_COSMIC_INFUSER },
        WeaponInfo{ TkNpcs::NPC_DEVASTATION, TkItems::ITEM_DEVASTATION },
        WeaponInfo{ TkNpcs::NPC_INFINITY_BLADES, TkItems::ITEM_INFINITY_BLADE },
        WeaponInfo{ TkNpcs::NPC_WARP_SLICER, TkItems::ITEM_WARP_SLICER },
        WeaponInfo{ TkNpcs::NPC_STAFF_OF_DISINTEGRATION, TkItems::ITEM_STAFF_OF_DISINTEGRATION },
        WeaponInfo{ TkNpcs::NPC_PHASESHIFT_BULWARK, TkItems::ITEM_PHASESHIFT_BULWARK },
    };

    bool equipped = false;
    for (auto const& weapon : weapons)
    {
        if (!ShouldBotLootWeapon(weapon.npcEntry))
            continue;

        if (bot->HasItemCount(Id(weapon.itemId), 1, false))
        {
            equipped |= EquipLegendaryWeapon(Id(weapon.itemId));
            continue;
        }

        return LootWeapon(Id(weapon.npcEntry), Id(weapon.itemId)) || equipped;
    }

    return equipped;
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::ShouldBotLootWeapon(TkNpcs weaponEntry)
{
    uint8 const tab = AiFactory::GetPlayerSpecTab(bot);

    bool const isDruid = bot->getClass() == CLASS_DRUID;
    bool const isHunter = bot->getClass() == CLASS_HUNTER;
    bool const isPaladin = bot->getClass() == CLASS_PALADIN;
    bool const isRogue = bot->getClass() == CLASS_ROGUE;
    bool const isShaman = bot->getClass() == CLASS_SHAMAN;

    bool const isDk = bot->getClass() == CLASS_DEATH_KNIGHT;
    bool const isFrostDk = (isDk && tab == DEATH_KNIGHT_TAB_FROST);

    bool const isWarrior = bot->getClass() == CLASS_WARRIOR;
    bool const isArmsWarrior = (isWarrior && tab == WARRIOR_TAB_ARMS);

    switch (weaponEntry)
    {
        case TkNpcs::NPC_NETHERSTRAND_LONGBOW: // Hunter
            return isHunter;

        case TkNpcs::NPC_COSMIC_INFUSER: // Healer
            return PlayerbotAI::IsHeal(bot);

        // Fury Warriors could use the axe, but their DPS is terrible at 70 so they're better off
        // looting the dagger to break MC (and DW 1H is better dps than Titan Grip at 70 anyway!?)
        case TkNpcs::NPC_DEVASTATION: // Arms Warrior, Blood/Unholy DK, Ret Paladin
            return isArmsWarrior || (isDk && !isFrostDk) || (isPaladin && PlayerbotAI::IsDps(bot));

        case TkNpcs::NPC_INFINITY_BLADES: // Rogue, Hunter, Fury/Prot Warrior, Frost DK, Enh Shaman
            return isRogue || isHunter || (isWarrior && !isArmsWarrior) || isFrostDk ||
                (isShaman && tab == SHAMAN_TAB_ENHANCEMENT);

        // Sublety will probably want to use the Sword, but the spec is currently unimplemented,
        // and what kind of madman is going to raid with Sub anyway?
        case TkNpcs::NPC_WARP_SLICER: // Prot Paladin, Frost DK, Combat Rogue
            return (isPaladin && PlayerbotAI::IsTank(bot)) || isFrostDk ||
                (isRogue && tab == ROGUE_TAB_COMBAT);

        case TkNpcs::NPC_STAFF_OF_DISINTEGRATION: // Dps caster, Feral Druid
            return (!isHunter && PlayerbotAI::IsRangedDps(bot)) ||
                (isDruid && tab == DRUID_TAB_FERAL);

        case TkNpcs::NPC_PHASESHIFT_BULWARK: // Prot Paladin/Warrior
            return (isWarrior || isPaladin) && PlayerbotAI::IsTank(bot);

        default:
            return false;
    }
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::LootWeapon(uint32 weaponEntry, uint32 itemId)
{
    ItemPosCountVec dest;
    if (bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1) != EQUIP_ERR_OK)
        return false;

    Creature* weapon = GetDeadLegendaryWeapon(botAI, weaponEntry);
    if (!weapon)
        return false;

    LootObject loot(bot, weapon->GetGUID());
    if (!loot.IsLootPossible(bot))
        return false;

    context->GetValue<LootObject>("loot target")->Set(loot);

    if (bot->GetDistance2d(weapon) > INTERACTION_DISTANCE - 1.0f)
    {
        float const targetDist = INTERACTION_DISTANCE - 2.0f;
        float const angle = weapon->GetAngle(bot);
        float const targetX = weapon->GetPositionX() + std::cos(angle) * targetDist;
        float const targetY = weapon->GetPositionY() + std::sin(angle) * targetDist;

        return MoveTo(
            TK_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
            false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    OpenLootAction open(botAI);
    bool opened = open.Execute(Event());
    if (!opened)
        return opened;

    if (bot->HasItemCount(itemId, 1, false))
        return false;

    bot->SetLootGUID(weapon->GetGUID());

    constexpr uint8 weaponIndex = 0;
    WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
    *packet << weaponIndex;
    bot->GetSession()->QueuePacket(packet);

    return true;
}

bool KaelthasSunstriderLootLegendaryWeaponsAction::EquipLegendaryWeapon(uint32 itemId)
{
    Item* legendaryItem = nullptr;
    auto const checkSlot = [&](uint8 bag, uint8 slot)
    {
        Item* item = bot->GetItemByPos(bag, slot);
        if (item && item->GetEntry() == itemId)
        {
            legendaryItem = item;
            return true;
        }
        return false;
    };

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (checkSlot(INVENTORY_SLOT_BAG_0, slot))
            break;
    }

    if (!legendaryItem)
    {
        for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
        {
            if (checkSlot(INVENTORY_SLOT_BAG_0, slot))
                break;
        }
    }

    if (!legendaryItem)
    {
        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
        {
            if (Bag const* pBag = bot->GetBagByPos(bag))
            {
                for (uint8 slot = 0; slot < pBag->GetBagSize(); ++slot)
                {
                    if (checkSlot(bag, slot))
                        break;
                }
            }
            if (legendaryItem)
                break;
        }
    }

    if (!legendaryItem)
        return false;

    ItemTemplate const* proto = legendaryItem->GetTemplate();
    if (!proto)
        return false;

    if (proto->InventoryType == INVTYPE_NON_EQUIP)
        return false;

    uint8 dstSlot = EQUIPMENT_SLOT_MAINHAND;
    if (proto->InventoryType == INVTYPE_RANGED)
    {
        dstSlot = EQUIPMENT_SLOT_RANGED;
    }
    else if (proto->InventoryType == INVTYPE_SHIELD ||
        proto->InventoryType == INVTYPE_WEAPONOFFHAND)
    {
        dstSlot = EQUIPMENT_SLOT_OFFHAND;
    }

    // Infinity Blade prefers OH when MH already holds a legendary (so Combat Rogues and Frost DKs
    // will equip Warp Slicer MH, Infinity Blade OH)
    if (dstSlot == EQUIPMENT_SLOT_MAINHAND && itemId == Id(TkItems::ITEM_INFINITY_BLADE) &&
        bot->CanDualWield())
    {
        if (Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
        {
            uint32 const mhEntry = mhItem->GetEntry();
            if (IsLegendaryWeaponItem(mhEntry) && mhEntry != itemId)
                dstSlot = EQUIPMENT_SLOT_OFFHAND;
        }
    }

    Item* alreadyEquipped = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, dstSlot);
    if (alreadyEquipped && alreadyEquipped->GetEntry() == itemId)
        return false;

    bot->CastStop();

    // Moves an equipped item into the first free backpack slot so the slot it leaves can be
    // filled. False means the backpack is full and nothing could be freed.
    auto const stowEquippedItem = [&](uint8 equipSlot)
    {
        uint16 const from = (INVENTORY_SLOT_BAG_0 << 8) | equipSlot;
        for (uint8 bpSlot = INVENTORY_SLOT_ITEM_START; bpSlot < INVENTORY_SLOT_ITEM_END; ++bpSlot)
        {
            if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bpSlot))
                continue;

            bot->SwapItem(from, (INVENTORY_SLOT_BAG_0 << 8) | bpSlot);
            return true;
        }

        return false;
    };

    // A 2h occupies the offhand, so it has to be unequipped before the legendary can be equipped.
    if (dstSlot == EQUIPMENT_SLOT_OFFHAND)
    {
        Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        if (mhItem && mhItem->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
            return stowEquippedItem(EQUIPMENT_SLOT_MAINHAND);
    }

    uint16 srcPos = (legendaryItem->GetBagSlot() << 8) | legendaryItem->GetSlot();
    uint16 const dstPos = (INVENTORY_SLOT_BAG_0 << 8) | dstSlot;

    if (!alreadyEquipped)
    {
        bot->SwapItem(srcPos, dstPos);
        return true;
    }

    bool const oldIs2H = alreadyEquipped->GetTemplate()->InventoryType == INVTYPE_2HWEAPON;
    bool const newIs2H = proto->InventoryType == INVTYPE_2HWEAPON;

    bot->SwapItem(srcPos, dstPos);

    // Changing between a 2H and a 1H leaves a stale offhand behind.
    if (((oldIs2H && !newIs2H && proto->InventoryType != INVTYPE_SHIELD) ||
         (!oldIs2H && newIs2H)) && bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
    {
        stowEquippedItem(EQUIPMENT_SLOT_OFFHAND);
    }

    // If the bot swapped from a 2H to a 1H, fill the OH from the inventory if one is available.
    if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
        return true;

    Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (mhItem && mhItem->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
        return true;

    StatsWeightCalculator calculator(bot);
    calculator.SetItemSetBonus(false);
    calculator.SetOverflowPenalty(false);

    Item* bestOH = nullptr;
    float bestScore = 0.0f;

    auto const scanSlots = [&](uint8 bag, uint8 start, uint8 end)
    {
        for (uint8 slot = start; slot < end; ++slot)
        {
            Item* item = bot->GetItemByPos(bag, slot);
            if (!item || item == legendaryItem)
                continue;

            ItemTemplate const* itemProto = item->GetTemplate();
            if (!itemProto)
                continue;

            uint8 const invType = itemProto->InventoryType;
            if (invType != INVTYPE_WEAPONOFFHAND && invType != INVTYPE_SHIELD &&
                invType != INVTYPE_HOLDABLE && invType != INVTYPE_WEAPON)
            {
                continue;
            }

            if (invType == INVTYPE_WEAPON && !bot->CanDualWield())
                continue;

            if (bot->CanUseItem(itemProto) != EQUIP_ERR_OK)
                continue;

            float const score = calculator.CalculateItem(
                itemProto->ItemId, item->GetItemRandomPropertyId());
            if (score > bestScore)
            {
                bestScore = score;
                bestOH = item;
            }
        }
    };

    scanSlots(INVENTORY_SLOT_BAG_0, INVENTORY_SLOT_ITEM_START, INVENTORY_SLOT_ITEM_END);
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* pBag = bot->GetBagByPos(bag))
            scanSlots(bag, 0, pBag->GetBagSize());
    }

    if (bestOH)
    {
        WorldPacket ohPacket(CMSG_AUTOEQUIP_ITEM_SLOT, 2);
        ohPacket << bestOH->GetGUID() << uint8(EQUIPMENT_SLOT_OFFHAND);

        WorldPackets::Item::AutoEquipItemSlot ohNicePacket(std::move(ohPacket));
        ohNicePacket.Read();
        bot->GetSession()->HandleAutoEquipItemSlotOpcode(ohNicePacket);
    }

    return true;
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_HUNTER)
        return UseNetherstrandLongbow();

    if (bot->getClass() != CLASS_DRUID && PlayerbotAI::IsTank(bot))
        return UsePhaseshiftBulwark();

    return UseStaffOfDisintegration();
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UsePhaseshiftBulwark()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas || !kaelthas->HasAura(Id(TkSpells::SPELL_SHOCK_BARRIER)))
        return false;

    if (bot->HasAura(Id(TkSpells::SPELL_ARCANE_BARRIER)))
        return false;

    Item* offHand = GetEquippedItemInSlot(
        bot, EQUIPMENT_SLOT_OFFHAND, Id(TkItems::ITEM_PHASESHIFT_BULWARK));
    if (!offHand)
        return false;

    return UseEquippedItemWithPacket(offHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseStaffOfDisintegration()
{
    if (bot->HasAura(Id(TkSpells::SPELL_MENTAL_PROTECTION_FIELD)))
        return false;

    Item* mainHand = GetEquippedItemInSlot(
        bot, EQUIPMENT_SLOT_MAINHAND, Id(TkItems::ITEM_STAFF_OF_DISINTEGRATION));
    if (!mainHand)
        return false;

    return UseEquippedItemWithPacket(mainHand);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseNetherstrandLongbow()
{
    // Make legendary arrows if none are left.
    if (bot->HasItemCount(Id(TkItems::ITEM_NETHER_SPIKES), 1, false))
        return false;

    Item* ranged = GetEquippedItemInSlot(
        bot, EQUIPMENT_SLOT_RANGED, Id(TkItems::ITEM_NETHERSTRAND_LONGBOW));
    if (!ranged)
        return false;

    return UseEquippedItemWithPacket(ranged);
}

bool KaelthasSunstriderUseLegendaryWeaponsAction::UseEquippedItemWithPacket(Item* item)
{
    if (!item || bot->CanUseItem(item) != EQUIP_ERR_OK || bot->IsNonMeleeSpellCast(true))
        return false;

    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 castCount = 1;
    ObjectGuid itemGuid = item->GetGUID();
    uint32 glyphIndex = 0;
    uint8 castFlags = 0;
    uint32 spellId = 0;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (item->GetTemplate()->Spells[i].SpellId > 0 &&
            item->GetTemplate()->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
        {
            spellId = item->GetTemplate()->Spells[i].SpellId;
            break;
        }
    }

    if (!spellId)
        return false;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << castCount << spellId << itemGuid << glyphIndex << castFlags;

    uint32 targetFlag = TARGET_FLAG_UNIT;
    packet << targetFlag << bot->GetPackGUID();

    bot->GetSession()->HandleUseItemOpcode(packet);
    return true;
}

bool KaelthasSunstriderTanksPositionBossAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    // Off-tanks are repositioned if Kael aggroes them, but only the main tank picks him up.
    constexpr float tolerance = 4.0f;
    return MoveToTankPosition(
        kaelthas, KAELTHAS_TANK_POSITION, tolerance, PlayerbotAI::IsMainTank(bot));
}

bool KaelthasSunstriderAvoidFlameStrikeAction::Execute(Event /*event*/)
{
    constexpr float hazardRadius = 10.0f;
    Unit* flameStrike = GetNearestFlameStrikeInRadius(bot, hazardRadius);
    if (!flameStrike)
        return false;

    bot->CastStop();
    constexpr uint32 minInterval = 0;
    return FleePosition(flameStrike->GetPosition(), hazardRadius, minInterval);
}

bool KaelthasSunstriderAssignFinalPhaseTargetAction::Execute(Event /*event*/)
{
    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
    {
        if (Unit* phoenix = GetAssignedPhoenix())
            return AssistTankPicksUpPhoenix(phoenix);

        Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
        if (!kaelthas || kaelthas->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return false;

        return AI_VALUE(Unit*, "current target") != kaelthas && Attack(kaelthas);
    }

    return NonTanksAssignTargetAndAvoidPhoenixes();
}

// If there is more than one Phoenix up, the assist tanks will each pick up one.
Unit* KaelthasSunstriderAssignFinalPhaseTargetAction::GetAssignedPhoenix()
{
    std::vector<Unit*> phoenixes;
    for (auto const& targetGuid : AI_VALUE(GuidVector, "possible targets no los"))
    {
        Unit* target = botAI->GetUnit(targetGuid);
        if (target && target->GetEntry() == Id(TkNpcs::NPC_PHOENIX))
            phoenixes.push_back(target);
    }

    if (phoenixes.empty())
        return nullptr;

    std::sort(phoenixes.begin(), phoenixes.end(),
        [](Unit* first, Unit* second) { return first->GetGUID() < second->GetGUID(); });

    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) && phoenixes.size() >= 2)
        return phoenixes[1];

    return phoenixes[0];
}

bool KaelthasSunstriderAssignFinalPhaseTargetAction::AssistTankPicksUpPhoenix(Unit* phoenix)
{
    if (AI_VALUE(Unit*, "current target") != phoenix)
        return Attack(phoenix);

    if (phoenix->GetVictim() != bot)
        return false;

    constexpr float safeDistance = 12.0f;
    if (!GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return false;

    return MoveFromGroup(safeDistance);
}

// Priority: (1) Kael with Shock Barrier (to interrupt Pyroblast), (2) Eggs (ranged only), (3) Kael
// without Shock Barrier, and (4) Phoenixes but only for ranged during Kael's RP power-up scene.
// Phoenixes kill themselves so having them included is only because bots have nothing else to do
// during the scene.
bool KaelthasSunstriderAssignFinalPhaseTargetAction::NonTanksAssignTargetAndAvoidPhoenixes()
{
    // Phoenixes that turn into eggs remain alive and on threat lists. They simply become
    // unattackable and invisible on top of the egg.
    //
    // Silly AC bug: the "dead" phoenix keeps its Burn aura (36720) so the egg sits inside an active
    // 8-yard, ~5k-per-2s Burn. The egg is therefore ranged-only, like the phoenix.
    Unit* phoenix = AI_VALUE2(Unit*, "find target", "phoenix");
    if (phoenix && phoenix->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        phoenix = nullptr;

    if (phoenix)
    {
        constexpr float safeDistance = 15.0f;
        float const currentDistance = bot->GetExactDist2d(phoenix);

        // A Phoenix is survivable and a Flame Strike is not, so don't try to avoid Phoenixes when
        // too close to a Flame Strike
        constexpr float flameStrikeRadius = 15.0f;
        if (currentDistance < safeDistance &&
            !GetNearestFlameStrikeInRadius(bot, flameStrikeRadius))
        {
            return MoveAway(phoenix, safeDistance - currentDistance);
        }
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    bool const isKaelthasAttackable =
        kaelthas && !kaelthas->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE);

    Unit* target = nullptr;
    if (isKaelthasAttackable && kaelthas->HasAura(Id(TkSpells::SPELL_SHOCK_BARRIER)))
        target = kaelthas;
    else if (Creature* egg = PlayerbotAI::IsRanged(bot) ? GetPhoenixEgg(bot) : nullptr)
        target = egg;
    else if (isKaelthasAttackable)
        target = kaelthas;
    else if (PlayerbotAI::IsRanged(bot))
        target = phoenix;

    return target && AI_VALUE(Unit*, "current target") != target && Attack(target);
}

bool KaelthasSunstriderBreakMindControlAction::Execute(Event /*event*/)
{
    Player* mcTarget = nullptr;
    float closestDist = std::numeric_limits<float>::max();

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot)
            continue;

        if (member->HasAura(Id(TkSpells::SPELL_KAELTHAS_MIND_CONTROL)))
        {
            float distToMcMember = bot->GetExactDist2d(member);
            if (distToMcMember < closestDist)
            {
                closestDist = distToMcMember;
                mcTarget = member;
            }
        }
    }

    if (!mcTarget)
        return false;

    if (!bot->IsWithinMeleeRange(mcTarget))
    {
        return MoveTo(
            TK_MAP_ID, mcTarget->GetPositionX(), mcTarget->GetPositionY(), mcTarget->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    char const* spell = nullptr;
    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            spell = "blood strike";
            break;

        case CLASS_HUNTER:
            spell = "wing clip";
            break;

        case CLASS_ROGUE:
            spell = (AiFactory::GetPlayerSpecTab(bot) == ROGUE_TAB_COMBAT) ?
                "shiv" : "sinister strike";
            break;

        case CLASS_SHAMAN:
            spell = "stormstrike";
            break;

        case CLASS_WARRIOR:
            spell = "hamstring";
            break;

        default:
            return false;
    }

    return botAI->CanCastSpell(spell, mcTarget) && botAI->CastSpell(spell, mcTarget);
}

// The vast majority of this action is not to address avoidance but to implement the Gravity Lapse
// mechanic, which does not otherwise properly affect bots due to bots not having a packet handler
// for flight and instead only toggling their flight flags manually upon movement.
bool KaelthasSunstriderSpreadOutInMidairAction::Execute(Event /*event*/)
{
    if (!bot->HasAura(Id(TkSpells::SPELL_GRAVITY_LAPSE_AURA)))
        return DropToGround();

    return HoverAndSpread();
}

bool KaelthasSunstriderSpreadOutInMidairAction::DropToGround()
{
    if (bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING))
    {
        if (!bot->movespline->Finalized())
            return false;

        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR);

        if (!bot->IsRooted())
            bot->SendMovementFlagUpdate();

        return true;
    }

    if (!bot->movespline->Finalized())
        return false;

    float const x = bot->GetPositionX();
    float const y = bot->GetPositionY();
    float const floorZ = bot->GetMapHeight(x, y, bot->GetPositionZ(), true, MAX_FALL_DISTANCE);

    if (floorZ <= INVALID_HEIGHT)
        return false;

    float const heightAboveFloor = bot->GetPositionZ() - floorZ;
    if (heightAboveFloor <= 1.0f && !bot->IsFlying() && !bot->CanFly())
        return false;

    if (bot->IsFlying() || bot->CanFly())
    {
        bot->RemoveUnitMovementFlag(
            MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_FLYING);

        if (!bot->IsRooted())
            bot->SendMovementFlagUpdate();
    }

    if (heightAboveFloor <= 0.5f)
        return false;

    bot->GetMotionMaster()->Clear();
    bot->GetMotionMaster()->MoveFall();

    return true;
}

bool KaelthasSunstriderSpreadOutInMidairAction::HoverAndSpread()
{
    if (!bot->IsFlying() || !bot->CanFly())
    {
        bot->AddUnitMovementFlag(
            MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_FLYING);

        if (!bot->IsRooted())
            bot->SendMovementFlagUpdate();
    }

    MotionMaster* motionMaster = bot->GetMotionMaster();
    if (motionMaster->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE)
        motionMaster->MovementExpiredOnSlot(MOTION_SLOT_CONTROLLED);

    auto const roll = [](uint32 value, uint32 salt)
    {
        uint32 hash = value + salt;
        hash ^= hash >> 16;
        hash *= 0x85EBCA6Bu;
        hash ^= hash >> 13;
        hash *= 0xC2B2AE35u;
        hash ^= hash >> 16;
        return (hash >> 8) / static_cast<float>(1 << 24);
    };

    Aura* lapse = bot->GetAura(Id(TkSpells::SPELL_GRAVITY_LAPSE_AURA));
    if (!lapse)
        return false;

    uint32 const seed = bot->GetGUID().GetCounter() ^ static_cast<uint32>(lapse->GetApplyTime());

    constexpr float minHoverHeight = 10.0f;
    constexpr float maxHoverHeight = 35.0f;
    constexpr uint32 heightSalt = 1u;
    float const desiredHeight =
        minHoverHeight + roll(seed, heightSalt) * (maxHoverHeight - minHoverHeight);

    constexpr int32 minReactionMs = 250;
    constexpr int32 maxReactionMs = 1600;
    constexpr uint32 reactionSalt = 2u;
    int32 const reactionDelayMs = minReactionMs + static_cast<int32>(roll(seed, reactionSalt) *
        (maxReactionMs - minReactionMs));

    if (lapse->GetMaxDuration() - lapse->GetDuration() < reactionDelayMs)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    constexpr float maxAimBias = 0.20f;
    constexpr uint32 aimSalt = 3u;
    float const aimBias = (roll(seed, aimSalt) * 2.0f - 1.0f) * maxAimBias;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const botZ = bot->GetPositionZ();

    constexpr float safeDistance = 18.0f;
    float pushX = 0.0f;
    float pushY = 0.0f;
    float pushZ = 0.0f;
    float closestDist = std::numeric_limits<float>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        float const distToMember = bot->GetExactDist(member);
        closestDist = std::min(closestDist, distToMember);
        if (distToMember >= safeDistance)
            continue;

        float const violation = safeDistance - distToMember;

        if (distToMember < 0.1f)
        {
            float const bearing = bot->GetAngle(member) + M_PI;
            pushX += std::cos(bearing) * violation;
            pushY += std::sin(bearing) * violation;
            continue;
        }

        pushX += (botX - member->GetPositionX()) / distToMember * violation;
        pushY += (botY - member->GetPositionY()) / distToMember * violation;
        pushZ += (botZ - member->GetPositionZ()) / distToMember * violation;
    }

    float const floorZ = bot->GetMapHeight(botX, botY, botZ, true, MAX_FALL_DISTANCE);
    if (floorZ <= INVALID_HEIGHT)
        return false;

    float const cosBias = std::cos(aimBias);
    float const sinBias = std::sin(aimBias);
    float const biasedPushX = pushX * cosBias - pushY * sinBias;
    pushY = pushX * sinBias + pushY * cosBias;
    pushX = biasedPushX;

    constexpr float heightPull = 0.35f;
    pushZ += (desiredHeight - (botZ - floorZ)) * heightPull;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    constexpr float containmentRadius = 40.0f;
    float const anchorX = kaelthas->GetPositionX();
    float const anchorY = kaelthas->GetPositionY();
    float const anchorDist = bot->GetExactDist2d(anchorX, anchorY);

    if (anchorDist > containmentRadius)
    {
        float const pull = anchorDist - containmentRadius;
        pushX += (anchorX - botX) / anchorDist * pull;
        pushY += (anchorY - botY) / anchorDist * pull;
    }

    constexpr float chainDangerDistance = 15.0f;
    constexpr float minMoveDistance = 4.0f;
    if (closestDist > chainDangerDistance &&
        std::sqrt(pushX * pushX + pushY * pushY + pushZ * pushZ) < minMoveDistance)
    {
        return false;
    }

    constexpr float stepFraction = 0.4f;
    float targetX = botX + pushX * stepFraction;
    float targetY = botY + pushY * stepFraction;
    float targetZ = botZ + pushZ * stepFraction;

    float const targetFloorZ = bot->GetMapHeight(
        targetX, targetY, targetZ, true, MAX_FALL_DISTANCE);
    if (targetFloorZ <= INVALID_HEIGHT)
        return false;

    targetZ = std::clamp(targetZ, targetFloorZ + minHoverHeight, targetFloorZ + maxHoverHeight);

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, botX, botY, botZ, targetX, targetY, targetZ, false))
    {
        return false;
    }

    if (bot->GetExactDist(targetX, targetY, targetZ) <= 1.0f)
        return false;

    return MoveTo(
        TK_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
        MovementPriority::MOVEMENT_FORCED, false, false);
}
