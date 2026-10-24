/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERVEZAX_H
#define PLAYERBOTS_ULDENCOUNTERVEZAX_H

#include "Position.h"
#include "UldData.h"

#include "ObjectGuid.h"

#include <vector>

class Creature;
class Player;
class PlayerbotAI;
class Spell;
class Unit;

// General Vezax. The Shadow Crash field (63277) is a persistent area aura, so a DynamicObject that
// AvoidAoeAction can't see. Vapors are left alone: only a kill drops a puddle, and a kill ends hard mode.

enum UlduarVezaxIds
{
    // General Vezax
    SPELL_MARK_OF_THE_FACELESS = 63276,
    // 62660 is the cast, 62659 the 10 yd impact, 63277 the 8 yd field it leaves for 20s.
    SPELL_VEZAX_SHADOW_CRASH_CAST = 62660,
    SPELL_VEZAX_SHADOW_CRASH_DMG = 62659,
    SPELL_VEZAX_SHADOW_CRASH_FIELD = 63277,
    SPELL_VEZAX_SEARING_FLAMES = 62661,
    SPELL_VEZAX_SURGE_OF_DARKNESS = 62662,
    SPELL_VEZAX_SARONITE_BARRIER = 63364,
    // Both cast by a vapor on itself: the first only from JustDied, which ends hard mode, the second
    // when the six merge.
    SPELL_VEZAX_SARONITE_VAPORS_AURA = 63323,
    SPELL_VEZAX_SUMMON_SARONITE_ANIMUS = 63145,
    // His own cast, every 30s. The sixth one with no vapor dead is the merge.
    SPELL_VEZAX_SUMMON_SARONITE_VAPORS = 63081,

    // General Vezax
    NPC_VEZAX = 33271,
    NPC_VEZAX_SARONITE_ANIMUS = 33524,
    // Nothing here fights one. It is listed so targeting can refuse it: killing a vapor calls
    // DoAction(1) on the boss and ends hard mode permanently.
    NPC_VEZAX_SARONITE_VAPORS = 33488,
};

// 62660 is instant with missile Speed 10, so the destination freezes at cast time and the flight is
// the whole dodge window. The impact knocks back, so standing still never holds a spot.
inline constexpr float ULDUAR_VEZAX_SHADOW_CRASH_IMPACT_RADIUS = 10.0f;
inline constexpr float ULDUAR_VEZAX_SHADOW_CRASH_DODGE_CLEARANCE = 12.0f;
inline constexpr float ULDUAR_VEZAX_SHADOW_CRASH_DODGE_SEARCH_RADIUS = 25.0f;
// How long a stored impact outlives distance over speed. Landings trail that by up to 0.2s, the rest
// is room for a slow tick.
inline constexpr uint32 ULDUAR_VEZAX_SHADOW_CRASH_LAND_SLACK_MS = 500;

// The Animus is taunt-immune, so only threat holds it. Everyone but tanks and redirectors waits until a
// tank has been its victim this long, and the cap stops a dead tank parking the raid on the Barrier.
inline constexpr uint32 ULDUAR_VEZAX_ANIMUS_TANK_LEAD_MS = 2000;
inline constexpr uint32 ULDUAR_VEZAX_ANIMUS_HOLD_CAP_MS = 6000;
// From this summon on the Animus is at least 38s out, past the 30s cooldown on Misdirection and
// Tricks, so saving them from here has both ready when it lands.
inline constexpr uint8 ULDUAR_VEZAX_REDIRECT_SAVE_SUMMON = 5;
inline constexpr float ULDUAR_VEZAX_TRICKS_RANGE = 20.0f;
// The Animus swaps victim at 110% threat in melee range, 130% outside. 90% of the main tank leaves room
// for a cast in flight and lets him take it back at 110%.
inline constexpr float ULDUAR_VEZAX_ANIMUS_THREAT_SHARE = 0.9f;
// Hard mode only: a strong raid kills him before the sixth vapor merges into the Animus. Tanks get the
// lower line so they keep threat while everyone else waits.
inline constexpr float ULDUAR_VEZAX_HOLD_HEALTH_PCT = 15.0f;
inline constexpr float ULDUAR_VEZAX_TANK_HOLD_HEALTH_PCT = 2.0f;
// Close enough that the Animus, following its victim, ends up in the main tank's melee range.
inline constexpr float ULDUAR_VEZAX_ANIMUS_BRING_BACK_RADIUS = 3.0f;
// Shadow Crash skips anyone within this plus both combat reaches (SelectTarget -3, 3D), so 12.5 yd for
// most players. The margin covers the two bosses shuffling.
inline constexpr float ULDUAR_VEZAX_SHADOW_CRASH_MIN_RANGE = 3.0f;
inline constexpr float ULDUAR_VEZAX_ANIMUS_MELEE_MARGIN = 2.0f;
// Sideways spacing between melee spots on the line between the two bosses, and how many spots.
inline constexpr float ULDUAR_VEZAX_ANIMUS_MELEE_SPREAD = 2.0f;
inline constexpr uint8 ULDUAR_VEZAX_ANIMUS_MELEE_LANES = 5;

