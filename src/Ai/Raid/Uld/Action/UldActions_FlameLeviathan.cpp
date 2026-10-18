/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldActions_FlameLeviathan.h"
#include "UldActions_Shared.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "AiObjectContext.h"
#include "CharmInfo.h"
#include "DBCEnums.h"
#include "GameObject.h"
#include "Group.h"
#include "LastMovementValue.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Position.h"
#include "EncounterHelpers.h"
#include "RaidDifficulty.h"
#include "ScriptedCreature.h"
#include "ServerFacade.h"
#include "UldEncounter_FlameLeviathan.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "Unit.h"
#include "Vehicle.h"

namespace
{
// Bearing offsets tried off a blocked direction, nearest first.
constexpr float HAZARD_FAN[] = {0.0f, 0.6f, -0.6f, 1.2f, -1.2f, 1.8f, -1.8f, 2.4f, -2.4f, 3.0f};

// Whether driving straight from `from` to `to` stays `margin` outside every hazard's reach. A leg that
// starts inside one still counts as clear while it leads out of it. A point is a leg with no length.
bool LegClearOfHazards(Unit* vehicle, Position const& from, Position const& to, std::vector<Unit*> const& hazards,
                       float margin)
{
    float const dx = to.GetPositionX() - from.GetPositionX();
    float const dy = to.GetPositionY() - from.GetPositionY();
    float const lengthSq = dx * dx + dy * dy;

    for (Unit* hazard : hazards)
    {
        float const safe = FlameLeviathanHazardReach(hazard, vehicle) + margin;
        float const start = hazard->GetExactDist2d(from.GetPositionX(), from.GetPositionY());
        if (start < safe)
        {
            if (hazard->GetExactDist2d(to.GetPositionX(), to.GetPositionY()) <= start)
                return false;
            continue;
        }

        float along = 0.0f;
        if (lengthSq > 0.0f)
            along = std::clamp(((hazard->GetPositionX() - from.GetPositionX()) * dx +
                                (hazard->GetPositionY() - from.GetPositionY()) * dy) / lengthSq,
                               0.0f, 1.0f);

        if (hazard->GetExactDist2d(from.GetPositionX() + along * dx, from.GetPositionY() + along * dy) < safe)
            return false;
    }

    return true;
}

// Self-casts skip CanCastVehicleSpell (it rejects their BAD_TARGETS) and target the hull, as CastVehicleSpell
// turns to face any other target. It returns true on a refusal too, so cooldown only once the energy drops.
bool CastVehicleSelfSpell(PlayerbotAI* botAI, Unit* vehicleBase, uint32 spellId, uint32 cost, uint32 cooldownMs)
{
    if (!vehicleBase || vehicleBase->HasSpellCooldown(spellId))
        return false;

    uint32 const before = vehicleBase->GetPower(POWER_ENERGY);
    if (before < cost)
        return false;

    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    CharmInfo* charm = vehicleBase->GetCharmInfo();
    if (info && charm && charm->GetGlobalCooldownMgr().HasGlobalCooldown(info))
        return false;

    if (!botAI->CastVehicleSpell(spellId, vehicleBase))
        return false;

    if (cost && vehicleBase->GetPower(POWER_ENERGY) >= before)
        return false;

    vehicleBase->AddSpellCooldown(spellId, 0, cooldownMs);
    return true;
}
}  // namespace

bool FlameLeviathanVehicleAction::CastVehicle(uint32 spellId, Unit* target, uint32 cooldownMs)
{
    if (!target || !vehicleBase_)
        return false;

    if (!botAI->CanCastVehicleSpell(spellId, target) || !botAI->CastVehicleSpell(spellId, target))
        return false;

    vehicleBase_->AddSpellCooldown(spellId, 0, cooldownMs);
    return true;
}

// The crate falls straight down where the lift dies, and the demolishers ride round him near the middle,
// so only a lift over the arena, at its closest pass to the centre. Lifts fly facing along their loop.
Unit* FlameLeviathanVehicleAction::FindMechanolift()
{
    Position const centre = FlameLeviathanArenaCentre();
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->GetEntry() != NPC_FL_MECHANOLIFT || !FlameLeviathanInArena(unit->GetPosition()))
            continue;

        // Centre still ahead: the pass gets closer yet. Every lift that crosses the arena is still shot once.
        if (unit->GetExactDist2d(&centre) > ULDUAR_FL_LIFT_DROP_RADIUS && unit->HasInArc(float(M_PI), &centre))
            continue;

        return unit;
    }

    return nullptr;
}

// Only what is already fighting, so a cannon with 70 yd of reach never opens on a pack nobody pulled.
Unit* FlameLeviathanVehicleAction::NearestFightingHostile()
{
    Unit* nearest = nullptr;
    for (auto const& guid : AI_VALUE(GuidVector, "possible targets"))
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !unit->IsInCombat())
            continue;

        uint32 const entry = unit->GetEntry();
        if (entry == NPC_FLAME_LEVIATHAN || entry == NPC_FL_TURRET || entry == NPC_FL_DEFENSE_TURRET)
            continue;

        if (!nearest || vehicleBase_->GetExactDist(unit) < vehicleBase_->GetExactDist(nearest))
            nearest = unit;
    }

    return nearest;
}

bool FlameLeviathanVehicleAction::Execute(Event /*event*/)
{
    vehicleBase_ = bot->GetVehicleBase();
    if (!vehicleBase_ || !bot->GetVehicle())
        return false;

    Unit* boss = FlameLeviathanBoss(botAI);

    // A rider's threat lands on its vehicle, so "attackers" never holds him and is only good for adds.
    Unit* add = nullptr;
    GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
    for (auto const& guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        uint32 const entry = unit->GetEntry();
        if (entry == NPC_FL_TURRET || entry == NPC_FL_DEFENSE_TURRET || entry == NPC_FLAME_LEVIATHAN)
            continue;

        if (!add || bot->GetExactDist(add) > bot->GetExactDist(unit))
            add = unit;
    }

    // Only once he's fighting: the instance lookup finds him anywhere in Ulduar, unattackable and 1000 yd off.
    Unit* target = boss && boss->IsInCombat() ? boss : add;

    // "attackers" is built from the riders' own threat references, and the core hands a rider's threat
    // to its vehicle, so the trash a vehicle is fighting may never appear in it.
    if (!target)
        target = NearestFightingHostile();

    switch (vehicleBase_->GetEntry())
    {
        case NPC_SALVAGED_DEMOLISHER:
            return DemolisherAction(target);
        case NPC_SALVAGED_DEMOLISHER_TURRET:
            return DemolisherTurretAction(target);
        case NPC_SALVAGED_SIEGE_ENGINE:
            return SiegeEngineAction(target);
        case NPC_SALVAGED_SIEGE_ENGINE_TURRET:
            return SiegeEngineTurretAction(target);
        case NPC_VEHICLE_CHOPPER:
            return ChopperAction(target);
        default:
            break;
    }

    return false;
}

