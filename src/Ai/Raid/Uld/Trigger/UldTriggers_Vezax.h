/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_VEZAX_H
#define PLAYERBOTS_ULDTRIGGERS_VEZAX_H

#include "BurstCooldowns.h"
#include "EventMap.h"
#include "GenericTriggers.h"
#include "UldEncounter_Vezax.h"
#include "Trigger.h"

// General Vezax

// Vezax out of combat or dead, and this instance still holds slot assignments.
class VezaxResetEncounterStateTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax reset encounter state";

    VezaxResetEncounterStateTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 5) {}
    bool IsActive() override;
};

class VezaxMarkOfTheFacelessTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax mark of the faceless";

    VezaxMarkOfTheFacelessTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Standing inside the leech around someone else's mark. Melee only: the camp is far enough out that
// nothing marked in the ball reaches it.
class VezaxMarkOfTheFacelessBreakTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax mark of the faceless break";

    VezaxMarkOfTheFacelessBreakTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, Name)
    {
    }
    bool IsActive() override;
};

// A Shadow Crash missile in flight with this bot standing where it will land.
class VezaxShadowCrashDodgeTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax shadow crash dodge";

    VezaxShadowCrashDodgeTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class VezaxSearingFlamesInterruptTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax searing flames interrupt";

    VezaxSearingFlamesInterruptTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, Name)
    {
    }
    bool IsActive() override;
};

class VezaxSurgeOfDarknessTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax surge of darkness";

    VezaxSurgeOfDarknessTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Hard mode: Saronite Animus alive, bot not already attacking it. Tanks, hunters and rogues go at once,
// everyone else once a tank has held it or the cap runs out.
class VezaxSaroniteAnimusTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax saronite animus";

    VezaxSaroniteAnimusTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;

private:
    // Per bot, and polled every tick while it waits, or the dwell restarts.
    BurstHoldState tankHold;
};

// Hunter or rogue already on the Animus, in its first 6s: redirect onto the main tank.
class VezaxAnimusRedirectTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax animus redirect";

    VezaxAnimusRedirectTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// The Animus is hitting a non-tank. The main tank can't go to it without dragging Vezax off the anchor,
// so its victim brings it to him.
class VezaxAnimusBringBackTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax animus bring back";

    VezaxAnimusBringBackTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Melee bot on the Animus standing far enough from Vezax to be a Shadow Crash pick.
class VezaxAnimusMeleeSpotTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax animus melee spot";

    VezaxAnimusMeleeSpotTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Mana caster with a Shadow Crash field in walking range. The field is the fight's damage and mana
// answer, so this moves bots in rather than out.
class VezaxShadowCrashSoakTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax shadow crash soak";

    // Two seconds, not every tick: this is the one Vezax trigger that has to sweep the grid, and the
    // field lands on a 10s cadence and lasts 20s, so reacting a little late costs almost nothing.
    VezaxShadowCrashSoakTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 2) {}
    bool IsActive() override;
};

// Not on the boss or the Animus. The target guard silences the generic pickers, so this is the only
// target source, and a bot with none never calls Attack and spends the fight buffing.
class VezaxHoldTargetTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax hold target";

    VezaxHoldTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Hard mode hold: a swing, cast or pet still on him past this bot's stop line, or a pet the hold
// parked waiting for its stance back.
class VezaxDamageHoldTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax damage hold";

    VezaxDamageHoldTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class VezaxRaidPositionTrigger : public Trigger
{
public:
    static constexpr char const* Name = "vezax raid position";

    VezaxRaidPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 2) {}
    bool IsActive() override;
};

#endif
