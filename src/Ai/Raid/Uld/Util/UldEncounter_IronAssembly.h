/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERIRONASSEMBLY_H
#define PLAYERBOTS_ULDENCOUNTERIRONASSEMBLY_H

#include "ObjectGuid.h"
#include "Position.h"
#include "UldData.h"

#include <vector>

class Player;
class PlayerbotAI;
class Unit;

// Assembly of Iron. Killing one member restores the other two to full, so focus fire is the fight.

enum UlduarIronAssemblyIds
{
    // Unit::CastSpell difficulty-maps every id, so each pair is 10-man then 25-man and callers test both.
    SPELL_LIGHTNING_TENDRILS_10_MAN = 61887,
    SPELL_LIGHTNING_TENDRILS_25_MAN = 63486,
    SPELL_OVERLOAD_10_MAN = 61869,
    SPELL_OVERLOAD_25_MAN = 63481,
    SPELL_CHAIN_LIGHTNING_10_MAN = 61879,
    SPELL_CHAIN_LIGHTNING_25_MAN = 63479,
    SPELL_LIGHTNING_WHIRL_10_MAN = 61915,
    SPELL_LIGHTNING_WHIRL_25_MAN = 63483,
    SPELL_RUNE_OF_DEATH_10_MAN = 62269,
    SPELL_RUNE_OF_DEATH_25_MAN = 63490,
    SPELL_SHIELD_OF_RUNES_10_MAN = 62274,
    SPELL_SHIELD_OF_RUNES_25_MAN = 63489,
    SPELL_FUSION_PUNCH_10_MAN = 61903,
    SPELL_FUSION_PUNCH_25_MAN = 63493,
    SPELL_OVERWHELMING_POWER_10_MAN = 64637,
    SPELL_OVERWHELMING_POWER_25_MAN = 61888,
    // The rune's ground pulse, reapplied every 0.8s to anything standing within 5 yd of it. One id
    // for both raid sizes, and it is what marks a boss as standing in his own damage buff.
    SPELL_RUNE_OF_POWER = 64320,
    // The rune itself, the only id in the 61973 chain with a persistent area aura, so the DynamicObject to
    // sweep for. The boss carrying 64320 can walk off it, the object stays where it dropped.
    SPELL_RUNE_OF_POWER_AREA = 63513,
    // NPC_STEELBREAKER / NPC_MOLGEIM / NPC_BRUNDIR come from core ulduar.h via UldScripts.h
};

// Nothing anchor-centred sits past 30 yd, where the 45 and 135 degree diagonals lose the floor.

// Overload 61878 is 20,000 nature and a knockdown in 20 yd, Tendrils 61886/63485 3000/5000 a second in
// 18 (61884's 10 is a dummy). Clearance adds arrival slack: a bot stopping on the edge is still in.
inline constexpr float ULDUAR_IRON_ASSEMBLY_OVERLOAD_RADIUS = 20.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_OVERLOAD_CLEARANCE = 25.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_TENDRILS_RADIUS = 18.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_TENDRILS_CLEARANCE = 28.0f;

// Rune of Death 62269/63490: a 13 yd persistent area aura ticking 2750 shadow every half second for
// 30s. Search wide enough to see one dropped anywhere in the formation.
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_RADIUS = 13.0f;

// DBC 13 yd, but the aura searcher adds object size at both ends (15.4 measured). Run inside DANGER,
// stand at CLEARANCE: with one radius the escape parks on the edge and the rune comes straight back.
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_DANGER_RADIUS = 16.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_CLEARANCE = 21.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_SEARCH_RADIUS = 40.0f;

// Ring and bearing steps of the escape's hazard sweep.
inline constexpr float ULDUAR_IRON_ASSEMBLY_ESCAPE_SWEEP_STEP = 2.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_ESCAPE_SWEEP_ANGLE_STEP = static_cast<float>(M_PI) / 8.0f;

