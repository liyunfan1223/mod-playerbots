/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalActions.h"
#include "EncounterHelpers.h"
#include "HyjalHelpers.h"
#include "Playerbots.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <vector>

using namespace HyjalHelpers;
using namespace EncounterHelpers;

// General

bool HyjalResetEncounterStatesAction::Execute(Event /*event*/)
{
    bool reset = false;

    Action* winterchillAction = context->GetAction("rage winterchill spread ranged in circle");
    if (winterchillAction && static_cast<RageWinterchillSpreadRangedInCircleAction*>(
            winterchillAction)->ResetWinterchillPositionReached())
    {
        reset = true;
    }

    Action* anetheronAction = context->GetAction("anetheron spread ranged in circle");
    if (anetheronAction && static_cast<AnetheronSpreadRangedInCircleAction*>(
            anetheronAction)->ResetAnetheronPositionReached())
    {
        reset = true;
    }

    if (AI_VALUE(bool, "kaz'rogal below mana threshold"))
    {
        RESET_AI_VALUE(bool, "kaz'rogal below mana threshold");
        reset = true;
    }

    reset |= archimondeAirBurstTargets.erase(bot->GetInstanceId()) > 0;

    return reset;
}

bool HyjalMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || !mainTank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (!bot->HasAura(Id(HyjalSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", boss) && botAI->CastSpell("steady shot", boss);
}

bool HyjalMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    if (AI_VALUE(Unit*, "current target") != boss)
        return Attack(boss);

    if (boss->GetVictim() != bot || !bot->IsWithinMeleeRange(boss))
        return false;

    if (bot->GetHealthPct() < _bailBelowHealthPct)
        return false;

    constexpr float arrivalDist = 4.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, _position, arrivalDist, boss, moveX, moveY, backwards))
        return false;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool HyjalRemoveDangerousDotAction::Execute(Event /*event*/)
{
    uint32 const spellId = GetSelfImmunitySpell(bot);
    return spellId && botAI->CanCastSpell(spellId, bot) && botAI->CastSpell(spellId, bot);
}

// Rage Winterchill

// This is essentially a modified "avoid aoe" due to the default Playerbots.MaxAoeAvoidRadius in
// the config being 15y (>25y would be needed for avoid aoe to work for D&D).
bool RageWinterchillRangedGetOutOfDeathAndDecayAction::Execute(Event /*event*/)
{
    Position pool;
    if (!GetDeathAndDecayPosition(botAI, pool))
        return false;

    constexpr uint32 minInterval = 0;
    return FleePosition(pool, DEATH_AND_DECAY_RADIUS, minInterval);
}