// Barrels fly 1-3 s, so the aura lags every cast. A landing shows as a stack gained or the duration jumping
// back up, and the last one timed sets the refresh lead.
Aura* FlameLeviathanVehicleAction::TrackBarrels(Unit* target)
{
    Aura* own = target->GetAura(SPELL_FL_BLUE_PYRITE_DOT, vehicleBase_->GetGUID());
    uint8 const stacks = own ? own->GetStackAmount() : 0;
    int32 const duration = own ? own->GetDuration() : 0;
    uint32 const now = getMSTime();

    if (target->GetGUID() != barrelTarget_)
    {
        barrelTarget_ = target->GetGUID();
        barrelsInFlight_.clear();
    }
    else if (own && !barrelsInFlight_.empty() &&
             (stacks > lastPyriteStacks_ || duration > lastPyriteDurationMs_ + ULDUAR_FL_PYRITE_LANDING_JUMP_MS))
    {
        barrelLeadMs_ = std::clamp<uint32>(getMSTimeDiff(barrelsInFlight_.front(), now), ULDUAR_FL_PYRITE_FLIGHT_MIN_MS,
                                           ULDUAR_FL_PYRITE_FLIGHT_TIMEOUT_MS);
        size_t const landed =
            std::min<size_t>(std::max(int(stacks) - int(lastPyriteStacks_), 1), barrelsInFlight_.size());
        barrelsInFlight_.erase(barrelsInFlight_.begin(), barrelsInFlight_.begin() + landed);
    }

    barrelsInFlight_.erase(std::remove_if(barrelsInFlight_.begin(), barrelsInFlight_.end(),
                                          [now](uint32 cast)
                                          { return getMSTimeDiff(cast, now) >= ULDUAR_FL_PYRITE_FLIGHT_TIMEOUT_MS; }),
                           barrelsInFlight_.end());

    lastPyriteStacks_ = stacks;
    lastPyriteDurationMs_ = duration;
    return own;
}

bool FlameLeviathanVehicleAction::DemolisherAction(Unit* target)
{
    if (!target)
        return false;

    // Ahead of the thaw, which returns early: a landing nobody reads is timed out as a lost barrel.
    Aura* own = TrackBarrels(target);

    // Thaw first, a frozen hull is a minute of nothing. Aimed at the ally on purpose: DEST_LOCATION lets a
    // friendly target pass, the blast is enemy-only, and its Flames strip the stun.
    if (Unit* frozen = FlameLeviathanFrozenVehicle(bot, vehicleBase_, ULDUAR_FL_HURL_BOULDER_MIN_RANGE,
                                                   ULDUAR_FL_HURL_BOULDER_MAX_RANGE))
        if (CastVehicle(SPELL_FL_HURL_BOULDER, frozen))
            return true;

    // Our own Blue Pyrite aura, not another demolisher's. Barrels go on him only, as demolisher energy never
    // regenerates; adds and trash get boulders.
    bool wantBarrel = false;
    if (target->GetEntry() == NPC_FLAME_LEVIATHAN)
    {
        uint32 const energy = vehicleBase_->GetPower(POWER_ENERGY);
        uint32 const stacks = std::min<uint32>((own ? own->GetStackAmount() : 0) + barrelsInFlight_.size(),
                                               ULDUAR_FL_PYRITE_MAX_STACKS);
        uint32 const burst = energy >= (ULDUAR_FL_PYRITE_MAX_STACKS - stacks) * ULDUAR_FL_PYRITE_BARREL_COST +
                                           ULDUAR_FL_PYRITE_REFRESH_RESERVE
                                 ? ULDUAR_FL_PYRITE_MAX_STACKS
                                 : ULDUAR_FL_PYRITE_BURST_STACKS;

        uint32 const lead = barrelLeadMs_ ? barrelLeadMs_ : ULDUAR_FL_PYRITE_FLIGHT_MS;

        wantBarrel = energy >= ULDUAR_FL_PYRITE_BARREL_COST &&
                     (stacks < burst || (own && barrelsInFlight_.empty() &&
                                         own->GetDuration() <= int32(lead + ULDUAR_FL_PYRITE_REFRESH_SLACK_MS)));
    }

    // Our own cooldown and the GCD a boulder shares are waits, so no cast while either runs.
    SpellInfo const* barrelInfo = sSpellMgr->GetSpellInfo(SPELL_FL_HURL_PYRITE_BARREL);
    CharmInfo* charm = vehicleBase_->GetCharmInfo();
    bool const ready = barrelInfo && !vehicleBase_->HasSpellCooldown(SPELL_FL_HURL_PYRITE_BARREL) &&
                       !(charm && charm->GetGlobalCooldownMgr().HasGlobalCooldown(barrelInfo));

    bool thrown = false;
    if (wantBarrel && ready)
    {
        thrown = CastVehicle(SPELL_FL_HURL_PYRITE_BARREL, target);
        if (thrown)
            barrelsInFlight_.push_back(getMSTime());
    }

    if (thrown)
        return true;

    // Adds get what the barrel doesn't want, except Ram: free, knocks back, and covers the melee band under
    // Hurl Boulder's 10 yd floor.
    if (Unit* add = FlameLeviathanBestAdd(botAI, vehicleBase_, 0.0f, ULDUAR_FL_RAM_CONE_RADIUS, 0.0f))
        if (FlameLeviathanFaceForCone(vehicleBase_, add, ULDUAR_FL_RAM_CONE_HALF_ANGLE,
                                      ULDUAR_FL_RAM_CONE_RADIUS))
            if (CastVehicle(SPELL_FL_DEMOLISHER_RAM, add, ULDUAR_FL_DEMOLISHER_RAM_COOLDOWN_MS))
                return true;

    if (!wantBarrel)
        if (Unit* add = FlameLeviathanBestAdd(botAI, vehicleBase_, ULDUAR_FL_HURL_BOULDER_MIN_RANGE,
                                              ULDUAR_FL_HURL_BOULDER_MAX_RANGE, ULDUAR_FL_CANNON_SPLASH))
            if (CastVehicle(SPELL_FL_HURL_BOULDER, add))
                return true;

    return CastVehicle(SPELL_FL_HURL_BOULDER, target);
}

