/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERFLAMELEVIATHAN_H
#define PLAYERBOTS_ULDENCOUNTERFLAMELEVIATHAN_H

#include "Position.h"
#include "UldData.h"

#include <vector>

class Player;
class PlayerbotAI;
class Unit;

// Flame Leviathan. Everything here reads the vehicle a bot rides, never the bot's own position or spells.

enum UlduarFlameLeviathanIds
{
    // Hard-mode tower hazards, one ground NPC per tower.
    NPC_FL_THORIM_HAMMER_TARGET = 33364,     // Storm: static lightning-strike marks
    // Flame: the head walks a path and drops an NPC_FL_MIMIRONS_INFERNO patch every 2 s, each burning
    // 30 s, so the hazard is a trail behind a moving head, not one circle.
    NPC_FL_MIMIRONS_INFERNO_TARGET = 33369,
    NPC_FL_MIMIRONS_INFERNO = 33370,
    // Frost: the reticle chases a random target and commits only once it stops (FollowMovementGenerator
    // informs within 0.5 yd), striking ~6 s later. Then it stays stunned 5 s more.
    NPC_FL_HODIRS_FURY_TARGET = 33108,
    NPC_FL_HODIRS_FURY = 33212,

    // Flame Leviathan. Vehicle entries come from core ulduar.h via UldScripts.h; these are the
    // boss's own spells and the units the vehicles interact with.
    SPELL_FL_PURSUED = 62374,          // random vehicle every 31s, 35s; the boss then drives at it
    SPELL_FL_GATHERING_SPEED = 62375,  // +5% speed, stacks to 20, re-applied every 15s
    SPELL_FL_BATTERING_RAM = 62376,    // used on the pursued target inside 15 yd
    SPELL_FL_FLAME_VENTS = 62396,      // 10s channel; Electroshock interrupts it
    SPELL_FL_MISSILE_BARRAGE = 62400,
    NPC_FL_TURRET = 33139,             // boss-mounted turrets; never worth a vehicle's cast
    NPC_FL_DEFENSE_TURRET = 33142,
    NPC_FL_POOL_OF_TAR = 33090,        // faction 1965, i.e. the boss's; harmless to the raid
    NPC_FL_PYRITE_CONTAINER = 33189,   // grabbable crate, +25 energy to a demolisher
    NPC_FL_MECHANOLIFT = 33214,        // shot down with Anti-Air Rocket to drop a crate

    // Life: four corner wards from 30 s in, each firing a wave every 29 s. The adds stay until killed only
    // with world DB update 2026_09_10_03 and core 4d4ae4f95; the first hull to hit one holds it.
    NPC_FL_FREYA_WARD_TARGET = 33366,  // reticle at the corner
    NPC_FL_FREYA_WARD = 33367,         // the summoner itself, parked 40 yd overhead
    NPC_FL_WRITHING_LASHER = 33387,    // 190k hp, melees and Lashes its victim every 2s
    NPC_FL_WARD_OF_LIFE = 34275,       // 504k hp, same script
    // On him from engage while the Life tower stands (ActivateTowers), 30s before the first ward.
    SPELL_FL_TOWER_OF_LIFE = 64482,

    // Salvaged Siege Engine (33060) driver seat.
    SPELL_FL_RAM = 62345,
    SPELL_FL_ELECTROSHOCK = 62522,
    SPELL_FL_STEAM_RUSH = 62346,       // CHARGE_DEST along our own facing, not toward a target

    // Salvaged Siege Turret (33067), the siege engine's gunner.
    SPELL_FL_FIRE_CANNON = 62358,      // physical school despite the name; never ignites tar
    SPELL_FL_ANTI_AIR_ROCKET_SIEGE = 62359,
    SPELL_FL_SHIELD_GENERATOR = 64677,

    // Salvaged Chopper (33062).
    SPELL_FL_SONIC_HORN = 62974,
    SPELL_FL_TAR = 62286,              // pool spawns 9 yd BEHIND the chopper
    SPELL_FL_SPEED_BOOST = 62299,

    // Hodir's Fury's 60 s stun, on the vehicle and the crew. Nothing dispels it, but the Flames Hurl Boulder
    // and Mortar trigger remove it (spell_linked_spell -62297), and both are safe to aim at a frozen ally.
    SPELL_FL_HODIRS_FURY_STUN = 62297,