// The Shadow Crash field (63277) is 8 yd, plus a yard of slack since a bot that stops exactly on the
// boundary is still inside it.
inline constexpr float ULDUAR_VEZAX_HAZARD_RADIUS = 8.0f;
inline constexpr float ULDUAR_VEZAX_HAZARD_CLEARANCE = 9.0f;
// Wide enough for the whole camp. Callers that only need what's under their feet pass the local one:
// the sweep is two grid searches and the raid runs it often.
inline constexpr float ULDUAR_VEZAX_HAZARD_SEARCH_RADIUS = 60.0f;
inline constexpr float ULDUAR_VEZAX_HAZARD_LOCAL_SEARCH_RADIUS = 25.0f;

// How far a caster walks to soak a field, the whole mana economy here. Past 25 it outgrows the local
// hazard sweep.
inline constexpr float ULDUAR_VEZAX_SHADOW_CRASH_SOAK_MAX_TRAVEL = 20.0f;

// Camp bearing: north, the side the raid comes in from. The southern half is left for the Mark spots.
inline constexpr float ULDUAR_VEZAX_ARC_ORIENTATION = 1.5708f;

// Two groups of two files by five rows, boss-relative. Rows 27 to 36 sit between the Mark floor (15)
// and reach spell's stop (28.5), each plus both combat reaches, with ~2 yd slack at each end.
inline constexpr float ULDUAR_VEZAX_CAMP_RADIUS = 31.5f;
inline constexpr float ULDUAR_VEZAX_CAMP_ROW_SPACING = 2.25f;
inline constexpr float ULDUAR_VEZAX_CAMP_FILE_SPACING = 3.0f;
// Each group's centre off the camp line. 4.0 puts the inner files 5 yd apart: one set of heals and one
// lust cover both, but they're not one clump.
inline constexpr float ULDUAR_VEZAX_CAMP_GROUP_OFFSET = 4.0f;
// The far-side member crosses the impact, so this minus the group's 3 yd width has to clear 10 yd.
inline constexpr float ULDUAR_VEZAX_CAMP_STRAFE = 15.0f;
inline constexpr uint8 ULDUAR_VEZAX_CAMP_ROWS = 5;
inline constexpr uint8 ULDUAR_VEZAX_CAMP_FILES = 2;

// Slot index space: [0,10) group L, [10,20) group R.
// The main tank isn't in here: it has exactly one holder and comes straight off IsMainTank.
inline constexpr uint8 ULDUAR_VEZAX_GROUP_SLOTS = ULDUAR_VEZAX_CAMP_ROWS * ULDUAR_VEZAX_CAMP_FILES;
inline constexpr uint8 ULDUAR_VEZAX_TOTAL_SLOTS = 2 * ULDUAR_VEZAX_GROUP_SLOTS;

// The arrival deadband is twice the tolerance, so anything at or above 1.5 would be wider than the
// 3 yd gap between neighbours and let a bot settle on someone else's slot.
inline constexpr float ULDUAR_VEZAX_SLOT_TOLERANCE = 1.2f;
inline constexpr float ULDUAR_VEZAX_TANK_SLOT_TOLERANCE = 3.0f;

