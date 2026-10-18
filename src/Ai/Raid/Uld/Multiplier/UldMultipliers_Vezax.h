/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDMULTIPLIERS_VEZAX_H
#define PLAYERBOTS_ULDMULTIPLIERS_VEZAX_H

#include "Define.h"
#include "Multiplier.h"

#include <unordered_map>

// Life Tap returns nothing here: its mana half, 31818, is a bare SPELL_EFFECT_ENERGIZE that Aura of
// Despair is immune to, and the glyph proc (63320) rides that same energize.
class VezaxSuppressLifeTapMultiplier : public Multiplier
{
public:
    VezaxSuppressLifeTapMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "vezax suppress life tap multiplier")
    {
    }

    float GetValue(Action* action) override;
};

// Ranged dps don't cast outside a Shadow Crash field: full price buys mana that can't be made back.
// Healers are exempt, since a held heal lands inside the leech spikes that kill people.
class VezaxHoldCastOutsideFieldMultiplier : public Multiplier
{
public:
    VezaxHoldCastOutsideFieldMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "vezax hold cast outside field multiplier")
    {
    }

    float GetValue(Action* action) override;
};

// The Animus is taunt-immune, so only the main tank's threat keeps it. A dps bot on it holds its
// damage once it gets close to his, which also lets him win it back from whoever pulled it.
class VezaxAnimusThreatMultiplier : public Multiplier
{
public:
    VezaxAnimusThreatMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "vezax animus threat multiplier") {}

    float GetValue(Action* action) override;
};

// The spell half of the hard mode hold: nothing aimed at him and no area damage. Melee, pets and the
// damaging summons are a Block rule.
class VezaxDamageHoldMultiplier : public Multiplier
{
public:
    VezaxDamageHoldMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "vezax damage hold multiplier") {}

    float GetValue(Action* action) override;

private:
    enum class Reach : uint8
    {
        None,
        CurrentTarget,
        Area
    };

    std::unordered_map<Action*, Reach> reaches;
};

#endif