    // Salvaged Demolisher (33109) driver seat.
    SPELL_FL_HURL_BOULDER = 62306,
    SPELL_FL_HURL_PYRITE_BARREL = 62490,
    SPELL_FL_DEMOLISHER_RAM = 62308,
    SPELL_FL_BLUE_PYRITE_DOT = 68605,  // stacking DoT the barrel leaves on the boss, 10s, 10 stacks

    // Salvaged Demolisher Mechanic Seat (33167), the demolisher's gunner.
    SPELL_FL_MORTAR = 62634,
    SPELL_FL_ANTI_AIR_ROCKET = 64979,
    SPELL_FL_GRAB_CRATE = 62479,
    SPELL_FL_INCREASED_SPEED = 62471,

};

// Flame Leviathan hard-mode tower bitmask, used to pick which ground hazards to dodge.
enum FlameLeviathanTowerFlags
{
    FL_TOWER_STORM = 0x1,
    FL_TOWER_FLAMES = 0x2,
    FL_TOWER_FROST = 0x4,
    FL_TOWER_LIFE = 0x8,
    FL_TOWER_ALL = 0xF
};

// Warning band past a hazard's own reach (FlameLeviathanHazardReach); 8 puts a chopper's Inferno scan at ~18 yd.
inline constexpr float ULDUAR_FL_TOWER_HAZARD_MARGIN = 8.0f;

// How far the clear step looks for other hazards. Wider than the band on purpose: it has to see the next
// patches of an Inferno trail, or it steps out of one and into the next.
inline constexpr float ULDUAR_FL_TOWER_HAZARD_CLEAR_SCAN = 45.0f;

// Fury and Hammer are creature casts at a destination, so their radius is flat from the hull's centre.
// Inferno is a dynamic object aura, which adds both object sizes: a siege engine burns out to 17.1 yd.
inline constexpr float ULDUAR_FL_FURY_RADIUS = 10.0f;
inline constexpr float ULDUAR_FL_HAMMER_RADIUS = 7.0f;
inline constexpr float ULDUAR_FL_INFERNO_RADIUS = 9.0f;

// A Fury reticle only hurts once stopped: the 5 s fuse in npc_hodirs_fury, then ~1.1 s to 62297. Before
// that it chases at 12 yd/s, faster than any hull, and dodging only drags it along.
inline constexpr uint32 ULDUAR_FL_FURY_ARMED_MS = 6500;
// The fuse plus the 5 s it stays stunned after the strike. A reticle still stunned past this has
// retargeted and stopped again between two scans.
inline constexpr uint32 ULDUAR_FL_FURY_STUN_MS = 10000;

// The four NPC_FREYA_WARD_TARGET spawns from SummonTowerHelpers. The kite ring and every in-arena test
// derive from these, so nothing else hardcodes arena geometry.
extern std::vector<Position> const ULDUAR_FL_ARENA_CORNERS;

// Stand distances are added to his combat reach: he's huge, and a centre distance puts melee inside him.
// Ram reaches 15 yd plus his 15 yd reach, so 8 is well inside it.
inline constexpr float ULDUAR_FL_SIEGE_STAND_DIST = 8.0f;

// Sonic Horn reaches 35 yd plus his 15, so 20 keeps every shot and keeps the chopper out of Battering
// Ram's sphere except on the tar lead.
inline constexpr float ULDUAR_FL_CHOPPER_STAND_DIST = 20.0f;
// Outward from his reach like the others. HoldStation clamps it: reach 15 + 50 + the arrival deadband
// lands past the barrel's 70 yd, and a barrel that won't cast drops the stack.
inline constexpr float ULDUAR_FL_DEMOLISHER_BAND = 50.0f;

// Demolisher station lead in seconds of his travel, capped so the band stays ~44 yd from his centre,
// clear of Battering Ram's 25. Demolishers only: a chopper at 20 would be led into the sphere.
inline constexpr float ULDUAR_FL_STATION_LEAD_S = 1.5f;
inline constexpr float ULDUAR_FL_STATION_LEAD_MAX = 10.0f;

