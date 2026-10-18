/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERMIMIRON_H
#define PLAYERBOTS_ULDENCOUNTERMIMIRON_H

#include "ObjectGuid.h"
#include "Position.h"
#include "UldData.h"

#include <string>
#include <vector>

class Creature;
class Player;
class PlayerbotAI;
class Spell;
class Unit;

// Proximity Mines and Rocket Strike markers are non-selectable, so pathing can't see them: every
// destination goes through the spot-safety helpers below.

enum UlduarMimironIds
{
    NPC_LEVIATHAN_MKII = 33432,
    NPC_LEVIATHAN_MKII_CANNON = 34071,  // rides the MK II and does its Plasma Blast / Napalm casting
    NPC_VX001 = 33651,
    NPC_AERIAL_COMMAND_UNIT = 33670,
    NPC_BOMB_BOT = 33836,
    NPC_ROCKET_STRIKE_N = 34047,
    NPC_ASSAULT_BOT = 34057,
    NPC_PROXIMITY_MINE = 34362,
    NPC_MIMIRON_DB_TARGET = 33576,  // the barrage beams aim at this; it laps the room clockwise
    NPC_JUNK_BOT = 33855,
    NPC_MAGNETIC_CORE = 34068,
    SPELL_P3WX2_LASER_BARRAGE_1 = 63293,  // the only one that damages: a 104 degree cone, 50000 yd
    SPELL_P3WX2_LASER_BARRAGE_2 = 63297,  // SPELL_EFFECT_DUMMY, places a beam visual, harmless
    SPELL_SPINNING_UP = 63414,
    SPELL_SHOCK_BLAST = 63631,
    SPELL_P3WX2_LASER_BARRAGE_3 = 64042,  // SPELL_EFFECT_DUMMY, places a beam visual, harmless
    SPELL_P3WX2_LASER_BARRAGE_AURA_1 = 63274,
    SPELL_P3WX2_LASER_BARRAGE_AURA_2 = 63300,
    SPELL_MIMIRON_PLASMA_BLAST = 62997,
    SPELL_MIMIRON_PLASMA_BLAST_25 = 64529,
    SPELL_MIMIRON_NAPALM_SHELL = 63666,
    SPELL_MIMIRON_MAGNETIC_FIELD = 64668,
    ITEM_MIMIRON_MAGNETIC_CORE = 46029,  // 100% drop from the Assault Bot; grounds the ACU
    SPELL_MIMIRON_MAGNETIC_CORE_AURA = 64436,  // the 20s grounding itself, not the field 64668

    // Mimiron hard mode ("Firefighter", Big Red Button pressed): mechs empowered, two extra hazards.
    // NPC_MIMIRON (the boss; sits in his pod, never a bot attack target) comes from core ulduar.h via UldScripts.h.
    NPC_FLAMES_SPREAD = 34121,     // persistent spreading ground-fire node (non-selectable)
    SPELL_MIMIRON_FLAMES_AURA = 64561,  // what makes a node burn: ticks 64566 every second
    NPC_FROST_BOMB = 34149,        // VX-001's Frost Bomb; detonates in a large AoE
    NPC_EMERGENCY_FIRE_BOT = 34147,  // puts the flames out; three spawn every 45s

    // Lands on the player it was aimed at, not VX-001: six ticks over 3 s, all along the facing it had
    // when the aura landed, so the carrier gives the exact centreline.
    SPELL_MIMIRON_RAPID_BURST = 63382
};

// Mimiron P3Wx2 Laser Barrage. The damage is 63293, a TARGET_UNIT_CONE_ENEMY_104 cone: 104 degrees
// wide with a 50000 yd radius, so distance from VX-001 buys nothing and only bearing matters.
inline constexpr float ULDUAR_MIMIRON_BARRAGE_HALF_ANGLE = 52.0f * static_cast<float>(M_PI) / 180.0f;
// 15, not 12: the cone lands damage every 250 ms and turns 2.7 degrees in that time, so 12 is about one
// bot reaction tick with nothing to spare. This still leaves a 120 degree safe wedge.
inline constexpr float ULDUAR_MIMIRON_BARRAGE_MARGIN = 15.0f * static_cast<float>(M_PI) / 180.0f;

// Beams hit anything inside this whatever the bearing: IsWithinBoundaryRadius skips the bearing test
// within max(bounding radius, MIN_MELEE_REACH 2.0) centre to centre. 2.0 plus a yard.
inline constexpr float ULDUAR_MIMIRON_BARRAGE_BOUNDARY = 3.0f;

// Waypoint velocity on path 13395, clockwise. The 707 yd lap fits a circle 2.7 yd off the room centre,
// so rotating 33576 about the centre stays within a couple of degrees over a barrage.
inline constexpr float ULDUAR_MIMIRON_DB_TARGET_SPEED = 20.8988f;

// How far the barrage read looks for the DB Target, which laps the whole room.
inline constexpr float ULDUAR_MIMIRON_DB_TARGET_SEARCH_RANGE = 250.0f;

// 63274 runs 10 s once 63414's single tick starts it. Only used while Spinning Up, when the barrage
// aura does not exist yet to be read; from ignition on the live duration is preferred.
inline constexpr float ULDUAR_MIMIRON_BARRAGE_FIRE_SECONDS = 10.0f;

// Bounded steps: creatures aren't in the navmesh, so a long chord walks through VX-001 and crosses
// every bearing the cone covers. A 40 degree chord stays within 6% of the ring radius.
inline constexpr float ULDUAR_MIMIRON_BARRAGE_STEP = 40.0f * static_cast<float>(M_PI) / 180.0f;

// Added to VX-001's combat reach (8) to get the smallest ring a bot may orbit on. Melee sit inside
// that, and an orbit at their own radius runs through the model.
inline constexpr float ULDUAR_MIMIRON_BARRAGE_RING_MARGIN = 6.0f;

