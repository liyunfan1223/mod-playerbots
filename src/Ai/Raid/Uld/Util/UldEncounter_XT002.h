/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERXT002_H
#define PLAYERBOTS_ULDENCOUNTERXT002_H

#include <vector>

#include "Position.h"
#include "UldData.h"

class Player;
class PlayerbotAI;
class Unit;

// XT-002 Deconstructor.

enum UlduarXT002Ids
{
    // NPC_HEART_OF_DECONSTRUCTOR come from core ulduar.h via UldScripts.h.
    PB_NPC_XT002_PUMMELLER = 33344,    // aggressive add, wants an off-tank
    PB_NPC_XT002_BOOMBOT = 33346,      // explodes on reaching XT or at 50% health; melee must not touch it
    PB_NPC_XT002_LIFE_SPARK = 34004,   // hard mode only, spawned by an expiring Searing Light
    PB_NPC_XT002_VOID_ZONE = 34001,    // hard mode only, dropped by an expiring Gravity Bomb
    // 10-man ids. The core casts these and lets the difficulty table pick, so a 25-man raid carries
    // 65121, 64234 and 64193 instead: read them through sSpellMgr->GetSpellIdForDifficulty.
    SPELL_XT002_SEARING_LIGHT = 63018,
    SPELL_XT002_GRAVITY_BOMB = 63024,
    SPELL_XT002_HEARTBREAK = 65737,     // permanent hard-mode empower once the Heart dies
    SPELL_XT002_EXPOSED_HEART = 63849,  // channeled by the Heart while it is vulnerable
    SPELL_XT002_SUBMERGE = 37751,
};

// XT-002: how far a carrier walks from the raid. Twice the 12 yd splash, since only the carrier moves and a
// bomb landing at the edge still clips whoever drifted a yard.
inline constexpr float ULDUAR_XT002_DEBUFF_CLEAR_RADIUS = 25.0f;

// XT-002: Gravity Bomb's expiry pull (63025/64233 effect 1). The damage only reaches 12 yd, but the pull into
// a fresh Void Zone is what kills, so a stop-short drop point has to clear this.
inline constexpr float ULDUAR_XT002_GRAVITY_BOMB_PULL_RADIUS = 20.0f;

// XT-002: Boom is about 10 yd and a Boombot also blows at 50% health, so melee keep a margin.
inline constexpr float ULDUAR_XT002_BOOMBOT_AVOID_RADIUS = 12.0f;

// XT-002 hard mode: Consumption (64208) is 5 yd and doesn't grow, plus a 1 yd buffer.
inline constexpr float ULDUAR_XT002_VOID_ZONE_RADIUS = 6.0f;

// XT-002: drift allowed off a spot before walking back. Ranged and healers own a slot each, so their band
// only absorbs drift; widening it instead stacks them on one point.
inline constexpr float ULDUAR_XT002_MAINTANK_SPOT_TOLERANCE = 3.0f;
inline constexpr float ULDUAR_XT002_RANGED_SPOT_TOLERANCE = 5.0f;

// XT-002: ranged/healer formation as ellipse half-axes, wider north-south since east-west is the line to XT
// and the tank. Centred north of the anchor; the ranged lots mirror about that centre.
inline constexpr float ULDUAR_XT002_RANGED_RING_OFFSET_Y = 6.0f;
inline constexpr float ULDUAR_XT002_RANGED_RING_INNER_X = 6.0f;
inline constexpr float ULDUAR_XT002_RANGED_RING_INNER_Y = 8.0f;
inline constexpr float ULDUAR_XT002_RANGED_RING_OUTER_X = 9.0f;
inline constexpr float ULDUAR_XT002_RANGED_RING_OUTER_Y = 12.0f;
inline constexpr size_t ULDUAR_XT002_RANGED_RING_INNER_SLOTS = 6;

// XT-002 hard mode: default Void Zone parking grid, walked +x from its origin and then away from the raid in
// y. Step is just over the Void Zone diameter so consecutive drops can't overlap.
inline constexpr float ULDUAR_XT002_BOMB_GRID_STEP = 6.0f;
inline constexpr int ULDUAR_XT002_BOMB_GRID_X_CELLS = 5;
inline constexpr int ULDUAR_XT002_BOMB_GRID_Y_CELLS = 4;

// XT-002 hard mode: melee lot columns. A lot is entered from the raid side, so a row of puddles seals
// everything behind it: wide beats deep.
inline constexpr int ULDUAR_XT002_BOMB_GRID_X_CELLS_MELEE = 7;

// XT-002 hard mode: arrival deadband for a parking cell and the drift allowed before re-issuing the move.
// Re-issuing every tick slides the bot in place so it can't cast.
inline constexpr float ULDUAR_XT002_BOMB_CELL_ARRIVED = 2.0f;
inline constexpr float ULDUAR_XT002_BOMB_CELL_REENGAGE = 5.0f;

// XT-002 hard mode: taken off the debuff before working out reach. Reaction delay, one tick, and a navmesh
// path longer than the straight line.
inline constexpr uint32 ULDUAR_XT002_BOMB_TRAVEL_MARGIN_MS = 1500;

// XT-002 hard mode: a carrier settling at the deadband edge is still outside Consumption. A preference,
// not a gate, or one puddle would run the lot out.
inline constexpr float ULDUAR_XT002_BOMB_CELL_PREFERRED_CLEARANCE =
    ULDUAR_XT002_VOID_ZONE_RADIUS + ULDUAR_XT002_BOMB_CELL_ARRIVED;

// XT-002 hard mode: the carrier outranks the hazard dodge, so walks to a cell rank by how close they pass a
// puddle, capped here. Not a gate: two taken cells in a row would read every cell behind them as blocked.
inline constexpr float ULDUAR_XT002_BOMB_APPROACH_CLEARANCE = 7.5f;