// The demolisher fan swings toward the arena centre, so a hull falling behind cuts inside his circle.
// Inside this radius the bearing to the centre is noise.
inline constexpr float ULDUAR_FL_CENTRE_TURN_DEADZONE = 15.0f;
inline constexpr float ULDUAR_FL_TAR_LEAD_DIST = 30.0f;      // how far ahead of him the lead chopper parks

// Re-facing costs a spline, so only correct a facing that has really drifted. Roughly 6 degrees.
inline constexpr float ULDUAR_FL_FACING_TOLERANCE = 0.1f;

// A loose arrival deadband on purpose: it is what stops a group of vehicles piling onto one
// coordinate, and a tight one makes a bot slide in place instead of ever holding still.
inline constexpr float ULDUAR_FL_ARRIVE_TOLERANCE = 8.0f;

// Only re-issue a MoveTo once the goal has drifted this far. Re-stamping the same destination
// restarts the spline every tick and the vehicle crawls instead of arriving.
inline constexpr float ULDUAR_FL_REPOSITION_EPSILON = 6.0f;

// Energy costs the strategy gates on. Only the ones used as thresholds are listed; the rest of the
// spellbook is free. Siege engines, turrets and choppers regenerate 20 per 2s against a cap of 100.
inline constexpr uint32 ULDUAR_FL_ELECTROSHOCK_COST = 20;
inline constexpr uint32 ULDUAR_FL_RAM_COST = 40;
inline constexpr uint32 ULDUAR_FL_STEAM_RUSH_COST = 40;
inline constexpr uint32 ULDUAR_FL_SONIC_HORN_COST = 20;
inline constexpr uint32 ULDUAR_FL_FIRE_CANNON_COST = 20;
inline constexpr uint32 ULDUAR_FL_SPEED_BOOST_COST = 50;
inline constexpr uint32 ULDUAR_FL_INCREASED_SPEED_COST = 25;
inline constexpr uint32 ULDUAR_FL_PYRITE_BARREL_COST = 5;

// Burst to 6 stacks, or to 10 when energy also covers the reserve (four refreshes). A burst barrel past
// 6 gains little, while a refresh at 10 stacks buys ~90 stack-seconds.
inline constexpr uint32 ULDUAR_FL_PYRITE_MAX_STACKS = 10;
inline constexpr uint32 ULDUAR_FL_PYRITE_BURST_STACKS = 6;
inline constexpr uint32 ULDUAR_FL_PYRITE_REFRESH_RESERVE = 20;

// Refresh lead on top of the measured flight: ~1.1 s between decisions plus a boulder's 1 s GCD. Late
// loses the whole stack, early only a second of it.
inline constexpr uint32 ULDUAR_FL_PYRITE_REFRESH_SLACK_MS = 2000;

// A barrel lobs for 1-3 s. The first refresh leads by this until a landing has been timed, and a
// barrel not seen landing within the timeout was lost.
inline constexpr uint32 ULDUAR_FL_PYRITE_FLIGHT_MS = 3000;
inline constexpr uint32 ULDUAR_FL_PYRITE_FLIGHT_TIMEOUT_MS = 5000;
inline constexpr uint32 ULDUAR_FL_PYRITE_FLIGHT_MIN_MS = 500;
// The aura's duration jumping back up by more than this means a barrel landed.
inline constexpr int32 ULDUAR_FL_PYRITE_LANDING_JUMP_MS = 250;

// Steam Rush charges this far along the hull's facing (62346, EffectRadiusIndex 21).
inline constexpr float ULDUAR_FL_STEAM_RUSH_DIST = 35.0f;
// Rush only with the goal inside this arc ahead: the charge runs along the hull's facing.
inline constexpr float ULDUAR_FL_STEAM_RUSH_AIM_ARC = float(M_PI) / 4.0f;
inline constexpr uint32 ULDUAR_FL_STEAM_RUSH_COOLDOWN_MS = 15000;

// Steam Rush starts 2 s of GCD category 133, which Electroshock shares (62346 StartRecoveryTime), so
// the vent reserve dashes 2-5 s ahead of a channel when it can. EVENT_VENT repeats every 20 s.
inline constexpr uint32 ULDUAR_FL_STEAM_RUSH_GCD_MS = 2000;
inline constexpr uint32 ULDUAR_FL_VENT_RUSH_LEAD_MS = 5000;
inline constexpr uint32 ULDUAR_FL_VENT_INTERVAL_MS = 20000;