// Melee orbit at reach 8 + 1.0 = 9.0, a yard under GetMeleeRange's 10.83 (or the chassis chases and
// drags the cone apex) and reach melee's 10.25 (or the charge guard vetoes it every tick).
inline constexpr float ULDUAR_MIMIRON_BARRAGE_MELEE_RING_MARGIN = 1.0f;

// How far in the fire shift may pull a melee bot off that ring. Inside 5 the orbit runs through
// VX-001's model, and a 40 degree step on a 5 yd ring is 3.5 yd of travel, which is still a move.
inline constexpr float ULDUAR_MIMIRON_BARRAGE_MELEE_RING_MIN = 5.0f;

// Radius shifts off the orbit, nearest first, for stepping round fire at a given bearing.
inline constexpr float ULDUAR_MIMIRON_BARRAGE_RADIUS_SHIFTS[] = {0.0f, 2.0f, -2.0f, 4.0f, -4.0f, 6.0f, -6.0f, 8.0f,
                                                                 -8.0f};

// Spare time a bot inside the ignition cone needs to leave by the trailing side before the beams
// light: two beam ticks for the leg to start and land.
inline constexpr float ULDUAR_MIMIRON_BARRAGE_IGNITION_BUFFER = 0.5f;

// Bearing step when looking inside the safe sector for ground that isn't burning. Under 2 yd at the
// ranged ring, against a 5 yd flame radius.
inline constexpr float ULDUAR_MIMIRON_BARRAGE_REFUGE_PROBE = 5.0f * static_cast<float>(M_PI) / 180.0f;

// Inside this, a worst-case 52 degree turn at 7 yd/s fits the 4 s Spinning Up with 1.6x to spare. Past
// 38 yd a bot can't out-turn the 10.6 deg/s sweep at all.
inline constexpr float ULDUAR_MIMIRON_SPREAD_RADIUS_MAX = 24.0f;

// Ranged ring, kept inside ULDUAR_MIMIRON_SPREAD_RADIUS_MAX so a barrage rotation from it still
// fits the Spinning Up window. The tolerance is what stops bots pacing over a yard of drift.
inline constexpr float ULDUAR_MIMIRON_SPREAD_RADIUS = 22.0f;
inline constexpr float ULDUAR_MIMIRON_SPREAD_TOLERANCE = 5.0f;

// A re-aimed Firefighter wedge holds its centreline this long: one re-aim per barrage, or a second
// look in the same Spinning Up walks the raid twice for one cast.
inline constexpr uint32 ULDUAR_MIMIRON_WEDGE_REAIM_MS = 20000;

// The phase 3 wedge's slide holds until a slot leaves range or the slide it needs moves this far.
// Under the slot tolerance, so one step never walks a bot off its own slot.
inline constexpr float ULDUAR_MIMIRON_SHIFT_SLACK = 4.0f;

// The outermost wedge row sits this far inside SpellDistance, which caps the row count. A slot past
// range never self-corrects: reach spell is ACTION_HIGH and the formation's ACTION_RAID wins every tick.
inline constexpr float ULDUAR_MIMIRON_SPREAD_RANGE_MARGIN = 4.0f;

// Everything inside this of the room centre is walkable and flat at Z 364.31. Past it a boss-anchored
// formation hangs over the edge once the MK II has been dragged to the wall.
inline constexpr float ULDUAR_MIMIRON_ROOM_RADIUS = 40.0f;

// Shock Blast 63631 is 15 yd round the caster on a 4 s cast, so 18 clears it. Centre to centre:
// GetDistance2d would take off both combat reaches, 8 and 1.5.
inline constexpr float ULDUAR_MIMIRON_SHOCK_BLAST_SAFE_DIST = 18.0f;
// The escape aims this far past SAFE_DIST, which the screen still tests: a hop landing exactly on the
// line fails it to float noise or a wall, and a fan that fails every bearing leaves the bot in the blast.
inline constexpr float ULDUAR_MIMIRON_SHOCK_FLEE_MARGIN = 2.0f;

// Ranged wedge spacing: Bomb Bots blast 5 yd, so 6 keeps one to a single victim. Must stay above the
// phase 1 disperse distance, or the unstacker shoves every bot off its slot.
inline constexpr float ULDUAR_MIMIRON_PHASE3_SPACING = 6.0f;
inline constexpr float ULDUAR_MIMIRON_PHASE3_MIN_RADIUS = 18.0f;

// Unstacker spacing for ranged while the MK II is up: past Napalm Shell's 5 yd splash, and under
// ULDUAR_MIMIRON_PHASE3_SPACING so it never fires on two bots both on their slots.
inline constexpr float ULDUAR_MIMIRON_DISPERSE_DISTANCE = 5.5f;

// Firefighter phase 1 camp, rows 6 apart for Napalm's 5 yd. First row 21: the MK II stands ~5 yd off
// the tank spot toward the camp. Outer rows sit past Napalm's 24.5 yd floor, so its pool never empties.
inline constexpr float ULDUAR_MIMIRON_PHASE1_CAMP_FIRST_ROW = 21.0f;
inline constexpr float ULDUAR_MIMIRON_PHASE1_CAMP_SPACING = 6.0f;
inline constexpr uint32 ULDUAR_MIMIRON_PHASE1_CAMP_ROWS = 3;

// Sixteen slots in three rows, never closer than 5.5 yd even full. The floor is clean from 288 to 74
// degrees off the tank spot, so a centreline from 326 to 36 keeps the whole wedge on it.
inline constexpr float ULDUAR_MIMIRON_PHASE1_CAMP_HALF_ANGLE = 38.0f * static_cast<float>(M_PI) / 180.0f;

