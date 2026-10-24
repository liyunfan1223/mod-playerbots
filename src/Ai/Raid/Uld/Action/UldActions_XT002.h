/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_XT002_H
#define PLAYERBOTS_ULDACTIONS_XT002_H

#include <list>
#include <string>
#include <utility>
#include <vector>

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "UldEncounter_XT002.h"

// XT-002 Deconstructor

// Shared 8-direction ring search, the same shape the generic MoveAwayFrom* actions use.
class XT002MoveClearAction : public MovementAction
{
public:
    XT002MoveClearAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

    bool isPossible() override;

protected:
    // What offering a destination to the pathfinder achieved.
    enum class MoveIssue
    {
        Taken,    // the command reached the MotionMaster, or the bot is already on its way there
        NoPath,   // this destination is refused; a different one may not be
        Refused   // the bot is not moving this tick whatever it is offered
    };

    // One destination offered to the pathfinder. Plain MoveTo also returns false for "already walking there",
    // and reading that as failure turns a carrier around mid-run.
    MoveIssue IssueMove(float x, float y, float z);

    // Steps to the nearest spot giving each unit in `avoid` its own clearance, else the best partial gain, trying
    // ranked candidates since this room refuses some walkable points. False if standing still is best.
    bool MoveClearOf(std::vector<std::pair<Unit*, float>> const& avoid);
};

// Every move a debuff carrier makes, both debuffs in one node: two movers at one relevance trade the tick.
// Gravity Bomb wins on a double carrier, its puddle holding raid floor for 180 s against a 9 s splash.
class XT002DebuffCarrierAction : public XT002MoveClearAction
{
public:
    static constexpr char const* Name = "xt002 debuff carrier action";

    XT002DebuffCarrierAction(PlayerbotAI* botAI) : XT002MoveClearAction(botAI, Name) {}

    bool Execute(Event event) override;

private:
    enum class ParkResult
    {
        None,        // no cell qualifies; the dynamic search is the fallback
        OutOfReach,  // a cell was chosen, but the bomb goes off before the bot could get there
        Moving,      // walking to a cell, so this action owns the tick
        Parked       // standing on one, so the tick is free for casts and heals
    };

    // Picks a free cell from the role's lots, ranked by reach, then walk-in clearance, room at the cell, walk
    // length. No LOS test: rays from the raid clip the building rim. `cellX`/`cellY` get the best cell regardless.
    ParkResult ParkVoidZone(std::list<Creature*> const& voidZones, float reach, float& cellX, float& cellY);

    // How far the bot can still walk before the bomb goes off. GetSpeed(MOVE_RUN) already carries Tympanic
    // Tantrum's 50% slow.
    float TravelReach(uint32 remainingMs) const;

    // Walks the bearing to a cell as far as the budget allows, so the puddle lands on the approach. False if that
    // point is in a Void Zone, inside the expiry pull or refused, which hands the tick to the ring search.
    bool StopShortOf(float cellX, float cellY, float reach, std::list<Creature*> const& voidZones);

    // The Searing Light spot, or the roomiest alternate clear of Void Zones that the pathfinder takes.
    // `heartbreak` is Execute's answer, since Void Zones only exist once XT carries it.
    bool MoveToSearingLightSpot(bool heartbreak);

    // Distance from (x, y) to the nearest living raider but this bot. Ranks Searing Light alternates by the room
    // they really have once the raid drifts off its slots.
    float RaidClearance(float x, float y) const;

    // How close the straight line to (x, y) passes a Void Zone. Only approximates the navmesh path, but the
    // carrier outranks the hazard dodge, so it's the only guard.
    float ApproachClearance(float x, float y, std::list<Creature*> const& voidZones) const;

    // Whether the bot is standing inside a live Void Zone, which no parking cell is allowed to be.
    bool StandingInVoidZone(std::list<Creature*> const& voidZones) const;

    // Whether the bot stands in any of its role's lots, by geometry only: right after a bomb expires, its own
    // fresh puddle occupies the cell it stands on.
    bool InsideParkingLot() const;

    // Latched once standing on a cell, so drift inside the deadband does not re-issue a move. A bot
    // that re-issues every tick slides in place and cannot cast.
    bool parked = false;
};

// Boombots and Void Zones in one node, since two movers at one relevance trade the tick. Puddles are skipped
// for a carrier: the carrier action owns where it stands, off its own bomb included.
class XT002AvoidHazardAction : public XT002MoveClearAction
{
public:
    static constexpr char const* Name = "xt002 avoid hazard action";

    XT002AvoidHazardAction(PlayerbotAI* botAI) : XT002MoveClearAction(botAI, Name) {}

    bool Execute(Event event) override;
};

class XT002PummellerTauntAction : public Action
{
public:
    static constexpr char const* Name = "xt002 pummeller taunt action";

    XT002PummellerTauntAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

class XT002RedirectThreatAction : public Action
{
public:
    static constexpr char const* Name = "xt002 redirect threat action";

    XT002RedirectThreatAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    // The tank this bot's threat should land on, or nullptr when there is nothing to redirect to.
    Player* GetRedirectTank(Unit* xt002);
};

// Main tank gets a point, ranged and healers a formation slot each. Melee are left alone: pinning them costs
// uptime on a boss that moves.
class XT002RaidPositionAction : public MovementAction
{
public:
    static constexpr char const* Name = "xt002 raid position action";

    XT002RaidPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Owns "current target" for every role while XT is up. No raid icons: they're group-global and would
// overwrite the player's. Healers get one too, or they never enter combat and lose their combat heals.
class XT002SetDpsPriorityAction : public AttackAction
{
public:
    static constexpr char const* Name = "xt002 set dps priority action";

    XT002SetDpsPriorityAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}

    bool Execute(Event event) override;

private:
    // Candidates for this bot's role, most urgent first; entries the role must not touch are left out. `xt002`
    // comes back as GetXT002 would return it, found in the same pass.
    std::vector<std::pair<uint32, Unit*>> BuildPriorityList(Unit*& xt002);

    // Nearest live candidate of `entry`, preferring the current target so two identical adds cannot
    // make the bot alternate between them every tick.
    Unit* SelectByEntry(Unit* currentTarget, uint32 entry, std::vector<Unit*> const& candidates) const;

    bool IsAllowedTarget(Unit* unit, Unit* xt002) const;

    Unit* ResolveTarget(Unit* currentTarget);
};

#endif
