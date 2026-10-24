/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EoEActions_Drakes.h"
#include "EoEData.h"
#include "EoEEncounter_Drakes.h"
#include "EoEEncounter_Malygos.h"
#include "MotionMaster.h"
#include "Playerbots.h"
#include "Timer.h"
#include "Vehicle.h"

#include <algorithm>
#include <cmath>
#include <vector>

bool EoEFlyDrakeAction::isPossible()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    return (vehicleBase && vehicleBase->GetEntry() == NPC_WYRMREST_SKYTALON);
}

bool EoEFlyDrakeAction::Execute(Event /*event*/)
{
    Unit* drake = bot->GetVehicleBase();
    if (!drake)
        return false;

    MotionMaster* mm = drake->GetMotionMaster();

    if (!_stackCalcAtMs || GetMSTimeDiffToNow(_stackCalcAtMs) >= DRAKE_STACK_RECALC_MS)
    {
        std::vector<Unit*> fields;
        GetStaticFields(bot, fields);
        _stackValid = GetDrakeStackPoint(bot, fields, _stackX, _stackY, _stackZ);
        _stackCalcAtMs = getMSTime();
    }

    if (_stackValid)
    {
        Unit* boss = GetMalygos(bot);

        if (drake->GetExactDist(_stackX, _stackY, _stackZ) > DRAKE_STACK_TOLERANCE)
        {
            bool const sameSpot =
                _issued && std::fabs(_stackX - _issuedX) < DRAKE_DESTINATION_EPSILON &&
                std::fabs(_stackY - _issuedY) < DRAKE_DESTINATION_EPSILON;

            // A finished leg takes the point generator with it, so not-POINT means the next leg is due.
            if (!sameSpot || mm->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
            {
                float legX;
                float legY;
                GetDrakeApproachPoint(drake, boss, _stackX, _stackY, legX, legY);

                // Straight 3d spline, same reason as the hover disks.
                drake->SetCanFly(true);
                mm->Clear(false);
                mm->MovePoint(0, legX, legY, _stackZ, FORCED_MOVEMENT_NONE, 0.0f, 0.0f,
                              /*generatePath*/ false, /*forceDestination*/ true);
                drake->SendMovementFlagUpdate();
                _issuedX = _stackX;
                _issuedY = _stackY;
                _issued = true;
            }
            return true;
        }

        _issued = false;
        if (mm->GetCurrentMovementGeneratorType() != IDLE_MOTION_TYPE)
        {
            mm->Clear(false);
            mm->MoveIdle();
            drake->SendMovementFlagUpdate();
        }

        // Everything a drake casts is aimed at the boss or itself, so healers can stay pinned too.
        if (boss)
        {
            if (!drake->HasInArc(CAST_ANGLE_IN_FRONT, boss))
                drake->SetFacingToObject(boss);
        }

        // Parked, so hand the tick to the drake rotation.
        return false;
    }

    // No boss: the flight is still forming up, or he is dead. Either way fan out behind the leader's
    // drake. A leader without one is falling, so nobody follows them.
    Player* master = botAI->GetMaster();
    if (!master || !master->IsInMap(bot))
        return false;
    Unit* masterVehicle = master->GetVehicleBase();
    if (!masterVehicle)
        return false;

    if (drake->GetExactDist(masterVehicle) > DRAKE_FORMUP_RADIUS)
    {
        uint8 const numPlayers =
            bot->GetRaidDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL ? EOE_RAID_SIZE_25MAN : EOE_RAID_SIZE_10MAN;
        // 3/4 of a circle, with a 90 deg frontal cone left clear
        float const quarter = static_cast<float>(M_PI_2);
        float const angle = static_cast<float>(botAI->GetGroupSlotIndex(bot)) * (EOE_TWO_PI - quarter) /
                                static_cast<float>(numPlayers) + quarter;
        drake->SetCanFly(true);
        mm->MoveFollow(masterVehicle, DRAKE_FORMUP_SPREAD, angle);
        drake->SendMovementFlagUpdate();
        return true;
    }
    return false;
}

// Riders keep the flight after the kill, but there is nothing left to cast at or heal through.
bool EoEDrakeAttackAction::isUseful() { return GetMalygos(bot) != nullptr; }

bool EoEDrakeAttackAction::isPossible()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    return (vehicleBase && vehicleBase->GetEntry() == NPC_WYRMREST_SKYTALON);
}

