/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERTHORIM_H
#define PLAYERBOTS_ULDENCOUNTERTHORIM_H

#include "ObjectGuid.h"
#include "Position.h"
#include "UldData.h"

#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;
class WorldObject;

enum UlduarThorimIds
{
    // Thorim
    NPC_DARK_RUNE_ACOLYTE_I = 32886,
    NPC_CAPTURED_MERCENARY_SOLDIER_ALLY = 32885,
    NPC_CAPTURED_MERCENARY_SOLDIER_HORDE = 32883,
    NPC_CAPTURED_MERCENARY_CAPTAIN_ALLY = 32908,
    NPC_CAPTURED_MERCENARY_CAPTAIN_HORDE = 32907,
    NPC_JORMUNGAR_BEHEMOT = 32882,
    NPC_DARK_RUNE_WARBRINGER = 32877,
    NPC_DARK_RUNE_EVOKER = 32878,
    NPC_DARK_RUNE_CHAMPION = 32876,
    NPC_DARK_RUNE_COMMONER = 32904,
    NPC_IRON_RING_GUARD = 32874,
    NPC_RUNIC_COLOSSUS = 32872,
    NPC_ANCIENT_RUNE_GIANT = 32873,
    NPC_DARK_RUNE_ACOLYTE_G = 33110,
    NPC_IRON_HONOR_GUARD = 32875,
    NPC_THORIM_THUNDER_ORB = 33378,  // the pillar orb that lights up 5s before Lightning Charge
    SPELL_UNBALANCING_STRIKE = 62130,
    // On Thorim, one stack per Lightning Charge: +15% damage and melee haste each.
    SPELL_THORIM_LIGHTNING_CHARGE_BUFF = 62279,

    // Corridor smash, one side per 5s cast. Cancelled once the Colossus is engaged.
    SPELL_THORIM_RUNIC_SMASH_LEFT = 62057,
    SPELL_THORIM_RUNIC_SMASH_RIGHT = 62058,
    // 2000 arcane back on every melee swing. Recast every 20s for 20s, so effectively always up.
    SPELL_THORIM_RUNIC_BARRIER = 62338,
    // Only warning before Lightning Charge, which has no cast bar. SpellInfoCorrections sets its amplitude to
    // 5000ms, so it ticks once, 5s before the cone.
    SPELL_THORIM_LIGHTNING_ORB_VISUAL = 62186,
    // Phase 1 orb marker: 15s on one Thunder Orb, Lightning Shock (62017) every second, ~3k nature at 35 yd in 3D.
    SPELL_THORIM_CHARGE_ORB = 62016,

    // Thorim hard mode (arena gauntlet cleared fast enough that Sif joins the fight).
    NPC_SIF = 33196,              // spawns at Thorim's throne, drops into the arena when she joins
    NPC_SIF_BLIZZARD = 32879,     // walks the arena dropping the zones below, only ever exists in hard mode
    // The zones do the damage, not the bunny: 10s, 8 yd, one every 2s, so up to six trail behind it.
    SPELL_SIF_BLIZZARD_ZONE_10 = 62576,
    SPELL_SIF_BLIZZARD_ZONE_25 = 62602,
};

inline constexpr float ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD = 429.6094f;
inline constexpr float ULDUAR_THORIM_AXIS_Z_PATHING_ISSUE_DETECT = 410.0f;

// Trigger radius for the generic flee off the bunny, not a damage radius. Wide on purpose so it fires early.
inline constexpr float ULDUAR_THORIM_SIF_BLIZZARD_RADIUS = 15.0f;

// How far the ring and camp keep off a zone. Damage stops at 9.8 yd (corrected DBC 8 plus both combat
// reaches), so 11 leaves a yard of slack.
inline constexpr float ULDUAR_THORIM_RING_BLIZZARD_CLEARANCE = 11.0f;

// How far out the Blizzard sweep looks. Every bot reads whichever bot's sweep filled the cache last, so
// it has to reach the whole trail from anywhere in the arena: the bunny's path alone spans 61 by 48 yd.
inline constexpr float ULDUAR_THORIM_BLIZZARD_SCAN_RANGE = 70.0f;

// How far a camp bot steps off a zone, and how far from Thorim it may end up. Short so the step can't run
// along the bunny's path into the cone. 32 is the boss range the camp table was solved to.
inline constexpr float ULDUAR_THORIM_CAMP_BLIZZARD_ESCAPE_RADIUS = 15.0f;
inline constexpr float ULDUAR_THORIM_CAMP_MAX_BOSS_RANGE = 32.0f;

// Ring and bearing steps of the camp's Blizzard sweep.
inline constexpr float ULDUAR_THORIM_SWEEP_STEP = 2.0f;
inline constexpr float ULDUAR_THORIM_SWEEP_ANGLE_STEP = static_cast<float>(M_PI) / 8.0f;