// Camp sight turn: the alcove walls can hide the MK II from edge slots, and a bot that loses sight
// drops its target and walks to its master. Fixed eye height so every bot builds the same camp.
inline constexpr float ULDUAR_MIMIRON_CAMP_SIGHT_STEP = 4.0f * static_cast<float>(M_PI) / 180.0f;
inline constexpr float ULDUAR_MIMIRON_CAMP_SIGHT_MAX = 16.0f * static_cast<float>(M_PI) / 180.0f;
inline constexpr float ULDUAR_MIMIRON_CAMP_SIGHT_EYE = 2.0f;
// Turning back waits this long after the last raise, or the camp swings with every MK II step.
inline constexpr uint32 ULDUAR_MIMIRON_CAMP_SIGHT_HOLD_MS = 15000;

// Threat lead over the highest non-tank the main tank needs before walking the MK II west. With less
// the boss switches to a melee dps mid-walk and stops following.
inline constexpr float ULDUAR_MIMIRON_TANK_THREAT_LEAD = 1.3f;

// Escape hatch for that hold: a threat table that never resolves (a human on the boss, a dead tank)
// must not pin the fight at the pull spot for the rest of the phase.
inline constexpr uint32 ULDUAR_MIMIRON_TANK_HOLD_MAX_MS = 10000;

// Fire nodes this close to a stack anchor count against it. Chains grow toward the nearest player, so
// the camp's own anchor burns first.
inline constexpr float ULDUAR_MIMIRON_STACK_FIRE_RADIUS = 10.0f;

// Move past the limit, only somewhere cleaner by the margin, then hold. Chains grow 1.22 yd/s, so a
// bare "stand on the cleanest" paces the raid across the arc all phase.
inline constexpr uint32 ULDUAR_MIMIRON_STACK_FIRE_LIMIT = 2;
inline constexpr uint32 ULDUAR_MIMIRON_STACK_FIRE_MARGIN = 2;
inline constexpr uint32 ULDUAR_MIMIRON_STACK_HOLD_MS = 15000;

// One Plasma Blast claim: ~9 s from cast start to the last tick, the next cast 22 s on. Shorter than
// the 9 lets a second button in on the same window's tail.
inline constexpr uint32 ULDUAR_MIMIRON_PLASMA_WINDOW_MS = 12000;

// The north-east and south-east add arms leave the centre at 59 degrees, so only a crowded outer row
// reaches one. Any narrower and a 25-man ranged group won't fit inside casting range.
inline constexpr float ULDUAR_MIMIRON_PHASE3_WEDGE_HALF_ANGLE = 60.0f * static_cast<float>(M_PI) / 180.0f;

// Loot range for an Assault Bot corpse, and how close the Magnetic Core has to be used: 64444 places
// its summon by nearest entry, so the bot has to be standing under the Aerial Command Unit.
inline constexpr float ULDUAR_MIMIRON_CORE_LOOT_RANGE = 5.0f;
inline constexpr float ULDUAR_MIMIRON_CORE_USE_RANGE = 12.0f;

// Cores banked before spending, then chained for one unbroken 40 s on the floor. Not fewer add holes:
// every landing is 45 s with no Assault Bot, chained or not. The hold ends once the next one is up.
inline constexpr uint32 ULDUAR_MIMIRON_CORE_BANK = 2;

// Escapes from that hold: item 46029 has duration 60, so a banked core expires, and below the health
// release the phase ends before another core could be spent.
inline constexpr uint32 ULDUAR_MIMIRON_CORE_HOLD_EXPIRY_MS = 10000;
inline constexpr float ULDUAR_MIMIRON_CORE_HOLD_RELEASE_PCT = 25.0f;

// Corpse search covers the whole room, 80 yd across and ~13 s at run speed. A corpse lasts 25 s, so
// the carrier walks to it rather than wait to be standing on one.
inline constexpr float ULDUAR_MIMIRON_CORE_SEARCH_RANGE = 90.0f;

// Range for a core already down. A live 34068 spans the arming, the 20 s landing and the climb back,
// and a second 64436 inside that replaces the first, so the unit climbs with the aura still on.
inline constexpr float ULDUAR_MIMIRON_CORE_PENDING_RANGE = 100.0f;

// A core just used is invisible to that check while it arms: the summon isn't there next tick and its
// 64436 reaches the unit 3.8 to 4.4 s later, so the carrier remembers its own use this long.
inline constexpr uint32 ULDUAR_MIMIRON_CORE_ARM_MS = 6000;

// Phase 4 ends only with all three parts in their 15 s Self Repair at once, so they come down level.
// Bots stop here and wait, since all three share one point and splash lands on every one.
inline constexpr float ULDUAR_MIMIRON_PHASE4_HOLD_PCT = 10.0f;

// Phase 4 focus ranks on percent (the ACU's HealthModifier is 200 against 300) in bands this wide,
// ~2.4 s of raid damage: raw percent flips every bot's target each time two parts cross.
inline constexpr float ULDUAR_MIMIRON_PHASE4_FOCUS_BAND_PCT = 2.0f;

// The others wait this far under the part furthest from death, usually the lagging ACU. Two focus
// bands, so splash doesn't flip the hold. Melee at the floor hit the ACU until the pair is twice this under.
inline constexpr float ULDUAR_MIMIRON_PHASE4_CONVERGE_PCT = 4.0f;

// Phase 4 tank spot under Firefighter: candidates on these rings round the room centre, and the
// centre itself. 24 yd plus the melee ring round the chassis stays inside ULDUAR_MIMIRON_ROOM_RADIUS.
inline constexpr float ULDUAR_MIMIRON_PHASE4_TANK_RINGS[] = {8.0f, 16.0f, 24.0f};
inline constexpr uint32 ULDUAR_MIMIRON_PHASE4_TANK_BEARINGS = 12;

// Fire nodes this close to a candidate count against it: the melee ring round the chassis is ~9 yd
// out and a node burns 5 yd.
inline constexpr float ULDUAR_MIMIRON_PHASE4_TANK_CLEAR_RADIUS = 14.0f;