bool FlameLeviathanVehicleAction::DemolisherTurretAction(Unit* target)
{
    Unit* demolisher = FlameLeviathanRiddenVehicle(bot);

    // He chases his threat victim exactly like a Pursued one, so that crew needs the speed as well.
    if ((FlameLeviathanIsPursued(bot) || FlameLeviathanIsRamTarget(bot)) && demolisher &&
        !demolisher->HasAura(SPELL_FL_INCREASED_SPEED))
    {
        if (CastVehicleSelfSpell(botAI, vehicleBase_, SPELL_FL_INCREASED_SPEED, ULDUAR_FL_INCREASED_SPEED_COST,
                                 ULDUAR_FL_SPEED_RECAST_MS))
        {
            // CastVehicleSpell says yes even on a refusal, and instants resolve inline, so check the aura.
            if (demolisher->HasAura(SPELL_FL_INCREASED_SPEED))
                return true;
        }
    }

    // Crates are the demolisher's only refill. Above the ceiling the +25 would be partly thrown away.
    if (demolisher && demolisher->GetPower(POWER_ENERGY) <= ULDUAR_FL_CRATE_GRAB_CEILING)
    {
        GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
        for (auto const& guid : npcs)
        {
            Unit* crate = botAI->GetUnit(guid);
            if (!crate || crate->GetEntry() != NPC_FL_PYRITE_CONTAINER)
                continue;

            // No claim: spell_vehicle_grab_pyrite credits every hit until the crate despawns, and the
            // credit goes to the grabbing gunner's own demolisher, so two gunners both gain from it.
            if (crate->GetDistance(bot) >= ULDUAR_FL_CRATE_GRAB_RANGE)
                continue;

            if (CastVehicle(SPELL_FL_GRAB_CRATE, crate))
                return true;
        }
    }

    // Mortar thaws like the boulder, via Flames 65044, and has no minimum range, so the gunner covers the
    // close-in case under the boulder's 10 yd floor.
    if (Unit* frozen = FlameLeviathanFrozenVehicle(bot, vehicleBase_, 0.0f, ULDUAR_FL_MORTAR_MAX_RANGE))
        if (CastVehicle(SPELL_FL_MORTAR, frozen, ULDUAR_FL_MORTAR_COOLDOWN_MS))
            return true;

    if (Unit* lift = FindMechanolift())
        if (CastVehicle(SPELL_FL_ANTI_AIR_ROCKET, lift, ULDUAR_FL_ANTI_AIR_ROCKET_COOLDOWN_MS))
            return true;

    // Mortar is free and already cast every tick, so an add only costs the target swap. No minimum range
    // either, so it covers adds inside Fire Cannon's 10 yd floor.
    if (Unit* add = FlameLeviathanBestAdd(botAI, vehicleBase_, 0.0f, ULDUAR_FL_MORTAR_MAX_RANGE,
                                          ULDUAR_FL_MORTAR_SPLASH))
        if (CastVehicle(SPELL_FL_MORTAR, add, ULDUAR_FL_MORTAR_COOLDOWN_MS))
            return true;

    return CastVehicle(SPELL_FL_MORTAR, target, ULDUAR_FL_MORTAR_COOLDOWN_MS);
}

bool FlameLeviathanVehicleAction::SiegeEngineAction(Unit* target)
{
    // No early return on a null target: a posted engine is 90 yd from him and shoots adds, so it has
    // work to do whether or not he is alive and reachable.

    // Earmark what this vehicle still owes: the interrupt duty travels with its fuel, and a pursued
    // driver or the vent reserve needs Steam Rush more than it needs a Ram.
    uint32 needed = ULDUAR_FL_RAM_COST;
    if (FlameLeviathanIsVentInterrupter(botAI, bot))
        needed += ULDUAR_FL_ELECTROSHOCK_COST;
    if (FlameLeviathanIsPursued(bot) || FlameLeviathanIsVentReserve(bot))
        needed += ULDUAR_FL_STEAM_RUSH_COST;

    if (vehicleBase_->GetPower(POWER_ENERGY) < needed)
        return false;

    // An add in the cone outranks him (190k against 230M, and adds pile up). Picked once: falling back to him
    // mid-turn swings the engine round again and it faces neither.
    Unit* add = FlameLeviathanBestAdd(botAI, vehicleBase_, 0.0f, ULDUAR_FL_RAM_CONE_RADIUS, 0.0f);
    Unit* shot = add ? add : target;

    if (!shot)
        return false;

    // Cone, not range: CastVehicleSpell only turns past 120 degrees, and a tick turning beats 40 wasted energy.
    // The reserve keeps facing him and an idle post its corner; with an add the drive faces the same pick.
    if (FlameLeviathanIsVentReserve(bot) || (!add && FlameLeviathanCornerPost(botAI, bot) >= 0))
    {
        if (!FlameLeviathanInCone(vehicleBase_, shot, ULDUAR_FL_RAM_CONE_HALF_ANGLE,
                                  ULDUAR_FL_RAM_CONE_RADIUS))
            return false;
    }
    else if (!FlameLeviathanFaceForCone(vehicleBase_, shot, ULDUAR_FL_RAM_CONE_HALF_ANGLE,
                                        ULDUAR_FL_RAM_CONE_RADIUS))
        return false;

    return CastVehicle(SPELL_FL_RAM, shot);
}

bool FlameLeviathanVehicleAction::SiegeEngineTurretAction(Unit* target)
{
    if (FlameLeviathanIsPursued(bot))
    {
        Unit* siege = FlameLeviathanRiddenVehicle(bot);
        if (siege && !siege->HasAura(SPELL_FL_SHIELD_GENERATOR))
            if (CastVehicleSelfSpell(botAI, vehicleBase_, SPELL_FL_SHIELD_GENERATOR, 0,
                                     ULDUAR_FL_SHIELD_GENERATOR_COOLDOWN_MS))
                if (siege->HasAura(SPELL_FL_SHIELD_GENERATOR))
                    return true;
    }

    if (Unit* lift = FindMechanolift())
        if (CastVehicle(SPELL_FL_ANTI_AIR_ROCKET_SIEGE, lift, ULDUAR_FL_ANTI_AIR_ROCKET_COOLDOWN_MS))
            return true;

    if (vehicleBase_->GetPower(POWER_ENERGY) < ULDUAR_FL_FIRE_CANNON_COST)
        return false;

    // A posted engine's gun serves its corner first, so its hull is the first thing a fresh add has
    // threat on and the add stays on the engine that can knock it back.
    if (Unit* hull = FlameLeviathanRiddenVehicle(bot))
    {
        int8 const post = FlameLeviathanHullCornerPost(botAI, bot, hull);
        if (post >= 0)
        {
            Position const point = FlameLeviathanCornerPostPoint(static_cast<uint8>(post));
            if (Unit* add = FlameLeviathanBestAdd(botAI, vehicleBase_, ULDUAR_FL_FIRE_CANNON_MIN_RANGE,
                                                  ULDUAR_FL_FIRE_CANNON_MAX_RANGE, ULDUAR_FL_CANNON_SPLASH, &point,
                                                  ULDUAR_FL_CORNER_HOLD_RADIUS))
                if (CastVehicle(SPELL_FL_FIRE_CANNON, add))
                    return true;
        }
    }

    // Heaviest and widest gun the raid has: 76k in 20 yd against a 190k lasher. Next to adds that never
    // despawn, what this costs his 230M is noise.
    if (Unit* add = FlameLeviathanBestAdd(botAI, vehicleBase_, ULDUAR_FL_FIRE_CANNON_MIN_RANGE,
                                          ULDUAR_FL_FIRE_CANNON_MAX_RANGE, ULDUAR_FL_CANNON_SPLASH))
        if (CastVehicle(SPELL_FL_FIRE_CANNON, add))
            return true;

    if (!target)
        return false;

    return CastVehicle(SPELL_FL_FIRE_CANNON, target);
}