// Arena and corridor, short of the upper hallway.
inline constexpr float ULDUAR_THORIM_FLOOR_RANGE = 110.0f;

// The corridor squad regroups on a master standing this close to the gate, until within the follow distance.
inline constexpr float ULDUAR_THORIM_GATE_RADIUS = 10.0f;
inline constexpr float ULDUAR_THORIM_GATE_FOLLOW_DISTANCE = 5.0f;

// Cheap first gate for both halves. Distance alone reaches Hodir's room, height doesn't: corridor z 412,
// arena 420, Hodir 433.
inline constexpr float ULDUAR_THORIM_ENCOUNTER_PROXIMITY = 200.0f;
inline constexpr float ULDUAR_THORIM_WING_MAX_Z = 425.0f;

// The ramp and upper hallway (z 425-438) overlap Hodir on height and distance alike, so this half is a box.
// X does the work: Hodir sits at x 1980 and nothing else in Ulduar is in here.
inline constexpr float ULDUAR_THORIM_BALCONY_BOX_MIN_X = 2100.0f;
inline constexpr float ULDUAR_THORIM_BALCONY_BOX_MAX_X = 2205.0f;
inline constexpr float ULDUAR_THORIM_BALCONY_BOX_MIN_Y = -455.0f;
inline constexpr float ULDUAR_THORIM_BALCONY_BOX_MAX_Y = -280.0f;
inline constexpr float ULDUAR_THORIM_BALCONY_BOX_MAX_Z = 450.0f;

// Sight-capped target values stop at 100 yd and the Colossus is 131 yd from the top corridor waypoints.
inline constexpr float ULDUAR_THORIM_COLOSSUS_SEARCH_RANGE = 150.0f;

// 5s cast, 1s delay, 3.5s of wave travel, plus slack so the lane doesn't flip back while the last bunny fires.
inline constexpr uint32 ULDUAR_THORIM_RUNIC_SMASH_LATCH_MS = 10000;

// Raid-wide sweeps run once per instance per interval, so no two bots disagree on the hot lane or the lit orb.
inline constexpr uint32 ULDUAR_THORIM_ENCOUNTER_SCAN_INTERVAL_MS = 500;

// Barrier only punishes melee swings, so a health band: step out below the first, back in above the second.
inline constexpr float ULDUAR_THORIM_BARRIER_BAIL_HEALTH_PCT = 55.0f;
inline constexpr float ULDUAR_THORIM_BARRIER_RESUME_HEALTH_PCT = 80.0f;
inline constexpr float ULDUAR_THORIM_BARRIER_BAIL_DISTANCE = 14.0f;  // clear of the 9.1 yd melee reach

// Melee reach is 9.08 yd centre to centre, so about as far out as the ring can sit and still swing. At 8.8 a
// 60 degree slot gap is 8.8 yd, past the 8 yd jump, and one Blizzard forbids 77 degrees of ring, not 87.
inline constexpr float ULDUAR_THORIM_MELEE_RING_RADIUS = 8.8f;
inline constexpr uint8 ULDUAR_THORIM_MELEE_SLOTS = 3;

// Min gap between ring bearings after the cone and Blizzard slides, 8.8 yd at the radius above. Can't be wider:
// a lit cone forbids 105 degrees, and three slots plus the tank anchor need 240 of the 255 left.
inline constexpr float ULDUAR_THORIM_RING_SLOT_SEPARATION = 1.0472f;  // 60 degrees

// How far a pet may drift off its owner's ring bearing before it's sent back, also the chase's own tolerance.
// About 3 yd of arc: keeps it out of the gap between slots without re-pathing every tick.
inline constexpr float ULDUAR_THORIM_PET_SLOT_TOLERANCE = 0.3927f;  // 22.5 degrees

// Chain Lightning jumps 8 yd centre to centre (spell_jump_distance 5 plus both combat reaches). Six is the most
// the room fits 11 yd apart, 22 off the tank spot and 12 off Sif's loop, and 11 holds with a bot off its point.
inline constexpr uint8 ULDUAR_THORIM_RANGED_SLOTS = 6;

// Ranged wait clear of the melee pile until the first Chain Lightning lands, 12.0-12.1s after he drops. Then
// home once he's on the anchor or the lit cone covers the wait spot, never past the max: Sif's bunny comes by.
inline constexpr uint8 ULDUAR_THORIM_OPENING_SLOTS = 4;
inline constexpr uint32 ULDUAR_THORIM_OPENING_HOLD_MIN_MS = 12500;
inline constexpr uint32 ULDUAR_THORIM_OPENING_HOLD_MAX_MS = 25000;
// He settles 4.1-6.1 yd off the tank spot and is 12-23 yd off it while being dragged.
inline constexpr float ULDUAR_THORIM_OPENING_SETTLED_RADIUS = 8.0f;