// Drag past the limit, only somewhere cleaner by the margin, then hold. A drag walks the chassis, the
// barrage apex and the wedge anchored on VX-001, so a few nodes at the circle's edge aren't worth one.
inline constexpr uint32 ULDUAR_MIMIRON_PHASE4_TANK_FIRE_LIMIT = 3;
inline constexpr uint32 ULDUAR_MIMIRON_PHASE4_TANK_FIRE_MARGIN = 2;
inline constexpr uint32 ULDUAR_MIMIRON_PHASE4_TANK_HOLD_MS = 30000;

// Lust waits this long into phase 4: past the opening Frost Bomb's fuse, so the raid is back in place,
// and the first barrage still falls inside the 40 s.
inline constexpr uint32 ULDUAR_MIMIRON_PHASE4_LUST_DELAY_MS = 15000;

// How far a grid scan looks for a mech that is not attackable yet. The MK II parks 58 yd off centre
// between phases and a ranged bot can be another 40 out on top of that.
inline constexpr float ULDUAR_MIMIRON_STAGING_SEARCH_RANGE = 200.0f;

// Mines fire inside 1.9 yd and blast 3 (66351). Ten land within 15 yd per Shock Blast, so a wide avoid
// radius leaves no clear ground and the raid paces: tight on purpose, the odd blast is cheaper.
inline constexpr float ULDUAR_MIMIRON_MINE_TRIGGER_RADIUS = 3.0f;
inline constexpr float ULDUAR_MIMIRON_MINE_CLEARANCE = 3.5f;
inline constexpr float ULDUAR_MIMIRON_MINE_MIN_STEP = 2.0f;
inline constexpr float ULDUAR_MIMIRON_MINE_MAX_STEP = 5.0f;
// A formation walk passing closer than this to a mine sets it off: the 1.9 yd poll plus slop. Every
// 500 ms at 7 yd/s, so a walk straight across one is inside it for a poll or two.
inline constexpr float ULDUAR_MIMIRON_MINE_LEG_CLEARANCE = 2.5f;

// Stand-off from a live Rocket Strike marker (5 s fuse, 3 yd blast) for any destination, or a bot that
// dodged walks straight back onto it.
inline constexpr float ULDUAR_MIMIRON_ROCKET_CLEARANCE = 8.0f;

// Where the dodge fires: 63041 is a flat 3.0 yd and a creature caster adds no combat reach, so 5
// covers it with slop. Run 5, stand 8, like the bomb and the siren.
inline constexpr float ULDUAR_MIMIRON_ROCKET_RUN_RADIUS = 5.0f;

// The grid scan behind the rocket and bomb holds looks this far past the hold, which is centre to centre.
inline constexpr float ULDUAR_MIMIRON_HOLD_SEARCH_PAD = 5.0f;

// How far past the clearance reach and follow moves stay held while a marker lives. The dodge stops
// at the clearance, so without a band past it "reach spell" walks the bot straight back on.
inline constexpr float ULDUAR_MIMIRON_ROCKET_HOLD_MARGIN = 4.0f;

// Napalm Shell splashes 5 yd around its target. Bomb Bots blast 5 yd on melee contact (63801) and
// match player run speed, so the extra yard here only buys time for ranged to kill them.
inline constexpr float ULDUAR_MIMIRON_NAPALM_RADIUS = 6.0f;
inline constexpr float ULDUAR_MIMIRON_BOMB_BOT_RADIUS = 8.0f;

// How much ground a Bomb Bot must still have to cover before a snare is worth a global. It runs
// 8.0 yd/s and dies in about three casts, so snaring one already inside this costs more than it buys.
inline constexpr float ULDUAR_MIMIRON_BOMB_BOT_SNARE_MIN_APPROACH = 15.0f;

// Rocket Strike markers and the MK II's cannon, anywhere in the room.
inline constexpr float ULDUAR_MIMIRON_ROOM_SEARCH_RANGE = 100.0f;

// Hand Pulse re-aims every 1.75 s, so the phase 4 spread is only about not bunching behind its target.
inline constexpr float ULDUAR_MIMIRON_PHASE4_DISPERSE_DISTANCE = 4.0f;

// Raid-wide scan interval. Equals the barrage damage tick.
inline constexpr uint32 ULDUAR_MIMIRON_SCAN_INTERVAL_MS = 250;

// Proximity Mines are non-selectable, so they never reach "possible targets" and pathing knows
// nothing about them. Movement actions check their intended destination through this first.
bool IsMimironSpotMineSafe(Player* bot, Position const& dest,
                           float clearance = ULDUAR_MIMIRON_MINE_CLEARANCE);

// Firefighter hazards gathered in one pass, empty when hard mode is off. The fan tests up to eleven
// bearings, and a scan per bearing would walk the whole fire field eleven times for one dodge.
struct MimironFirefighterHazards
{
    std::vector<Position> flames;
    std::vector<Position> bombs;
    std::vector<Position> fireBots;  // orientation is where the Water Spray line points
    // Origin and facing of every line a spray can take next: each bot's live line, plus the one toward
    // its nearest spread flame, which is where the script aims the next spray.
    std::vector<Position> sprayLanes;
};

MimironFirefighterHazards GetMimironFirefighterHazards(PlayerbotAI* botAI);

// The bomb test screens at ULDUAR_MIMIRON_FROST_BOMB_STAND_RADIUS, past where the flee trigger lets go.
bool IsMimironSpotFireSafe(MimironFirefighterHazards const& hazards, Position const& dest);
bool IsMimironSpotBombSafe(MimironFirefighterHazards const& hazards, Position const& dest);

// Out of every spray lane, and for a 25-man caster or healer `sirenRadius` off each fire bot. The bot's
// own spot takes the FIREBOT_SIREN_CLEARANCE and SPRAY_HALF_WIDTH pair, a destination the _STAND pair.
bool IsMimironSpotFireBotSafe(Player* bot, MimironFirefighterHazards const& hazards, Position const& dest,
                              float sirenRadius, float sprayHalfWidth);