// Spread ranged DPS in a circle initially. After the initial spread, movement is free.
bool RageWinterchillSpreadRangedInCircleAction::Execute(Event /*event*/)
{
    if (_winterchillPositionReached)
        return false;

    constexpr float healerRadius = 25.0f;
    constexpr float dpsRadius = 35.0f;
    float moveX;
    float moveY;
    float moveZ;
    if (!GetRangedRingStep(
            bot, WINTERCHILL_TANK_POSITION, healerRadius, dpsRadius, moveX, moveY, moveZ,
            _winterchillPositionReached))
    {
        return false;
    }

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Melee looks for an open position within the boss's melee range. If one isn't available (likely
// the case if D&D lands on melee, with its 20y radius), then melee takes the shortest path out of
// the hazard and waits it out.
bool RageWinterchillMeleeManeuverThroughDeathAndDecayAction::Execute(Event /*event*/)
{
    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return false;

    Position pool;
    if (!GetDeathAndDecayPosition(botAI, pool))
        return false;

    float moveX;
    float moveY;
    float moveZ;
    if (!GetMeleeHazardManeuverStep(
            bot, winterchill, { pool }, DEATH_AND_DECAY_RADIUS, {}, moveX, moveY, moveZ))
    {
        return false;
    }

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Anetheron

bool AnetheronMisdirectBossAndInfernalsToTanksAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    Player* tank = nullptr;
    Unit* enemy = nullptr;
    if (anetheron->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
    {
        tank = GetGroupMainTank(bot);
        enemy = anetheron;
    }
    else if (Unit* infernal = GetLooseInfernal(botAI))
    {
        tank = GetInfernalTank(bot);
        enemy = infernal;
    }

    if (!enemy || !tank || !tank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(HyjalSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", enemy) && botAI->CastSpell("steady shot", enemy);
}

// As with Winterchill, this is just an initial spread, though in the case of Anetheron, bots still
// try to spread a bit throughout the fight because of Carrion Swarm
bool AnetheronSpreadRangedInCircleAction::Execute(Event /*event*/)
{
    if (_anetheronPositionReached)
    {
        constexpr float safeDistFromPlayer = 6.0f;
        constexpr uint32 minInterval = 2000;
        if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
            return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);

        return false;
    }

    constexpr float healerRadius = 27.0f;
    constexpr float dpsRadius = 34.0f;
    float moveX;
    float moveY;
    float moveZ;
    if (!GetRangedRingStep(
            bot, ANETHERON_TANK_POSITION, healerRadius, dpsRadius, moveX, moveY, moveZ,
            _anetheronPositionReached))
    {
        return false;
    }

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Everyone standing near whoever Inferno is aimed at is about to be stunned, since the Infernal
// lands on that player's feet. The 3.5s cast is the whole window, and stepping out of it also
// starts the bot clear of the immolation aura the Infernal carries afterwards
bool AnetheronMoveAwayFromInfernoTargetAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    Player* infernoTarget = GetInfernoTarget(anetheron);
    if (!infernoTarget || infernoTarget == bot)
        return false;

    constexpr uint32 minInterval = 0;
    return FleePosition(infernoTarget->GetPosition(), INFERNAL_ESCAPE_DISTANCE, minInterval);
}

bool AnetheronBringInfernalToInfernalTankAction::Execute(Event /*event*/)
{
    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, GetInfernalTankPosition(bot), arrivalDist, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

// Note that Infernals cannot be taunted.
bool AnetheronInfernalTankTakePositionAction::Execute(Event /*event*/)
{
    constexpr float arrivalDist = 3.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, GetInfernalTankPosition(bot), arrivalDist, GetInfernalTargetingBot(botAI), moveX,
            moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// An Infernal burns everything within 10y of itself, so anybody who is not tanking it leaves.
bool AnetheronGetOutOfImmolationAction::Execute(Event /*event*/)
{
    Unit* infernal = GetNearestInfernal(botAI);
    if (!infernal)
        return false;

    constexpr uint32 minInterval = 0;
    return FleePosition(infernal->GetPosition(), INFERNAL_DANGER_RADIUS, minInterval);
}

// Melee stay on Anetheron throughout. Ranged attack Infernals if they are reasonably nearby.
bool AnetheronAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (PlayerbotAI::IsMelee(bot))
        return AI_VALUE(Unit*, "current target") != anetheron && Attack(anetheron);

    if (Unit* infernal = GetInfernalToAttack(botAI, anetheron))
    {
        // Wait for the tank to pick up the Infernal before attacking directly
        Player* infernalTank = GetInfernalTank(bot);
        if (!infernalTank || infernal->GetVictim() == infernalTank)
            return AI_VALUE(Unit*, "current target") != infernal && Attack(infernal);
    }

    return AI_VALUE(Unit*, "current target") != anetheron && Attack(anetheron);
}

// Kaz'rogal

bool KazrogalAssistTanksMoveInFrontAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    constexpr float arrivalDist = 4.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, mainTank->GetPosition(), arrivalDist, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KazrogalSpreadRangedInArcAction::Execute(Event /*event*/)
{
    std::vector<Player*> const rangedMembers = GetRangedMembers(bot);
    auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    if (findIt == rangedMembers.end())
        return false;

    size_t const count = rangedMembers.size();
    size_t const botIndex = std::distance(rangedMembers.begin(), findIt);

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal)
        return false;

    float const arcRadius = GetKazrogalRangedArcRadius(kazrogal);
    float const arcSpan = GetKazrogalRangedArcSpan(arcRadius);
    float const arcStart = KAZROGAL_RANGED_ARC_CENTER - arcSpan / 2.0f;

    float const angle = (count == 1) ? KAZROGAL_RANGED_ARC_CENTER :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    float const targetX = kazrogal->GetPositionX() + arcRadius * std::cos(angle);
    float const targetY = kazrogal->GetPositionY() + arcRadius * std::sin(angle);

    constexpr float moveDist = 3.5f;
    float moveX;
    float moveY;
    float moveZ;
    if (!CanTakeStepTowards(bot, targetX, targetY, moveDist, moveX, moveY, moveZ))
        return false;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KazrogalMoveAwayFromGroupAction::Execute(Event /*event*/)
{
    if (bot->GetPower(POWER_MANA) > MARK_REJOIN_MANA)
    {
        SET_AI_VALUE(bool, "kaz'rogal below mana threshold", false);
        return false;
    }

    Player* nearestPlayer = GetNearestPlayerInRadius(bot, MARK_ESCAPE_DISTANCE);
    if (!nearestPlayer)
        return false;

    float const step = MARK_ESCAPE_DISTANCE - bot->GetExactDist2d(nearestPlayer);

    // Away from whoever is nearest. This combination of away from Kaz'rogal and nearest player
    // gets the bots a distance away from the boss before spreading sideways.
    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (kazrogal && nearestPlayer->GetExactDist2d(kazrogal) > bot->GetExactDist2d(kazrogal))
        return MoveAway(kazrogal, step);

    return MoveAway(nearestPlayer, step);
}

bool KazrogalActivateAspectOfTheViperAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(HyjalSpells::SPELL_ASPECT_OF_THE_VIPER), bot) &&
        botAI->CastSpell(Id(HyjalSpells::SPELL_ASPECT_OF_THE_VIPER), bot);
}

bool KazrogalCancelImmunityAction::Execute(Event /*event*/)
{
    uint32 const spellId = GetSelfImmunitySpell(bot);
    if (!spellId || !bot->HasAura(spellId))
        return false;

    bot->RemoveOwnedAura(spellId, ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
    return true;
}

// Life Tap first, then cast Shadow Ward if there isn't enough health to do so
bool KazrogalWarlockManageManaAction::Execute(Event /*event*/)
{
    if (bot->GetPower(POWER_MANA) <= MARK_LIFE_TAP_MANA &&
        botAI->CanCastSpell("life tap", bot) && botAI->CastSpell("life tap", bot))
    {
        return true;
    }

    if (!HasMarkOfKazrogal(bot))
        return false;

    return botAI->CanCastSpell("shadow ward", bot) && botAI->CastSpell("shadow ward", bot);
}

// Azgalor

bool AzgalorDisperseRangedAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    constexpr float safeDistFromBoss = 30.0f; // arbitrary, but ~20 yards + both CombatReaches
    constexpr uint32 minInterval = 0;

    if (bot->GetExactDist2d(azgalor) < safeDistFromBoss &&
        FleePosition(azgalor->GetPosition(), safeDistFromBoss, minInterval))
    {
        return true;
    }

    Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
    constexpr float safeDistFromDoomguard = 10.0f; // War Stomp is 10 yards center-to-center

    if (doomguard && bot->GetExactDist2d(doomguard) < safeDistFromDoomguard)
        return FleePosition(doomguard->GetPosition(), safeDistFromDoomguard, minInterval);

    if (doomguard && AI_VALUE(Unit*, "current target") == doomguard)
        return false;

    // Don't spread if focused on the Doomguard. There's actually not much space due to the need
    // to maintain significant distance from Azgalor as a proxy to avoid getting in Cleave distance.
    constexpr float safeDistFromPlayer = 5.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);
}

// Similar to D&D avoidance, but there are two notable differences to account for: two RoFs can be
// active at a time, and escape cannot take the bot into Azgalor's frontal arc due to the Cleave.
bool AzgalorMeleeManeuverThroughFireAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    std::vector<Position> const pools = GetRainOfFirePositions(botAI);
    if (pools.empty())
        return false;

    std::vector<BlockedArc> const cleaveArc = {
        { azgalor->GetOrientation(), CLEAVE_DANGER_ARC / 2.0f } };

    auto cleaveSafe = [azgalor](float x, float y)
    {
        return IsSafeFromAzgalorCleave(azgalor, x, y);
    };

    float moveX;
    float moveY;
    float moveZ;
    if (!GetMeleeHazardManeuverStep(
            bot, azgalor, pools, RAIN_OF_FIRE_RADIUS, cleaveArc, moveX, moveY, moveZ, cleaveSafe))
    {
        return false;
    }

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

// Like with Winterchill, this is essentially a forced "avoid aoe" due to the default config being
// insufficient (in this case, >20y would be needed for safety).
bool AzgalorRangedGetOutOfRainOfFireAction::Execute(Event /*event*/)
{
    Position pool;
    if (!GetNearestRainOfFirePosition(botAI, pool))
        return false;

    constexpr uint32 minInterval = 0;
    return FleePosition(pool, RAIN_OF_FIRE_RADIUS, minInterval);
}

bool AzgalorMoveToDoomguardTankAction::Execute(Event /*event*/)
{
    constexpr float arrivalDist = 5.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, AZGALOR_DOOMGUARD_POSITION, arrivalDist, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool AzgalorTankPositionDoomguardAction::Execute(Event /*event*/)
{
    Position const& position = AZGALOR_DOOMGUARD_POSITION;
    constexpr float arrivalDist = 3.0f;

    Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
    if (doomguard)
    {
        if (AI_VALUE(Unit*, "current target") != doomguard)
            return Attack(doomguard);

        if (doomguard->GetVictim() != bot || !bot->IsWithinMeleeRange(doomguard))
            return false;
    }
    else if (bot->GetExactDist2d(position) <= arrivalDist)
    {
        // If at position and still no Doomguard, just wait.
        return true;
    }

    // With no Doomguard up, this moves to the tanking position preemptively.
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, doomguard, moveX, moveY, backwards))
        return false;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Only ranged DPS within 70y should attack Doomguards. This distance seems to reach enough bots to
// get decent DPS on the adds but not reach too far (which risks ranged running in front of Azgalor
// to get to the Doomguard or just wasting time).
bool AzgalorDetermineDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor)
        return false;

    if (PlayerbotAI::IsMelee(bot))
        return AI_VALUE(Unit*, "current target") != azgalor && Attack(azgalor);

    Unit* target = nullptr;
    if (azgalor->GetHealthPct() <= BOSS_BURN_HEALTH_PCT)
    {
        target = azgalor;
    }
    else
    {
        constexpr float doomguardEngageDist = 70.0f;
        Unit* doomguard = AI_VALUE2(Unit*, "find target", "lesser doomguard");
        if (doomguard && bot->GetExactDist2d(doomguard) < doomguardEngageDist)
            target = doomguard;
        else
            target = azgalor;
    }

    return AI_VALUE(Unit*, "current target") != target && Attack(target);
}