// Arrival on a tank or camp spot.
inline constexpr float ULDUAR_THORIM_PHASE2_SPOT_TOLERANCE = 1.0f;

// Off-tank sits near the main tank's bearing, in taunt range for the Unbalancing Strike swap. Doesn't split
// the pair for Chain Lightning: 20 degrees at the 4-6 yd they hold off the boss is under 2 yd.
inline constexpr float ULDUAR_THORIM_OFFTANK_BEARING_OFFSET = 0.3491f;  // 20 degrees

// Reach then hold: a tight deadband on a ring that moves with the boss keeps a bot sliding in place, and a
// moving bot casts nothing.
inline constexpr float ULDUAR_THORIM_RING_ARRIVE_TOLERANCE = 3.0f;
inline constexpr float ULDUAR_THORIM_RING_REPOSITION_TOLERANCE = 5.0f;

// Covers the arena (adds land 19-24 yd out, ranged stand on the far side) without reaching down the corridor.
inline constexpr float ULDUAR_THORIM_DPS_TARGET_RANGE = 50.0f;

// How much closer a same-tier add must be to switch, or the pick flips between equidistant adds mid-cast.
inline constexpr float ULDUAR_THORIM_TARGET_SWITCH_MARGIN = 8.0f;

// How far melee go for a higher-tier add before settling for the next tier: past this it has moved twice by
// the time they arrive. Ranged have no limit, they hit an Evoker from where they stand.
inline constexpr float ULDUAR_THORIM_MELEE_TARGET_REACH = 15.0f;

// spell_cone: 75 degrees at 150 yd. Margin covers Thorim turning onto the orb while the bot walks, clearance
// keeps a bot that just stepped out from reading as still inside.
inline constexpr float ULDUAR_THORIM_LIGHTNING_CHARGE_CONE_ANGLE = 1.3090f;   // 75 degrees
inline constexpr float ULDUAR_THORIM_LIGHTNING_CHARGE_MARGIN = 0.2618f;       // 15 degrees
inline constexpr float ULDUAR_THORIM_RING_CONE_CLEARANCE = 0.0873f;           // 5 degrees
inline constexpr float ULDUAR_THORIM_LIGHTNING_CHARGE_RANGE = 150.0f;

// Camp margin, far tighter than the ring's: a yard of boss drift is 7 degrees at 8 yd but 2-4 at 15-32 yd.
// Any wider and no shelter set fits the room.
inline constexpr float ULDUAR_THORIM_LIGHTNING_CHARGE_RANGED_MARGIN = 0.0436f;  // 2.5 degrees

// How close a lit orb has to be to one of the seven known positions before the shelter table trusts the
// match. Nothing within this and the bot stays home rather than walking off a guess.
inline constexpr float ULDUAR_THORIM_THUNDER_ORB_MATCH_RADIUS = 4.0f;

// ThorimAI::GetArenaPlayer()'s box, scanned every 5s for a living player. Nobody inside summons the
// raid-wiping Lightning Orb, so never widen it.
inline constexpr float ULDUAR_THORIM_ARENA_BOX_MIN_X = 2085.0f;
inline constexpr float ULDUAR_THORIM_ARENA_BOX_MAX_X = 2185.0f;
inline constexpr float ULDUAR_THORIM_ARENA_BOX_MIN_Y = -305.0f;
inline constexpr float ULDUAR_THORIM_ARENA_BOX_MAX_Y = -214.0f;
inline constexpr float ULDUAR_THORIM_ARENA_BOX_MAX_Z = 425.0f;

// Covers the add landings (19-24 yd out), well inside the box (42 yd) and short of the corridor mouth (45.8).
inline constexpr float ULDUAR_THORIM_ARENA_LEASH_RADIUS = 30.0f;

// Corridor pets have no room to be held to, so they're held to the owner. Same as the picker's range: a pet
// past it is beyond anything the owner could see to kill.
inline constexpr float ULDUAR_THORIM_PET_OWNER_LEASH_RADIUS = ULDUAR_THORIM_DPS_TARGET_RANGE;

// The gauntlet stops taking bodies once the arena would drop this low.
inline constexpr uint32 ULDUAR_THORIM_ARENA_MIN_MEMBERS = 3;

// Ranged and healers ring the tank outside Champion Whirlwind, as wide as the room allows: adds land from 19 yd
// out and the navmesh gives out past ~26 yd south. Two radii so a 25 man ring isn't shoulder to shoulder.
inline constexpr float ULDUAR_THORIM_ARENA_RING_INNER = 13.0f;
inline constexpr float ULDUAR_THORIM_ARENA_RING_OUTER = 18.0f;
inline constexpr uint8 ULDUAR_THORIM_ARENA_RING_INNER_SLOTS = 5;