bool IsMimironSpotInFireBotSpray(MimironFirefighterHazards const& hazards, Position const& dest,
                                 float halfWidth);

// Fire bots the raid spares so they keep putting the fire out: every living one in phase 3 until the
// cleanup, empty otherwise. Raid-wide, so every bot spares the same ones.
std::vector<ObjectGuid> GetMimironKeptFireBots(PlayerbotAI* botAI, Player* bot);
bool IsMimironFireBotProtected(PlayerbotAI* botAI, Player* bot, Unit* fireBot);

// Outside a Shock Blast being cast. The escape hands the tick back once out, so every other mover has
// to refuse the circle for the 4 s cast or it walks the bot back in.
bool IsMimironSpotShockSafe(PlayerbotAI* botAI, Position const& dest);

// Whether `dest` is ULDUAR_MIMIRON_ROCKET_CLEARANCE clear of every live Rocket Strike marker. For
// moves that screen the fire themselves and still have to miss the 5000000 blast.
bool IsMimironSpotRocketSafe(PlayerbotAI* botAI, Position const& dest);

// For anywhere a bot is told to stand: mines, live Rocket Strike markers, a Shock Blast being cast, and
// in hard mode fire, the Frost Bomb and fire bots. Else a bot that just dodged walks straight back in.
bool IsMimironSpotSafe(Player* bot, Position const& dest);

// The straight walk to `dest` stays out of fire, or the dodge fires halfway and throws the bot out the
// far side. Nodes the bot already stands in don't count: walking out of those is the point.
bool IsMimironWalkFireSafe(Player* bot, MimironFirefighterHazards const& hazards, Position const& dest);

// One leg the formation can send a bot on toward its slot.
struct MimironApproach
{
    Position dest;
};

// Formation legs toward the slot, best first; empty when nothing gets there or the bot already stands
// clear within the substitute radius. Every leg is cone-screened with its own travel time.
std::vector<MimironApproach> GetMimironSlotApproaches(PlayerbotAI* botAI, Player* bot, Position const& slot,
                                                      MimironFirefighterHazards const& hazards);

// The main tank's phase 1 and 4 slot holds the boss, so it's never refused: the MK II parks on the mines
// it just laid, and a tank that won't stand in them never brings it back.
bool IsMimironTankAnchorSlot(PlayerbotAI* botAI, Player* bot);

// First caller owns the Plasma Blast window being cast and everyone else stands down: a Shield Wall
// or a Pain Suppression carries it alone, and the second is wasted.
bool ClaimMimironPlasmaWindow(Player* bot);

// The main tank has the threat lead to start the walk west. Latches per pull: a mid-phase dip must not
// restart the drag with the raid already spread out behind him.
bool IsMimironTankDragReady(PlayerbotAI* botAI, Player* bot);

// Which ULDUAR_MIMIRON_PHASE1_STACK_SPOTS entry the camp points at. Per instance, not per bot: twelve
// bots each picking their own cleanest anchor is twelve camps.
Position const& GetMimironPhase1StackAnchor(PlayerbotAI* botAI, Player* bot);

// Drops the phase 1 latches. Called off the constructs rather than any one bot's combat state, so a
// wipe re-arms the tank hold and the stack goes back to its default anchor for the next pull.
void ResetMimironFightState(Player* bot);

// Only the fire dodge knows where the fire is. Stamping its escape lets the other movers hold still for
// ULDUAR_MIMIRON_FLAMES_HOLD_MS rather than undo it.
void NoteMimironFireDodge(Player* bot);
bool IsMimironFireHoldActive(Player* bot);

// A spot within `range` of `target` that isn't burning; false when in range, when the straight approach
// is clear or when nothing is. Endpoint only: walking through a node costs a tick, standing in one the phase.
bool GetMimironTargetApproach(PlayerbotAI* botAI, Player* bot, Unit* target, float range,
                              Position& out);

// The range the bot's own reach node would stop at, so the two agree on what "in range" means.
float GetMimironApproachRange(PlayerbotAI* botAI, Player* bot);

// Phase 1 unstacker spacing, read by both the node and its trigger: the trigger re-arms until it reads
// back what the action wrote, so a literal on either side wins every tick at ACTION_RAID.
float GetMimironPhase1DisperseDistance(PlayerbotAI* botAI);

// The 20 s a Magnetic Core buys: the ACU on the floor, passive, taking +50%, and no add for ~45 s. The
// only stretch of phase 3 it can be killed in.
bool IsMimironAcuGrounded(PlayerbotAI* botAI);

// Phase 3 with the unit in the air. Nothing melee can reach it then, and walking toward it only drags
// it, since it holds 30 yd from whoever it is on.
bool IsMimironAcuAirborne(PlayerbotAI* botAI, Player* bot);

// An Assault Bot corpse that still has its Magnetic Core: one core per corpse, taken off the corpse's
// own loot when the loot was filled, so a player who looted it first leaves nothing for the bot.
Creature* GetMimironCoreCorpse(Player* bot);

// The carrier has room in its bank and such a corpse is waiting: the 25 s fetch is on.
bool IsMimironCoreFetchDue(PlayerbotAI* botAI, Player* bot);

// Moves that corpse's core into the bot's bags and marks the corpse spent. False on full bags.
bool TakeMimironCore(Player* bot, Creature* corpse);

// Whether a core may go down now: the unit is in the air and no earlier core is still live or arming.
bool IsMimironCoreUseReady(PlayerbotAI* botAI, Player* bot);

// Starts that arming window. Held per instance, so a new carrier picking the next core up still sees
// the one its predecessor sent down.
void NoteMimironCoreSpent(Player* bot);