// Between channels the vent reserve waits outside Battering Ram's reach, a few yards from Electroshock's,
// and leaves that hold this long before the next one.
inline constexpr uint32 ULDUAR_FL_VENT_APPROACH_LEAD_MS = 2000;

// Cone reach is the effect radius for Electroshock and Sonic Horn (RangeEntry 1 skips CheckRange). Ram's
// RangeIndex 11 is a real 0-15 yd entry, so CheckRange caps it at 15, not its 18 yd effect radius.
inline constexpr float ULDUAR_FL_ELECTROSHOCK_CONE_RADIUS = 25.0f;
inline constexpr float ULDUAR_FL_RAM_CONE_RADIUS = 15.0f;
inline constexpr float ULDUAR_FL_SONIC_HORN_CONE_RADIUS = 35.0f;

// Half the world.spell_cone widths (HasInArc halves what it's given), and isInFront adds no target radius.
// CAST_ANGLE_IN_FRONT is 120 degrees, wider than all three, so CastVehicleSpell never turns for these.
inline constexpr float ULDUAR_FL_ELECTROSHOCK_CONE_HALF_ANGLE = 30.0f * float(M_PI) / 180.0f;
inline constexpr float ULDUAR_FL_RAM_CONE_HALF_ANGLE = 50.0f * float(M_PI) / 180.0f;
inline constexpr float ULDUAR_FL_SONIC_HORN_CONE_HALF_ANGLE = 25.0f * float(M_PI) / 180.0f;

// Kept by the module: Spell::SendSpellCooldown returns early for a creature caster, so nothing enforces
// Electroshock's 10 s. Retry is for a shot that didn't stop the channel, each try costs 20 energy.
inline constexpr uint32 ULDUAR_FL_ELECTROSHOCK_COOLDOWN_MS = 10000;
inline constexpr uint32 ULDUAR_FL_ELECTROSHOCK_RETRY_MS = 3000;

// Module-side cooldowns for the other vehicle spells, as the core sets none for a creature caster. The speed
// buffs retry this soon: the cast reports success on a refusal, and only the aura says whether it landed.
inline constexpr uint32 ULDUAR_FL_DEMOLISHER_RAM_COOLDOWN_MS = 4000;
inline constexpr uint32 ULDUAR_FL_MORTAR_COOLDOWN_MS = 1000;
inline constexpr uint32 ULDUAR_FL_SHIELD_GENERATOR_COOLDOWN_MS = 60000;
inline constexpr uint32 ULDUAR_FL_TAR_COOLDOWN_MS = 15000;
inline constexpr uint32 ULDUAR_FL_SPEED_RECAST_MS = 1000;
inline constexpr uint32 ULDUAR_FL_ANTI_AIR_ROCKET_COOLDOWN_MS = 250;  // shot until one lands

// A 25 yd sphere on whoever he's pursuing (TARGET_DEST_TARGET_ENEMY, radius index 20), not a cone off
// his front, so measure from the Pursued vehicle and never from him.
inline constexpr float ULDUAR_FL_BATTERING_RAM_RADIUS = 25.0f;

// He only casts it inside IsWithinCombatRange(victim, 15.0f), which adds both combat reaches on top.
inline constexpr float ULDUAR_FL_BATTERING_RAM_CAST_RANGE = 15.0f;

// Hurl Boulder's RangeIndex 164, shared by Hurl Pyrite Barrel: a demolisher on top of a frozen ally has
// to back off to thaw it. Mortar (RangeIndex 37) has no minimum but stops at 50.
inline constexpr float ULDUAR_FL_HURL_BOULDER_MIN_RANGE = 10.0f;
inline constexpr float ULDUAR_FL_HURL_BOULDER_MAX_RANGE = 70.0f;
inline constexpr float ULDUAR_FL_MORTAR_MAX_RANGE = 50.0f;

// Fire Cannon shares Hurl Boulder's 10-70 yd band (RangeIndex 164) and is the heaviest gun the raid
// has: its missile 62357 lands 76k in a 20 yd sphere, against 190k and 504k add health pools.
inline constexpr float ULDUAR_FL_FIRE_CANNON_MIN_RANGE = 10.0f;
inline constexpr float ULDUAR_FL_FIRE_CANNON_MAX_RANGE = 70.0f;