bool FlameLeviathanVehicleAction::ChopperAction(Unit* target)
{
    // Tar's pool spawns 9 yd behind the chopper, so it only lands in his path while he's behind us: whenever
    // this chopper is pursued or runs the lead.
    if (target && !vehicleBase_->HasInArc(M_PI / 2.0f, target))
        if (CastVehicleSelfSpell(botAI, vehicleBase_, SPELL_FL_TAR, 0, ULDUAR_FL_TAR_COOLDOWN_MS))
            return true;

    if (vehicleBase_->GetPower(POWER_ENERGY) < ULDUAR_FL_SONIC_HORN_COST)
        return false;

    // The tar lead never shoots adds, one turn toward an add costs a pool. The rest pick once, like the siege
    // engine, or they alternate and face neither.
    Unit* shot = FlameLeviathanIsTarLead(botAI, bot)
                     ? nullptr
                     : FlameLeviathanBestAdd(botAI, vehicleBase_, 0.0f,
                                             ULDUAR_FL_SONIC_HORN_CONE_RADIUS, 0.0f);
    if (!shot)
        shot = target;

    // The narrowest cone on the fight at 50 degrees, so it needs the facing more than the others do.
    if (!shot || !FlameLeviathanFaceForCone(vehicleBase_, shot, ULDUAR_FL_SONIC_HORN_CONE_HALF_ANGLE,
                                            ULDUAR_FL_SONIC_HORN_CONE_RADIUS))
        return false;

    return CastVehicle(SPELL_FL_SONIC_HORN, shot);
}

bool FlameLeviathanInterruptVentsAction::Execute(Event /*event*/)
{
    Unit* vehicleBase = bot->GetVehicleBase();
    if (!vehicleBase || vehicleBase->GetEntry() != NPC_SALVAGED_SIEGE_ENGINE)
        return false;

    Unit* boss = FlameLeviathanBoss(botAI);
    if (!boss || !FlameLeviathanIsVentChanneling(boss))
        return false;

    // Outranks the urgent drive, so never park a hull that has a blast or hazard to leave. Same test as the
    // drive (strict Hammer circle for the reserve), so both agree on whether a dodge is coming.
    uint32 const towerMask = FlameLeviathanActiveTowerMask(botAI);
    bool const dodging =
        FlameLeviathanShouldClearBatteringRam(botAI, bot) ||
        (towerMask && GetFlameLeviathanNearestTowerHazard(botAI, vehicleBase, towerMask, ULDUAR_FL_TOWER_HAZARD_MARGIN,
                                                          FlameLeviathanIsVentReserve(bot) ? FL_TOWER_STORM : 0));

    // A moving hull's spline overrides SetFacingTo, so stop before turning.
    if (!vehicleBase->HasInArc(ULDUAR_FL_ELECTROSHOCK_CONE_HALF_ANGLE * 2.0f, boss))
    {
        if (dodging)
            return false;

        vehicleBase->StopMoving();
    }

    // Turning owns the tick: CastVehicleSpell only turns past 120 degrees, wider than this 60 degree cone,
    // so nothing else ever points the hull at him.
    if (!FlameLeviathanFaceForCone(vehicleBase, boss, ULDUAR_FL_ELECTROSHOCK_CONE_HALF_ANGLE,
                                   ULDUAR_FL_ELECTROSHOCK_CONE_RADIUS))
        return true;

    if (!botAI->CanCastVehicleSpell(SPELL_FL_ELECTROSHOCK, boss))
    {
        // A Ram or Steam Rush GCD (category 133) is a wait, not a refusal. Hold the aim through it, or
        // the drive turns the hull away again.
        SpellInfo const* shockInfo = sSpellMgr->GetSpellInfo(SPELL_FL_ELECTROSHOCK);
        CharmInfo* charm = vehicleBase->GetCharmInfo();
        if (!dodging && shockInfo && charm && charm->GetGlobalCooldownMgr().HasGlobalCooldown(shockInfo))
        {
            vehicleBase->StopMoving();
            return true;
        }

        return false;
    }

    if (!botAI->CastVehicleSpell(SPELL_FL_ELECTROSHOCK, boss))
        return false;

    // Take the channel off the queue whichever way the shot goes, so the rest of the line does not
    // empty into it behind us.
    FlameLeviathanClaimVentChannel(bot);

    // CastVehicleSpell says yes even on a refusal, and the shock resolves inline, so a channel still running
    // is a miss: retry soon instead of charging the full cooldown.
    if (FlameLeviathanIsVentChanneling(boss))
    {
        vehicleBase->AddSpellCooldown(SPELL_FL_ELECTROSHOCK, 0, ULDUAR_FL_ELECTROSHOCK_RETRY_MS);
        return true;
    }

    vehicleBase->AddSpellCooldown(SPELL_FL_ELECTROSHOCK, 0, ULDUAR_FL_ELECTROSHOCK_COOLDOWN_MS);
    return true;
}

bool FlameLeviathanDriveAction::Execute(Event /*event*/)
{
    vehicleBase_ = bot->GetVehicleBase();
    if (!vehicleBase_ || !FlameLeviathanIsDriver(bot))
        return false;

    if (!FlameLeviathanEngaged(botAI))
        return false;

    Unit* boss = FlameLeviathanBoss(botAI);
    if (!boss)
        return false;

    // He chases his threat victim exactly like a Pursued vehicle whenever nobody holds the aura.
    bool const pursued = FlameLeviathanIsPursued(bot);
    if (pursued || FlameLeviathanIsRamTarget(bot))
    {
        dodgeGoal_.reset();
        return Kite(boss);
    }

    ResetKite();

    // Only the reserve is in Electroshock range, so a dodge out of it costs a whole channel. In the vent window
    // Hammer marks use the strict circle (Inferno keeps the full band) and dodges pick exits it can fire from.
    bool const ventDuty = FlameLeviathanIsVentReserve(bot) && FlameLeviathanVentWindowOpen(bot, boss);

    Unit* hazard = nullptr;
    if (uint32 towerMask = FlameLeviathanActiveTowerMask(botAI))
        hazard = GetFlameLeviathanNearestTowerHazard(botAI, vehicleBase_, towerMask, ULDUAR_FL_TOWER_HAZARD_MARGIN,
                                                     ventDuty ? FL_TOWER_STORM : 0);

    if (!hazard)
        dodgeGoal_.reset();

    // A hazard already cleared reports false rather than owning the tick, so fall through to the
    // station instead of failing the whole action and handing the tick to the on-foot rotation.
    if (hazard && ClearHazard(hazard, ventDuty ? boss : nullptr))
        return true;

    if (ClearBatteringRam(boss))
        return true;

    // Below the two dodges on purpose: a posted engine still has to get out of a tower blast and out
    // of Battering Ram, and the corner will still be there afterwards.
    if (int8 const corner = FlameLeviathanCornerPost(botAI, bot); corner >= 0)
    {
        uint8 const slot = static_cast<uint8>(corner);

        // Rush the 7 yd/s commute, wave 1 comes at 34 s. Steam Rush charges along the facing, so only with the
        // post dead ahead and over a charge away.
        Position const post = FlameLeviathanCornerPostPoint(slot);
        if (vehicleBase_->GetExactDist2d(post) > ULDUAR_FL_STEAM_RUSH_DIST + ULDUAR_FL_ARRIVE_TOLERANCE &&
            vehicleBase_->HasInArc(ULDUAR_FL_STEAM_RUSH_AIM_ARC, &post))
            CastVehicleSelfSpell(botAI, vehicleBase_, SPELL_FL_STEAM_RUSH, ULDUAR_FL_STEAM_RUSH_COST,
                                 ULDUAR_FL_STEAM_RUSH_COOLDOWN_MS);

        // Face the add the cast node will Ram, not the corner: it comes back from the arena side, often inside
        // Fire Cannon's floor. Same pick as SiegeEngineAction, so the two can't aim apart.
        Unit* add = FlameLeviathanBestAdd(botAI, vehicleBase_, 0.0f, ULDUAR_FL_RAM_CONE_RADIUS, 0.0f);

        // COMBAT, not FORCED like the kite: a Fury dodge is FORCED, and an equal-priority move waits
        // out this one for up to MaxWaitForMove (5 s) of a 6.5 s fuse.
        if (add)
            return DriveTo(post, add, false);

        return DriveTo(post, ULDUAR_FL_ARENA_CORNERS[slot]);
    }

    if (vehicleBase_->GetEntry() == NPC_SALVAGED_DEMOLISHER &&
        vehicleBase_->GetPower(POWER_ENERGY) < ULDUAR_FL_CRATE_DETOUR_ENERGY)
        if (DetourToCrate(boss))
            return true;

    return HoldStation(boss);
}