// Walks and goals inside ROUTE detour round the rune via fixed points on its CLEARANCE circle. Halfway,
// as a walk just clearing DANGER grazes it. Goals share it, or one between the radii never clears.
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_ROUTE_RADIUS = 18.5f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_DETOUR_STEP = 0.7854f;  // pi/4, chords stay 19.4 yd out

// Where the raid stands when a Rune of Death or Overload covers the stack point. A fixed set of anchor
// headings, since every bot picks on its own and they must agree.
inline constexpr uint8 ULDUAR_IRON_ASSEMBLY_STACK_SHIFT_HEADINGS = 8;
inline constexpr float ULDUAR_IRON_ASSEMBLY_STACK_SHIFT_RADIUS = 25.0f;

// Meltdown 61889: 29,250 nature in 15 yd round whoever Overwhelming Power expires on. Not dodged, the
// carrier dies anyway and melee stand in it. The ranged ring is sized off it.
inline constexpr float ULDUAR_IRON_ASSEMBLY_MELTDOWN_RADIUS = 15.0f;

// Rune of Power: 64320 within 5 yd, +50% damage, lives 60s. Lands on a council member, so ranged walk in
// and the tank spot shifts clear. The travel cap keeps a rune on Brundir from towing ranged into Overload.
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_RADIUS = 5.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_SOAK_MAX_TRAVEL = 25.0f;

// Tank spot clearance from a rune under his own boss: the 5 yd rune, the 5 the boss stops short of him,
// plus margin. A shifted spot, not a second mover: two at MOVEMENT_COMBAT cancel each other.
inline constexpr float ULDUAR_IRON_ASSEMBLY_TANK_RUNE_CLEARANCE = 12.0f;

// Bearing offsets tried in order at the boss's radius. Two steps already clear 12 yd on the 16 yd ring, and a fourth
// would put Steelbreaker's furthest candidate on Molgeim's spot, 90 degrees round.
inline constexpr float ULDUAR_IRON_ASSEMBLY_TANK_SHIFT_STEP = 0.3927f;  // pi/8, 22.5 degrees
inline constexpr uint8 ULDUAR_IRON_ASSEMBLY_TANK_SHIFT_STEPS = 3;

// Brundir parks at 28, not the 25 his Overload needs, so the stack 38 yd off never reacts to it and
// stays parked, free to kick Lightning Whirl.
inline constexpr float ULDUAR_IRON_ASSEMBLY_BRUNDIR_BEARING = 0.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_BRUNDIR_RADIUS = 28.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_STEELBREAKER_BEARING = 2.3562f;  // 3*pi/4
inline constexpr float ULDUAR_IRON_ASSEMBLY_MOLGEIM_BEARING = 3.9270f;       // 5*pi/4
inline constexpr float ULDUAR_IRON_ASSEMBLY_MELEE_BOSS_RADIUS = 16.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_STACK_BEARING = 3.1416f;         // pi
inline constexpr float ULDUAR_IRON_ASSEMBLY_STACK_RADIUS = 10.0f;
// Once Brundir is the last one up his isolation protects nothing, so the raid closes to a second
// point 25 yd short of him: outside Overload, inside caster range of him.
inline constexpr float ULDUAR_IRON_ASSEMBLY_BRUNDIR_LAST_STACK_RADIUS = 3.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_SLOT_TOLERANCE = 3.0f;

// Stops short of the hall doors, 84 and 86 yd from the anchor. Sight range is 100 yd with no LOS check,
// so without it the council drives bots from out in the corridor.
inline constexpr float ULDUAR_IRON_ASSEMBLY_ARENA_RADIUS = 78.0f;
inline constexpr float ULDUAR_IRON_ASSEMBLY_ARENA_HEIGHT = 10.0f;
// A tank that has drifted this far off its spot is walking, not parked.
inline constexpr float ULDUAR_IRON_ASSEMBLY_TANK_SPOT_TOLERANCE = 4.0f;