// Off him, and wide enough for reach spell's 38 yd stop and the widest strafed slot (41.4) so the gate
// opens on arrival. Any wider puts bots on paths through walls.
inline constexpr float ULDUAR_VEZAX_ARENA_RADIUS = 45.0f;
inline constexpr float ULDUAR_VEZAX_ARENA_HEIGHT = 10.0f;

// DBC 15, but IsWithinDist3d adds the victim's combat reach, so the leech reaches ~16.5. The marked bot
// is the one person it skips.
inline constexpr float ULDUAR_VEZAX_MARK_LEECH_RADIUS = 15.0f;
inline constexpr float ULDUAR_VEZAX_MARK_BREAK_DISTANCE = 18.0f;

// Arc length at CAMP_RADIUS on the bot's own group's side, never across him through the melee. Keeps
// its distance from him and clears the widest strafed slot by 20.
inline constexpr float ULDUAR_VEZAX_MARK_SIDE_OFFSET = 40.0f;

// A marked melee has no camp side, so it takes the nearest of three spots south of him, opposite the camp.
inline constexpr float ULDUAR_VEZAX_MARK_SPOT_RADIUS = 26.0f;
inline constexpr float ULDUAR_VEZAX_MARK_SPOT_ARC_OFFSET = 2.3208f;  // pi/2 + 0.75, clear of the camp
inline constexpr float ULDUAR_VEZAX_MARK_SPOT_TOLERANCE = 3.0f;
inline constexpr uint8 ULDUAR_VEZAX_MARK_SPOT_COUNT = 3;

// Vezax' own spawn point, and the point the Saronite Vapors charge to when they merge.
extern const Position ULDUAR_VEZAX_ANCHOR;

struct VezaxHazard
{
    Position position;
    float radius = ULDUAR_VEZAX_HAZARD_RADIUS;
};

// Slots are kept until the holder leaves: a guid rank re-derived every tick reshuffles the camp on a
// death. Row-major inside each group, so the low bit is the file.
struct VezaxEncounterState
{
    std::unordered_map<ObjectGuid, uint8> slotAssignments{};

    // The crash in flight, from its cast. One is enough: they come every 10s and fly under 5s.
    Position crashImpact;
    uint32 crashCastMs = 0;
    uint32 crashWindowMs = 0;

    bool vaporKilled = false;
    bool animusSummoned = false;
    uint32 animusSummonMs = 0;
    uint8 vaporSummons = 0;
};

// From the instance object map, not a target sweep: PossibleTargetsValue reruns a 100 yd search on
// every call, and the movement rule asks once per candidate.
Unit* GetVezax(PlayerbotAI* botAI);
bool VezaxEncounterActive(PlayerbotAI* botAI);

// Gate for the formation and the movement rule. Not presence: he's visible from outside his hall, and
// a presence gate walks bots into walls with their generic movers zeroed.
bool VezaxFormationActive(PlayerbotAI* botAI);

// GetDynamicObjectPositions is a grid search, so gather once per execution and pass the vector to every
// check below.
void GatherVezaxHazards(Player* bot, std::vector<VezaxHazard>& hazards,
                        float searchRadius = ULDUAR_VEZAX_HAZARD_SEARCH_RADIUS);

bool TryGetVezaxNearestHazard(Player* bot, std::vector<VezaxHazard> const& hazards,
                              VezaxHazard& hazard);

// Every camp member with a mana bar, healers included. The -70% cost is MOD_POWER_COST_SCHOOL_PCT with
// mask 127, so it covers a hunter's shots too.
bool VezaxCanSoakShadowCrashField(Player* bot);

// Takes the bot because the camp is centred on the boss, not the anchor.
bool TryGetVezaxSlotPosition(Player* bot, uint8 slotIndex, Position& position);
// The tank only has to be on the anchor; a camp slot is 3 yd from its neighbours and needs tighter.
float VezaxSlotTolerance(Player* bot);
void EnsureVezaxSlotAssignments(Player* bot);