void FlameLeviathanDriveAction::ResetKite()
{
    if (kiteIdx_ < 0)
        return;

    kiteIdx_ = -1;
    kiteDir_ = 0;

    // The last kite leg went out FORCED, and IsWaitingForLastMove holds an equal priority until that
    // leg's travel time runs out. Pursued is over, so nothing should wait on it, a Fury dodge least of all.
    AI_VALUE(LastMovement&, "last movement").priority = MovementPriority::MOVEMENT_NORMAL;
}

Unit* FlameLeviathanDriveAction::NearestCrate(float radius)
{
    Unit* nearest = nullptr;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (auto const& guid : npcs)
    {
        Unit* crate = botAI->GetUnit(guid);
        if (!crate || crate->GetEntry() != NPC_FL_PYRITE_CONTAINER)
            continue;

        if (vehicleBase_->GetExactDist2d(crate) > radius)
            continue;

        if (!nearest || vehicleBase_->GetExactDist2d(crate) < vehicleBase_->GetExactDist2d(nearest))
            nearest = crate;
    }

    return nearest;
}

bool FlameLeviathanDriveAction::DetourToCrate(Unit* boss)
{
    Unit* crate = NearestCrate(ULDUAR_FL_CRATE_DETOUR_RADIUS);
    if (!crate)
        return false;

    // Already inside the gunner's grab.
    float const grab = ULDUAR_FL_CRATE_GRAB_RANGE - ULDUAR_FL_ARRIVE_TOLERANCE;
    float const dist = vehicleBase_->GetExactDist2d(crate);
    if (dist <= grab)
        return false;

    // Stop at grab range on the hull's side, and only while the barrel stays in range: time past 70 yd costs
    // the stacks the crate is meant to keep up.
    float const angle = crate->GetAngle(vehicleBase_);
    Position const goal(crate->GetPositionX() + std::cos(angle) * grab, crate->GetPositionY() + std::sin(angle) * grab,
                        crate->GetPositionZ());

    float const leash = boss->GetCombatReach() + FlameLeviathanDemolisherStandDist(boss) + ULDUAR_FL_ARRIVE_TOLERANCE;
    if (boss->GetExactDist2d(goal) > leash)
        return false;

    DriveTo(goal, boss, false);
    return true;
}

bool FlameLeviathanDriveAction::ClearHazard(Unit* hazard, Unit* keepInReachOf)
{
    // Radial is the first thing tried, for all three reticles. A Hodir's Fury only gets here once it
    // has stopped and stunned itself, so it is a static mark like the other two.
    float const angle = hazard->GetAngle(vehicleBase_);

    float const reach = FlameLeviathanHazardReach(hazard, vehicleBase_);
    float const step = reach + 2.0f * ULDUAR_FL_ARRIVE_TOLERANCE - vehicleBase_->GetExactDist2d(hazard);

    // Guard, not a normal path: the scan margin is narrower than this clearance, so anything close
    // enough to be handed here still has ground to make up. Never step backwards if that changes.
    if (step <= 0.0f)
    {
        dodgeGoal_.reset();
        return false;
    }

    auto const pointFor = [this](float bearing, float reach)
    {
        return Position(vehicleBase_->GetPositionX() + std::cos(bearing) * reach,
                        vehicleBase_->GetPositionY() + std::sin(bearing) * reach,
                        vehicleBase_->GetPositionZ());
    };

    // The Inferno is a trail of ~15 patches, and straight out from the nearest often lands in the next one.
    // Fan off the radial until a point clears all of them.
    std::vector<Unit*> hazards;
    GetFlameLeviathanTowerHazards(botAI, vehicleBase_, FlameLeviathanActiveTowerMask(botAI),
                                  ULDUAR_FL_TOWER_HAZARD_CLEAR_SCAN, hazards);

    // Clear to stop on, with a straight leg there that enters no new reach. Past the kite ring's bounds is a
    // wall: the spline stops short and the hull stays in the fire.
    Position const here = vehicleBase_->GetPosition();
    auto const usable = [&](Position const& goal)
    {
        return FlameLeviathanInArena(goal) &&
               LegClearOfHazards(vehicleBase_, goal, goal, hazards, ULDUAR_FL_ARRIVE_TOLERANCE) &&
               LegClearOfHazards(vehicleBase_, here, goal, hazards, 0.0f);
    };

    // Latched while usable: re-planned off the nearest patch every tick, a hull inside several flips sides.
    if (dodgeGoal_ && !usable(*dodgeGoal_))
        dodgeGoal_.reset();

    if (!dodgeGoal_)
    {
        // Nearest clear point that is not also in range, kept only in case nothing in range clears.
        std::optional<Position> compromise;

        for (float travel : {step, step + reach, step + 2.0f * reach})
        {
            for (float offset : HAZARD_FAN)
            {
                Position const goal = pointFor(angle + offset, travel);
                if (!usable(goal))
                    continue;

                // The reserve owes a firing position: a dodge out of cone range drops its vent duty.
                if (keepInReachOf &&
                    goal.GetExactDist2d(keepInReachOf) - keepInReachOf->GetObjectSize() >
                        ULDUAR_FL_ELECTROSHOCK_CONE_RADIUS)
                {
                    if (!compromise)
                        compromise = goal;
                    continue;
                }

                dodgeGoal_ = goal;
                break;
            }

            if (dodgeGoal_)
                break;
        }

        if (!dodgeGoal_)
            dodgeGoal_ = compromise;
    }

    // Boxed in by the trail. Straight out from the nearest patch still beats standing in it, and is
    // not kept: the next tick may find a way out.
    Position const goal = dodgeGoal_ ? *dodgeGoal_ : pointFor(angle, step);
    if (!DriveTo(goal, keepInReachOf, false, MovementPriority::MOVEMENT_FORCED))
        dodgeGoal_.reset();

    return true;
}

