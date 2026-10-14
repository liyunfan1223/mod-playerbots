/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULHELPERS_H
#define PLAYERBOTS_GRUULHELPERS_H

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <type_traits>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

namespace GruulHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class GruulSpells : uint32
{
    // High King Maulgar <Lord of the Ogres>
    SPELL_WHIRLWIND     = 33238,

    // Krosh Firehand
    SPELL_SPELL_SHIELD  = 33054,

    // Hunter
    SPELL_MISDIRECTION  = 35079,

    // Mage
    SPELL_SPELLSTEAL    = 30449,

    // Gruul the Dragonkiller
    SPELL_GROUND_SLAM_1 = 33525,
    SPELL_GROUND_SLAM_2 = 39187,
    SPELL_CAVE_IN       = 36240,
};

enum class GruulNpcs : uint32
{
    NPC_WILD_FEL_STALKER = 18847,
};

inline constexpr uint32 GRUUL_MAP_ID = 565;

// High King Maulgar

// Ogre combat reaches:
// (1) Maulgar = 3.5y, (2) Olm = 2.2y, (3) Blindeye = 3.525y, (4) Krosh = 2y, (5) Kiggler = 3.3y

// For the "high king maulgar krosh mage tank" and "high king maulgar kiggler moonkin tank" values.
inline constexpr uint32 CASTER_TANK_CACHE_INTERVAL_MS = 1000;
// Hold cooldowns until Blindeye is at this percent health. Also the gate for on-pull Misdirects.
inline constexpr float BLINDEYE_ENGAGED_HEALTH_PCT = 75.0f;
// Radius is 30y with 2y of MoveAway padding. Stays inside the mod's "enemy out of spell" threshold
// (spellDistance + CONTACT_DISTANCE + both reaches, ~34y exact against Kiggler), allowing the
// boomie to attack Kiggler, even without reach-increasing talents, without being in range of
// Kiggler's Arcane Explosion.
inline constexpr float KIGGLER_ARCANE_EXPLOSION_SAFE_DISTANCE = 32.0f;
// Radius is 15y with 2y of MoveAway padding.
inline constexpr float KROSH_BLAST_WAVE_SAFE_DISTANCE = 17.0f;
// Radius is 8y, padded to 8 * sqrt(2) rather than the 2y used elsewhere. The tl;dr is MoveAway()'s
// fallback candidates (when running straight back is blocked) can end up moving the bot less than
// the prescribed distance. Maulgar is tanked against a wall and fears and charges and can end up
// turned away from the wall as a result, with melee dps between him and the wall and failing the
// straight-away movement.
inline constexpr float MAULGAR_WHIRLWIND_SAFE_DISTANCE = 12.0f;
// Distance for the multiplier, which as usual, is a little more than the escape distance.
inline constexpr float MAULGAR_WHIRLWIND_HOLD_DISTANCE = 15.0f;
// For the "high king maulgar wild fel stalkers" value.
inline constexpr uint32 WILD_FEL_STALKER_CACHE_INTERVAL_MS = 1000;
inline constexpr float WILD_FEL_STALKER_SEARCH_RADIUS = 50.0f;

inline Position const MAULGAR_TANK_POSITION  = {  90.686f, 167.047f, -13.234f };
inline Position const OLM_TANK_POSITION      = { 101.050f, 219.359f,  -9.503f };
inline Position const BLINDEYE_TANK_POSITION = {  99.681f, 213.989f, -10.345f };
inline Position const KROSH_TANK_POSITION    = { 116.880f, 166.208f, -14.231f };
inline Position const GRUUL_TANK_POSITION    = { 241.238f, 365.025f,  -4.220f };

bool IsMaulgarTank(Player* bot);
bool IsOlmTank(Player* bot);
bool IsBlindeyeTank(Player* bot);
ObjectGuid FindKroshMageTankGuid(Player* bot);
Player* GetKroshMageTank(PlayerbotAI* botAI);
bool IsKroshMageTank(PlayerbotAI* botAI);
ObjectGuid FindKigglerMoonkinTankGuid(Player* bot);
Player* GetKigglerMoonkinTank(PlayerbotAI* botAI);
bool IsKigglerMoonkinTank(PlayerbotAI* botAI);
GuidVector FindNearbyWildFelStalkerGuids(Player* bot);
std::vector<Unit*> GetNearbyWildFelStalkers(PlayerbotAI* botAI);

// Gruul the Dragonkiller

// For the "gruul the dragonkiller cave in" value.
inline constexpr uint32 CAVE_IN_CACHE_INTERVAL_MS = 100;
// Radius is 8y, plus the 1.5y CombatReach the membership check adds for the player, rounded up.
// The pool lasts 15s and is cast as often as every 3.5s, so 5 can be on the ground at once.
inline constexpr float CAVE_IN_RADIUS = 10.0f;
// Distance for the multiplier. As usual, wider than the avoidance to prevent running back in.
inline constexpr float CAVE_IN_CONTROL_RADIUS = CAVE_IN_RADIUS + 5.0f;
inline constexpr float CAVE_IN_SEARCH_RADIUS = CAVE_IN_CONTROL_RADIUS + 2.0f;
// Radius is 20y with 2y of MoveAway padding. Sort of. The details are not really important; I note
// only that damage has a linear relationship with distance.
inline constexpr float GRUUL_SHATTER_SAFE_DISTANCE = 22.0f;

bool GetNearestCaveInPosition(PlayerbotAI* botAI, Position& pool);
bool IsNearCaveIn(PlayerbotAI* botAI, float radius);
bool IsInCaveIn(PlayerbotAI* botAI);
bool HasGroundSlam(Player* bot);

}

#endif