// The 35 yd radius is 3D from an orb 13.5 yd up, so the field cuts the floor as a 32.3 yd circle. Margin
// covers a bot still walking when the next tick lands.
inline constexpr float ULDUAR_THORIM_CHARGED_ORB_RADIUS = 32.3f;
inline constexpr float ULDUAR_THORIM_CHARGED_ORB_MARGIN = 4.0f;

// Melee leash off the tank spot: 24 yd is the furthest an add lands, so no uptime lost. Keeps them off the
// lever gate, where CanAIAttack drops any target past x 2180 and the add re-rolls onto a healer.
inline constexpr float ULDUAR_THORIM_ARENA_MELEE_LEASH = 24.0f;

extern const Position ULDUAR_THORIM_NEAR_ARENA_CENTER;
extern const Position ULDUAR_THORIM_NEAR_ENTRANCE_POSITION;
extern const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_1;
extern const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_2;
extern const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_5_YARDS_1;
extern const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_1;
extern const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_2;
extern const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_3;
extern const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_1;
extern const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_2;
extern const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_5_YARDS_1;
extern const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_1;
extern const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_2;
extern const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_3;
// Upper hallway in walk order, off the centre line where two invisible Paralytic Field bunnies sit (12 yd,
// 15s stun). Hugs the east wall, then back to the middle for the north doorway.
extern const Position ULDUAR_THORIM_BALCONY_1;
extern const Position ULDUAR_THORIM_BALCONY_2;
extern const Position ULDUAR_THORIM_BALCONY_3;
extern const Position ULDUAR_THORIM_BALCONY_4;
extern const Position ULDUAR_THORIM_BALCONY_5;
extern const Position ULDUAR_THORIM_JUMP_START_POINT;
extern const Position ULDUAR_THORIM_JUMP_END_POINT;
// Where the corridor squad's ranged wait out the opening, up on his platform and 12 yd apart. Walking
// the hallway they arrive stacked within a yard, which is an eight hop Chain Lightning on the drop.
extern const Position ULDUAR_THORIM_BALCONY_HOLD1_SPOT;
extern const Position ULDUAR_THORIM_BALCONY_HOLD2_SPOT;
extern const Position ULDUAR_THORIM_BALCONY_HOLD3_SPOT;
extern const Position ULDUAR_THORIM_BALCONY_HOLD4_SPOT;
extern const Position ULDUAR_THORIM_BALCONY_HOLD5_SPOT;
extern const Position ULDUAR_THORIM_BALCONY_HOLD6_SPOT;
// Dragged this far west so the melee on him can't bridge Chain Lightning into the camp. The r 8 ring is
// walkable all the way round, the west arc on rim polys.
extern const Position ULDUAR_THORIM_PHASE2_TANK_SPOT;
// 22-32 yd east of the tank spot, 11 yd apart, 12 off Sif's loop. 22 keeps the melee ring from bridging in,
// past 32 the shorter nukes stop reaching. One cone still covers up to four, so covered slots take a shelter.
extern const Position ULDUAR_THORIM_PHASE2_RANGE1_SPOT;
extern const Position ULDUAR_THORIM_PHASE2_RANGE2_SPOT;
extern const Position ULDUAR_THORIM_PHASE2_RANGE3_SPOT;
extern const Position ULDUAR_THORIM_PHASE2_RANGE4_SPOT;
extern const Position ULDUAR_THORIM_PHASE2_RANGE5_SPOT;
extern const Position ULDUAR_THORIM_PHASE2_RANGE6_SPOT;
// Arena ranged wait out the opening here, two to a spot, clear of the melee pile and in range of him on the
// anchor. Opening only: Sif's bunny path runs right by them.
extern const Position ULDUAR_THORIM_PHASE2_OPENING1_SPOT;
extern const Position ULDUAR_THORIM_PHASE2_OPENING2_SPOT;
extern const Position ULDUAR_THORIM_PHASE2_OPENING3_SPOT;
extern const Position ULDUAR_THORIM_PHASE2_OPENING4_SPOT;
// Fallback when his live position gives no ring point. StaticMeleeSpot only takes one within 11 yd of him,
// so these sit on the r 8 ring round the tank spot.
extern const Position ULDUAR_THORIM_PHASE2_MELEE1_SPOT;
extern const Position ULDUAR_THORIM_PHASE2_MELEE2_SPOT;
extern const Position ULDUAR_THORIM_PHASE2_MELEE3_SPOT;
// Inside the melee ring rather than on it, and off the camp's bearing: taunt range for the
// Unbalancing Strike swap without adding a body between the boss and the ranged.
extern const Position ULDUAR_THORIM_PHASE2_OFFTANK_SPOT;