bool FlameLeviathanDriveAction::ClearBatteringRam(Unit* boss)
{
    if (!FlameLeviathanShouldClearBatteringRam(botAI, bot))
        return false;

    // Away from the rammed hull, where the blast is centred, not from him. Guns stay on him while backing out.
    Unit* centre = FlameLeviathanRamCentre(botAI, bot);
    if (!centre)
        return false;

    float const safeDist = ULDUAR_FL_BATTERING_RAM_RADIUS + vehicleBase_->GetObjectSize();

    // Twice the arrival deadband of overshoot, because DriveTo parks anywhere within one of it and a
    // single deadband of margin lets the vehicle stop back on the edge of the blast.
    float const angle = centre->GetAngle(vehicleBase_);
    float const step = safeDist - vehicleBase_->GetExactDist2d(centre) + 2.0f * ULDUAR_FL_ARRIVE_TOLERANCE;
    Position const goal(vehicleBase_->GetPositionX() + std::cos(angle) * step,
                        vehicleBase_->GetPositionY() + std::sin(angle) * step,
                        vehicleBase_->GetPositionZ());

    DriveTo(goal, boss, false, MovementPriority::MOVEMENT_FORCED);
    return true;
}

bool FlameLeviathanDriveAction::HoldStation(Unit* boss)
{
    float standDist = ULDUAR_FL_SIEGE_STAND_DIST;
    float stationLead = 0.0f;
    bool centreSide = false;
    switch (vehicleBase_->GetEntry())
    {
        case NPC_VEHICLE_CHOPPER:
            // The lead chopper runs ahead of him instead, back turned, so its tar pool lands in his path.
            if (FlameLeviathanIsTarLead(botAI, bot))
            {
                float const lead = FlameLeviathanTarLeadDistance(boss, FlameLeviathanPursuedVehicle(botAI, bot),
                                                                 vehicleBase_->GetObjectSize());
                // Zero: too tight to lead without parking in the blast, so hold the normal chopper station.
                if (lead > 0.0f)
                    return DriveTo(FlameLeviathanLeadPoint(boss, lead), boss, true);
            }
            standDist = ULDUAR_FL_CHOPPER_STAND_DIST;
            break;
        case NPC_SALVAGED_DEMOLISHER:
            // Clamped: stand distance counts from his 15 yd reach, so the 50 band plus the deadband would sit
            // past the barrel's 70. Never closing also keeps them off Battering Ram.
            standDist = FlameLeviathanDemolisherStandDist(boss);
            // Led, since the band is only worth holding if the hull gets there before he drives off.
            stationLead = FlameLeviathanStationLead(boss);
            // Swung toward the arena centre: a hull that falls behind him catches up on the inside of
            // the circle he runs, not round the outside of it.
            centreSide = true;
            break;
        default:
            break;
    }

    bool const ventReserve = FlameLeviathanIsVentReserve(bot);
    if (ventReserve)
    {
        // He speeds up to 10 yd/s against a flat 7 hull, so the rear arc with no lead is a tail chase.
        stationLead = FlameLeviathanStationLead(boss);
        centreSide = true;

        // A siege engine caught in Battering Ram's reach when Pursued lands can't drive clear, so wait outside
        // it, deadband included, and close in only for the channel.
        if (FlameLeviathanMsToNextVent(bot, boss) > ULDUAR_FL_VENT_APPROACH_LEAD_MS)
            standDist = ULDUAR_FL_BATTERING_RAM_CAST_RANGE + vehicleBase_->GetCombatReach() +
                        ULDUAR_FL_ARRIVE_TOLERANCE;
    }

    if (ventReserve && RushToVents(boss))
        return true;

    float fanHalf = 0.0f;
    float offset = FlameLeviathanStationBearingOffset(bot, vehicleBase_, boss->GetCombatReach() + standDist, &fanHalf);
    if (centreSide)
        offset += FlameLeviathanCentreTurn(boss, fanHalf);

    Position const station = FlameLeviathanRearPoint(boss, standDist, offset, stationLead);

    // A straight leg into the Inferno trail just gets dodged straight back, so go round it.
    Position goal = station;
    if (FlameLeviathanActiveTowerMask(botAI) & FL_TOWER_FLAMES)
        if (std::optional<Position> detour = DetourAroundFire(boss, station, true))
            goal = *detour;

    // Face what the cast node will shoot, or the park facing and the cone weapons undo each other. Same bands,
    // tar lead included; the reserve stays on him, since turning back would cost a tick each channel.
    float coneRadius = 0.0f;
    if (!ventReserve)
    {
        switch (vehicleBase_->GetEntry())
        {
            case NPC_SALVAGED_SIEGE_ENGINE:
            case NPC_SALVAGED_DEMOLISHER:
                coneRadius = ULDUAR_FL_RAM_CONE_RADIUS;
                break;
            case NPC_VEHICLE_CHOPPER:
                if (!FlameLeviathanIsTarLead(botAI, bot))
                    coneRadius = ULDUAR_FL_SONIC_HORN_CONE_RADIUS;
                break;
            default:
                break;
        }
    }

    if (coneRadius > 0.0f)
        if (Unit* add = FlameLeviathanBestAdd(botAI, vehicleBase_, 0.0f, coneRadius, 0.0f))
            return DriveTo(goal, add, false);

    return DriveTo(goal, boss, false);
}

bool FlameLeviathanDriveAction::RushToVents(Unit* boss)
{
    // Electroshock shares Steam Rush's 2 s GCD, so dash 2-5 s ahead of a channel, or at once into one
    // already running. Never outside that: the rush is also the reserve's only escape once Pursued.
    uint32 const toVent = FlameLeviathanMsToNextVent(bot, boss);
    if (toVent && (toVent < ULDUAR_FL_STEAM_RUSH_GCD_MS || toVent > ULDUAR_FL_VENT_RUSH_LEAD_MS))
        return false;

    // A FORCED dodge leg cancels the charge and wastes the 40 energy. Same test as the drive's, so it matches
    // the dodge this reserve will actually take.
    uint32 const towerMask = FlameLeviathanActiveTowerMask(botAI);
    if (FlameLeviathanShouldClearBatteringRam(botAI, bot) ||
        (towerMask && GetFlameLeviathanNearestTowerHazard(botAI, vehicleBase_, towerMask,
                                                          ULDUAR_FL_TOWER_HAZARD_MARGIN, FL_TOWER_STORM)))
        return false;

    // Measured from his edge, like the cone. Inside a full charge the dash would carry the hull through
    // him, and driving closes the rest.
    if (vehicleBase_->GetExactDist2d(boss) - boss->GetObjectSize() <= ULDUAR_FL_STEAM_RUSH_DIST)
        return false;

    if (vehicleBase_->HasSpellCooldown(SPELL_FL_STEAM_RUSH) ||
        vehicleBase_->GetPower(POWER_ENERGY) < ULDUAR_FL_STEAM_RUSH_COST + ULDUAR_FL_ELECTROSHOCK_COST)
        return false;

    SpellInfo const* rushInfo = sSpellMgr->GetSpellInfo(SPELL_FL_STEAM_RUSH);
    CharmInfo* charm = vehicleBase_->GetCharmInfo();
    if (!rushInfo || (charm && charm->GetGlobalCooldownMgr().HasGlobalCooldown(rushInfo)))
        return false;

    // The charge runs along our facing, and must not end inside the blast round whoever he is chasing.
    float const bearing = vehicleBase_->GetAngle(boss);
    Position const landing(vehicleBase_->GetPositionX() + std::cos(bearing) * ULDUAR_FL_STEAM_RUSH_DIST,
                           vehicleBase_->GetPositionY() + std::sin(bearing) * ULDUAR_FL_STEAM_RUSH_DIST,
                           vehicleBase_->GetPositionZ());
    if (Unit* centre = FlameLeviathanRamCentre(botAI, bot))
        if (centre->GetExactDist2d(landing) <= ULDUAR_FL_BATTERING_RAM_RADIUS + vehicleBase_->GetObjectSize())
            return false;

    // Nor through the Inferno trail.
    if (towerMask & FL_TOWER_FLAMES)
    {
        std::vector<Unit*> fires;
        GetFlameLeviathanTowerHazards(botAI, vehicleBase_, FL_TOWER_FLAMES, ULDUAR_FL_TOWER_HAZARD_CLEAR_SCAN, fires);
        if (!LegClearOfHazards(vehicleBase_, vehicleBase_->GetPosition(), landing, fires, 0.0f))
            return false;
    }

    // Stopped first: a moving hull's spline overrides the facing.
    if (!vehicleBase_->HasInArc(ULDUAR_FL_STEAM_RUSH_AIM_ARC, boss))
    {
        vehicleBase_->StopMoving();
        vehicleBase_->SetFacingToObject(boss);
        return true;
    }

    return CastVehicleSelfSpell(botAI, vehicleBase_, SPELL_FL_STEAM_RUSH, ULDUAR_FL_STEAM_RUSH_COST,
                                ULDUAR_FL_STEAM_RUSH_COOLDOWN_MS);
}

