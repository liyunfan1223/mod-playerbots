/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERKOLOGARN_H
#define PLAYERBOTS_ULDENCOUNTERKOLOGARN_H

#include "Position.h"
#include "UldData.h"

class Player;
class PlayerbotAI;
class Unit;
class WorldObject;

enum UlduarKologarnIds
{
    NPC_LEFT_ARM = 32933,
    NPC_RIGHT_ARM = 32934,
    NPC_RUBBLE = 33768,
    NPC_KOLOGARN_EYEBEAM_LEFT = 33632,
    NPC_KOLOGARN_EYEBEAM_RIGHT = 33802,

    // Both Overhead Smash variants trigger 63355. 64002 (-25%) is never applied here and is checked
    // only so the swap survives a core change.
    SPELL_CRUNCH_ARMOR = 63355,
    SPELL_CRUNCH_ARMOR_ALT = 64002,

    // Grip victims ride the right arm and are instakilled on expiry unless the arm releases them.
    SPELL_STONE_GRIP_10 = 62166,
    SPELL_STONE_GRIP_25 = 63981,

    SPELL_FOCUSED_EYEBEAM_10_2 = 63346,
    SPELL_FOCUSED_EYEBEAM_10 = 63347,
    SPELL_FOCUSED_EYEBEAM_25_2 = 63976,
    SPELL_FOCUSED_EYEBEAM_25 = 63977,
};

inline constexpr float ULDUAR_KOLOGARN_AXIS_Z_PATHING_ISSUE_DETECT = 420.0f;
inline constexpr float ULDUAR_KOLOGARN_EYEBEAM_RADIUS = 3.0f;

inline constexpr float ULDUAR_KOLOGARN_ROOM_SEARCH_RADIUS = 100.0f;

// Every destination is clamped to this walkway box: past x 1782 the pit bunny instakills. Entrance is -X.
inline constexpr float ULDUAR_KOLOGARN_WALKWAY_X_MIN = 1745.0f;
inline constexpr float ULDUAR_KOLOGARN_WALKWAY_X_MAX = 1780.0f;
inline constexpr float ULDUAR_KOLOGARN_WALKWAY_Y_MIN = -48.0f;
inline constexpr float ULDUAR_KOLOGARN_WALKWAY_Y_MAX = -2.0f;
inline constexpr float ULDUAR_KOLOGARN_WALKWAY_Z = 448.0f;

// Beam 63346 is a 3 yd AoE and the eye chases at 5.5 yd/s against a player's 7.0, so a bot that
// starts well before it lands outruns it.
inline constexpr float ULDUAR_KOLOGARN_EYEBEAM_REACT_RADIUS = 8.0f;
inline constexpr float ULDUAR_KOLOGARN_EYEBEAM_SAFE_DISTANCE = 12.0f;
inline constexpr float ULDUAR_KOLOGARN_EYEBEAM_RUN_STEP = 10.0f;

// Rubble outrun players (8.0 vs 7.0 yd/s), so the off-tank holds them rather than kiting. The offset
// is lateral, toward the dead arm's side: -X is the eyebeam escape lane and has to stay clear.
inline constexpr float ULDUAR_KOLOGARN_RUBBLE_HOLD_OFFSET = 18.0f;

// Crunch Armor is -20% armour per stack, 4 stacks, 45s, on a 14s Smash timer.
inline constexpr uint8 ULDUAR_KOLOGARN_CRUNCH_ARMOR_SWAP_STACKS = 2;

// Taunt range, so the off-tank only wanders off to DPS the arm while it can still take the body back.
inline constexpr float ULDUAR_KOLOGARN_TAUNT_RANGE = 30.0f;

// By entry, not "find target": that only walks the bot's threatened-by-me list, so a tank parked on
// the body never sees the arms.
Unit* GetKologarn(PlayerbotAI* botAI);
Unit* GetKologarnRightArm(PlayerbotAI* botAI);

// The lookups above only say "he's nearby" (SightDistance scan). Every target picker asks this
// instead, or the raid pulls him from 100 yd.
bool KologarnEncounterActive(PlayerbotAI* botAI);
Unit* GetKologarnNearestRubble(PlayerbotAI* botAI, WorldObject const* from);
bool KologarnHasRubble(PlayerbotAI* botAI);

// Nearest rubble not already on the off-tank, so the pickup sweeps the loose ones first instead of
// re-taunting whatever it is already holding. Falls back to the nearest of any.
Unit* GetKologarnLooseRubble(PlayerbotAI* botAI, Player* bot);

uint8 GetKologarnCrunchArmorStacks(Unit const* unit);

// Grip victims are stunned passengers on the right arm. The body tank is exempt: the core strips the
// caster's victim from the target list.
bool IsKologarnStoneGripped(Unit const* unit);

// The tank the boss is actually swinging at. Petrifying Breath goes out the moment this one leaves
// melee range, so it doubles as the "is the body covered" test.
Player* GetKologarnBodyTank(PlayerbotAI* botAI);
bool IsKologarnBodyTank(PlayerbotAI* botAI, Player* bot);

// Main tank or first assist tank, whichever is not currently holding the body. Derived rather than
// stored, so the rubble duty follows the taunt swap on its own.
bool IsKologarnOffTank(PlayerbotAI* botAI, Player* bot);

// Ranged take rubble first, melee stay on the arm so grips keep getting cut short. Triggers and actions
// both have to use this, or they disagree and the bot retargets every tick.
Unit* GetKologarnDpsTarget(PlayerbotAI* botAI, Player* bot);

// The off-tank with no rubble to hold adds damage to the arm, but only while it can still reach the
// body with a taunt.
Unit* GetKologarnOffTankTarget(PlayerbotAI* botAI, Player* bot);

// The eye chasing this bot, if any. Bystanders get the nearest eye inside the react radius instead.
Unit* GetKologarnEyebeamChasing(PlayerbotAI* botAI, Player* bot);
Unit* GetKologarnNearestEyebeam(PlayerbotAI* botAI, Player* bot, float radius);

// Held to the dead arm's side of the raid, never behind it: -X is the lane an eyebeam target runs down.
Position GetKologarnRubbleHoldSpot(PlayerbotAI* botAI, Unit* rubble);

// Next escape step, clamped to the walkway. -X first, then along the walkway: the eye lives 10s and
// -X runs out after ~5s, so the run turns rather than parking against the clamp.
Position GetKologarnEyebeamEscapeStep(Player* bot, Unit* eye);

#endif