// One per instance: raid-wide answers are folded once per interval, not once per bot.
struct ThorimEncounterState
{
    // Latched: ranking the raid by guid every tick lets one death renumber everyone behind the corpse.
    std::unordered_map<ObjectGuid, uint8> meleeSlots{};

    // Latched too. The shelter table below is keyed on this slot.
    std::unordered_map<ObjectGuid, uint8> rangedSlots{};

    // Opening wait spot per ranged bot: the arena squad's on the floor, the corridor squad's on the
    // platform. Latched like the camp slot, so a death does not reshuffle them.
    std::unordered_map<ObjectGuid, uint8> openingSlots{};
    std::unordered_map<ObjectGuid, uint8> balconyHoldSlots{};

    // One way. A bot that has left its wait spot never goes back, even when the cone goes dark or a
    // tank swap nudges him off the anchor again.
    GuidUnorderedSet openingReleased{};

    // When he first dropped below the floor line in combat. The whole opening is timed off this.
    uint32 phase2StartMs{};

    // Whoever he hit last scan, and whether that hold turn already got its call. See ThorimTickHumanTankCalls.
    ObjectGuid humanCallHolder;
    bool humanCallTurnDone = false;
    uint32 humanCallScanMs = 0;

    // One entry per slot, solved as a set: the three hold ULDUAR_THORIM_RING_SLOT_SEPARATION and everyone in a
    // slot walks to the same point.
    struct RingSlot
    {
        float latched = 0.0f;   // off the anchor, struck once for the phase
        float composed = 0.0f;  // where the cone and Blizzard slides left it
    };

    std::array<RingSlot, ULDUAR_THORIM_MELEE_SLOTS> ringSlots;
    float ringAnchor = 0.0f;
    bool ringLatched = false;
    ObjectGuid ringOrb;
    uint32 ringSolveMs = 0;

    // Per camp bot, whether it's on its shelter and which orb sent it there. Wiped when that orb goes dark, so
    // the walk home is done before the next orb lights and brings its Chain Lightning.
    struct RangedShelter
    {
        ObjectGuid orb;
        uint8 orbIndex = 0;
        bool sheltered = false;
    };

    std::unordered_map<ObjectGuid, RangedShelter> rangedShelters;

    // 0 until seen, else SPELL_THORIM_RUNIC_SMASH_LEFT / _RIGHT. The side is sticky (the lane to walk), the
    // timestamp only says a wave is still rolling.
    uint32 runicSmashSide{};
    uint32 runicSmashSeenMs = 0;
    uint32 smashScanMs = 0;

    // Cached apart from lightningOrbGuid: a shared slot would let each caller evict the other and re-sweep the grid.
    ObjectGuid chargedOrbGuid{};
    uint32 orbScanMs = 0;

    ObjectGuid lightningOrbGuid{};
    uint32 lightningOrbScanMs = 0;

    // Sif's Blizzard zones and bunny, swept once per instance per interval. Zones never move, so the interval
    // only delays seeing a fresh one, and those come every 2s.
    std::vector<Position> blizzardSpots;
    uint32 blizzardScanMs = 0;

    // Where each bot was sent to get out of the Charge Orb field.
    std::unordered_map<ObjectGuid, Position> orbEscapes{};

    // Struck once, then read from the map. Sight-capped lookups stop at 100 yd and the upper hallway is
    // 100-145 yd from the platform.
    ObjectGuid bossGuid;

    ObjectGuid colossusGuid;
    uint32 colossusScanMs = 0;

    ObjectGuid runeGiantGuid;
    uint32 runeGiantScanMs = 0;

    GuidUnorderedSet barrierBailing{};

    // The one bot paladin on Frost Resistance Aura, latched. The shared boss-aura node answers only for the
    // first living paladin in the group, so a human in that spot would lock every bot out.
    ObjectGuid frostAuraPaladin{};
    uint32 frostAuraScanMs = 0;

    // Latched on the first Sif Blizzard of the pull. She only fights in hard mode.
    bool sifJoined{};

    // Which half of the raid each member belongs to, struck once and then left alone. Recomputing it
    // per tick would let a role predicate flipping mid-fight walk the arena squad into the corridor.
    std::unordered_map<ObjectGuid, uint8> squads{};
    bool squadsAssigned{};

    // Humans' squad entries are refreshed from position: the split only picks bots.
    uint32 humanSquadScanMs = 0;

    // Which add each bot was told to burn.
    std::unordered_map<ObjectGuid, ObjectGuid> dpsTargets{};

    // Bots the arena node took "follow master" away from, so the reset can hand it back.
    GuidUnorderedSet followMasterStripped{};

    // How far along the upper hallway each bot has walked. Monotonic: from position alone it'd hand back the
    // nearest waypoint, and the one behind you walks the squad down the corridor again.
    std::unordered_map<ObjectGuid, uint8> balconyStep{};