bool EoEDrakeAttackAction::Execute(Event /*event*/)
{
    Unit* drake = bot->GetVehicleBase();
    if (!drake)
        return false;

    // Derived once: the roster walk sorts the whole group, and the heal branch reads it twice.
    std::vector<ObjectGuid> healers;
    GetDrakeHealerGuids(botAI, healers);

    // Healers only ever cast on their own drake, so they need no boss at all.
    if (IsDrakeHealer(botAI, healers))
        return HealRotation(drake, healers);

    Unit* boss = GetMalygos(bot);
    if (!boss)
        return false;

    return DpsRotation(drake, boss);
}

bool EoEDrakeAttackAction::CastDrakeSpell(Unit* target, uint32 spellId)
{
    return botAI->CanCastVehicleSpell(spellId, target) && botAI->CastVehicleSpell(spellId, target);
}

bool EoEDrakeAttackAction::DpsRotation(Unit* drake, Unit* target)
{
    // Closing the gap belongs to EoEFlyDrakeAction alone: two owners fight over the destination,
    // and MoveForwards has no answer for a point in mid-air.
    if (drake->GetExactDist(target) > DRAKE_ATTACK_RANGE)
        return false;

    uint8 comboPoints = drake->GetComboPoints(target);

    // The slots stay set for a whole SURGE_CYCLE_MS, so the raw flag would keep the drake quiet for
    // more than twice the window it is protecting.
    uint32 surgeElapsed = 0;
    if (GetDrakeSurgeElapsedMs(botAI, surgeElapsed) && surgeElapsed < SURGE_BEAM_END_MS)
    {
        // Shielded, beam still landing: keep spiking but never finish. At the reserve the shield runs
        // out a second short of the last tick, and the re-shield needs a combo point waiting for it.
        if (drake->HasAura(SPELL_FLAME_SHIELD))
        {
            return DrakeCanAffordWithShield(drake, SPELL_FLAME_SPIKE) &&
                   CastDrakeSpell(target, SPELL_FLAME_SPIKE);
        }

        // Before the beam: spend the bank while the bar still covers the shield, rebuild, then let
        // it climb.
        if (comboPoints >= DRAKE_ENGULF_SURGE_COMBO && DrakeCanAffordWithShield(drake, SPELL_ENGULF_IN_FLAMES))
            return CastDrakeSpell(target, SPELL_ENGULF_IN_FLAMES);

        // At zero the shield has nothing to spend, so that point is worth going under the reserve.
        if (comboPoints < DRAKE_SHIELD_RESERVE_COMBO &&
            (!comboPoints || DrakeCanAffordWithShield(drake, SPELL_FLAME_SPIKE)))
            return CastDrakeSpell(target, SPELL_FLAME_SPIKE);

        return false;
    }

    if (comboPoints >= DRAKE_ENGULF_COMBO)
        return CastDrakeSpell(target, SPELL_ENGULF_IN_FLAMES);
    else
        return CastDrakeSpell(target, SPELL_FLAME_SPIKE);
}