// Ranged snare for a Bomb Bot, or empty. Trigger and action both read it or they disagree on who's
// covered. No roots: HandleBreakableCCAuraProc breaks them on damage, and hitting it is the plan.
std::string GetMimironBombBotSnare(Player* bot);

// The Bomb Bot's distance to its own victim, not the caster, so nobody snares one already on a healer.
// The caster's own distance until it picks a victim.
float GetMimironBombBotApproach(Player* bot, Unit* bombBot);

// It runs 8.0 yd/s against a player's 7.0, so its victim shoots it and everyone else in the blast steps
// out. The DPS list and the sidestep both read this, or a ranged bot inside 8 yd does neither.
Unit* GetMimironBombBotChasing(PlayerbotAI* botAI, Player* bot);

// The mech the ranged formation is shaped around. Phase order is MK II, VX-001, Aerial Command Unit,
// then all three together, and VX-001 is the one that stays parked once they reassemble.
Unit* GetMimironRingFocus(PlayerbotAI* botAI);

// The next mech during a handover, when they all carry UNIT_FLAG_NOT_SELECTABLE and only a grid scan
// sees them. Tells a phase 4 handover apart, which puts the main tank on the chassis spot early.
Unit* GetMimironStagingFocus(Player* bot);

// Cheap screen before a grid scan: the hard mode switch is a config read that holds all over Ulduar.
// Bounded by the staging search range, not the room, so it never cuts a bot out of the fight.
bool IsNearMimironRoom(Player* bot);

// The encounter's tick: the fight state reset after a wipe. Throttled per instance, so every bot may
// call it every tick.
void MimironTick(PlayerbotAI* botAI);

// Any of the three constructs actually in combat, not merely present.
bool IsMimironEngaged(PlayerbotAI* botAI);

// Keyed on VX-001 riding the chassis, not all three attackable: a part under 15000 health goes
// NON_ATTACKABLE for Self Repair, which is when the phase is most time-critical.
bool IsMimironPhase4(Player* bot);

// Phase 1: the MK II is up and neither later construct is. Phase 4 excluded by the vehicle seat,
// since a part self-repairing drops off the target list and leaves the MK II looking alone.
bool MimironPhase1Active(PlayerbotAI* botAI);

// Phase 2: VX-001 on the floor on its own. Riding the chassis is phase 4.
bool IsMimironPhase2(PlayerbotAI* botAI);

// Members at or under LowHealth before the phase 2 raid cooldowns may go. Their shared trigger, 6 at 65%,
// fires on the walk-in Rapid Burst and leaves them on cooldown for the storm after the first barrage.
inline constexpr uint32 ULDUAR_MIMIRON_STORM_COOLDOWN_LOW_COUNT = 6;

// Phase 3: the Aerial Command Unit up on its own. "possible targets no los" drops unselectable units,
// so the idle MK II and VX-001 are not in it and the unit only is once it can be attacked.
bool IsMimironPhase3(PlayerbotAI* botAI);

// Hard mode phase 3 Frost Resistance Aura paladin, or nullptr. Never the fire carrier; a tank first,
// since it stands central, then a non-healer, then a healer. A lone paladin keeps fire.
Player* GetMimironFrostResistancePaladin(PlayerbotAI* botAI, Player* bot);

// Phase 4 target; nullptr means hold, everything allowed is at the floor. `melee` is a parameter so the
// pet node can ask for a melee answer on a hunter's behalf.
Unit* GetMimironPhase4Focus(PlayerbotAI* botAI, Player* bot, bool melee);

// The main tank's phase 4 target: the MK II, the one part that swings and walks after its victim, so
// "lose aggro" can see it. The shared focus only while the MK II is self-repairing. Never a hold.
Unit* GetMimironPhase4TankFocus(PlayerbotAI* botAI, Player* bot);

// Core carrier. Group order so every bot agrees, melee first: they already stand on the Assault Bot
// when it dies, where a ranged bot 22 yd out never comes within loot range.
Player* GetMimironCoreCarrier(PlayerbotAI* botAI);

// Seconds to ignition, or -1. Spinning Up is a 4 s channel that leaves no aura on VX-001, so HasAura
// never sees it: poll FindCurrentSpellBySpellId like the boss script does.
float GetMimironSpinningUpSeconds(Unit* vx001);

// The barrage cone as it will be, worked out live. The centreline is the bearing to 33576, but that
// moves 42.6 degrees during Spinning Up, so the cone ignites well clockwise of the boss's facing.
struct MimironBarrageWindow
{
    bool valid = false;
    float lead = 0.0f;       // world bearing of the centreline at ignition, or right now if firing
    float sweep = 0.0f;      // clockwise radians still to come, below one full turn
    float rate = 0.0f;       // radians per second, 0 while still spinning up
    float untilLive = 0.0f;  // seconds until the first damage tick, 0 once firing
};

// Live every tick, so a moving apex and a rotating chassis fall out for free, and every bot derives
// the same cone without coordinating.
MimironBarrageWindow GetMimironBarrageWindow(Player* bot, Unit* vx001);

// Clear of the sweep `travelSeconds` from now, since a FORCED leg holds the lock while the cone turns.
// Clockwise from ignition, never folded: the band is 240 degrees wide. Takes the window so a fan shares one scan.
bool IsMimironSpotBarrageSafe(Unit* vx001, MimironBarrageWindow const& window, Position const& dest,
                              float travelSeconds);

// This bot's formation slot; false for roles it doesn't place, and for everyone in a handover bar the
// phase 4 main tank. Trigger and action must both call this or they disagree where the bot belongs.
bool GetMimironSpreadSlot(PlayerbotAI* botAI, Player* bot, Position& out);

// The cannon's Plasma Blast cast, or nullptr. Matches both ids: spelldifficulty_dbc remaps 62997 to
// 64529 on 25-man, so the script's id never matches a 25-man cast.
Spell* GetMimironPlasmaBlastCast(Unit* cannon);

