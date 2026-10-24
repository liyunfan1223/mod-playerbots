/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_FLAMELEVIATHAN_H
#define PLAYERBOTS_ULDACTIONS_FLAMELEVIATHAN_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Position.h"
#include "UldTriggers.h"
#include "Vehicle.h"

#include <optional>
#include <vector>

// Flame Leviathan

// Every vehicle spell on this fight is instant, so casting never competes with driving. This action
// owns the casts only; all movement belongs to FlameLeviathanDriveAction.
class FlameLeviathanVehicleAction : public Action
{
public:
    static constexpr char const* Name = "flame leviathan vehicle";

    FlameLeviathanVehicleAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;

protected:
    bool DemolisherAction(Unit* target);
    bool DemolisherTurretAction(Unit* target);
    bool SiegeEngineAction(Unit* target);
    bool SiegeEngineTurretAction(Unit* target);
    bool ChopperAction(Unit* target);

    // Reads our own Blue Pyrite on the target and matches landings to the barrels in flight.
    Aura* TrackBarrels(Unit* target);

    // Casts and records the cooldown the core will not apply itself: Spell::SendSpellCooldown
    // returns early for creature casters, so without this the action re-fires every tick.
    bool CastVehicle(uint32 spellId, Unit* target, uint32 cooldownMs = 1000);

    Unit* FindMechanolift();

    // The nearest hostile already in combat, him and his turrets aside: what to shoot on the approach.
    Unit* NearestFightingHostile();

private:
    Unit* _vehicleBase = nullptr;

    // Cast times of barrels not yet seen landing, and the last flight timed.
    std::vector<uint32> _barrelsInFlight;
    uint32 _barrelLeadMs = 0;  // ULDUAR_FL_PYRITE_FLIGHT_MS until a landing has been timed
    ObjectGuid _barrelTarget;
    uint8 _lastPyriteStacks = 0;
    int32 _lastPyriteDurationMs = 0;
};

class FlameLeviathanEnterVehicleAction : public MovementAction
{
public:
    static constexpr char const* Name = "flame leviathan enter vehicle";

    FlameLeviathanEnterVehicleAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;

protected:
    bool EnterVehicle(Unit* vehicleBase, bool moveIfFar);
    bool ShouldEnter(Unit* vehicleBase);
    bool AllMainVehiclesOnUse();
};

// Electroshock. Flame Vents is a 10s channel and the boss script maps Electroshock to
// InterruptNonMeleeSpells, so this genuinely stops it. One siege engine fires per channel.
class FlameLeviathanInterruptVentsAction : public Action
{
public:
    static constexpr char const* Name = "flame leviathan interrupt vents";

    FlameLeviathanInterruptVentsAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

// The only thing that steers a vehicle here: two actions on one MotionMaster bounce rather than compromise,
// so every kind of movement is a branch of this one.
class FlameLeviathanDriveAction : public MovementAction
{
public:
    static constexpr char const* Name = "flame leviathan drive";

    FlameLeviathanDriveAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;

protected:
    bool Kite(Unit* boss);
    // A step round the Inferno trail when the straight leg to `goal` crosses it. A kite passes
    // `mayClose` false, since a detour toward him trades a burn for Battering Ram.
    std::optional<Position> DetourAroundFire(Unit* boss, Position const& goal, bool mayClose);
    // `keepInReachOf` ranks exits by whether the hull can still shoot him, not by nearest, and parks facing
    // him. Only the vent reserve passes it.
    bool ClearHazard(Unit* hazard, Unit* keepInReachOf = nullptr);
    bool ClearBatteringRam(Unit* boss);
    bool DetourToCrate(Unit* boss);
    bool HoldStation(Unit* boss);

    // The vent reserve's Steam Rush into Electroshock reach. True while it owns the tick, turning or
    // dashing.
    bool RushToVents(Unit* boss);

    // Reach-then-hold. Returns true while genuinely travelling and false once parked and facing, so
    // lower-priority nodes still run and a silently failing move shows up as a stationary vehicle.
    bool DriveTo(Position const& goal, Unit* faceTarget, bool faceAway,
                 MovementPriority priority = MovementPriority::MOVEMENT_COMBAT);

    // Same, facing a fixed point. A posted siege engine faces its corner, which is a place and not
    // a unit, and there is nothing standing there to aim at.
    bool DriveTo(Position const& goal, Position const& facePoint,
                 MovementPriority priority = MovementPriority::MOVEMENT_COMBAT);

    // The orientation, if any, is applied only once parked. Empty means "hold what you are on".
    bool DriveToImpl(Position const& goal, std::optional<float> facing, MovementPriority priority);

    Unit* NearestCrate(float radius);
    void ResetKite();

private:
    Unit* _vehicleBase = nullptr;

    Position _issued;
    bool _hasIssued = false;
    bool _parked = false;

    int32 _kiteIdx = -1;
    int8 _kiteDir = 0;  // picked at the start of each kite, then held

    // Held until it stops being clear or the hull parks on it, and dropped once no hazard owns the tick.
    std::optional<Position> _dodgeGoal;
};

#endif