// Splash of the aimed shot, only for ranking adds: Fire Cannon and Hurl Boulder land at radius index 9,
// Mortar's 62635 at index 42.
inline constexpr float ULDUAR_FL_CANNON_SPLASH = 20.0f;
inline constexpr float ULDUAR_FL_MORTAR_SPLASH = 11.0f;

// Puts the spawn past Fire Cannon's 10 yd floor and inside Ram's 15. Parked on the arena side, so Ram's
// knockback (Effect 98) throws adds deeper into the corner.
inline constexpr float ULDUAR_FL_CORNER_STANDOFF = 12.0f;

// Ram's ~20 yd throw from a post 12 yd in, plus margin. The post's turret shoots adds in here first, and
// nobody else shoots one held here: a 76k Fire Cannon hit passes the 130% ranged switch and drags it out.
inline constexpr float ULDUAR_FL_CORNER_HOLD_RADIUS = 30.0f;

// Hodir's Fury's 10 yd blast plus room for a vehicle drifting inside its arrival deadband.
inline constexpr float ULDUAR_FL_STATION_SPACING = 12.0f;

// The widest the fan may open. Past this the outer slots stop being "behind him" at all.
inline constexpr float ULDUAR_FL_STATION_MAX_ARC = 2.0f * float(M_PI) / 3.0f;

// A pyrite crate energizes for 25, so grabbing one above this wastes part of it.
inline constexpr uint32 ULDUAR_FL_CRATE_GRAB_CEILING = 75;

// A yard inside Grab Crate's 50 yd range (62479).
inline constexpr float ULDUAR_FL_CRATE_GRAB_RANGE = 49.0f;

// Demolishers never regenerate (no UNIT_FLAG2_REGENERATE_POWER), so crates are the only refill. Below
// this a starved demolisher detours to one.
inline constexpr uint32 ULDUAR_FL_CRATE_DETOUR_ENERGY = 30;
inline constexpr float ULDUAR_FL_CRATE_DETOUR_RADIUS = 60.0f;  // how far a starved demolisher leaves its band

// The crate falls straight down from the lift. This near the arena centre, holding a lift for its closest
// pass gains nothing, so it's shot at once.
inline constexpr float ULDUAR_FL_LIFT_DROP_RADIUS = 20.0f;

// Kite ring corners, inset off the walls so MoveTo has mesh to land on and chamfered into two nodes so
// a kiting hull rounds them instead of driving in while he cuts the diagonal.
inline constexpr float ULDUAR_FL_KITE_WALL_INSET = 15.0f;
inline constexpr float ULDUAR_FL_KITE_CORNER_CHAMFER = 35.0f;
inline constexpr float ULDUAR_FL_KITE_ADVANCE_DIST = 30.0f;    // switch nodes on approach, never on arrival
inline constexpr float ULDUAR_FL_KITE_BOSS_CLEARANCE = 50.0f;  // a node this close to him is not a destination

// The Inferno trail loops just inside the ring, so a leg across it burns. Each tick checks the next
// stretch of the leg, and a blocked one steps round it this far at a time.
inline constexpr float ULDUAR_FL_KITE_FIRE_LOOKAHEAD = 30.0f;
inline constexpr float ULDUAR_FL_KITE_DETOUR_STEP = 25.0f;

// A trigger and its action must call the same helper here, or they disagree about who is doing what.

// From the instance script. Threat sits on the vehicle, so "attackers" usually lacks him, and the target
// scan stops at SightDistance while a posted engine sits up to 190 yd out.
Unit* FlameLeviathanBoss(PlayerbotAI* botAI);

// The encounter's tick: resets the per-instance state after a wipe. Throttled per instance, so every
// bot may call it every tick.
void FlameLeviathanTick(PlayerbotAI* botAI);

// Everything else stays inert until this is true, so the raid can still drive into the arena and
// pull normally instead of being dragged at the boss the moment it seats.
bool FlameLeviathanEngaged(PlayerbotAI* botAI);