bool EoEDrakeAttackAction::HealRotation(Unit* drake, std::vector<ObjectGuid> const& healers)
{
    // Both stay on our own drake: combo points are held for one target at a time, so chasing the
    // lowest resets the count. DrakeCanAfford stands in for the BAD_TARGETS-reporting check.
    uint32 surgeElapsed = 0;
    bool const fixated = GetDrakeSurgeElapsedMs(botAI, surgeElapsed) && surgeElapsed < SURGE_BEAM_END_MS;

    // Only until the shield lands. After that the ladder below is right: rebuilding from an empty bank
    // can't reach DRAKE_LIFE_BURST_COMBO before the beam ends, so it can't spend what the re-shield needs.
    if (fixated && !drake->HasAura(SPELL_FLAME_SHIELD))
    {
        uint8 const comboPoints = drake->GetComboPoints();
        if (comboPoints >= DRAKE_LIFE_BURST_COMBO && DrakeCanAffordWithShield(drake, SPELL_LIFE_BURST))
            return botAI->CastVehicleSpell(SPELL_LIFE_BURST, drake);

        if (comboPoints < DRAKE_SHIELD_RESERVE_COMBO &&
            (!comboPoints || DrakeCanAffordWithShield(drake, SPELL_REVIVIFY)))
            return botAI->CastVehicleSpell(SPELL_REVIVIFY, drake);

        return false;
    }

    if (drake->GetComboPoints() < DRAKE_LIFE_BURST_COMBO)
    {
        // Revivify is break-even against the regen, so a shielded drake that keeps casting never
        // climbs back to the 25 its second shield costs unless this holds the difference back.
        if (fixated ? !DrakeCanAffordWithShield(drake, SPELL_REVIVIFY) : !DrakeCanAfford(drake, SPELL_REVIVIFY))
            return false;

        return botAI->CastVehicleSpell(SPELL_REVIVIFY, drake);
    }

    bool const canBurst = DrakeCanAfford(drake, SPELL_LIFE_BURST);

    // Life Burst is a flat heal, so the worst drake decides this, not the raid-wide total.
    std::vector<Unit*> flight;
    uint8 healerRank = 0;
    GetDrakeFlightAndHealerRank(botAI, healers, flight, healerRank);

    uint8 worstPct = 100;
    uint32 worstMissing = 0;
    for (Unit* other : flight)
    {
        uint32 const max = other->GetMaxHealth();
        if (!max)
            continue;

        uint8 const pct = static_cast<uint8>(other->GetHealth() * 100 / max);
        if (pct < worstPct)
        {
            worstPct = pct;
            worstMissing = max - other->GetHealth();
        }
    }

    if (canBurst && worstPct <= DRAKE_BURST_EMERGENCY_PCT)
        return botAI->CastVehicleSpell(SPELL_LIFE_BURST, drake);

    // Life Burst's self buff doubles as a record of who burst and when, real players included, and a
    // cast that quietly failed leaves no trace to mislead the rest.
    bool recentBurst = false;
    for (Unit* other : flight)
    {
        if (other == drake)
            continue;

        if (DrakeAuraRemainingMs(other, SPELL_LIFE_BURST) > DRAKE_LIFE_BURST_BUFF_MS - DRAKE_BURST_STAGGER_MS)
        {
            recentBurst = true;
            break;
        }
    }

    if (canBurst && !recentBurst)
    {
        if (DrakeAuraRemainingMs(drake, SPELL_LIFE_BURST) < DRAKE_LIFE_BURST_REFRESH_MS)
            return botAI->CastVehicleSpell(SPELL_LIFE_BURST, drake);

        if (worstPct <= DRAKE_BURST_HEALTH_PCT)
        {
            uint32 const wanted = (worstMissing + DRAKE_LIFE_BURST_HEAL - 1) / DRAKE_LIFE_BURST_HEAL;
            if (healerRank < wanted)
                return botAI->CastVehicleSpell(SPELL_LIFE_BURST, drake);
        }
    }

    // Revivify costs exactly one global's regen, so without this floor the bar never reaches 50.
    if (drake->GetPower(POWER_ENERGY) >= DRAKE_HOLD_ENERGY_FLOOR)
        return botAI->CastVehicleSpell(SPELL_REVIVIFY, drake);

    return false;
}

bool DrakeSurgeShieldAction::isPossible()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    return (vehicleBase && vehicleBase->GetEntry() == NPC_WYRMREST_SKYTALON);
}

bool DrakeSurgeShieldAction::Execute(Event /*event*/)
{
    Unit* drake = bot->GetVehicleBase();
    if (!drake)
        return false;

    // Ahead of the aura check, so the clock keeps running while a shield is up. Otherwise it goes
    // stale for the shield's whole duration and misreads the next pick as a continuation of this one.
    uint32 elapsed = 0;
    if (!GetDrakeSurgeElapsedMs(botAI, elapsed))
        return false;

    if (drake->HasAura(SPELL_FLAME_SHIELD))
        return false;

    uint8 const comboPoints = drake->GetComboPoints();

    // A finisher on an empty bank is SPELL_FAILED_NO_COMBO_POINTS, reported as a success.
    if (!comboPoints)
        return false;

    // Held until the cover still reaches the end of the beam; at the fixate it expires early.
    uint32 const cover = DRAKE_SHIELD_BASE_MS + comboPoints * DRAKE_SHIELD_MS_PER_COMBO;
    uint32 const castAt = std::min(cover >= SURGE_BEAM_END_MS ? 0u : SURGE_BEAM_END_MS - cover, SURGE_BEAM_DELAY_MS);

    if (elapsed < castAt)
        return false;

    // A big bank is worth more as a finisher, so yield until the beam is actually landing.
    if (comboPoints > DRAKE_SHIELD_MAX_COMBO && elapsed < SURGE_BEAM_DELAY_MS)
        return false;

    if (!DrakeCanAfford(drake, SPELL_FLAME_SHIELD))
        return false;

    // Safe mid-dodge: self-cast, so CastVehicleSpell skips the turn-the-vehicle and stop-moving
    // branches, and it is instant. Forced, because CanCastVehicleSpell reports BAD_TARGETS.
    botAI->CastVehicleSpell(SPELL_FLAME_SHIELD, drake);

    // Hand the tick on either way, or a Static Field dodge below this one stops halfway.
    return false;
}
