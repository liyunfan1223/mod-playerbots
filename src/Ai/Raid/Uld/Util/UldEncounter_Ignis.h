/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERIGNIS_H
#define PLAYERBOTS_ULDENCOUNTERIGNIS_H

#include "Position.h"
#include "UldData.h"

class Creature;
class Player;
class PlayerbotAI;
class Unit;
class WorldObject;

enum UlduarIgnisIds
{
    // Ignis the Furnace Master
    NPC_IGNIS = 33118,
    NPC_IGNIS_IRON_CONSTRUCT = 33121,
    NPC_IGNIS_SCORCHED_GROUND = 33123,
    // Dormant constructs wear this alongside UNIT_FLAG_NOT_SELECTABLE; Activate Construct strips it.
    SPELL_IGNIS_CONSTRUCT_INACTIVE = 38757,
    // Self-buff on the boss for the 3 s he is rooted and rotation-locked; the patch spawns when it
    // falls off, from the orientation frozen at cast start.
    SPELL_IGNIS_SCORCH = 62546,
    SPELL_IGNIS_SCORCH_25 = 63474,
    SPELL_IGNIS_FLAME_JETS = 62680,
    SPELL_IGNIS_FLAME_JETS_25 = 63472,
    SPELL_IGNIS_MOLTEN = 62373,
    SPELL_IGNIS_BRITTLE_10 = 62382,
    SPELL_IGNIS_BRITTLE_25 = 67114,
    SPELL_IGNIS_SLAG_POT_10 = 62717,
    SPELL_IGNIS_SLAG_POT_25 = 63477,
    SPELL_IGNIS_STRENGTH_OF_THE_CREATOR = 64473,
};

// Ignis: a Molten construct turns Brittle once it is this close to one of the room's two water
// triggers (boss_ignis.cpp polls FindNearestCreature(NPC_WATER_TRIGGER, 18.0f) once a second).
inline constexpr float ULDUAR_IGNIS_WATER_BRITTLE_RADIUS = 18.0f;

// Room floor is back at full height 12 yd out from the sunken trigger. The construct trails ~5 yd behind and
// still trips the 18 yd poll, which is 3D and counts both bounding radii.
inline constexpr float ULDUAR_IGNIS_WATER_STANDOFF = 12.0f;

// Dodge for everyone else, sized over 62548's 13 yd burn. The construct tank parks inside on purpose, so the
// construct following him into melee stacks Heat.
inline constexpr float ULDUAR_IGNIS_SCORCHED_GROUND_AVOID_RADIUS = 15.0f;
inline constexpr float ULDUAR_IGNIS_SCORCHED_GROUND_PARK_DISTANCE = 3.0f;

// boss_ignis.cpp never lights a patch within 25 yd of a water trigger, so one there stacks no Heat.
inline constexpr float ULDUAR_IGNIS_SCORCHED_GROUND_INERT_WATER_RADIUS = 25.0f;

// Sized against Shatter (62383, 18850 in 13 yd), not the Molten pulse, which only reaches ~7 yd.
inline constexpr float ULDUAR_IGNIS_MOLTEN_AVOID_RADIUS = 15.0f;

// Whole room: a tank at a pool is ~120 yd from Ignis' spawn, past the 100 yd SightDistance cap and the LOS
// filter on "nearest npcs".
inline constexpr float ULDUAR_IGNIS_ROOM_SEARCH_RADIUS = 200.0f;

// Scorch summons its patch 20 yd along the boss's facing, and he faces the main tank.
inline constexpr float ULDUAR_IGNIS_SCORCH_SPAWN_RANGE = 20.0f;

// Melee range is ~10.8 yd off Ignis' 8.0 combat reach. 9.5 leaves slack so a step off the spot keeps him in range.
inline constexpr float ULDUAR_IGNIS_TANK_RADIUS = 9.5f;
inline constexpr float ULDUAR_IGNIS_TANK_SPOT_TOLERANCE = 2.0f;

// Three slots 60 degrees apart, one step per Scorch, so the patches fan out instead of stacking.
inline constexpr uint8 ULDUAR_IGNIS_TANK_ARC_SLOTS = 3;
inline constexpr float ULDUAR_IGNIS_TANK_ARC_STEP = static_cast<float>(M_PI) / 3.0f;

// South: patches south, pools east and west, raid north, so the three never overlap.
inline constexpr float ULDUAR_IGNIS_TANK_BEARING = -static_cast<float>(M_PI) / 2.0f;

// Melee are admitted to a Brittle construct only once its aura has less than this left, as a
// fallback for a raid with nobody ranged in position. Brittle itself runs 15 s.
inline constexpr uint32 ULDUAR_IGNIS_BRITTLE_MELEE_FALLBACK_MS = 7000;