std::optional<Position> FlameLeviathanDriveAction::DetourAroundFire(Unit* boss, Position const& goal, bool mayClose)
{
    std::vector<Unit*> fires;
    GetFlameLeviathanTowerHazards(botAI, vehicleBase_, FL_TOWER_FLAMES, ULDUAR_FL_TOWER_HAZARD_CLEAR_SCAN, fires);
    if (fires.empty())
        return std::nullopt;

    Position const here = vehicleBase_->GetPosition();
    float const bearing = vehicleBase_->GetAngle(&goal);
    auto const along = [&here](float angle, float dist)
    {
        return Position(here.GetPositionX() + std::cos(angle) * dist, here.GetPositionY() + std::sin(angle) * dist,
                        here.GetPositionZ());
    };

    // Only the next stretch: the leg is re-planned every tick, and fire further along may be gone or
    // passed by then.
    float const lookahead = std::min(vehicleBase_->GetExactDist2d(goal), ULDUAR_FL_KITE_FIRE_LOOKAHEAD);
    if (LegClearOfHazards(vehicleBase_, here, along(bearing, lookahead), fires, ULDUAR_FL_TOWER_HAZARD_MARGIN))
        return std::nullopt;

    float const bossDist = vehicleBase_->GetExactDist2d(boss);
    for (float offset : HAZARD_FAN)
    {
        Position const detour = along(bearing + offset, ULDUAR_FL_KITE_DETOUR_STEP);
        if (!FlameLeviathanInArena(detour) || (!mayClose && boss->GetExactDist2d(detour) < bossDist))
            continue;

        if (LegClearOfHazards(vehicleBase_, here, detour, fires, ULDUAR_FL_TOWER_HAZARD_MARGIN))
            return detour;
    }

    return std::nullopt;
}

bool FlameLeviathanDriveAction::Kite(Unit* boss)
{
    std::vector<Position> const& ring = FlameLeviathanKiteRing();
    int32 const count = static_cast<int32>(ring.size());
    if (count < 2)
        return false;

    auto blocked = [boss](Position const& node)
    { return node.GetExactDist2d(boss->GetPositionX(), boss->GetPositionY()) < ULDUAR_FL_KITE_BOSS_CLEARANCE; };

    auto nearestNode = [this, &ring, count]
    {
        int32 best = 0;
        for (int32 i = 1; i < count; ++i)
            if (vehicleBase_->GetExactDist2d(ring[i]) < vehicleBase_->GetExactDist2d(ring[best]))
                best = i;
        return best;
    };

    if (kiteIdx_ < 0)
    {
        // Direction picked per kite, away from him, then held: turning round mid-kite runs back into him.
        int32 const here = nearestNode();
        float const ahead = ring[(here + 1) % count].GetExactDist2d(boss->GetPositionX(), boss->GetPositionY());
        float const behind =
            ring[(here - 1 + count) % count].GetExactDist2d(boss->GetPositionX(), boss->GetPositionY());
        kiteDir_ = ahead >= behind ? 1 : -1;

        kiteIdx_ = here;
        for (int32 i = 0; i < count && blocked(ring[kiteIdx_]); ++i)
            kiteIdx_ = (kiteIdx_ + kiteDir_ + count) % count;
    }
    else if (vehicleBase_->GetExactDist2d(ring[kiteIdx_]) <= ULDUAR_FL_KITE_ADVANCE_DIST)
    {
        // Advance on approach, never on arrival: waiting until the vehicle reaches the node drives
        // it into the node, and in a corner that is exactly where the boss cuts the diagonal.
        kiteIdx_ = (kiteIdx_ + kiteDir_ + count) % count;
        if (blocked(ring[kiteIdx_]))
            kiteIdx_ = (kiteIdx_ + kiteDir_ + count) % count;
    }

    // The kite outranks the hazard dodge, so it steers round the Inferno itself. Not once he can Ram, though:
    // blasts stack +50% damage taken, so a slowed leg costs far more than the fire.
    bool const ramming = vehicleBase_->HasAura(SPELL_FL_BATTERING_RAM) ||
                         boss->IsWithinCombatRange(vehicleBase_, ULDUAR_FL_BATTERING_RAM_CAST_RANGE);

    Position goal = ring[kiteIdx_];
    if (!ramming && (FlameLeviathanActiveTowerMask(botAI) & FL_TOWER_FLAMES))
        if (std::optional<Position> detour = DetourAroundFire(boss, goal, false))
            goal = *detour;

    // FORCED: IsWaitingForLastMove yields only to a strictly higher priority, and the station leg is COMBAT.
    DriveTo(goal, nullptr, false, MovementPriority::MOVEMENT_FORCED);

    // Escape buttons, spent only once already running away from him.
    if (!vehicleBase_->HasInArc(M_PI / 2.0f, boss))
    {
        switch (vehicleBase_->GetEntry())
        {
            case NPC_SALVAGED_SIEGE_ENGINE:
                // Steam Rush charges along our own facing, so with him in front it would dash into him.
                CastVehicleSelfSpell(botAI, vehicleBase_, SPELL_FL_STEAM_RUSH, ULDUAR_FL_STEAM_RUSH_COST,
                                     ULDUAR_FL_STEAM_RUSH_COOLDOWN_MS);
                break;
            case NPC_VEHICLE_CHOPPER:
                if (!vehicleBase_->HasAura(SPELL_FL_SPEED_BOOST))
                    CastVehicleSelfSpell(botAI, vehicleBase_, SPELL_FL_SPEED_BOOST, ULDUAR_FL_SPEED_BOOST_COST,
                                         ULDUAR_FL_SPEED_RECAST_MS);
                break;
            default:
                break;
        }
    }

    return true;
}