// Static Disruption targets someone beyond 10 yd and only exists from Steelbreaker's phase 2, hence a
// ranged, hard-mode-only spread. Fallback radius, for when he has no tank spot.
inline constexpr float ULDUAR_IRON_ASSEMBLY_SPREAD_RING_RADIUS = 18.0f;
inline constexpr uint8 ULDUAR_IRON_ASSEMBLY_SPREAD_SLOTS = 16;

// Centred on Steelbreaker's tank spot, so every slot clears Meltdown by 7 yd. Don't go past 23: at 24
// the 135 degree slot falls into a hole in the navmesh.
inline constexpr float ULDUAR_IRON_ASSEMBLY_EMPOWERED_SPREAD_RING_RADIUS = 22.0f;

// Molgeim's last rune outlives him on the stack point, inside this ring, and blocks about 140 degrees
// of it at CLEARANCE. The clear arc is found by sampling this many bearings.
inline constexpr uint8 ULDUAR_IRON_ASSEMBLY_SPREAD_RING_SAMPLES = 64;

// Room centre, read off Brundir's out-of-combat wander in boss_assembly_of_iron.cpp. On mesh, floor 427.267.
extern const Position ULDUAR_IRON_ASSEMBLY_ANCHOR;

struct IronAssemblyTargets
{
    Unit* steelbreaker = nullptr;
    Unit* molgeim = nullptr;
    Unit* brundir = nullptr;

    uint8 AliveCount() const;
};

// By entry, never "find target": that walks only the bot's threat list by localized name, and Brundir
// wipes his threat table on every Tendrils landing, so it goes quietly inert.
Unit* GetIronAssemblyMember(PlayerbotAI* botAI, uint32 entry);
void GatherIronAssemblyTargets(PlayerbotAI* botAI, IronAssemblyTargets& targets);
bool IronAssemblyEncounterActive(PlayerbotAI* botAI);

// Per-bot cache for one tick: the value behind it is a 100 yd grid sweep and a vector copy per read.
// Keyed on getMSTime() as a bot never ticks twice in one ms. Holds guids, so a dead member reads null.
class IronAssemblyScan
{
public:
    explicit IronAssemblyScan(PlayerbotAI* botAI) : botAI(botAI) {}

    // The three members, alive, exactly as three GetUldUnitByEntry calls return them.
    void Members(IronAssemblyTargets& targets);
    Unit* Member(uint32 entry);

    // Rune of Death positions around this bot, off one grid visit. By value: a caller can hold the
    // list across another scan read, and that read refills this one the moment the clock moves on.
    std::vector<Position> RunesOfDeath();

private:
    void RefreshMembers();
    Unit* Resolve(ObjectGuid const& guid) const;

    PlayerbotAI* botAI;

    uint32 membersAtMs = 0;
    ObjectGuid steelbreaker;
    ObjectGuid molgeim;
    ObjectGuid brundir;

    uint32 runesAtMs = 0;
    std::vector<Position> runes;
};

// This bot's IronAssemblyScan, held by the "iron assembly scan" value.
IronAssemblyScan& GetIronAssemblyScan(PlayerbotAI* botAI);

// Gate for positioning and targeting. The strategy runs out of combat too and the council is visible
// from the corridor, so a presence gate walks bots through the doors and opens the pull early.
bool IronAssemblyFormationActive(PlayerbotAI* botAI);

// True once Brundir is the only member left. His isolation protects nothing at that point, so the
// raid closes onto a second stack point instead of holding the opening one.
bool IronAssemblyBrundirIsLast(PlayerbotAI* botAI);

// What every non-tank hits: a human's skull on a living member, else the configured kill order. Never
// null while a member lives, which is what makes the unconditional targeting suppression safe.
Unit* IronAssemblyFocusTarget(PlayerbotAI* botAI);

// Which member this bot tanks, or nullptr. Bot tanks rank by guid and claim from the front of Brundir,
// Steelbreaker, Molgeim. Surplus ones get nullptr and fall through to the focus target.
Unit* IronAssemblyAssignedBoss(PlayerbotAI* botAI, Player* bot);