    // Last pet sent home, keyed by owner.
    std::unordered_map<ObjectGuid, ObjectGuid> petRecalls{};

    // Last recall per pet. MoveFollow every tick restarts the walk so the pet never arrives, and a one-shot
    // latch would miss a pet that strays twice. Keyed by pet so one stray can't mute another.
    std::unordered_map<ObjectGuid, uint32> petRecallMs;

    // Phase 1 arrival latch, kept apart from ringArrived: only the z threshold keeps the two phases apart.
    GuidUnorderedSet arenaAnchorArrived{};

    // Reach-then-hold latch. A bot that has arrived stops issuing moves until it drifts past the
    // wider tolerance, because a moving bot casts nothing.
    GuidUnorderedSet ringArrived{};

    // Raid icons are cleared once per pull rather than every tick: a human is free to re-mark something
    // mid-fight and having the bots wipe it back off would be worse than the stale mark ever was.
    bool marksCleared{};

    // Whether he's ever been engaged this raid. An untouched Thorim looks just reset, and the reset clears the
    // squad split the corridor forms up on before the pull, so without this the two fight every tick.
    bool engagedSeen = false;

    // Bots whose latches were cleared since he was last in combat. Per bot: a raid-wide gate would let the first
    // bot through skip everyone else's reset. Cleared while he's engaged, so every pull gets a fresh round.
    std::unordered_set<ObjectGuid> resetDone;
};

enum class ThorimSquad : uint8
{
    None,
    Arena,
    Gauntlet
};

enum class ThorimPhase2Role : uint8
{
    None,
    MainTank,
    OffTank,
    Ranged,
    MeleeRing
};

// By entry, never "find target": that value walks only the bot's own threat list and matches on the
// localized creature name, so a bot that is on an add goes blind to the boss.
Unit* GetThorim(PlayerbotAI* botAI);

// Not sight-capped: the Colossus is 131 yd from the top corridor waypoints, right where its telegraph matters.
Unit* GetThorimRunicColossus(PlayerbotAI* botAI);

// Target priority

// Bucketed by entry in one sweep, same reason GetThorim reads by entry.
struct ThorimEncounterTargets
{
    std::vector<Unit*> acolytes;
    std::vector<Unit*> evokers;
    std::vector<Unit*> champions;
    std::vector<Unit*> warbringers;
    std::vector<Unit*> commoners;
    std::vector<Unit*> guards;  // Iron Ring and Iron Honor Guard, the corridor's own trash
    std::vector<Unit*> trash;   // the six the raid walks in on, before Thorim starts the waves
    Unit* colossus = nullptr;
    Unit* runeGiant = nullptr;
};

void GatherThorimEncounterTargets(PlayerbotAI* botAI, ThorimEncounterTargets& out);

// False for Thorim and Sif while he's on the balcony: ThorimArenaTargetGuardMultiplier reads a target
// outside the arena box as a reason to stop acting, so a bot holding one does nothing at all.
bool ThorimDpsTargetAllowed(PlayerbotAI* botAI, Unit* target);

// nullptr leaves the generic picker alone. Not a raid icon: RtiTargetValue hands an icon back before the
// smart picker runs and IsHighPriority pins it, so a wrong mark sticks until the bot leaves combat.
Unit* GetThorimDpsTarget(PlayerbotAI* botAI, Player* bot, Unit* currentTarget);

// Reads what GetThorimDpsTarget cached on its last pass, since the targeting guard asks per action per tick.
bool ThorimHasDpsTarget(PlayerbotAI* botAI, Player* bot);

// Drops stale icons and pinned targets. Icons stay until overwritten and "prioritized targets" only resets
// out of combat, so either can hold a bot on a corpse or the balcony all fight. Raid-wide half runs once.
void ThorimClearStaleMarks(PlayerbotAI* botAI, Player* bot);

// Corridor gauntlet
inline constexpr uint8 ULDUAR_THORIM_GAUNTLET_WAYPOINTS = 6;

// Lanes are index-matched by y, so a lane swap keeps the index. Each lane sits inside its own hand's 10 yd
// blast and clear of the other's.
Position const& GetThorimGauntletWaypoint(bool leftLane, uint8 index);

// True when `who` is standing at one of this lane's waypoints, reporting the nearest one.
bool ThorimGauntletLaneIndexInLane(WorldObject const* who, bool leftLane, uint8& index);
bool ThorimGauntletLaneIndex(WorldObject const* who, uint8& index, bool& leftLane);

// The squad's waypoint index: the master's, or this bot's own when the master is on none (centre line, or
// off on an add).
bool ThorimResolveGauntletIndex(PlayerbotAI* botAI, Player* bot, uint8& index);