// XT-002: adds further than this from XT are still at their toy pile, and every add comes in by itself.
// Life Sparks are exempt: they spawn on the Searing Light carrier and chase players.
inline constexpr float ULDUAR_XT002_ADD_LEASH_RADIUS = 60.0f;

// XT-002: how far from itself a bot goes for an add. Ranged reach the Searing Light spot, where sparks spawn;
// melee barely move, since adds walk to XT or chase a player. Tanks are exempt.
inline constexpr float ULDUAR_XT002_RANGED_ENGAGE_RANGE = 35.0f;
inline constexpr float ULDUAR_XT002_MELEE_ENGAGE_RANGE = 15.0f;

// Taunt, Growl, Dark Command and Hand of Reckoning are all 30yd.
inline constexpr float ULDUAR_XT002_TAUNT_RANGE = 30.0f;

// XT-002: anchored on XT so one scan covers every lot. The far corner of the northern lot is 92 yd from XT's
// most easterly position, and a puddle outside this is a cell the ranking thinks is free.
inline constexpr float ULDUAR_XT002_VOID_ZONE_SEARCH_RADIUS = 110.0f;

// XT-002: ranked destinations a mover offers per tick. findSmoothPath refuses some walkable points here, the
// same ones every tick, so a lone winner never moves. Each try costs Playerbots.MaxMovementSearchTime builds.
inline constexpr size_t ULDUAR_XT002_MOVE_CANDIDATE_ATTEMPTS = 5;

// XT-002: radius of the Searing Light spot's alternates. Clears the Void Zone radius, so one puddle can't cover
// both the spot and the ring.
inline constexpr float ULDUAR_XT002_SEARING_LIGHT_DETOUR = 10.0f;

// XT-002: room a Searing Light destination keeps from every formation slot, the 8 yd splash plus margin.
inline constexpr float ULDUAR_XT002_SEARING_LIGHT_SLOT_CLEARANCE = 12.0f;

// XT-002 normal mode: bots stop damaging the exposed Heart here so an in-flight hit cannot kill it
// and flip the raid into hard mode by accident.
inline constexpr float ULDUAR_XT002_HEART_SAFE_HP_PCT = 15.0f;

// XT-002: the last Heart phase is over below this, so held burst cooldowns are free. In hard mode it means
// the Heart outlived every window and no Heartbreak is coming.
inline constexpr float ULDUAR_XT002_FINAL_PUSH_HP_PCT = 25.0f;

extern const Position ULDUAR_XT002_MAINTANK_SPOT;
extern const Position ULDUAR_XT002_RANGED_SPOT;
extern const Position ULDUAR_XT002_SEARING_LIGHT_SPOT;
extern const Position ULDUAR_XT002_GRAVITY_BOMB_ORIGIN_MELEE;
extern const Position ULDUAR_XT002_GRAVITY_BOMB_ORIGIN_RANGED_SOUTH;
extern const Position ULDUAR_XT002_GRAVITY_BOMB_ORIGIN_RANGED_NORTH;

// One Void Zone parking grid, walked +x and `yDirection` in y so the origin is the corner nearest the raid.
// Cell counts are per lot: only the melee one has open ground to the east.
struct XT002BombLot
{
    Position origin;
    float yDirection;
    int xCells = ULDUAR_XT002_BOMB_GRID_X_CELLS;
    int yCells = ULDUAR_XT002_BOMB_GRID_Y_CELLS;
};

// Read off "nearest npcs", not "find target": the Heart never attacks, so it's on no threat list. Each call
// rescans (grid visit plus a LOS ray per npc), so look XT up once per evaluation and pass him down.
Unit* GetXT002(PlayerbotAI* botAI);

// The Heart while it's vulnerable: alive, selectable and channeling Exposed Heart. Damage to it transfers
// to XT, which makes this window the encounter's damage multiplier.
Unit* GetXT002ExposedHeart(PlayerbotAI* botAI);

// XT is down in a Heart phase: not selectable and not attacking anyone. False without an XT.
bool IsXT002Submerged(Unit* xt002);

// Difficulty-mapped debuff ids (the 10- and 25-man versions are separate spells).
uint32 GetXT002SearingLightSpellId(Player* bot);
uint32 GetXT002GravityBombSpellId(Player* bot);
uint32 GetXT002HeartbreakSpellId(Player* bot);

// This bot owns the Pummeller: the first assist tank, else the main tank once no second tank is left. Taunt
// and target priority both read it, so they agree on who holds the add.
bool IsXT002PummellerTank(PlayerbotAI* botAI, Player* bot);

// Close enough to XT to engage; anything further is still at its toy pile. True without an XT, so the gate
// never strands a bot with nothing to hit.
bool IsXT002AddEngageable(Unit* xt002, Unit* unit);

// Nearest live add of `entry` inside the leash and within `botReach`. Nearest, not first found: an add stuck
// at its pile would mask the one hitting the raid.
Unit* GetXT002EngageableAdd(PlayerbotAI* botAI, Player* bot, uint32 entry, float botReach);

// This bot's formation slot. Every bot sorts the same roster, so none needs telling; healers take the centre
// and inner ring to keep the tank in range. False for melee and tanks, or without a group.
bool GetXT002RangedSlot(Player* bot, Unit* xt002, Position& out);

// Every formation slot except this bot's own, as laid out before any Void Zone re-deal. Empty when the
// group is gone.
std::vector<Position> GetXT002OtherFormationSlots(Player* bot);

// The parking grids this bot's role may drop a Void Zone in. The mover and the "am I in a lot" test both read
// it, so they can't disagree.
std::vector<XT002BombLot> GetXT002BombLots(PlayerbotAI* botAI, Player* bot);

#endif