// Archimonde

bool ArchimondeSetTremorTotemAction::Execute(Event /*event*/)
{
    if (AI_VALUE2(bool, "has totem", "tremor totem"))
        return false;

    constexpr uint32 tremorTotem = Id(HyjalSpells::SPELL_TREMOR_TOTEM);
    return botAI->CanCastSpell(tremorTotem, bot) && botAI->CastSpell(tremorTotem, bot);
}

// Air Burst knocks everyone around its target into the air. Losing the whole melee group at once
// is what has to be avoided, since Archimonde starts wagging his fat finger and one-shotting ranged
// when nobody is left in melee range. Thus, the avoidance is to get away from the tank.
bool ArchimondeKeepAirBurstAwayFromTankAction::Execute(Event /*event*/)
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    Unit* activeTank = archimonde->GetVictim();
    if (!activeTank)
        return false;

    AirBurstData airBurst;
    if (!GetPendingAirBurstCast(bot->GetInstanceId(), airBurst))
        return false;

    if (airBurst.targetGuid != activeTank->GetGUID() && airBurst.targetGuid != bot->GetGUID())
        return false;

    float const distanceToActiveTank = bot->GetExactDist2d(activeTank);
    if (distanceToActiveTank >= AIR_BURST_SAFE_DISTANCE)
        return false;

    return MoveAway(activeTank, AIR_BURST_SAFE_DISTANCE - distanceToActiveTank);
}

