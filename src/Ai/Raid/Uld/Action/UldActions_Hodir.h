/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_HODIR_H
#define PLAYERBOTS_ULDACTIONS_HODIR_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidRedirectThreat.h"
#include "UldTriggers.h"
#include "Vehicle.h"

// No isUseful overrides: each action has exactly one trigger node, which is already the gate, and
// building that trigger on the stack to check silently breaks any trigger with per-bot state.

// Run to the Snowpacked Icicle Target the rest of the raid is running to. Standing inside its Safe
// Area is the only way to survive Flash Freeze.
class HodirMoveSnowpackedIcicleAction : public MovementAction
{
public:
    static constexpr char const* Name = "hodir move snowpacked icicle";

    HodirMoveSnowpackedIcicleAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
};

// Cast directly, not via ChangeStrategy: the shared node adds "rfrost" and nothing removes it, so the
// pick outlives the encounter. The hodir paladin aura rule holds the slot open meanwhile.
class HodirFrostResistanceAction : public Action
{
public:
    static constexpr char const* Name = "hodir frost resistance action";

    HodirFrostResistanceAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

// Step out from under an icicle that has not detonated yet.
class HodirIcicleDodgeAction : public MovementAction
{
public:
    static constexpr char const* Name = "hodir icicle dodge action";

    HodirIcicleDodgeAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;

private:
    Position _dest;
    // Closest the bot has got to _dest so far, so a drag away from it can be told from ordinary walking.
    float _destDist = 0.0f;
};

// Shed Biting Cold by moving: a stack drops only on the second moving tick in a row, so this chains 6 yd
// legs down to ULDUAR_HODIR_BITING_COLD_SHED_FLOOR, each held until walked.
class HodirBitingColdShedAction : public MovementAction
{
public:
    static constexpr char const* Name = "hodir biting cold shed";

    HodirBitingColdShedAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;

private:
    bool _shedding = false;
    // The leg being walked, and the closest the bot has got to it, so a drag away from it can be told
    // from ordinary walking.
    Position _leg;
    float _legDist = 0.0f;
};

// Hold the bot's anchor: a spot just past the hold point for the two tanks, a Starlight or fire
// stand for ranged and healers, and the nearest spot that fixes a broken constraint when there is none.
class HodirRaidPositionAction : public MovementAction
{
public:
    static constexpr char const* Name = "hodir raid position action";

    HodirRaidPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
};

// Trapped raiders, then flash-frozen helpers, then the boss.
class HodirSetDpsPriorityAction : public AttackAction
{
public:
    static constexpr char const* Name = "hodir set dps priority action";

    HodirSetDpsPriorityAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;

private:
    Unit* ResolveTarget();
};

// Taunt Hodir off the tank Frozen Blows would kill, and take him back when it drops.
class HodirFrozenBlowsSwapAction : public AttackAction
{
public:
    static constexpr char const* Name = "hodir frozen blows swap action";

    HodirFrozenBlowsSwapAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
};

// Misdirection and Tricks go to whichever tank holds Hodir now. A taunt only matches top threat, so the
// incoming tank is level with the best DPS and the redirect buys back the lead.
class HodirRedirectThreatAction : public RaidRedirectThreatAction
{
public:
    static constexpr char const* Name = "hodir redirect threat action";

    HodirRedirectThreatAction(PlayerbotAI* botAI) : RaidRedirectThreatAction(botAI, Name) {}

protected:
    Player* GetRedirectTank() override;
    Unit* GetThreatDumpTarget() override;
};

// Hold the carrier on its rally so the charges pulse into a gathering stack. State is in the shared
// per-carry latch, not here, since the receivers read the same point.
class HodirSpreadStormCloudAction : public MovementAction
{
public:
    static constexpr char const* Name = "hodir spread storm cloud";

    HodirSpreadStormCloudAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
};

// Step inside the 3 yd pulse of somebody else's carry.
class HodirCollectStormPowerAction : public MovementAction
{
public:
    static constexpr char const* Name = "hodir collect storm power";

    HodirCollectStormPowerAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
};

#endif
