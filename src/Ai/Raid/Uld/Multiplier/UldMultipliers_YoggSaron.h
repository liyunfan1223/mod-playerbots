/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDMULTIPLIERS_YOGGSARON_H
#define PLAYERBOTS_ULDMULTIPLIERS_YOGGSARON_H

#include "Define.h"
#include "Multiplier.h"
#include "RaidAntiFear.h"

class CastSpellAction;
class Unit;

// Reach stands down only where an encounter node owns the bot's position: it is the only generic closer.
// Flee is off in phase 1: Dark Volley reaches 35 yd, the nova is a death explosion, so backing off costs the station.
class YoggSaronMovementGuardMultiplier : public Multiplier
{
public:
    YoggSaronMovementGuardMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "yogg-saron movement guard multiplier")
    {
    }

    float GetValue(Action* action) override;

private:
    float FleeGuard();
    float SetBehindGuard();
    bool ReachIsWrong(Action* action);
    bool CrusherReachIsWrong();
};

// Phase 1 AoE, held while it would finish a Guardian nobody is on (an untargeted stack Guardian still loses 5.6%/s).
// Heals are never held. Covers the Guardians phase 1 leaves behind too, until the last one dies.
class YoggSaronPhase1AoeHoldMultiplier : public Multiplier
{
public:
    YoggSaronPhase1AoeHoldMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "yogg-saron phase 1 aoe hold multiplier")
    {
    }

    float GetValue(Action* action) override;
};

// Anything hitting more than one tentacle, held in an illusion room: each tentacle reflects 60%, so three return 180%.
// Most melee cleaves carry no AoE threat type, hence the names. Heals are never held.
class YoggSaronIllusionMultiTargetHoldMultiplier : public Multiplier
{
public:
    YoggSaronIllusionMultiTargetHoldMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "yogg-saron illusion multi target hold multiplier")
    {
    }

    float GetValue(Action* action) override;
};

// Cast-time spells and channels, held while a bot walks out ahead of Induce Madness, since each one stops the walk.
// Bladestorm and the like too, since they stop the portal. Other instants still go.
class YoggSaronExitHoldCastMultiplier : public Multiplier
{
public:
    YoggSaronExitHoldCastMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "yogg-saron exit hold cast multiplier")
    {
    }

    float GetValue(Action* action) override;
};

// Spells at something the bot is holding off, which the resolver can't stop: class nodes aim at "attackers" alone.
// Held: Brain DoTs and channels, spells at a hunter-dragged Guardian, a non-tank's at a threat-gated Immortal Guardian.
class YoggSaronHeldTargetCastMultiplier : public Multiplier
{
public:
    YoggSaronHeldTargetCastMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "yogg-saron held target cast multiplier")
    {
    }

    float GetValue(Action* action) override;

private:
    bool GuardianThreatHeld(CastSpellAction* spellAction, Unit* target);
};

// A non-tank's area spell in phase 3, held unless ULDUAR_YOGG_SARON_PHASE_3_AREA_MIN_WORTH units worth
// hitting stand round its target and every Guardian there passes the threat gate.
class YoggSaronPhase3AreaMultiplier : public Multiplier
{
public:
    YoggSaronPhase3AreaMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "yogg-saron phase 3 area multiplier") {}

    float GetValue(Action* action) override;
};

// Phase 1 walks back to the stack or station, held for a Sara's Fervor holder near a live nova (dodge: inside 17 yd)
// and for a walk that would cross a cloud, which waits for the cloud to pass.
class YoggSaronPhase1WalkGuardMultiplier : public Multiplier
{
public:
    YoggSaronPhase1WalkGuardMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "yogg-saron phase 1 walk guard multiplier")
    {
    }

    float GetValue(Action* action) override;

private:
    bool WalkCrossesCloud(bool station, bool reachMelee);
};

class YoggSaronAntiFearTotemGuardMultiplier : public RaidAntiFearTotemGuardMultiplier
{
public:
    YoggSaronAntiFearTotemGuardMultiplier(PlayerbotAI* botAI)
        : RaidAntiFearTotemGuardMultiplier(botAI, "yogg-saron anti fear totem guard multiplier")
    {
    }

protected:
    bool FearWindowActive() override;
};

#endif