bool FlameLeviathanDriveAction::DriveTo(Position const& goal, Unit* faceTarget, bool faceAway,
                                        MovementPriority priority)
{
    std::optional<float> facing;
    if (faceTarget)
        facing = faceAway ? faceTarget->GetAngle(vehicleBase_) : vehicleBase_->GetAngle(faceTarget);

    return DriveToImpl(goal, facing, priority);
}

bool FlameLeviathanDriveAction::DriveTo(Position const& goal, Position const& facePoint, MovementPriority priority)
{
    return DriveToImpl(goal, vehicleBase_->GetAngle(facePoint.GetPositionX(), facePoint.GetPositionY()),
                       priority);
}

bool FlameLeviathanDriveAction::DriveToImpl(Position const& goal, std::optional<float> facing,
                                            MovementPriority priority)
{
    if (vehicleBase_->GetExactDist2d(goal) <= ULDUAR_FL_ARRIVE_TOLERANCE)
    {
        if (!parked_)
        {
            vehicleBase_->StopMoving();
            parked_ = true;
        }

        // Park facing the target so casts go out without a turn. Only fix real drift: SetFacingTo starts a
        // spline, and one every tick jitters the hull and reads as moving.
        if (facing)
        {
            // NormalizeOrientation lands in [0, 2PI), so fold the far half back into [0, PI].
            float error = Position::NormalizeOrientation(*facing - vehicleBase_->GetOrientation());
            if (error > float(M_PI))
                error = 2.0f * float(M_PI) - error;

            if (error > ULDUAR_FL_FACING_TOLERANCE)
                vehicleBase_->SetFacingTo(*facing);
        }

        // Yield once parked, so the rotation runs and a move that silently failed shows up as a
        // vehicle standing still rather than as this action quietly owning every tick.
        return false;
    }

    // Re-stamping the same destination restarts the spline, so the vehicle would crawl and never
    // arrive. Only re-issue on real drift, or when the previous move has stopped.
    if (hasIssued_ && !parked_ && issued_.GetExactDist2d(goal) < ULDUAR_FL_REPOSITION_EPSILON &&
        vehicleBase_->isMoving())
        return true;

    // This action is the vehicle's only mover, so a leg in flight at this priority is our own stale one,
    // and IsWaitingForLastMove would hold the new goal for up to MaxWaitForMove. A higher leg still wins.
    LastMovement& lastMove = AI_VALUE(LastMovement&, "last movement");
    MovementPriority const held = lastMove.priority;
    if (held == priority && priority != MovementPriority::MOVEMENT_IDLE)
        lastMove.priority = static_cast<MovementPriority>(static_cast<int>(priority) - 1);

    if (!MoveTo(vehicleBase_->GetMapId(), goal.GetPositionX(), goal.GetPositionY(), goal.GetPositionZ(), false, false,
                false, false, priority))
    {
        lastMove.priority = held;
        return false;
    }

    issued_ = goal;
    hasIssued_ = true;
    parked_ = false;
    return true;
}

bool FlameLeviathanEnterVehicleAction::Execute(Event /*event*/)
{
    // do not switch vehicles yet
    if (bot->GetVehicle())
        return false;
    Unit* vehicleToEnter = nullptr;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles far");
    for (ObjectGuid const& guid : npcs)
    {
        Unit* vehicleBase = botAI->GetUnit(guid);
        if (!vehicleBase)
            continue;

        if (vehicleBase->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            continue;

        if (!ShouldEnter(vehicleBase))
            continue;

        if (!vehicleToEnter || bot->GetExactDist(vehicleToEnter) > bot->GetExactDist(vehicleBase))
            vehicleToEnter = vehicleBase;
    }

    if (!vehicleToEnter)
        return false;

    if (EnterVehicle(vehicleToEnter, true))
        return true;

    return false;
}

bool FlameLeviathanEnterVehicleAction::EnterVehicle(Unit* vehicleBase, bool moveIfFar)
{
    float dist = bot->GetDistance(vehicleBase);

    if (dist > INTERACTION_DISTANCE && !moveIfFar)
        return false;

    if (dist > INTERACTION_DISTANCE)
        return MoveTo(vehicleBase);

    botAI->RemoveShapeshift();
    // Use HandleSpellClick instead of Unit::EnterVehicle to handle special vehicle script (ulduar)
    vehicleBase->HandleSpellClick(bot);

    if (!bot->IsOnVehicle(vehicleBase))
        return false;

    // dismount because bots can enter vehicle on mount
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    return true;
}

bool FlameLeviathanEnterVehicleAction::ShouldEnter(Unit* target)
{
    Vehicle* vehicleKit = target->GetVehicleKit();
    if (!vehicleKit)
        return false;

    bool isMelee = PlayerbotAI::IsMelee(bot);
    bool allMain = AllMainVehiclesOnUse();
    bool inUse = vehicleKit->IsVehicleInUse();
    int32 entry = target->GetEntry();
    if (entry != NPC_SALVAGED_DEMOLISHER && entry != NPC_SALVAGED_SIEGE_ENGINE && entry != NPC_VEHICLE_CHOPPER)
        return false;
    // two phase enter (make all main vehicles in use -> next player enter)
    if (!allMain)
    {
        if (inUse)
            return false;
        if (entry != NPC_SALVAGED_DEMOLISHER && entry != NPC_SALVAGED_SIEGE_ENGINE)
            return false;
        if (entry == NPC_SALVAGED_DEMOLISHER && isMelee)
            return false;
        if (entry == NPC_SALVAGED_SIEGE_ENGINE && !isMelee)
            return false;
        return true;
    }

    if (!vehicleKit->GetAvailableSeatCount())
        return false;

    // do not enter useless seat
    if (entry == NPC_SALVAGED_SIEGE_ENGINE)
    {
        Unit* turret = vehicleKit->GetPassenger(7);
        if (!turret)
            return false;
        Vehicle* turretVehicle = turret->GetVehicleKit();
        if (!turretVehicle)
            return false;
        if (turretVehicle->IsVehicleInUse())
            return false;
        return true;
    }

    if (entry == NPC_SALVAGED_DEMOLISHER)
    {
        if (vehicleKit->GetPassenger(0))
        {
            Unit* target2 = vehicleKit->GetPassenger(1);
            if (!target2)
                return false;
            Vehicle* vehicle2 = target2->GetVehicleKit();
            if (!vehicle2)
                return false;
            if (vehicle2->GetPassenger(0))
                return false;
        }
        return true;
    }

    if (entry == NPC_VEHICLE_CHOPPER && vehicleKit->GetAvailableSeatCount() <= 1)
        return false;

    return true;
}

bool FlameLeviathanEnterVehicleAction::AllMainVehiclesOnUse()
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;
    int demolisher = 0;
    int siege = 0;
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player)
            continue;
        Unit* vehicleBase = player->GetVehicleBase();
        if (!vehicleBase)
            continue;
        if (vehicleBase->GetEntry() == NPC_SALVAGED_DEMOLISHER)
            ++demolisher;
        else if (vehicleBase->GetEntry() == NPC_SALVAGED_SIEGE_ENGINE)
            ++siege;
    }
    int maxC = IsRaid25Man(bot) ? 5 : 2;
    return demolisher >= maxC && siege >= maxC;
}