// Sticky once a hand goes up, or the formation walks back into the lane it just left. False before the
// first telegraph and once the Colossus is engaged, since JustEngagedWith cancels EVENT_RC_RUNIC_SMASH.
bool ThorimPreferredGauntletLane(PlayerbotAI* botAI, bool& useLeftLane);

// True only while a wave is still rolling, which is what separates the urgent dodge from the
// standing lane preference.
bool ThorimRunicSmashImminent(PlayerbotAI* botAI);

// Upper hallway
inline constexpr uint8 ULDUAR_THORIM_BALCONY_WAYPOINTS = 6;

// Arrival radius for hallway waypoints, the last one included. Wide because nothing up here dodges
// anything, and an exact-waypoint MoveTo won't reliably land inside a tight one.
inline constexpr float ULDUAR_THORIM_BALCONY_ARRIVE_TOLERANCE = 6.0f;

// The platform wait spots, and how close counts as standing on one. 2D, since the walk moves on exact
// points and a z mismatch would reissue the same move every few seconds.
inline constexpr uint8 ULDUAR_THORIM_BALCONY_HOLD_SLOTS = 6;
inline constexpr float ULDUAR_THORIM_BALCONY_HOLD_TOLERANCE = 1.5f;

Position const& GetThorimBalconyWaypoint(uint8 index);

// The Ancient Rune Giant, while it is still worth walking to. Its death opens the second doors and
// sets _isHitAllowed on Thorim, so "gone" is what says the hallway is the squad's next job.
Unit* GetThorimAncientRuneGiant(PlayerbotAI* botAI);

// True once the hallway is open: the Giant is dead and Thorim is not, so there is a platform to walk
// to and a boss to hit when the squad gets there.
bool ThorimBalconyOpen(PlayerbotAI* botAI, Player* bot);

// Advances the latch on arrival. Passing a waypoint's y counts too: the hallway runs one way, and a bot
// shoved past a point shouldn't turn back for it.
uint8 ThorimAdvanceBalconyStep(Player* bot);

// Barrier is always up, so non-tank melee back out on a health band and keep their target for ranged and
// instant casts. The tick moves the band, the read only reports it.
void ThorimTickBarrierBail(PlayerbotAI* botAI, Player* bot);
bool ThorimBarrierBailLatched(PlayerbotAI* botAI, Player* bot);

// From 6 stacks, ~95 s into phase 2, his swings have about doubled.
inline constexpr uint32 ULDUAR_THORIM_HUMAN_CALL_STACKS = 6;
inline constexpr float ULDUAR_THORIM_HUMAN_CALL_LOW_PCT = 50.0f;
// Calls a human holding him in phase 2: once per hold turn from ULDUAR_THORIM_HUMAN_CALL_STACKS, and
// whenever he's under ULDUAR_THORIM_HUMAN_CALL_LOW_PCT. Bots keep their class triggers.
void ThorimTickHumanTankCalls(PlayerbotAI* botAI);

// Phase 1 split

// The arena must never empty: one 5s scan finding nobody alive in its box summons the raid-wiping
// Lightning Orb. Set MEMBER_FLAG_MAINTANK, or a human tank earlier in roster order takes the role.
bool ThorimSplitActive(PlayerbotAI* botAI);

// Latched on first ask for the pull. No entry (a late arrival) means arena, the side that can't wipe the
// raid by being short.
ThorimSquad GetThorimSquad(PlayerbotAI* botAI, Player* bot);

// The boss script's own box, so the two sides cannot disagree about what counts as "in the arena".
bool ThorimInArenaBox(WorldObject const* who);

// Arena, corridor and the hallway above. Distance alone reaches Hodir's room, so above the wing ceiling
// only the balcony box counts.
bool NearThorimEncounter(Player const* bot);

// Melee are held to the tank spot and everyone else to the wider box radius, because melee are the
// only ones who have to walk out to an add at all.
bool ThorimArenaLeashBreached(PlayerbotAI* botAI, Player* bot);

// Re-elects the paladin holding Frost Resistance Aura. Nobody in normal mode, where Sif never leaves her
// balcony.
void ThorimTickFrostAuraPaladin(PlayerbotAI* botAI);

// Whether this bot is that paladin. Drives both the cast and the hold that stops its own buff strategy from
// trading the aura slot back.
bool IsThorimFrostAuraPaladin(Player const* bot);

// Pets outside their squad's bound (the room for the arena squad, the owner for the corridor) and not
// already walking back. Position only, never hazards: pets have the resistances to eat this fight.
bool ThorimStrayPets(PlayerbotAI* botAI, Player* bot, std::vector<Unit*>& out);

// Recall only, never an attack command, so it can't undo a stay or a follow.
void ThorimRecallPet(Player* bot, Unit* pet);

