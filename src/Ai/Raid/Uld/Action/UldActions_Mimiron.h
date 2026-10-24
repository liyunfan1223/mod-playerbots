/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_MIMIRON_H
#define PLAYERBOTS_ULDACTIONS_MIMIRON_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "RaidRedirectThreat.h"
#include "Playerbots.h"
#include "UldEncounter_Mimiron.h"
#include "UldTriggers.h"
#include "Vehicle.h"

class MimironResetEncounterStateAction : public Action
{
public:
    static constexpr char const* Name = "mimiron reset encounter state action";

    MimironResetEncounterStateAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// Shared by the Mimiron flee actions. Proximity Mines are non-selectable, so pathing knows nothing
// about them and a bot stepping out of one hazard can land in the ten that follow a Shock Blast.
class MimironFleeAction : public MovementAction
{
public:
    MimironFleeAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    // fallbackUnfiltered: plain MoveAway once every bearing is refused. interrupt: stops a cast, for lethal hazards.
    // clearRadius: hop solved per bearing onto that circle. sectorScreen: off the other group's Rapid Burst line.
    bool MoveAwayClearOfMines(Unit* from, float distance,
                              MovementPriority priority = MovementPriority::MOVEMENT_COMBAT,
                              bool fallbackUnfiltered = true, bool interrupt = false, float clearRadius = 0.0f,
                              bool sectorScreen = false, bool allowFire = false);

    // Same fan, fleeing a point: the fire has no single unit to flee, so the flames dodge passes its centroid.
    bool MoveAwayClearOfMines(Position const& from, float distance,
                              MovementPriority priority = MovementPriority::MOVEMENT_COMBAT,
                              bool fallbackUnfiltered = true, bool interrupt = false, float clearRadius = 0.0f);

    // Same fan aimed at `dest`, by fleeing the point mirrored through the bot. allowFire drops the fire screen,
    // only for escapes from something deadlier than a ~3.1k/s node, like a Water Spray line.
    bool MoveTowardClearOfMines(Position const& dest,
                                MovementPriority priority = MovementPriority::MOVEMENT_COMBAT,
                                bool fallbackUnfiltered = true, bool interrupt = false, bool allowFire = false);

private:
    // fallbackFrom is only for the unfiltered MoveAway once every bearing is refused; without one, the
    // fallback walks straight away from `from`.
    bool FleeFan(Position const& from, Unit* fallbackFrom, float distance, MovementPriority priority,
                 bool fallbackUnfiltered, bool interrupt, float clearRadius = 0.0f, bool allowFire = false,
                 bool sectorScreen = false);
};

class MimironShockBlastAction : public MimironFleeAction
{
public:
    static constexpr char const* Name = "mimiron shock blast action";

    MimironShockBlastAction(PlayerbotAI* botAI) : MimironFleeAction(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    bool FleeShockToAnchor();
};

class MimironPhase1PositioningAction : public MovementAction
{
public:
    static constexpr char const* Name = "mimiron phase 1 positioning action";