// Where that tank stands, so the boss follows him instead of being dragged through the raid. Rotated
// off its bearing while his boss stands in a Rune of Power, the only answer to that rune.
bool TryGetIronAssemblyTankSpot(PlayerbotAI* botAI, Player* bot, Position& position);

// The same spot for a named boss rather than the caller's own assignment, so the ranged ring can be
// built on Steelbreaker's without a tank having to exist.
bool TryGetIronAssemblyBossTankSpot(Player* bot, Unit* boss, Position& position);

// Where ranged and healers stand. Stacked normally; on the hard-mode spread ring once Steelbreaker
// is empowered, because Static Disruption only exists from his phase 2.
bool TryGetIronAssemblyRaidSpot(PlayerbotAI* botAI, Player* bot, Position& position);

bool IronAssemblyOverloadActive(Unit* brundir);
bool IronAssemblyTendrilsActive(Unit* brundir);

// Rune of Death is a persistent area aura, so it is a DynamicObject and AvoidAoeAction cannot see
// it. Both difficulty ids are swept.
void GatherIronAssemblyRunesOfDeath(Player* bot, std::vector<Position>& runes);

// DANGER asks whether a bot has to move, CLEARANCE whether a spot is somewhere to stand. One radius for
// both parks bots on the rune's edge.
bool IsIronAssemblyPositionClearOfRunes(Position const& spot, std::vector<Position> const& runes,
                                        float radius);

// Next step toward goal: goal itself, or a point round a Rune of Death the straight line would cross.
// False means hold, goal or detour point is covered. Walking through a rune hands the bot to the escape.
bool TryGetIronAssemblyRouteStep(PlayerbotAI* botAI, Player* bot, Position const& goal, Position& step);

// The council member standing in Molgeim's Rune of Power, if any. It lands on a boss, not a player
// (DoSelectLowestHpFriendly), so the tank walks his boss out while everyone else walks in.
Unit* IronAssemblyRuneOfPowerCarrier(PlayerbotAI* botAI);
bool TryGetIronAssemblyRuneOfPowerSoakSpot(PlayerbotAI* botAI, Player* bot, Position& position);

// A bot walking out of a hazard can't cast, so it can't be the interrupter. From the hazards, not
// isMoving(), so every bot reaches the same answer about every other bot.
bool IronAssemblyMemberMustMove(PlayerbotAI* botAI, Player* member);

// The interrupt this bot could land on Brundir now, or nullptr. Stuns yes, silences no: his
// creature_immunities mask 0x24CB375F has SILENCE but neither STUN nor INTERRUPT.
char const* IronAssemblyReadyInterrupt(Player* bot, Unit* target);

// "whirl", "chain" or nullptr. Bots with an interrupt ready rank by guid: 0 takes Lightning Whirl, 1
// Chain Lightning, so with one interrupt up Chain Lightning goes through to save it for the next Whirl.
char const* IronAssemblyInterruptDuty(PlayerbotAI* botAI, Player* bot, Unit* brundir);

bool IronAssemblyLightningWhirlActive(Unit* brundir);
bool IronAssemblyChainLightningCasting(Unit* brundir);

// Shield of Runes absorbs 20,000 and pays Molgeim +50% damage for 15s if it is drained rather than
// removed, so stripping it early is worth more than the dispel costs.
bool IronAssemblyShieldOfRunesUp(Unit* molgeim);
bool IronAssemblyHasFusionPunch(Unit* unit);

// Another grouped tank, human or bot, has boss on him.
bool IronAssemblyHeldByOtherTank(Player* bot, Unit* boss);

// Reset once the council is back at full health, never on "nobody in combat": Brundir spends 16s of
// every Lightning Tendrils out of combat by design.
void ResetIronAssemblyEncounterState(Player* bot, bool clearInstance);
bool IronAssemblyEncounterStateIsStale(PlayerbotAI* botAI);
// Nothing to reset means nothing to do, which keeps the reset node from swallowing every tick
// before the pull, when all three are alive at full health by definition.
bool IronAssemblyBotHasEncounterState(Player* bot);

#endif