// The vehicle a bot is actually riding into battle. A gunner sits in seat 0 of a turret creature
// that is itself bolted into the siege engine / demolisher, so this walks one link up for them.
Unit* FlameLeviathanRiddenVehicle(Player* bot);

// Seat 0 of the turret and mechanic seat both carry CAN_CONTROL, so IsInVehicle(true) is true for
// a gunner and MoveTo would happily steer the bolted-on turret model. Gate movement on this.
bool FlameLeviathanIsDriver(Player* bot);

// Pursued lands on whichever unit the boss's spell picked, which may be the ridden vehicle or the
// parent underneath it, so both links are checked.
bool FlameLeviathanIsPursued(Player* bot);

bool FlameLeviathanIsVentChanneling(Unit* boss);

// One siege engine per channel: highest energy, guid on ties, then claimed, since a cast's 20 energy
// promotes the next engine the same tick. Never a Pursued one, it would turn to face him mid-escape.
bool FlameLeviathanIsVentInterrupter(PlayerbotAI* botAI, Player* bot);

// Claims the channel now being cast at, so the ranking above stops handing it to the next engine.
// Called once the interrupt has actually gone out.
void FlameLeviathanClaimVentChannel(Player* bot);

// Milliseconds to the next Flame Vents channel: 0 while one runs, UINT32_MAX before the first. A
// skipped channel keeps the 20 s cadence rather than going negative.
uint32 FlameLeviathanMsToNextVent(Player* bot, Unit* boss);

// A channel running, or one due within the rush lead. Only the vent reserve reads it: it's the one engine
// in Electroshock range, so walking it out costs the whole channel.
bool FlameLeviathanVentWindowOpen(Player* bot, Unit* boss);

// True when the shot is on; otherwise starts the turn and leaves the cast for a later tick.
// CastVehicleSpell only turns outside 120 degrees, wider than any of these cones.
bool FlameLeviathanFaceForCone(Unit* vehicleBase, Unit* target, float halfAngle, float radius);

// The same test without the turn, for a vehicle whose facing belongs elsewhere: the vent reserve's to
// him, an idle post's to its corner.
bool FlameLeviathanInCone(Unit* vehicleBase, Unit* target, float halfAngle, float radius);

// Alive, mounted, and neither the hull nor the driver frozen. Any role handed out by election has to
// ask this, or a 60 s Hodir's Fury stun takes the role with the vehicle.
bool FlameLeviathanCrewUsable(Player* member);

// Nearest frozen crewed vehicle in the thawing band, `from` excluded. A gunner passes its turret, so its
// own frozen demolisher stays eligible: the seat isn't stunned with the hull, and Mortar has no minimum.
Unit* FlameLeviathanFrozenVehicle(Player* bot, Unit* from, float minRange, float maxRange);

// The vehicle currently wearing Pursued, or null.
Unit* FlameLeviathanPursuedVehicle(PlayerbotAI* botAI, Player* bot);

// Where Battering Ram lands next: the Pursued vehicle, else his threat victim's hull. With nobody
// Pursued he still rams whoever holds threat, for up to 31 s after the Pursued hull died.
Unit* FlameLeviathanRamCentre(PlayerbotAI* botAI, Player* bot);

// True for the crew of the hull he rams while nobody is Pursued. He chases it like a Pursued one, so it
// kites like one.
bool FlameLeviathanIsRamTarget(Player* bot);

// True while this vehicle is inside the 25 yd sphere Battering Ram drops on `centre`.
bool FlameLeviathanInBatteringRamBlast(Unit* vehicleBase, Unit* centre);

// True when this vehicle shares the blast with the ram centre and he's close enough to fire. One test for
// the urgent trigger and the drive action, since a wider trigger demotes the cast node for nothing.
bool FlameLeviathanShouldClearBatteringRam(PlayerbotAI* botAI, Player* bot);

// Lowest-guid chopper driver that is neither Pursued nor frozen. It runs ahead of the boss dropping
// tar in his path; a Pursued chopper is already driving away from him and drops tar for free.
bool FlameLeviathanIsTarLead(PlayerbotAI* botAI, Player* bot);

