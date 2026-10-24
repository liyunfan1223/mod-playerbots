/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDDATA_H
#define PLAYERBOTS_ULDDATA_H

#include "Common.h"

// Ulduar values no single encounter owns, and no behaviour. Boss ids, tuning and positions go in that
// boss's UldEncounter_<Boss>.h, never here
inline constexpr uint32 ULDUAR_MAP_ID = 603;

// Class spells the encounters name by id rather than by the bot's own spellbook.
inline constexpr uint32 SPELL_FROST_TRAP = 13809;
// Hunter buff; its charges are what the redirect action spends.
inline constexpr uint32 SPELL_MISDIRECTION = 35079;

#endif