// Slag Pot ticks for ten seconds and cannot be dispelled or moved out of, but it does not kill from
// full, so healers only pile onto the victim once the ticks have actually opened a gap.
inline constexpr float ULDUAR_IGNIS_SLAG_POT_HEAL_HP_PCT = 85.0f;

extern const Position ULDUAR_IGNIS_BOSS_ANCHOR;
extern const Position ULDUAR_IGNIS_WATER_POOL_WEST;
extern const Position ULDUAR_IGNIS_WATER_POOL_EAST;

// Not "find target": a bot parked on a construct never has Ignis on its threat list. Not "nearest npcs"
// either, the room is wider than SightDistance.
Unit* GetIgnis(PlayerbotAI* botAI);

// GetIgnis, but null unless `wanted` holds for him. Same answer as testing GetIgnis's result, only
// the grid search is skipped whenever he fails the test, which is most of the time.
Unit* GetIgnisIf(PlayerbotAI* botAI, bool (*wanted)(Creature const*));

// Alive and actually fighting. Every Ignis node hangs off this: the room is 200 yd wide and the boss
// is visible from the whole of it, so proximity alone has bots dodging and kiting on the way in.
Unit* GetEngagedIgnis(PlayerbotAI* botAI);
bool IsIgnisEngaged(PlayerbotAI* botAI);

// Activated = Ignis has cast Activate Construct on it: selectable, aggressive, and worth tanking.
bool IsIgnisConstructActivated(Unit const* construct);

// 10 Heat stacks from standing in Scorched Ground. Molten also resets the construct's threat.
bool IsIgnisConstructMolten(Unit const* construct);

// A Molten construct brought to the water. One hit of 5000 (10-man) / 3000 (25-man) shatters it.
bool IsIgnisConstructBrittle(Unit const* construct);

// Lowest GUID of the Brittle constructs, not the nearest one: every bot has to converge on the same
// construct without anything shared to agree through, and one hit ends it.
Unit* GetIgnisBrittleConstruct(PlayerbotAI* botAI);
Unit* GetIgnisNearestMoltenConstruct(PlayerbotAI* botAI, WorldObject const* from);

// Nearest activated construct not yet Brittle, sticky per tank: a closer new one must not steal the walk, since
// Molten wiped the held one's threat and it would peel into the raid.
Unit* GetIgnisDrivenConstruct(PlayerbotAI* botAI, Player* tank);

// Claimed construct or null, never picks a new one. Asked per popped action, so it stays a hash lookup.
Unit* GetIgnisHeldConstruct(PlayerbotAI* botAI, Player* tank);

// Nearest lit patch. One inside the inert water radius stacks no Heat, so parking there stalls Molten for good.
Unit* GetIgnisNearestScorchedGround(PlayerbotAI* botAI, WorldObject const* from);
Position const& GetIgnisNearestWaterPool(WorldObject const* from);

// The lit patch nearest this tank's own pool, so the two construct tanks work opposite sides of the
// fan instead of both walking to whichever patch happens to be closest.
Unit* GetIgnisAssignedScorchedGround(PlayerbotAI* botAI, WorldObject const* from, int8 tankIndex);

// 0 or 1 for the two assist tanks, -1 otherwise. Assist tanks only: a main tank kiting drags Ignis along, and a
// DPS holding a Molten construct dies.
int8 GetIgnisConstructTankIndex(PlayerbotAI* botAI, Player* bot);

// By tank index, not distance: the anchor is nearly midway, so nearest-pool always picks the same one and both
// constructs would shatter in one spot.
Position const& GetIgnisAssignedWaterPool(int8 tankIndex);

// ULDUAR_IGNIS_WATER_STANDOFF short of the pool, on the tank's own bearing. He has no reason to go in: the Brittle
// poll measures the construct.
Position GetIgnisWaterApproach(Player* tank, Position const& pool);

// Arc slot around the anchor, advanced on each Scorch while Ignis is rooted. `boss` is the caller's GetIgnis
// result, so it isn't looked up twice.
Position GetIgnisMainTankPosition(PlayerbotAI* botAI, Player* bot, Unit* boss);

// The 3 s window in which Ignis is rooted and rotation-locked. The patch lands when it ends.
bool IsIgnisScorchWindow(Unit const* boss);

// Flame Jets is a 2.7 s observable cast, unlike most of what this raid throws, so bots can see it
// coming and stop feeding casts into the knockback lockout that follows.
bool IsIgnisFlameJetsCasting(Unit const* boss);

Player* GetIgnisSlagPotVictim(PlayerbotAI* botAI);
bool IsIgnisSlagPotVictim(Player* bot);

#endif