bool ArchimondeSpreadRangedAction::Execute(Event /*event*/)
{
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, ARCHIMONDE_RANGED_SPREAD_DISTANCE);
    if (!nearestPlayer)
        return false;

    constexpr uint32 minInterval = 3000;
    return FleePosition(
        nearestPlayer->GetPosition(), ARCHIMONDE_RANGED_SPREAD_DISTANCE, minInterval);
}

bool ArchimondeAvoidDoomfireAction::Execute(Event /*event*/)
{
    std::vector<Position> const trail = GetDoomfirePositions(botAI);

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    Position const* nearest = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    float totalDx = 0.0f;
    float totalDy = 0.0f;

    for (Position const& patch : trail)
    {
        float const d = bot->GetExactDist2d(patch);
        if (d >= DOOMFIRE_FIELD_RADIUS)
            continue;

        if (d < nearestDistance)
        {
            nearest = &patch;
            nearestDistance = d;
        }

        if (d > 0.0f)
        {
            float const weight = (DOOMFIRE_FIELD_RADIUS - d) / DOOMFIRE_FIELD_RADIUS;
            totalDx += (botX - patch.GetPositionX()) / d * weight;
            totalDy += (botY - patch.GetPositionY()) / d * weight;
        }
    }

    float norm = std::sqrt(totalDx * totalDx + totalDy * totalDy);
    float moveDist = (nearest && nearestDistance < DOOMFIRE_DANGER_RADIUS) ?
        DOOMFIRE_DANGER_RADIUS - nearestDistance : 0.0f;

    constexpr float minFieldStrength = 0.05f;
    if (nearest && nearestDistance < DOOMFIRE_BURN_RADIUS && norm < minFieldStrength)
    {
        constexpr uint8 fanSteps = 12;
        constexpr float escapeStep = 10.0f;
        bool found = false;

        for (uint8 pass = 0; pass < 2 && !found; ++pass)
        {
            bool const validate = (pass == 0);
            float bestClearance = -1.0f;

            for (uint8 i = 0; i < fanSteps; ++i)
            {
                float const angle = 2.0f * static_cast<float>(M_PI) * i / fanSteps;
                float const testX = botX + std::cos(angle) * DOOMFIRE_DANGER_RADIUS;
                float const testY = botY + std::sin(angle) * DOOMFIRE_DANGER_RADIUS;

                float clearance = DOOMFIRE_FIELD_RADIUS;
                for (Position const& patch : trail)
                    clearance = std::min(clearance, patch.GetExactDist2d(testX, testY));

                if (clearance <= bestClearance)
                    continue;

                float stepX = testX;
                float stepY = testY;
                float stepZ = bot->GetPositionZ();
                if (validate &&
                    !CanTakeStepTowards(bot, testX, testY, escapeStep, stepX, stepY, stepZ))
                {
                    continue;
                }

                bestClearance = clearance;
                totalDx = stepX - botX;
                totalDy = stepY - botY;
                found = true;
            }
        }

        norm = std::sqrt(totalDx * totalDx + totalDy * totalDy);
        moveDist = norm;
    }

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde)
        return false;

    if (norm > 0.0f && moveDist >= 0.5f)
    {
        float const targetX = botX + (totalDx / norm) * moveDist;
        float const targetY = botY + (totalDy / norm) * moveDist;

        MovementPriority const priority = PlayerbotAI::IsHeal(bot) ?
            MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_FORCED;

        bool const backwards = archimonde->GetVictim() == bot;

        return MoveTo(
            HYJAL_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
            false, false, priority, true, backwards);
    }

    bool const inPosition = PlayerbotAI::IsRanged(bot) ?
        bot->IsWithinCombatRange(archimonde, botAI->GetRange("spell")) :
        bot->GetExactDist2d(archimonde) <= bot->GetMeleeRange(archimonde) - MELEE_RANGE_INSET;

    if (inPosition)
        return false;

    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, archimonde->GetPosition(), arrivalDist, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    if (IsPositionNearDoomfire(botAI, moveX, moveY, DOOMFIRE_DANGER_RADIUS))
        return false;

    return MoveTo(
        HYJAL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}