    MimironPhase1PositioningAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironP3Wx2LaserBarrageAction : public MovementAction
{
public:
    static constexpr char const* Name = "mimiron p3wx2 laser barrage action";

    MimironP3Wx2LaserBarrageAction(PlayerbotAI* botAI)
        : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Where a bot stands when nothing is trying to kill it this second.
class MimironArcSpreadAction : public MovementAction
{
public:
    static constexpr char const* Name = "mimiron arc spread action";

    MimironArcSpreadAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironAerialCommandUnitAction : public Action
{
public:
    static constexpr char const* Name = "mimiron aerial command unit action";

    MimironAerialCommandUnitAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

class MimironRocketStrikeAction : public MimironFleeAction
{
public:
    static constexpr char const* Name = "mimiron rocket strike action";

    MimironRocketStrikeAction(PlayerbotAI* botAI) : MimironFleeAction(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironPhase4FocusAction : public AttackAction
{
public:
    static constexpr char const* Name = "mimiron phase 4 focus action";

    MimironPhase4FocusAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// One designated bot carries the Magnetic Core to the Aerial Command Unit and grounds it. Without
// this the phase only ends when ranged whittle the ACU down from the floor.
class MimironMagneticCoreAction : public MovementAction
{
public:
    static constexpr char const* Name = "mimiron magnetic core action";

    MimironMagneticCoreAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:

    // Set on every spend, cleared once the bags are empty with no core down. The count alone can't tell:
    // one core left after spending the first of a pair reads as a bank of one.
    bool chaining = false;
};

// Cast directly, not through ChangeStrategy: nothing removes an added "rfrost", so it would outlive phase 3.
// The mimiron paladin aura rule holds the slot open meanwhile.
class MimironFrostResistanceAction : public Action
{
public:
    static constexpr char const* Name = "mimiron frost resistance action";

    MimironFrostResistanceAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

// One big defensive per Plasma Blast window, spent at cast start: a health threshold only notices by the third
// tick. Tank's own first since it costs the raid nothing, a healer external only when he has none, never both.
class MimironPlasmaBlastDefensiveAction : public Action
{
public:
    static constexpr char const* Name = "mimiron plasma blast defensive action";

    MimironPlasmaBlastDefensiveAction(PlayerbotAI* botAI)
        : Action(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    // Whichever of these the healer has, in the order they are worth spending. Hand of Protection is
    // deliberately absent: it sheds threat and would hand the boss straight back to the raid.
    static constexpr char const* HEALER_EXTERNALS[] = {"pain suppression", "guardian spirit",
                                                       "hand of sacrifice"};

    Unit* PlasmaVictim();
    bool TankHasDefensive(Unit* victim);
};

// Phase 1 only: the MK II must stay on the main tank through the walk west, and the generic Tricks picker
// aims at the top melee instead.
class MimironRedirectThreatAction : public RaidRedirectThreatAction
{
public:
    static constexpr char const* Name = "mimiron redirect threat action";

    MimironRedirectThreatAction(PlayerbotAI* botAI)
        : RaidRedirectThreatAction(botAI, Name) {}

protected:
    Player* GetRedirectTank() override;
    Unit* GetThreatDumpTarget() override;
};

// Owns "current target" for every non-tank while any mech is up. Raid icons stay cosmetic: they're
// group-global, so a bot whose "rti" string differs from the marker's reads back the wrong one.
class MimironSetDpsPriorityAction : public AttackAction
{
public:
    static constexpr char const* Name = "mimiron set dps priority action";

    MimironSetDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, Name) {}

    bool Execute(Event event) override;

private:
    // Ordered candidates for this bot's role, most urgent first. Entries the role must not touch are
    // left out entirely rather than filtered later.
    std::vector<std::pair<uint32, Unit*>> BuildPriorityList();

    // Nearest live candidate of `entry`, preferring the current target so two identical adds cannot
    // make the bot alternate between them every tick.
    Unit* SelectByEntry(Unit* currentTarget, uint32 entry, std::vector<Unit*> const& candidates) const;

    bool IsAllowedTarget(Unit* unit) const;

    // nullptr when nothing is allowed and `allowFallback` is false; otherwise the generic picker's
    // answer stands in.
    Unit* ResolveTarget(Unit* currentTarget, bool allowFallback);
};

// Shortest step that clears every mine. The generic MoveAwayFromCreatureAction maximises distance over a
// 30 yd fan, which in a fresh field is a long run to a different answer every tick.
class MimironProximityMineAction : public MimironFleeAction
{
public:
    static constexpr char const* Name = "mimiron proximity mine action";

    MimironProximityMineAction(PlayerbotAI* botAI)
        : MimironFleeAction(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironBombBotAction : public MoveAwayFromCreatureAction
{
public:
    static constexpr char const* Name = "mimiron bomb bot action";

    MimironBombBotAction(PlayerbotAI* botAI)
        : MoveAwayFromCreatureAction(botAI, Name, NPC_BOMB_BOT,
                                     ULDUAR_MIMIRON_BOMB_BOT_RADIUS) {}
};

// Where a pet goes in phases 3 and 4: the adds while the Aerial Command Unit hovers out of reach, the
// unit itself while a Magnetic Core has it down, and whatever the melee are on in phase 4.
class MimironPetControlAction : public Action
{
public:
    static constexpr char const* Name = "mimiron pet control action";

    MimironPetControlAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

// Hard mode (Firefighter): close on the target by its clear side. Can't screen "reach melee"/"reach spell"
// instead: action names are registered once for every bot on the server.
class MimironApproachTargetAction : public MovementAction
{
public:
    static constexpr char const* Name = "mimiron approach target action";

    MimironApproachTargetAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Hard mode (Firefighter): step out of the ground fire.
class MimironDodgeFlamesAction : public MimironFleeAction
{
public:
    static constexpr char const* Name = "mimiron dodge flames action";

    MimironDodgeFlamesAction(PlayerbotAI* botAI) : MimironFleeAction(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

// Hard mode (Firefighter): clear the Frost Bomb before it goes off. Not MoveAwayFromCreatureAction: it won't
// move at all unless a candidate clears the full 30 yd, which most don't.
class MimironFrostBombAction : public MimironFleeAction
{
public:
    static constexpr char const* Name = "mimiron frost bomb action";

    MimironFrostBombAction(PlayerbotAI* botAI) : MimironFleeAction(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

// Hard mode (Firefighter): step out of an Emergency Fire Bot's Water Spray line, sideways since the
// line runs 15 yd ahead of it, or for a caster or healer in 25-man out of its silence aura.
class MimironFireBotAction : public MimironFleeAction
{
public:
    static constexpr char const* Name = "mimiron fire bot action";

    MimironFireBotAction(PlayerbotAI* botAI) : MimironFleeAction(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

// Bomb Bots are the one Mimiron add that takes a snare: immunity set -263 leaves SNARE and ROOT off, unlike
// the Assault Bot's -285.
class MimironSlowBombBotAction : public Action
{
public:
    static constexpr char const* Name = "mimiron slow bomb bot action";

    MimironSlowBombBotAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