// The live Rapid Burst cone, invalid when nothing fires. Centreline from the 63382 carrier, not VX-001's
// orientation, which is only the last thing the server wrote.
struct MimironRapidBurstWindow
{
    bool valid = false;
    float centreline = 0.0f;  // world bearing from VX-001 to the aura carrier
};

MimironRapidBurstWindow GetMimironRapidBurstWindow(PlayerbotAI* botAI, Player* bot, Unit* vx001);

// Whether `dest` is outside the cone as it is pointing now, the true 60 degrees with no margin.
bool IsMimironSpotRapidBurstSafe(Unit* vx001, MimironRapidBurstWindow const& window,
                                 Position const& dest);

// Hard mode phase 2 bearings where this bot shares every Rapid Burst aimed at the other group: their
// sector widened by the cone's half-width. Invalid elsewhere.
struct MimironBurstSector
{
    bool valid = false;
    float centre = 0.0f;  // world bearing from VX-001
    float halfWidth = 0.0f;
};

MimironBurstSector GetMimironOtherGroupSector(PlayerbotAI* botAI, Player* bot, Unit* vx001);

bool IsMimironSpotInSector(Unit* vx001, MimironBurstSector const& sector, Position const& dest);

// 64566 reaches 3 yd, plus the node footprint and pathing slop. Chains grow in 7 yd steps, so a
// destination clears every known node, not just the nearest: a short hop lands on the next one.
inline constexpr float ULDUAR_MIMIRON_FLAMES_RADIUS = 5.0f;

// Stand-in search for a spot that won't do: bearings alternate either side of the preferred one, and the
// slot search widens in rings this far apart.
inline constexpr uint32 ULDUAR_MIMIRON_STAND_IN_BEARINGS = 12;
inline constexpr float ULDUAR_MIMIRON_STAND_IN_BEARING_STEP = static_cast<float>(M_PI) / 6.0f;
inline constexpr float ULDUAR_MIMIRON_STAND_IN_RING_STEP = 2.0f;

// A bot already on clear ground this close to an unclear slot stays put: a substitute moves every time
// the fire grows.
inline constexpr float ULDUAR_MIMIRON_SLOT_SUBSTITUTE_RADIUS = 6.0f;

// The substitute search outside phase 1, where a 6 yd ring round a burning slot burns too. The wedge
// sits 17 to 27 yd out against a 35 yd cast range, and candidates keep the disperse distance.
inline constexpr float ULDUAR_MIMIRON_SLOT_SUBSTITUTE_RADIUS_WIDE = 12.0f;

// Detour waypoint turns off the direct bearing, degrees, tried in order. The waypoint sits over the
// walk's midpoint, so 65 makes the walk 2.4 times as long. Past that it is walking away.
inline constexpr float ULDUAR_MIMIRON_DETOUR_TURNS_DEG[] = {20.0f, 35.0f, 50.0f, 65.0f};

// Fire clearance to eat or drink. Drinking pushes the next AI check back 12 to 18 s, and a chain grows
// 1.22 yd/s toward the nearest player, usually the one sitting still.
inline constexpr float ULDUAR_MIMIRON_DRINK_FIRE_CLEARANCE = 15.0f;

// Frost Bomb Explosion 65333: 30 yd, 47124 base and a knockback, about twice a bot's pool. Summons on a
// burning flame node and detonates 10 s later, the whole warning.
inline constexpr float ULDUAR_MIMIRON_FROST_BOMB_RADIUS = 30.0f;
// Where a destination has to be: the flee trigger's FindNearestCreature adds both bounding radii and
// lets go near 31, so a spot at 30.5 flees again next tick.
inline constexpr float ULDUAR_MIMIRON_FROST_BOMB_STAND_RADIUS = 32.0f;
// Where to stand rather than what the blast reaches: the extra clears the knockback and the yard or
// two a leg overshoots by.
inline constexpr float ULDUAR_MIMIRON_FROST_BOMB_CLEARANCE = 34.0f;
// Reach moves stay held this far past the clearance while a bomb lives, or reach spell or a heal reach
// walks the fled bot straight back in for the rest of the fuse.
inline constexpr float ULDUAR_MIMIRON_FROST_BOMB_HOLD_MARGIN = 4.0f;

// Fire bots are spared through phase 3 to put the fire out, then all go on the kill list at this ACU
// health. None may reach phase 4, where they spray straight into the rendezvous.
inline constexpr float ULDUAR_MIMIRON_FIREBOT_CLEANUP_PCT = 15.0f;
// Junk Bots up before melee leave the Assault Bot for them: one arrives every 10 s and nothing else
// kills them. Ranged keep the Assault Bot, the only Magnetic Core source.
inline constexpr uint32 ULDUAR_MIMIRON_JUNK_BOT_PILE = 4;

// Water Spray is SPELL_ATTR0_CU_CONE_LINE: 15 yd ahead, both object sizes wide, ~2.3 yd each side. The
// extra covers a step's overshoot and the bot turning to its next flame.
inline constexpr float ULDUAR_MIMIRON_FIREBOT_SPRAY_LENGTH = 16.0f;
inline constexpr float ULDUAR_MIMIRON_FIREBOT_SPRAY_HALF_WIDTH = 3.5f;
// Destinations stay this far off a lane; the sidestep lands at the run width plus 1.5. A lane jumps when
// its flame goes out, so without the gap a slot just outside the run width flips with the dodge.
inline constexpr float ULDUAR_MIMIRON_FIREBOT_SPRAY_STAND_HALF_WIDTH = 4.5f;
inline constexpr float ULDUAR_MIMIRON_FIREBOT_SPRAY_SIDESTEP = 1.5f;
// Script reach: within this of its nearest flame a fire bot sprays at once, else it walks to this far
// short and sprays on arrival, facing the way it walked. That makes the next lane predictable.
inline constexpr float ULDUAR_MIMIRON_FIREBOT_SPRAY_REACH = 5.0f;
// Deafening Siren 64616 is a 10 yd area silence, on the 25-man bot only (creature_template_addon), and
// area auras add both object sizes to the radius.
inline constexpr float ULDUAR_MIMIRON_FIREBOT_SIREN_CLEARANCE = 13.0f;
// Where a caster's destination has to be, and where the siren dodge aims. Kept fire bots walk to the
// next flame, usually in the raid, so a spot just past 13 is back inside a moment later.
inline constexpr float ULDUAR_MIMIRON_FIREBOT_SIREN_STAND = 17.0f;
inline constexpr float ULDUAR_MIMIRON_FIREBOT_SIREN_FLEE = 19.0f;
// Damage AoE is held while a kept fire bot is this close to the bot or its target. The widest raid AoE
// is about 10 yd (Death and Decay, Hurricane, Blizzard), so 12 covers the splash that reaches one.
inline constexpr float ULDUAR_MIMIRON_FIREBOT_AOE_CLEARANCE = 12.0f;