// False for melee, who hold the boss. The main tank gets the anchor: a tank on his spawn keeps him there.
bool TryGetVezaxSlot(Player* bot, Position& position);

// Where this bot carries the mark. A camp member arcs round him on its own group's side, anyone else
// takes the nearest southern spot.
bool TryGetVezaxMarkSpot(Player* bot, Position& position);

// The nearest living ally carrying the mark, or nullptr. Not the bot itself: its own leech skips it.
Unit* GetVezaxMarkedAlly(Player* bot);

// Somewhere outside the leech round a marked ally. Melee only: no mark in the ball reaches the camp.
bool TryGetVezaxMarkBreakSpot(Player* bot, Unit* marked, Position& spot);

// Where the missile in flight will land. From the cast hook, not his current spell: his next cast takes
// that slot while the missile still flies.
bool TryGetVezaxShadowCrashImpact(PlayerbotAI* botAI, Position& impact);
void VezaxNoteShadowCrash(Unit* vezax, Position const& impact, uint32 flightMs);

// Hard mode is still on the table: the switch is on, he's in combat, no vapor has died and the
// Animus isn't up yet.
bool VezaxHardModePending(PlayerbotAI* botAI);
// Hard mode still pending and he's at or under this bot's stop line. The Barrier check backs up the
// summon hook, in case it ever misses.
bool VezaxDamageHeld(PlayerbotAI* botAI);
// The hold parks pets passive. Stances are kept per instance so any engine can give them back, wipes
// included.
void VezaxHushPet(Creature* pet);
void VezaxUnhushPet(Creature* pet);
bool VezaxIsHushedPet(Creature* pet);
bool VezaxHarmfulCastOn(Spell const* spell, Unit* vezax);
void VezaxNoteVaporKilled(Unit* vapor);
void VezaxNoteAnimusSummoned(Unit* vapor);
void VezaxNoteVaporSummon(Unit* vezax);

// Hard mode still pending and the fifth vapor summon is out: the Animus is next.
bool VezaxAnimusDue(PlayerbotAI* botAI);
// Milliseconds since the Animus summon, or false when there hasn't been one this pull.
bool TryGetVezaxAnimusAge(PlayerbotAI* botAI, uint32& ageMs);
// Misdirection for a hunter, Tricks of the Trade for a rogue, nullptr for anyone else.
char const* VezaxAnimusRedirectSpell(Player* bot);
// The Animus is attacking this bot. Reads the bot's own attacker set, cheap enough for every tick.
bool IsVezaxAnimusOnBot(Player* bot);
// The Animus is alive: he carries the Saronite Barrier from its summon until it dies.
bool VezaxAnimusPhaseActive(PlayerbotAI* botAI);
// How far from Vezax a melee bot can stand and still never be a Shadow Crash pick, margin included.
float VezaxMeleeHoldDistance(Player* bot, Unit* vezax);
// A spot in melee range of the Animus and inside the hold distance from Vezax, on the line between
// them, one sideways lane per melee bot. False when they stand too far apart for one.
bool TryGetVezaxAnimusMeleeSpot(Player* bot, Unit* animus, Position& spot);

// Everyone but the main tank, who'd drag both bosses with him.
bool VezaxDodgesShadowCrash(Player* bot);
// Somewhere clear of that impact and still inside the band this bot's role is allowed to stand in.
bool TryGetVezaxDodgeSpot(Player* bot, Position const& impact, Position& spot);

// Drop this instance's assignments once he's out of combat, wipes included, or they carry into the
// next pull.
void ResetVezaxEncounterState(Player* bot, bool clearInstance);
bool VezaxHasEncounterState(Player* bot);

// The interrupt this bot could land on target right now, or nullptr. Stuns are deliberately absent:
// Vezax is a boss and immune to them, so bash and hammer of justice would never connect.
char const* VezaxReadyInterrupt(Player* bot, Unit* target);

// One kick per Searing Flames, or three bots spend theirs on one cast and miss the next. Lowest guid with
// an interrupt off cooldown wins, so cooldowns rotate the duty.
bool VezaxIsSearingFlamesInterrupter(Player* bot, Unit* boss);

#endif
