/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PALADINHELPER_H
#define PLAYERBOTS_PALADINHELPER_H

#include "Player.h"
#include "Unit.h"
#include <initializer_list>

namespace ai::paladin
{
static constexpr uint32 SPELL_HAND_OF_PROTECTION = 1022;
static constexpr uint32 SPELL_HAND_OF_SALVATION = 1038;
static constexpr uint32 SPELL_HAND_OF_FREEDOM = 1044;
static constexpr uint32 SPELL_HAND_OF_SACRIFICE = 6940;
static constexpr uint32 SPELL_BLESSING_OF_SANCTUARY = 20911;
static constexpr uint32 SPELL_GREATER_BLESSING_OF_SANCTUARY = 25899;

// Holy Shield lasts 10 s on an 8 s cooldown, so a recast in its last 2 s keeps it up. The trigger and the
// action both need the margin, or the action turns down the early recast the trigger asks for.
static constexpr uint32 HOLY_SHIELD_REFRESH_MS = 2000;

inline bool HasHandFromCaster(Unit* target, Player* caster, std::initializer_list<uint32> spellIds)
{
    if (!target || !caster)
        return false;

    for (uint32 spellId : spellIds)
    {
        if (target->HasAura(spellId, caster->GetGUID()))
            return true;
    }

    return false;
}

inline bool HasAnyPaladinHandFromCaster(Unit* target, Player* caster)
{
    return HasHandFromCaster(target, caster,
        { SPELL_HAND_OF_PROTECTION, SPELL_HAND_OF_SALVATION, SPELL_HAND_OF_FREEDOM, SPELL_HAND_OF_SACRIFICE });
}
}

#endif