// Above this, fire is cheaper than the trip: a node ticks ~3100 against a 22000 to 24000 pool, seven
// ticks of margin, and the round trip is 24 yd of uptime or a cast.
inline constexpr float ULDUAR_MIMIRON_FLAMES_DODGE_HEALTH_PCT = 60.0f;

// How many nodes overrule the health gate. One node is about 8 s of margin from full; two is 4, and
// four seconds is not enough to notice a health bar moving and then walk 12 yd.
inline constexpr uint32 ULDUAR_MIMIRON_FLAMES_DODGE_NODE_OVERRIDE = 2;

// Generic movers stay out this long after a fire dodge, since they don't know the floor burns. Long
// enough to stop the walk back in, short enough to strand nobody while fire covers a third of the floor.
inline constexpr uint32 ULDUAR_MIMIRON_FLAMES_HOLD_MS = 2500;

// A bot closing on a target parks this far inside its stop distance: on the edge, one shuffle leaves
// it a step short, and that step is another walk through the fire.
inline constexpr float ULDUAR_MIMIRON_APPROACH_INSET = 2.0f;

// Hops tried past the burning cluster's edge. Rungs under the 7 yd chain step are safe only because
// each is screened against the whole field, and a short one gives uptime back.
inline constexpr float ULDUAR_MIMIRON_FLAMES_STEP_LADDER[] = {3.0f, 5.0f, 7.0f, 10.0f};

// Longest fire dodge leg. It's FORCED and holds the lock against the other dodges, 12 yd being ~1.7 s;
// a shortened hop that still lands in fire is refused by the fan like any other.
inline constexpr float ULDUAR_MIMIRON_FLAMES_MAX_HOP = 12.0f;

// Flee fan bearings off straight away, out to 112.5 degrees: two stacked filters can empty the first
// quadrant, and past ~120 the geometry turns back inward.
inline constexpr float ULDUAR_MIMIRON_FLEE_FAN_STEP = static_cast<float>(M_PI) / 8.0f;
inline constexpr float ULDUAR_MIMIRON_FLEE_FAN_MAX = 5.0f * ULDUAR_MIMIRON_FLEE_FAN_STEP;

// Least share of the asked hop a bearing must cover: on PATHFIND_INCOMPLETE CheckCollisionAndGetValidCoords
// still succeeds with the destination at the bot's feet, which the back-tracking test passes.
inline constexpr float ULDUAR_MIMIRON_FLEE_MIN_PROGRESS_PCT = 0.5f;

// Rapid Burst 64531/64532 is a 60 degree cone, 100 yd deep. The spell_cone row says so and is read
// before the DBC's 104 default, but that table is wrong for the barrage, so this one was measured too.
inline constexpr float ULDUAR_MIMIRON_RAPID_BURST_HALF_ANGLE = 30.0f * static_cast<float>(M_PI) / 180.0f;

// Hard mode phase 2 melee sector, opposite the ranged wedge. ~32 degrees of slot slack puts a bot 77
// off centre at worst, still clear of the wedge's 60 plus the Rapid Burst cone's 30.
inline constexpr float ULDUAR_MIMIRON_PHASE2_MELEE_HALF_ANGLE = 45.0f * static_cast<float>(M_PI) / 180.0f;

// Added to VX-001's combat reach for the furthest a melee stand-in may sit: 10.0, inside the 10.25
// where "reach melee" fires and walks the bot straight at the boss off its sector.
inline constexpr float ULDUAR_MIMIRON_PHASE2_MELEE_REACH_MARGIN = 2.0f;

// VX-001 fights here and the Aerial Command Unit is summoned overhead, so a ring anchored to this
// point holds still while the mechs turn and charge about.
extern const Position ULDUAR_MIMIRON_ROOM_CENTER;
// Phase 3 staging, 18 yd east of the room centre. The add pads sit on the west, north-east and
// south-east arms, so east is the one stretch of floor nothing walks down.
extern const Position ULDUAR_MIMIRON_PHASE3_STAGE;
extern const Position ULDUAR_MIMIRON_PHASE4_TANK_SPOT;

// Firefighter phase 1, 53 yd west of centre, so the fire seeded round the raid stays off VX-001's floor.
// 51.5 yd from Mimiron's spawn, and he evades past 80: the encounter's only leash.
extern const Position ULDUAR_MIMIRON_PHASE1_TANK_SPOT;

// Phase 1 camp anchors: middle-row centres 27 yd off the tank spot at 25 and 345 degrees, 0 the default.
// 18.5 yd apart to clear a node cluster plus a chain step; wider puts edge slots in the alcove's shadow.
inline constexpr uint8 ULDUAR_MIMIRON_PHASE1_STACK_COUNT = 2;
extern const Position ULDUAR_MIMIRON_PHASE1_STACK_SPOTS[ULDUAR_MIMIRON_PHASE1_STACK_COUNT];

#endif
