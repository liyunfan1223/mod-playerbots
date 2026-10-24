/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_VEZAX_H
#define PLAYERBOTS_ULDACTIONS_VEZAX_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldEncounter_Vezax.h"
#include "UldTriggers.h"
#include "Vehicle.h"

class VezaxResetEncounterStateAction : public Action
{
public:
    static constexpr char const* Name = "vezax reset encounter state action";

    VezaxResetEncounterStateAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// Carry the mark away from everyone else. The leech skips its holder, so this takes ten seconds of
// 5000 a tick off the people nearby.
class VezaxMarkOfTheFacelessAction : public MovementAction
{
public:
    static constexpr char const* Name = "vezax mark of the faceless action";

    VezaxMarkOfTheFacelessAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// The other side of it: step out of the leech around someone else's mark.
class VezaxMarkOfTheFacelessBreakAction : public MovementAction
{
public:
    static constexpr char const* Name = "vezax mark of the faceless break action";

    VezaxMarkOfTheFacelessBreakAction(PlayerbotAI* botAI)
        : MovementAction(botAI, Name)
    {
    }

    bool Execute(Event event) override;
};

// Step out of where the missile in flight will land. Each bot walks its own group's strafe, so the
// group arrives with its shape intact and the impact keeps its bearing relative to all of them.
class VezaxShadowCrashDodgeAction : public MovementAction
{
public:
    static constexpr char const* Name = "vezax shadow crash dodge action";

    VezaxShadowCrashDodgeAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

class VezaxSearingFlamesInterruptAction : public Action
{
public:
    static constexpr char const* Name = "vezax searing flames interrupt action";

    VezaxSearingFlamesInterruptAction(PlayerbotAI* botAI)
        : Action(botAI, Name)
    {
    }

    bool Execute(Event event) override;
};

class VezaxSurgeOfDarknessAction : public Action
{
public:
    static constexpr char const* Name = "vezax surge of darkness action";

    VezaxSurgeOfDarknessAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// Hard mode: switch to and kill the invulnerability-granting Saronite Animus.
class VezaxSaroniteAnimusAction : public AttackAction
{
public:
    static constexpr char const* Name = "vezax saronite animus action";

    VezaxSaroniteAnimusAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Misdirection or Tricks of the Trade on the main tank.
class VezaxAnimusRedirectAction : public Action
{
public:
    static constexpr char const* Name = "vezax animus redirect action";

    VezaxAnimusRedirectAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// Walk to the main tank, with the Animus following.
class VezaxAnimusBringBackAction : public MovementAction
{
public:
    static constexpr char const* Name = "vezax animus bring back action";

    VezaxAnimusBringBackAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Back in between the Animus and Vezax, where his Shadow Crash can't pick the bot.
class VezaxAnimusMeleeSpotAction : public MovementAction
{
public:
    static constexpr char const* Name = "vezax animus melee spot action";

    VezaxAnimusMeleeSpotAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Walk into the nearest Shadow Crash field and hold it.
class VezaxShadowCrashSoakAction : public MovementAction
{
public:
    static constexpr char const* Name = "vezax shadow crash soak action";

    VezaxShadowCrashSoakAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Put the boss under the bot's crosshair, whether it was on a Saronite Vapor or on nothing at all.
class VezaxHoldTargetAction : public AttackAction
{
public:
    static constexpr char const* Name = "vezax hold target action";

    VezaxHoldTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Stops what the hold's rules can't reach: a swing already going, a cast or channel on him, and pets.
// Gives the pets their stance back once the hold lifts.
class VezaxDamageHoldAction : public Action
{
public:
    static constexpr char const* Name = "vezax damage hold action";

    VezaxDamageHoldAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

class VezaxRaidPositionAction : public MovementAction
{
public:
    static constexpr char const* Name = "vezax raid position action";

    VezaxRaidPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;

private:
    bool _slotReached = false;
};

#endif