// Pets and guardians off their owner's ring bearing. Chain Lightning skips only totems and PetAI parks pets
// anywhere in a 90 degree arc behind the boss, so one between two slots bridges them.
bool ThorimPetsOffRingSlot(PlayerbotAI* botAI, Player* bot, std::vector<Unit*>& out);

// Back onto its owner's ring bearing, still on the boss. Chase angles are relative to the target's facing,
// so this is worked out from his live orientation every time.
void ThorimSendPetToRingSlot(PlayerbotAI* botAI, Player* bot, Unit* pet);

// Shared by trigger and action. Tank on the centre, ranged and healers on a ring, melee on the centre only
// out of combat. Not gated on ThorimSplitActive, which needs combat: the squad walks over before the pull.
bool GetThorimArenaAnchor(PlayerbotAI* botAI, Player* bot, Position& out);

// Updates this bot's arrival latch and reports whether it still needs to walk. Idempotent, so the
// trigger and the action can both ask.
bool ThorimArenaAnchorNeedsMove(PlayerbotAI* botAI, Player* bot, Position const& spot);

// The only window the movement guard covers: outside it the generic movers are what bring a bot back.
bool ThorimArenaAnchorSettled(PlayerbotAI* botAI, Player* bot);

void ThorimNoteFollowMasterStripped(Player* bot);
bool ThorimFollowMasterStripped(Player const* bot);

// Phase 2
bool ThorimPhase2Active(PlayerbotAI* botAI);

// Time since he first dropped for phase 2. False before that and whenever he is out of combat: his walk
// home after a wipe is below the floor line too, and must not start the next pull's clock.
bool ThorimPhase2ElapsedMs(PlayerbotAI* botAI, uint32& elapsedMs);

// Whether this ranged bot still waits on holdSpot. Letting go is latched, see openingReleased.
bool ThorimPhase2OpeningHold(PlayerbotAI* botAI, Player* bot, Position const& holdSpot);

// The platform spot a corridor ranged bot waits on, for as long as that wait lasts.
bool ThorimBalconyHoldSpot(PlayerbotAI* botAI, Player* bot, Position& out);

ThorimPhase2Role GetThorimPhase2Role(PlayerbotAI* botAI, Player* bot);
bool TryGetThorimPhase2Spot(PlayerbotAI* botAI, Player* bot, ThorimPhase2Role role, Position& position);

// Reads the arrival latch, never writes it: trigger and action both ask within a tick, and latching here
// would give them different answers, the trigger saying go and the action returning before MoveTo.
bool ThorimRingWantsMove(PlayerbotAI* botAI, Player* bot, Position const& spot);

// The latch itself. The trigger sets it when the answer above is "stay", the action clears it right
// before it moves. The action only runs when there is a move to make, so it never gets to set it.
void ThorimRingMarkArrived(Player* bot);
void ThorimRingClearArrived(Player* bot);

// The only window the movement guard covers: outside it the generic movers are what bring a bot back.
bool ThorimMeleeRingSettled(PlayerbotAI* botAI, Player* bot);

// Whether a spot is inside the reach of a live Blizzard zone or the bunny about to drop the next one.
bool ThorimSpotUnderBlizzard(Player* bot, Position const& spot);

// Same reach, tested along the straight walk between two points, both ends included.
bool ThorimWalkUnderBlizzard(Player* bot, Position const& from, Position const& to);

// Whether a spot is under the lit orb's cone, on the camp's tight margin. False while nothing is lit.
bool ThorimCampSpotInLitCone(PlayerbotAI* botAI, Position const& spot);

// Nearest spot a camp bot can step to that is off every Blizzard zone, out of the lit cone and still in
// range of the boss, favouring its own home or shelter. False when nothing within reach qualifies.
bool ThorimCampBlizzardEscape(PlayerbotAI* botAI, Player* bot, Position& out);

// Thunder Orb carrying markerSpell, or nullptr. Neither orb mechanic has a cast bar, so the aura is the
// only tell.
Unit* ThorimChargedThunderOrb(PlayerbotAI* botAI, uint32 markerSpell);

// Where this bot should stand to be clear of the Charge Orb field, or false if it is already clear.
// Pure, so the trigger can ask as often as it likes; the action records the answer separately.
bool ThorimChargedOrbEscape(PlayerbotAI* botAI, Player* bot, Position& out);
void ThorimNoteOrbEscape(Player* bot, Position const& spot);

void ResetThorimEncounterState(Player* bot, bool clearInstance);
bool ThorimEncounterStateIsStale(PlayerbotAI* botAI);
// Nothing to reset, nothing to do. Keeps the reset node from swallowing every tick before the pull, while
// he sits at full health on his balcony through the gauntlet.
bool ThorimBotHasEncounterState(Player* bot);

#endif