// Best Freya's Ward add for a shot from `from`: most adds inside `splash`, nearest on ties. Skips one a
// posted engine holds unless `from` rides that hull. Off "possible targets": a crewed "attackers" is empty.
Unit* FlameLeviathanBestAdd(PlayerbotAI* botAI, Unit* from, float minRange, float maxRange, float splash,
                            Position const* around = nullptr, float aroundRadius = 0.0f);

// Corner this siege engine is posted to, or -1. Ranks 1..4 own corners 0..3; the vent reserve never
// posts and rank 0 takes its corner. Gated on his Tower of Life aura, on him from engage.
int8 FlameLeviathanCornerPost(PlayerbotAI* botAI, Player* bot);

// The same answer for whoever drives `hull`, for a gunner, whose seat has no rank of its own.
int8 FlameLeviathanHullCornerPost(PlayerbotAI* botAI, Player* bot, Unit* hull);

// The siege engine kept on the boss while the others post: the nearest usable, non-Pursued one, kept
// until it stops being either. It faces him rather than turning for a Ram.
bool FlameLeviathanIsVentReserve(Player* bot);

// Where that engine parks: ULDUAR_FL_CORNER_STANDOFF in from the corner, toward the arena centre.
Position FlameLeviathanCornerPostPoint(uint8 index);

bool FlameLeviathanInArena(Position const& pos, float margin = 0.0f);

// Where a demolisher parks outward from his combat reach: the band, clamped so reach plus the
// arrival deadband stays inside Hurl Pyrite Barrel's 70 yd.
float FlameLeviathanDemolisherStandDist(Unit* boss);

// How close a vehicle's centre can get to this hazard before it is hit. See ULDUAR_FL_FURY_RADIUS.
float FlameLeviathanHazardReach(Unit* hazard, Unit* vehicle);

// Whether this Hodir's Fury reticle has stopped on a target and its strike is still to land. Updated
// once per scan from the reticle's own stun.
bool FlameLeviathanFuryArmed(Player* bot, Unit* reticle);

// Nearest active-tower hazard within `margin` of its reach, or null. `strictMask` towers count only once
// inside their reach: Storm for a hull that owes a firing position, never Flames (a patch burns 30 s).
Unit* GetFlameLeviathanNearestTowerHazard(PlayerbotAI* botAI, Unit* from, uint32 towerMask,
                                          float margin = ULDUAR_FL_TOWER_HAZARD_MARGIN, uint32 strictMask = 0);

// Every active-tower hazard within `radius` of `from`, an unarmed Fury excepted. A step that only knows
// the nearest Inferno patch walks along the trail.
void GetFlameLeviathanTowerHazards(PlayerbotAI* botAI, Unit* from, uint32 towerMask, float radius,
                                   std::vector<Unit*>& out);

// `lead` shifts the point along his heading, so a hull that spends a second or two driving to it
// arrives where he will be rather than where he was. See FlameLeviathanStationLead.
Position FlameLeviathanRearPoint(Unit* boss, float standDist, float bearingOffset = 0.0f, float lead = 0.0f);

// How far ahead of him a station point is placed while he is moving, capped by
// ULDUAR_FL_STATION_LEAD_MAX. Zero when he is standing still.
float FlameLeviathanStationLead(Unit* boss);

// This vehicle's angle off its class bearing in the fan, so one Hodir's Fury costs one vehicle. `fanHalf`
// gets the angle from the fan's middle to its outermost slot.
float FlameLeviathanStationBearingOffset(Player* bot, Unit* vehicleBase, float radius, float* fanHalf = nullptr);

// How far to swing a fan built off his rear toward the arena centre, signed, and capped so its
// outermost slot stays in his rear half. Zero within ULDUAR_FL_CENTRE_TURN_DEADZONE of the centre.
float FlameLeviathanCentreTurn(Unit* boss, float fanHalf);

Position FlameLeviathanArenaCentre();
Position FlameLeviathanLeadPoint(Unit* boss, float standDist);

// How far ahead of him the lead chopper can sit without parking inside Battering Ram's sphere.
// Zero when there is no room at all, which means the slot has to be given up for now.
float FlameLeviathanTarLeadDistance(Unit* boss, Unit* pursued, float size);

// Eight nodes hugging the arena walls, corners chamfered. Built once from ULDUAR_FL_ARENA_CORNERS.
std::vector<Position> const& FlameLeviathanKiteRing();

#endif
