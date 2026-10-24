/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERYOGGSARON_H
#define PLAYERBOTS_ULDENCOUNTERYOGGSARON_H

#include "ObjectGuid.h"
#include "Position.h"
#include "UldData.h"

#include <vector>

class PlayerbotAI;
class Creature;
class Item;
class Player;
class Unit;

// Yogg-Saron: P1 tentacles and clouds on the platform, P2 the brain room and illusions, P3 the boss himself. Phase is
// read off the boss's aura, not a timer. Sanity (63050) at zero means Insane and lost.

enum UlduarYoggSaronIds
{
    ACTION_ILLUSION_DRAGONS = 1,
    ACTION_ILLUSION_ICECROWN = 2,
    ACTION_ILLUSION_STORMWIND = 3,
    NPC_GUARDIAN_OF_YS = 33136,
    NPC_YOGG_SARON = 33288,
    NPC_OMINOUS_CLOUD = 33292,
    NPC_RUBY_CONSORT = 33716,
    NPC_AZURE_CONSORT = 33717,
    NPC_BRONZE_CONSORT = 33718,
    NPC_EMERALD_CONSORT = 33719,
    NPC_OBSIDIAN_CONSORT = 33720,
    NPC_ALEXTRASZA = 33536,
    NPC_MALYGOS_ILLUSION = 33535,
    NPC_NELTHARION = 33523,
    NPC_YSERA = 33495,
    GO_DRAGON_SOUL = 194462,
    NPC_SARA_PHASE_1 = 33134,
    NPC_LICH_KING_ILLUSION = 33441,
    NPC_IMMOLATED_CHAMPION = 33442,
    NPC_SUIT_OF_ARMOR = 33433,
    NPC_GARONA = 33436,
    NPC_KING_LLANE = 33437,
    NPC_DEATHSWORN_ZEALOT = 33567,
    NPC_INFLUENCE_TENTACLE = 33943,
    NPC_DEATH_RAY = 33881,
    NPC_DEATH_ORB = 33882,
    NPC_BRAIN = 33890,
    NPC_CRUSHER_TENTACLE = 33966,
    NPC_CONSTRICTOR_TENTACLE = 33983,
    NPC_CORRUPTOR_TENTACLE = 33985,
    NPC_IMMORTAL_GUARDIAN = 33988,
    NPC_LAUGHING_SKULL = 33990,
    NPC_SANITY_WELL = 33991,
    NPC_DESCEND_INTO_MADNESS = 34072,
    NPC_MARKED_IMMORTAL_GUARDIAN = 36064,
    SPELL_SANITY = 63050,
    SPELL_SARAS_BLESSING = 63134,  // heals 27,000, then 6,000 shadow every 2s for 20s
    SPELL_SARAS_FERVOR = 63138,  // +20% damage done and +100% damage taken, 15s
    SPELL_BRAIN_LINK = 63802,
    // The two the aura casts on its partner every second, apart and together. Neither leaves an aura,
    // so the cast is the only place the pair is visible from outside the aura script.
    SPELL_BRAIN_LINK_DAMAGE = 63803,
    SPELL_BRAIN_LINK_OK = 63804,
    SPELL_MALADY_OF_THE_MIND = 63830,
    SPELL_SHADOW_BARRIER = 63894,
    SPELL_TELEPORT_TO_CHAMBER = 63997,
    SPELL_TELEPORT_TO_ICECROWN = 63998,
    SPELL_TELEPORT_TO_STORMWIND = 63989,
    SPELL_TELEPORT_BACK = 63992,
    SPELL_CANCEL_ILLUSION_AURA = 63993,
    SPELL_INDUCE_MADNESS = 64059,
    SPELL_LUNATIC_GAZE_YS = 64163,
    SPELL_DARK_VOLLEY = 63038,  // Guardian's 1.5s cast, 35 yd, distance is no answer, only a kick
    SPELL_DARK_VOLLEY_H = 65330,  // 25 normal casts 63038, so the split is not 10/25: test both
    SPELL_SQUEEZE = 64125,  // Constrictor Tentacle's grip; base id, difficulty-mapped at runtime
    SPELL_WEAKENED = 64162,  // Immortal Guardian's killable window; Thorim's Titanic Storm executes it
    SPELL_DIMINISH_POWER = 64145,  // Crusher's 5 min channel, -21% damage done raid-wide
    SPELL_LUNATIC_GAZE_SKULL = 64168,  // 64167 on a Laughing Skull fires this every second, 30 yd
    SPELL_SHADOW_NOVA_SARA = 65719,  // only cast from a Guardian's JustDied, so the cast is its death
    SPELL_SHATTERED_ILLUSION = 64173,  // stun on every platform tentacle once a brain room is cleared
    SPELL_SHADOW_BEACON = 64465,  // 10 s mark on a Guardian, entry swapped to 36064 while it lasts
    // Landed by Yogg's missile right after the mark ends: 37,500 a second for 20 s (25), on every ally
    // within 20 yd of the marked Guardian when it lands. One application, not an area aura.
    SPELL_EMPOWERING_SHADOWS_10 = 64468,
    SPELL_EMPOWERING_SHADOWS_25 = 64486,
    SPELL_GUARDIAN_DRAIN_LIFE = 64159,  // 5 s channel below 85% health, heals the Guardian 10x the drain
    SPELL_GUARDIAN_DRAIN_LIFE_H = 64160,
    // +100% to the caster's own aura for 6 s (aura 108, 99 base points), so only the Shadow Resistance paladin gains.
    // With its Concentration Aura up it also casts 64364: 40 yd silence immunity, only for raiders with that copy.
    SPELL_AURA_MASTERY = 31821,
    SPELL_CONCENTRATION_AURA = 19746,
    // 25-man with fewer than 4 Keepers: 2.3 s cast, then 4 s of silence within 50 yd.
    SPELL_DEAFENING_ROAR = 64189,
    SPELL_DISTRACTING_SHOT = 20736,  // 6 s taunt on an 8 s cooldown, 35 yd
    SPELL_WARRIOR_TAUNT = 355,
    SPELL_PALADIN_HAND_OF_RECKONING = 62124,
    SPELL_DEATH_KNIGHT_DARK_COMMAND = 56222,
    SPELL_DRUID_GROWL = 6795,
    SPELL_DRUID_BEAR_FORM = 5487,
    SPELL_DRUID_DIRE_BEAR_FORM = 9634,
    SPELL_HEROISM_SHAMAN = 32182,
    SPELL_BLOODLUST_SHAMAN = 2825,
    SPELL_SATED = 57724,
    SPELL_EXHAUSTION = 57723,
    ITEM_VALANYR = 46017,  // Val'anyr, Hammer of Ancient Kings
    GO_FLEE_TO_THE_SURFACE_PORTAL = 194625,
    // One per illusion, opened by the Brain the moment the last Influence Tentacle in that room dies.
    // The entries run in the same order as ACTION_ILLUSION_DRAGONS/ICECROWN/STORMWIND.
    GO_CHAMBER_ILLUSION_DOORS = 194635,
    GO_ICECROWN_ILLUSION_DOORS = 194636,
    GO_STORMWIND_ILLUSION_DOORS = 194637,
};

inline constexpr float ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT = 300.0f;
inline constexpr float ULDUAR_YOGG_SARON_BRAIN_ROOM_AXIS_Z_PATHING_ISSUE_DETECT = 200.0f;
// Searches over the whole encounter: the platform and the brain level under it.
inline constexpr float ULDUAR_YOGG_SARON_SEARCH_RANGE = 200.0f;
// The illusion rooms and the brain room share one floor 108-124 yd apart, so overlapping radii make the first test win.
// 60 covers every landing spot (55.9 yd at most) and Laughing Skull spawn (52.8); the nearest rival middle is 190 yd.
inline constexpr float ULDUAR_YOGG_SARON_STORMWIND_KEEPER_RADIUS = 60.0f;
inline constexpr float ULDUAR_YOGG_SARON_ICECROWN_CITADEL_RADIUS = 60.0f;
inline constexpr float ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_RADIUS = 60.0f;
inline constexpr float ULDUAR_YOGG_SARON_BRAIN_ROOM_RADIUS = 50.0f;

// Floor of the brain level. The three illusion rooms and the brain room all sit on it; the boss
// platform is 85 yd above.
inline constexpr float ULDUAR_YOGG_SARON_BRAIN_LEVEL_MIN_Z = 230.0f;
inline constexpr float ULDUAR_YOGG_SARON_BRAIN_LEVEL_MAX_Z = 250.0f;

// Reduced-Keeper hard mode: a bot at or below this Sanity (63050) pulls behind Yogg and faces away. No Freya means no
// Sanity Wells, so the drain is one-way; kept low so only near-Insane bots pull out.
inline constexpr uint32 ULDUAR_YOGG_SARON_SANITY_CONSERVE_THRESHOLD = 15;

// Below this a bot sits the next wave out and refills at a well: portals open ~22 s after the team is back. There is no
// time to top up between waves.
inline constexpr uint32 ULDUAR_YOGG_SARON_BRAIN_TEAM_MIN_SANITY = 60;

// Below this a bot heads for a Sanity Well even with something to kill.
inline constexpr uint32 ULDUAR_YOGG_SARON_SANITY_LOW = 40;

// Nitro Boosts on the walk from a cleared illusion room to the Brain; shorter walks end before 5 s of +150% pays.
// Chamber only: 3 min cooldown against a Chamber every 240 s, so it is ready for the longest walk.
inline constexpr float ULDUAR_YOGG_SARON_NITRO_MIN_WALK = 40.0f;

// A warrior's charge at the Brain lands on the floor ring at both combat reaches from it (31.5 yd), so
// only worth breaking the walk for when that ring is at least this much nearer the bot's brain spot.
inline constexpr float ULDUAR_YOGG_SARON_BRAIN_CHARGE_MIN_GAIN = 8.0f;

// A cloud summons a Guardian on any player within 8.5 yd: _IsWithinDist adds both combat reaches (1.0 cloud, 1.5
// player). Not dodgeable (fixed orbits, 10 s re-arm), so this only keeps bots moving for other reasons out of it.
inline constexpr float ULDUAR_YOGG_SARON_CLOUD_AVOID_RADIUS = 9.5f;

// Candidates are tested against where a cloud will be as well as where it is. The lead must outlast a crossing: a cloud
// sweeps a stationary bot for about 5.7 s.
inline constexpr uint32 ULDUAR_YOGG_SARON_CLOUD_LEAD_MS = 6000;

// How many points along a walk are checked against where the clouds will be at that point. Eight puts
// the longest walk in the room, the run in from the door at about 17 s, inside one lead per step.
inline constexpr uint32 ULDUAR_YOGG_SARON_WALK_CLOUD_STEPS = 8;

// Shadow Nova: DBC radius 15 plus both object sizes at apply time, 16.2 at the furthest. Melee and tanks eat it. The
// trigger sits inside the clear radius; that band is margin, not exposure.
inline constexpr float ULDUAR_YOGG_SARON_SHADOW_NOVA_TRIGGER_RADIUS = 17.0f;
inline constexpr float ULDUAR_YOGG_SARON_SHADOW_NOVA_CLEAR_RADIUS = 20.0f;

// Hold radius for a Sara's Fervor holder near a live nova: the clear radius plus 2 yd of slack. The dodge, station and
// leash would otherwise walk it back inside the blast.
inline constexpr float ULDUAR_YOGG_SARON_FERVOR_HOLD_RADIUS = 22.0f;

// A Guardian at or under this is about to detonate. Running from every Guardian instead scatters the
// raid to the rim and leaves bots to be picked off one at a time.
inline constexpr float ULDUAR_YOGG_SARON_GUARDIAN_NOVA_HEALTH_PCT = 20.0f;

// A Fervor holder leaves earlier since the nova hits it twice as hard. Widest gate whose worst case still covers the
// ~2.5 s walk out of 15 yd.
inline constexpr float ULDUAR_YOGG_SARON_FERVOR_NOVA_HEALTH_PCT = 50.0f;

// EXPECTED is the unresisted hit, the only safe planning figure since resistance rolls per hit. RESISTED applies at
// AT = 130 resistance, read off exclusive aura 143, which keeps only the highest.
inline constexpr uint32 ULDUAR_YOGG_SARON_NOVA_EXPECTED_DAMAGE = 28600;
inline constexpr uint32 ULDUAR_YOGG_SARON_NOVA_RESISTED_DAMAGE = 25740;
inline constexpr uint32 ULDUAR_YOGG_SARON_NOVA_RESISTED_AT = 130;

// Two gates: at 10% a bot spends its own button and a healer its external, and a 5 s window always covers the hit.
// Divine Sacrifice goes at 20%: one cast covers 25 raiders for 10 s. Everything in the ladder is instant.
inline constexpr float ULDUAR_YOGG_SARON_NOVA_SOON_HEALTH_PCT = 20.0f;
inline constexpr float ULDUAR_YOGG_SARON_NOVA_NOW_HEALTH_PCT = 10.0f;
inline constexpr uint32 ULDUAR_YOGG_SARON_NOVA_SOON_WINDOW_MS = 12000;
inline constexpr uint32 ULDUAR_YOGG_SARON_NOVA_NOW_WINDOW_MS = 5000;

// Counting past two low Guardians buys nothing: a pair already covers every case.
inline constexpr uint32 ULDUAR_YOGG_SARON_NOVA_MAX_CHAIN = 2;

// Divine Sacrifice is the only raid-wide reduction on this core; one cast covers 25 raiders, so wait for this many. Its
// 20% is the Divine Guardian talent's 70940 raid aura (value -20), visible only in the DBC.
inline constexpr uint32 ULDUAR_YOGG_SARON_NOVA_RAID_COUNT = 3;

// Claim on a Guardian, for cover and external alike: it must outlive the Guardian that took it, which detonates within
// 3.7 s. Pairs die 1.8-4.5 s apart, so a wall-clock window would leave the second Guardian uncovered.
inline constexpr uint32 ULDUAR_YOGG_SARON_NOVA_WINDOW_MS = 10000;

// The two cloud-proof spots, fixed by the orbits (reach 8.5 yd): inside 2.89 yd of Sara nothing reaches, and 21.5 is
// the middle of the band only orbit two reaches. No radius clears every orbit: gaps are 9.4-9.9 yd against 17 swept.
inline constexpr float ULDUAR_YOGG_SARON_P1_CLOUD_FREE_RADIUS = 2.8f;
inline constexpr float ULDUAR_YOGG_SARON_P1_RANGED_STATION_RADIUS = 21.5f;
inline constexpr float ULDUAR_YOGG_SARON_P1_RANGED_BAND_TOLERANCE = 1.0f;

// Orbit radii, midpoints of the measured spread. Not used to steer, only to say which orbits can reach a bot that ended
// up somewhere else.
inline constexpr float ULDUAR_YOGG_SARON_CLOUD_ORBITS[] = {11.6f, 21.4f, 31.2f, 41.0f, 50.9f, 60.8f};
inline constexpr float ULDUAR_YOGG_SARON_CLOUD_SUMMON_REACH = 8.5f;

// Past the outer orbit's reach no cloud can touch a bot, so phase 1 positioning waits until the bot is inside it.
// Combat is no gate: every Guardian's Reset puts the whole instance in combat.
inline constexpr float ULDUAR_YOGG_SARON_P1_ROOM_RADIUS =
    ULDUAR_YOGG_SARON_CLOUD_ORBITS[5] + ULDUAR_YOGG_SARON_CLOUD_SUMMON_REACH;

// Melee/tanks stay within 6.5 yd of Sara (21.5 less 15): a nova beyond it hits the 21.5 yd back line, a kill beyond 15
// does nothing. Release is the cloud-free radius: anything between lets go of the bot where the innermost orbit sweeps.
inline constexpr float ULDUAR_YOGG_SARON_P1_LEASH = 6.5f;
inline constexpr float ULDUAR_YOGG_SARON_P1_LEASH_RELEASE = ULDUAR_YOGG_SARON_P1_CLOUD_FREE_RADIUS;

// Tighter for a non-tanking melee bot: the inner orbit is 11.6 yd out and a cloud wakes within 8.5 yd of anyone. The
// release clears it by more than a step, or reach melee drags the bot back out and the two trade the tick.
inline constexpr float ULDUAR_YOGG_SARON_P1_PILE_LEASH =
    ULDUAR_YOGG_SARON_CLOUD_ORBITS[0] - ULDUAR_YOGG_SARON_CLOUD_SUMMON_REACH;
inline constexpr float ULDUAR_YOGG_SARON_P1_PILE_RELEASE = 2.0f;

// Fetch range: Taunt, Hand of Reckoning, Dark Command and Growl all use spell range index 4, 30 yd. Unbounded, the pick
// is the lowest-health Guardian in sight, e.g. one a pet grinds down at the wall.
inline constexpr float ULDUAR_YOGG_SARON_P1_TAUNT_RANGE = 30.0f;

// How far 65719 reaches Sara, so how far out a kill still counts. Wider than the leash on purpose, so the focus does
// not flip on a yard of drift.
inline constexpr float ULDUAR_YOGG_SARON_P1_SARA_NOVA_RADIUS = 15.0f;

// A Guardian outside the leash is worn down only to here, then left for the tank to fetch (~12 s). An untargeted
// Guardian near the station still loses up to ~3%/s to splash.
inline constexpr float ULDUAR_YOGG_SARON_P1_PARK_HEALTH_PCT = 35.0f;

// Minimum time between two Guardian novas: melee stand in every one on the stack.
inline constexpr uint32 ULDUAR_YOGG_SARON_P1_NOVA_GAP_MS = 6000;

// Below this a Guardian is close enough to dying that the gap has to protect it. Damage from the tank, humans, DoTs and
// cleaves means it buys ~3 s in practice, not 6.
inline constexpr float ULDUAR_YOGG_SARON_P1_NOVA_GAP_HEALTH_PCT = 30.0f;

// A Guardian's nova also hits every Guardian within 15 yd for 25,000-27,501 (65209 effect 1), ~2.7% of one. 18 covers
// the 16.2 yd measured on players.
inline constexpr float ULDUAR_YOGG_SARON_P1_NOVA_CHAIN_RADIUS = 18.0f;

// Death and Decay and Explosive Trap reach 10, Blizzard and Consecration 8.
inline constexpr float ULDUAR_YOGG_SARON_P1_AOE_HOLD_RADIUS = 10.0f;

// Ranged and healers stack rather than spread, which holds only because the nova cannot reach the station. A cloud is
// in contact (8.5 + blob + 8.5) / 3 s and re-arms 10 s after a summon: a blob under 13 yd costs one Guardian per pass.
inline constexpr float ULDUAR_YOGG_SARON_P1_RANGED_STACK_RADIUS = 5.0f;
// A parked back line may spread this much wider before it is walked back to the spot.
inline constexpr float ULDUAR_YOGG_SARON_P1_PARKED_STACK_SCALE = 1.5f;

// The second cap is the load-bearing one: a bot dodging outward otherwise walks out of spell range
// and stops contributing for the rest of the phase.
inline constexpr float ULDUAR_YOGG_SARON_SPACING_SEARCH_RADIUS = 25.0f;
inline constexpr float ULDUAR_YOGG_SARON_SPACING_MAX_FROM_MIDDLE = 35.0f;
// Ring sweeps for a dodge spot: rings this far apart, bearings this far round.
inline constexpr float ULDUAR_YOGG_SARON_SWEEP_STEP = 2.0f;
inline constexpr float ULDUAR_YOGG_SARON_SWEEP_ANGLE_STEP = static_cast<float>(M_PI) / 8.0f;

// Backstop only: "is the held spot still clear" normally invalidates first.
inline constexpr uint32 ULDUAR_YOGG_SARON_SPACING_HOLD_MS = 3000;

// How far a pet pulled off a Crusher looks for something else to hit. It must find something: an idle pet re-acquires
// the Crusher it was pulled from.
inline constexpr float ULDUAR_YOGG_SARON_PET_TARGET_RADIUS = 40.0f;

// How far from Yogg's middle the phase 3 pets look for leftover tentacles. Wave tentacles spawn up to
// ~50 yd out.
inline constexpr float ULDUAR_YOGG_SARON_PET_LEFTOVER_SEARCH_RADIUS = 70.0f;

// Crush: the Crusher's 100% proc on its white swings, a 23 yd physical cone +-5 degrees off its facing, which tracks
// its victim. Danger is standing collinear with tentacle and victim, not closeness; melee range is no exemption.
inline constexpr float ULDUAR_YOGG_SARON_CRUSH_RANGE = 25.0f;       // DBC radius 23 plus both object sizes
inline constexpr float ULDUAR_YOGG_SARON_CRUSH_TRIGGER_ARC = 8.0f;  // degrees either side of the facing
inline constexpr float ULDUAR_YOGG_SARON_CRUSH_CLEAR_ARC = 14.0f;   // ~4 yd of lateral room at 20 yd

// The Crusher's melee reach on a player, centre to centre: CombatReach 8 (display 28814) + 1.5 + 4/3 = 10.83. Inside it
// any melee hit, even a caster's weapon swing, makes the bot the victim of the next Crush.
inline constexpr float ULDUAR_YOGG_SARON_CRUSHER_REACH_TRIGGER_RADIUS = 11.0f;
inline constexpr float ULDUAR_YOGG_SARON_CRUSHER_REACH_CLEAR_RADIUS = 13.0f;

// Shattered Illusion lifts at Induce Madness end or when the Brain drops under 30%; a freed Crusher swings at whoever
// hit it last. 13 yd is under 2 s at run speed, doubled for the tick and the path.
inline constexpr uint32 ULDUAR_YOGG_SARON_STUN_EXIT_LEAD_MS = 4000;
// The lead above can cost ~4.3% of Brain health, so stay above this floor. Not while the brain team holds it.
inline constexpr float ULDUAR_YOGG_SARON_STUN_BRAIN_FLOOR_PCT = 35.0f;

// The brain team stops short of 30% while the platform has targets, so the stun runs on instead of ending with phase 3.
// DoTs stop earlier as they keep ticking; the finish starts RESUME_MS before the cast ends.
inline constexpr float ULDUAR_YOGG_SARON_BRAIN_HOLD_PCT = 32.0f;
inline constexpr float ULDUAR_YOGG_SARON_BRAIN_DOT_STOP_PCT = 36.0f;
inline constexpr uint32 ULDUAR_YOGG_SARON_BRAIN_RESUME_MS = 8000;
inline constexpr float ULDUAR_YOGG_SARON_BRAIN_HOLD_GUARD_PCT = 31.0f;  // stun reads as unheld below this
// A brain team bot's hold counts this long after it was last seen, so a team that died below lets go.
inline constexpr uint32 ULDUAR_YOGG_SARON_BRAIN_HOLD_SEEN_MS = 2000;
// The Brain's health share where phase 3 starts.
inline constexpr float ULDUAR_YOGG_SARON_BRAIN_PHASE_3_SHARE = 0.3f;

// Death Rays walk 9 yd legs every 1625 ms along a re-rolled cardinal axis, so trigger to clear is ~1 s of travel. The
// Death Orb that drops them is a marker 27 yd overhead and never reaches anybody.
inline constexpr float ULDUAR_YOGG_SARON_DEATH_RAY_TRIGGER_RADIUS = 9.0f;  // DBC 3 yd plus room to react
inline constexpr float ULDUAR_YOGG_SARON_DEATH_RAY_CLEAR_RADIUS = 14.0f;

// Wider than phase 1's: a Crush wedge can only be left sideways, and at 25 yd out that is a long walk.
inline constexpr float ULDUAR_YOGG_SARON_P2_SPACING_SEARCH_RADIUS = 35.0f;

// Phase 2 needs its own cap: phase 1's 35 yd puts the box inside where the raid stands, rejecting every outward
// candidate. The tentacle ring sits 41-48 yd out and the room ends at the outermost cloud orbit, 60.8 yd.
inline constexpr float ULDUAR_YOGG_SARON_P2_SPACING_MAX_FROM_MIDDLE = 55.0f;

// Yogg's body knocks players away every second: 64022 is a permanent self aura triggering 64020, 14 yd, no conditions
// row. His model sits 4.5 yd up, so the floor ring is 13.26 yd (sqrt of 14^2 minus 4.5^2). Nothing to sweep.
inline constexpr float ULDUAR_YOGG_SARON_BODY_KNOCKBACK_RADIUS = 13.3f;
inline constexpr float ULDUAR_YOGG_SARON_BODY_KNOCKBACK_CLEAR_RADIUS = 15.0f;

// The ring lights ~18 s after Sara dies, so melee and the tank walk out the moment she dies. The hold keeps claiming
// the bot after the ring lit, or reach melee walks straight back under the knock back.
inline constexpr uint32 ULDUAR_YOGG_SARON_HANDOVER_HOLD_MS = 6000;

// Where a bot crossing the room detours around the body: wide enough that both legs keep their distance. The worst case
// is a half-turn, whose chord passes 24 * cos(45 deg) = 17.0 yd from the middle.
inline constexpr float ULDUAR_YOGG_SARON_BODY_DETOUR_RADIUS = 24.0f;

// Brain Link: past 20 yd apart both take 63803 and lose 2 Sanity a second. The bot closes to a margin under that. The
// aura hides the partner, but casts 63803 (apart) or 63804 (together) on it every second; TTL is three missed ticks.
inline constexpr float ULDUAR_YOGG_SARON_BRAIN_LINK_RANGE = 20.0f;
inline constexpr float ULDUAR_YOGG_SARON_BRAIN_LINK_CLOSE = 15.0f;
inline constexpr uint32 ULDUAR_YOGG_SARON_BRAIN_LINK_PAIR_TTL_MS = 3000;

// How far a bot keeps from a player carrying Malady of the Mind.
inline constexpr float ULDUAR_YOGG_SARON_MALADY_SPREAD_RADIUS = 15.0f;

// Hand of Protection (physical immunity, IMMUNITY_PURGES_EFFECT) removes Squeeze, and its aura script kills the
// tentacle on removal. The claim is per victim, not per window, since grabs overlap. 30 yd is the spell's own range.
inline constexpr float ULDUAR_YOGG_SARON_HAND_OF_PROTECTION_RANGE = 30.0f;
inline constexpr uint32 ULDUAR_YOGG_SARON_SQUEEZE_CLAIM_MS = 3000;

// EVENT_SARA_P2_OPEN_PORTALS fires 60 s after phase 2 starts, then every 80 s; clearing a room does not move it. A
// portal is one use and despawns after 25 s, so the spread must finish before it opens. Debounce sits above that.
inline constexpr uint32 ULDUAR_YOGG_SARON_PORTAL_FIRST_WAVE_MS = 60000;
inline constexpr uint32 ULDUAR_YOGG_SARON_PORTAL_WAVE_PERIOD_MS = 80000;
inline constexpr uint32 ULDUAR_YOGG_SARON_PORTAL_WAVE_DEBOUNCE_MS = 30000;

// Adaptive like the exit lead below: time spent waiting on a portal spot is time not spent killing tentacles.
inline constexpr uint32 ULDUAR_YOGG_SARON_PORTAL_SPREAD_LEAD_FLOOR_MS = 8000;
inline constexpr float ULDUAR_YOGG_SARON_PORTAL_SPREAD_LEAD_SAFETY = 2.0f;
inline constexpr float ULDUAR_YOGG_SARON_PORTAL_SEARCH_RADIUS = 100.0f;

// One radius for both "arrived" and "can click": split values let a bot read as holding yet never find the portal.
inline constexpr float ULDUAR_YOGG_SARON_PORTAL_CLICK_RADIUS = 2.0f;

// AddPortals spawns RAID_MODE(4, 10), which is the first four table entries in 10-man and all ten in
// 25-man, so the brain team is capped by how many portals actually exist.
inline constexpr uint32 ULDUAR_YOGG_SARON_PORTAL_SPOTS_10MAN = 4;
inline constexpr uint32 ULDUAR_YOGG_SARON_PORTAL_SPOTS_25MAN = 10;

// Lunatic Gaze: 64167 on the skull triggers 64168 every 1000 ms, 1750 shadow and -2 Sanity at 30 yd. The skull is
// unselectable and unkillable and picks targets by HasInArc(M_PI, caster), so facing away is the only defence.
inline constexpr float ULDUAR_YOGG_SARON_LAUGHING_SKULL_RADIUS = 30.0f;

// How far a heading may drift before it is corrected, about 6 degrees. A node that re-faces every tick fights the
// engine's own target facing.
inline constexpr float ULDUAR_YOGG_SARON_FACING_TOLERANCE = 0.1f;

// Headings tried when turning a bot with no target off the skulls, 22.5 degrees apart.
inline constexpr uint32 ULDUAR_YOGG_SARON_SKULL_HEADINGS = 16;

// How far a bot with a target may turn off it to get a skull out of its front half: 40 degrees, under
// set facing's 45, so nothing turns it back. AttackAction and CastSpell allow 60.
inline constexpr float ULDUAR_YOGG_SARON_SKULL_TARGET_TURN = 0.6981f;
inline constexpr float ULDUAR_YOGG_SARON_SKULL_TARGET_STEP = 0.0873f;

// A skull straight behind the target cannot be turned from, only walked round; 15 yd is enough of a sidestep.
inline constexpr float ULDUAR_YOGG_SARON_ILLUSION_FACING_SEARCH_RADIUS = 15.0f;

// The sweep needs a circle to ring outward from, and a bot has no business standing under a skull
// anyway. Small on purpose: the constraint that matters is angular, not radial.
inline constexpr float ULDUAR_YOGG_SARON_SKULL_CLEAR_RADIUS = 5.0f;

// How far the brain team's healer may stray from its room's middle. Every tentacle is within 43 yd of it, and a heal
// reach step toward a mate across the Chamber stays inside this.
inline constexpr float ULDUAR_YOGG_SARON_ILLUSION_HEALER_STATION_RADIUS = 10.0f;

// Close enough to the spot the Brain is hit from to stop walking. The melee spot has 2.7 yd of melee
// range in hand, so a bot parked this far off it still reaches.
inline constexpr float ULDUAR_YOGG_SARON_BRAIN_SPOT_ARRIVED_RADIUS = 3.0f;

// Lead before Induce Madness (strips all Sanity below z 300, so a mind control is always a death), measured, not flat:
// a bot on a portal needs the floor, one in from the Chamber door ~44 yd. Timed at base run speed: bots make 4-6 yd/s.
inline constexpr uint32 ULDUAR_YOGG_SARON_EXIT_LEAD_FLOOR_MS = 3000;
inline constexpr float ULDUAR_YOGG_SARON_EXIT_LEAD_SAFETY = 1.5f;

// Phase 3 station. The radius is how far a bot may drift; the tank leash is wider since chasing a Guardian to the edge
// is its job.
inline constexpr float ULDUAR_YOGG_SARON_PHASE_3_STATION_RADIUS = 15.0f;
inline constexpr float ULDUAR_YOGG_SARON_PHASE_3_TANK_LEASH = 30.0f;
// The tank holding the pile walks back once it is further than this from the melee spot.
inline constexpr float ULDUAR_YOGG_SARON_PHASE_3_STACK_RADIUS = 5.0f;

// Ranged keep this far from a live Crusher: inside its 10.83 yd reach a first hit makes the bot its victim. 15.8 is a
// hunter's Auto Shot dead zone, plus 2 for where bots actually stop.
inline constexpr float ULDUAR_YOGG_SARON_PHASE_3_RANGED_CRUSHER_CLEARANCE = 18.0f;

// Guardian distance from the melee spot for phase 3 melee to take it, during a gaze or once beacon-healed. Guardian
// control keeps the tank within 5 yd of the spot and a full-health Guardian swings from up to 14 yd.
inline constexpr float ULDUAR_YOGG_SARON_PHASE_3_MELEE_GUARDIAN_RANGE = 20.0f;

// Without Thorim a Guardian cannot die, so melee bring it only down to the floor, where it hits about a third as hard
// as at full. The rest of the damage goes on Yogg. Under the Weakened line it hits for ~2k.
inline constexpr float ULDUAR_YOGG_SARON_PHASE_3_GUARDIAN_FLOOR_PCT = 30.0f;
inline constexpr float ULDUAR_YOGG_SARON_GUARDIAN_WEAKENED_PCT = 10.0f;

// Phase 3 area spells need this many units worth hitting within the trigger's 8 yd of the target (Yogg, a leftover
// tentacle, a held Guardian). The trigger counts anything, and Yogg's 30 yd reach is in every count at the pile.
inline constexpr float ULDUAR_YOGG_SARON_PHASE_3_AREA_RADIUS = 8.0f;
inline constexpr uint32 ULDUAR_YOGG_SARON_PHASE_3_AREA_MIN_WORTH = 3;
inline constexpr float ULDUAR_YOGG_SARON_COMBAT_REACH = 30.0f;

// A non-tank starts on an Immortal Guardian only under the start share of the holder's threat, and lets go past the
// keep share. Taking one over needs 110% in melee and 130% at range.
inline constexpr float ULDUAR_YOGG_SARON_GUARDIAN_THREAT_START_SHARE = 0.8f;
inline constexpr float ULDUAR_YOGG_SARON_GUARDIAN_THREAT_KEEP_SHARE = 0.95f;

// The tank leaves its Guardian only once its lead over the top non-tank is past this, and only for one with a thinner
// lead. 1.3 is what pulls one at range. Without switching, the raid waits on the newest Guardian.
inline constexpr float ULDUAR_YOGG_SARON_TANK_THREAT_SWITCH_LEAD = 1.5f;

// Smallest angle between target and Yogg, seen from the bot, that leaves a heading with the target inside set facing's
// 45 degrees and Yogg outside the gaze's 90: 65 degrees, 10 spare each side.
inline constexpr float ULDUAR_YOGG_SARON_GAZE_SAFE_SEPARATION = 1.1345f;

// Forced walk: time allowed without closing distance, and the arrival radius. The radius is generous because MoveTo
// stops where its own tolerance leaves the bot, not on the point.
inline constexpr uint32 ULDUAR_YOGG_SARON_WALK_GIVE_UP_MS = 6000;
inline constexpr float ULDUAR_YOGG_SARON_WALK_ARRIVED_RADIUS = 5.0f;

// How long a given-up walk stays given up. A permanent give-up strands bots, and standing still below the platform is a
// death. The escape below it needs a gap in the asking, and these nodes ask every tick.
inline constexpr uint32 ULDUAR_YOGG_SARON_WALK_RETRY_MS = 4000;

// How long a bot must have stood still before its unfinished MoveTo booking is dropped. A spline that
// is still running keeps isMoving() true, so this only ever fires on a walk that has already died.
inline constexpr uint32 ULDUAR_YOGG_SARON_WALK_STALL_MS = 500;

// Grim Reprisal returns 60% of every direct hit on a tentacle to the attacker (DoTs excepted), so a damage dealer below
// the stop line stays off until healed past the resume line. Worst single burst is 35% of max health in 0.2 s.
inline constexpr float ULDUAR_YOGG_SARON_REPRISAL_STOP_PCT = 40.0f;
inline constexpr float ULDUAR_YOGG_SARON_REPRISAL_RESUME_PCT = 70.0f;

// Induce Madness time left under which nobody holds back: an uncleared room has no way out and charms everyone in it,
// held or not.
inline constexpr uint32 ULDUAR_YOGG_SARON_REPRISAL_ALL_IN_MS = 15000;

// A held bot further than this from its room's healer walks toward it and stops about this short of it, inside the 38.5
// yd heal range with room for both to move.
inline constexpr float ULDUAR_YOGG_SARON_REPRISAL_REGROUP_RANGE = 25.0f;
inline constexpr float ULDUAR_YOGG_SARON_REPRISAL_REGROUP_STOP = 15.0f;

// Stormwind bots whose first tentacle is inside this of the stage spot keep clearing once it dies, the rest wait at the
// door. It takes the two at 31 yd; the next are 53 out.
inline constexpr float ULDUAR_YOGG_SARON_STORMWIND_HELPER_RADIUS = 40.0f;

// Induce Madness time left under which the Icecrown and Stormwind shares lift and everyone clears
// whatever's alive. An uncleared room charms everyone in it.
inline constexpr uint32 ULDUAR_YOGG_SARON_SHARE_LIFT_MS = 25000;

// Close enough to the stage spot. Small because the spot is only ~3 yd inside either room circle.
inline constexpr float ULDUAR_YOGG_SARON_STAGE_ARRIVED_RADIUS = 1.5f;

// How far out to look for a Guardian worth kicking. Wider than any interrupt's range on purpose:
// the action drops the ones it cannot reach, and a short list would hide a cast from a bot who could.
inline constexpr float ULDUAR_YOGG_SARON_INTERRUPT_SEARCH_RADIUS = 40.0f;

// How long one bot's kick holds a cast for itself, so the next bot spends its cooldown elsewhere.
inline constexpr uint32 ULDUAR_YOGG_SARON_INTERRUPT_CLAIM_MS = 1500;

// Phase 3 without Thorim: search radius for Shadow Beacon marked Guardians, and how long a dragger holds one without a
// heal. Yogg's missile goes out when the mark ends and flies at 30 yd/s, 0.3-1.1 s from the drag spots.
inline constexpr float ULDUAR_YOGG_SARON_BEACON_DRAG_SEARCH_RADIUS = 70.0f;
inline constexpr uint32 ULDUAR_YOGG_SARON_BEACON_HEAL_WAIT_MS = 3000;
inline constexpr float ULDUAR_YOGG_SARON_BEACON_DRAG_ARRIVED_RADIUS = 3.0f;

// A drag whose taunt has not turned its Guardian this long after the cast goes to the next bot. Every taunt has landed
// by the second line: Distracting Shot flies at 40 yd/s, 0.85 s at its 34 yd.
inline constexpr uint32 ULDUAR_YOGG_SARON_BEACON_TAUNT_CHECK_MS = 1500;
inline constexpr uint32 ULDUAR_YOGG_SARON_BEACON_TAUNT_LAND_MS = 1000;

// Shadow Beacon comes 45 s into phase 3, then 45 s after each one that lands, retried every 5 s. Draggers walk to a
// staging spot this long before it is due and give up this long after.
inline constexpr uint32 ULDUAR_YOGG_SARON_BEACON_PERIOD_MS = 45000;
inline constexpr uint32 ULDUAR_YOGG_SARON_BEACON_STAGE_LEAD_MS = 5000;
inline constexpr uint32 ULDUAR_YOGG_SARON_BEACON_STAGE_TIMEOUT_MS = 8000;
// Draggers staged for each beacon.
inline constexpr uint32 ULDUAR_YOGG_SARON_BEACON_STAGERS_10 = 1;
inline constexpr uint32 ULDUAR_YOGG_SARON_BEACON_STAGERS_25 = 3;

// Kickers held back for the marks' Drain Life while a beacon lasts, ready within this long. A channelling Guardian does
// not move, so one channel during a drag keeps its mark in the pile.
inline constexpr uint32 ULDUAR_YOGG_SARON_MARK_KICKERS = 2;
inline constexpr uint32 ULDUAR_YOGG_SARON_MARK_KICKER_READY_MS = 2000;

// A beacon-healed Guardian climbs ~9% a second, and past this health it hits a non-tank for ~17k and up.
inline constexpr float ULDUAR_YOGG_SARON_BEACON_DANGER_HEALTH_PCT = 40.0f;
// How long a released dragger walks its healed Guardian back toward the tank, and how long one external
// holds its victim for itself.
inline constexpr uint32 ULDUAR_YOGG_SARON_BEACON_RETURN_MS = 8000;
inline constexpr uint32 ULDUAR_YOGG_SARON_BEACON_EXTERNAL_CLAIM_MS = 8000;
// A tank holding the pile presses its own button once this many Guardians on it carry another Guardian's beacon heal,
// at any health, and gets an external under the line. A plain health line fires on one full-power swing.
inline constexpr uint32 ULDUAR_YOGG_SARON_BEACON_PILE_RESET_GUARDIANS = 2;
inline constexpr float ULDUAR_YOGG_SARON_BEACON_TANK_EXTERNAL_PCT = 50.0f;

// Empowering Shadows heals everything on Yogg's side within 20 yd of the mark, tentacles too, 750k over 20 s. The mark
// stands in its dragger's reach, a few yards off the spot.
inline constexpr float ULDUAR_YOGG_SARON_BEACON_HEAL_RADIUS = 20.0f;
inline constexpr float ULDUAR_YOGG_SARON_BEACON_HEAL_SLACK = 5.0f;

// Mortal Strike's -50% healing. A weaker cut on the Guardian still leaves the strike worth it.
inline constexpr int32 ULDUAR_YOGG_SARON_HEALING_CUT_PCT = -50;

// An Immortal Guardian under this casts Drain Life, again 9.5 s after each channel ends, kicked or not.
inline constexpr float ULDUAR_YOGG_SARON_DRAIN_LIFE_HEALTH_PCT = 85.0f;

// Both tanks stand on the melee spot, so Consecration keeps the whole pile on whoever has the most threat. The other
// tank takes it once the holder is under the swap line with this many on it; the holder sheds for a while.
inline constexpr float ULDUAR_YOGG_SARON_TANK_SWAP_PCT = 50.0f;
inline constexpr float ULDUAR_YOGG_SARON_TANK_SWAP_TAKER_PCT = 70.0f;
inline constexpr uint32 ULDUAR_YOGG_SARON_TANK_SWAP_MIN_GUARDIANS = 3;
inline constexpr uint32 ULDUAR_YOGG_SARON_TANK_SWAP_MS = 8000;

// Lay on Hands is kept for a tank under this in phase 3, one paladin per tank per claim.
inline constexpr float ULDUAR_YOGG_SARON_TANK_LOH_PCT = 20.0f;
inline constexpr uint32 ULDUAR_YOGG_SARON_TANK_LOH_CLAIM_MS = 3000;
inline constexpr float ULDUAR_YOGG_SARON_TANK_LOH_RANGE = 40.0f;

// Avenging Wrath locks Divine Protection for 30 s, so a phase 3 tank only presses it with Divine
// Protection that far from ready anyway.
inline constexpr uint32 ULDUAR_YOGG_SARON_TANK_WRATH_LOCK_MS = 30000;

// A Drain Life nobody has kicked by this point gets a mid-cast bot's own cast stopped so it can kick, only when no free
// bot can and no ready one finishes within the wait. One nudge per channel.
inline constexpr uint32 ULDUAR_YOGG_SARON_KICK_LAST_RESORT_MS = 1500;
inline constexpr uint32 ULDUAR_YOGG_SARON_KICK_WAIT_MS = 1000;
inline constexpr uint32 ULDUAR_YOGG_SARON_KICK_NUDGE_MS = 2000;
// Drain Life ticks once a second, so under this left it has one tick to go. Late in phase 3 a fresh channel starts
// about every second and a kick there saves all five.
inline constexpr int32 ULDUAR_YOGG_SARON_KICK_MIN_LEFT_MS = 1000;
// Hunter drags. Distracting Shot is a 6 s taunt against a 10 s mark, so it goes out with this much left and covers the
// heal 0.3-1.1 s after. Feign Death if the Guardian is still on the hunter this long after the release.
inline constexpr uint32 ULDUAR_YOGG_SARON_BEACON_SHOT_LEFT_MS = 5000;
inline constexpr uint32 ULDUAR_YOGG_SARON_BEACON_FEIGN_DELAY_MS = 1000;
inline constexpr float ULDUAR_YOGG_SARON_DISTRACTING_SHOT_RANGE = 34.0f;

// Deafening Roar comes 30 s into phase 3, then every 60 s. The paladin on turn raises Concentration Aura this early, as
// its immunity only reaches raiders with its own copy. Another paladin may take the roar after the handoff delay.
inline constexpr uint32 ULDUAR_YOGG_SARON_KEEPER_COUNT = 4;
inline constexpr uint32 ULDUAR_YOGG_SARON_ROAR_FIRST_MS = 30000;
inline constexpr uint32 ULDUAR_YOGG_SARON_ROAR_PERIOD_MS = 60000;
inline constexpr uint32 ULDUAR_YOGG_SARON_ROAR_PRESWITCH_MS = 10000;
inline constexpr uint32 ULDUAR_YOGG_SARON_ROAR_HANDOFF_MS = 500;

// Heroism waits until this share of the living group is at its station or in reach of its target, or the fallback after
// phase 3 starts.
inline constexpr float ULDUAR_YOGG_SARON_LUST_POSITION_SHARE = 0.8f;
inline constexpr uint32 ULDUAR_YOGG_SARON_LUST_FALLBACK_MS = 30000;

// What the raid clears out of an illusion room before the Brain: the Influence Tentacle and the six entries it is
// disguised as. The other mobs are not targets (faction 35, or the Lich King); listing one blocks the Brain.
extern const std::vector<uint32> ULDUAR_YOGG_SARON_ILLUSION_MOBS;

// The ten portal spots, straight from the core's yoggPortalLoc. A table because the brain team must stand on its spot
// before any portal exists. All ten are on the navmesh, 20.1-23.2 yd from the body.
extern const Position ULDUAR_YOGG_SARON_PORTAL_SPOTS[ULDUAR_YOGG_SARON_PORTAL_SPOTS_25MAN];

extern const Position ULDUAR_YOGG_SARON_MIDDLE;
extern const Position ULDUAR_YOGG_SARON_STORMWIND_KEEPER_MIDDLE;
extern const Position ULDUAR_YOGG_SARON_ICECROWN_CITADEL_MIDDLE;
extern const Position ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_MIDDLE;
extern const Position ULDUAR_YOGG_SARON_BRAIN_ROOM_MIDDLE;

// Icecrown's nine tentacles stand in three groups of three, 33-58 yd apart: west, south, east. A tentacle belongs to
// the nearest centroid. creature_summon_groups summonerId 33890 group 2.
inline constexpr uint8 ULDUAR_YOGG_SARON_ICECROWN_CLUSTER_COUNT = 3;
extern const Position ULDUAR_YOGG_SARON_ICECROWN_CLUSTERS[ULDUAR_YOGG_SARON_ICECROWN_CLUSTER_COUNT];

// Where a bot done with its share waits for the door, as close as the room test allows: both doors sit inside the brain
// room's 50 yd circle (Icecrown 49.0, Stormwind 45.7), so a bot at one reads as already in there.
extern const Position ULDUAR_YOGG_SARON_ICECROWN_STAGE_SPOT;
extern const Position ULDUAR_YOGG_SARON_STORMWIND_STAGE_SPOT;

// Where melee hit the Brain from: 12.3 yd west of it and behind, since it faces east all fight. 30.1 yd 3D against a
// 32.8 yd melee range. Anywhere in front, set behind walks the bot 15 yd out of range (3D distance as floor radius).
extern const Position ULDUAR_YOGG_SARON_BRAIN_MELEE_SPOT;

// Ranged and healers hit the Brain from a few yards in from each room's exit portal: west Stormwind, south Icecrown,
// north Chamber. Door to spot to portal is 53, 62 and 71 yd against 93-106 through the middle.
extern const Position ULDUAR_YOGG_SARON_BRAIN_WEST_SPOT;
extern const Position ULDUAR_YOGG_SARON_BRAIN_SOUTH_SPOT;
extern const Position ULDUAR_YOGG_SARON_BRAIN_NORTH_SPOT;
extern const Position ULDUAR_YOGG_SARON_STORMWIND_KEEPER_ENTRANCE;
extern const Position ULDUAR_YOGG_SARON_ICECROWN_CITADEL_ENTRANCE;
extern const Position ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_ENTRANCE;
extern const Position ULDUAR_YOGG_SARON_P1_RANGED_SPOT;

// Where melee and the tank sit out the handover: on the station ring, so the only orbit reaching it is the one the back
// line wakes. 25 yd round from the station spot, so a dying Guardian cannot reach the nearest raider.
extern const Position ULDUAR_YOGG_SARON_HANDOVER_MELEE_SPOT;
extern const Position ULDUAR_YOGG_SARON_PHASE_3_MELEE_SPOT;
extern const Position ULDUAR_YOGG_SARON_PHASE_3_RANGED_SPOT;

// Where ranged stand while a Crusher is too close to their spot, tried in order: the ranged spot turned -15, +15, -30,
// +30 degrees round Yogg's middle, same 39 yd out. On the navmesh, 21.6-25.2 yd from the melee spot.
extern const std::vector<Position> ULDUAR_YOGG_SARON_PHASE_3_RANGED_STANDBY_SPOTS;

// Beacon drag spots without Thorim, one per marked Guardian. The heal hits every Guardian within 20 yd of the mark,
// once, and holds it full 20 s. North and south are ~39 yd out; north-east is 33.7 yd from the melee spot.
extern const Position ULDUAR_YOGG_SARON_BEACON_DRAG_NORTH;
extern const Position ULDUAR_YOGG_SARON_BEACON_DRAG_SOUTH;
extern const Position ULDUAR_YOGG_SARON_BEACON_DRAG_NORTHEAST;

// Where a hunter stands to pull a marked Guardian with Distracting Shot: on the line to each drag spot, inside the
// shot's 35 yd. A Guardian that walks to the hunter stops ~29 yd off the pile.
extern const Position ULDUAR_YOGG_SARON_BEACON_HUNTER_NORTH;
extern const Position ULDUAR_YOGG_SARON_BEACON_HUNTER_SOUTH;

// Where a non-hunter dragger waits for the next beacon, 20 yd along the PATHFIND_NORMAL line from the melee spot to
// each drag spot. A 30 yd taunt still reaches a mark anywhere in the pile; the walk after is ~19 yd instead of ~39.
extern const Position ULDUAR_YOGG_SARON_BEACON_STAGE_NORTH;
extern const Position ULDUAR_YOGG_SARON_BEACON_STAGE_SOUTH;
extern const Position ULDUAR_YOGG_SARON_BEACON_STAGE_NORTHEAST;

// One place recognises the encounter; every node routes through it. Yogg is not reliably on a threat list, so it scans
// for him. Not Sara's presence (static spawn, alive at 1 health in P2/P3) nor her combat flag (friendly all of P1).
uint32 YoggSaronPhase(PlayerbotAI* botAI);

// FindNearestCreature(entry, 200, alive) for Sara, Yogg, the Brain and the Sanity Wells, answered once per trigger pass
// (see EncounterTriggerPassId); live anywhere else and for any other entry.
Creature* YoggSaronNearestCreature(PlayerbotAI* botAI, uint32 entry);

bool YoggSaronEncounterActive(PlayerbotAI* botAI);
bool YoggSaronInPhase1(PlayerbotAI* botAI);
bool YoggSaronInPhase2(PlayerbotAI* botAI);
bool YoggSaronInPhase3(PlayerbotAI* botAI);

// Which room on the brain level a position is in, or the boss platform above it.
enum YoggSaronRoom : uint32
{
    YOGG_SARON_ROOM_NONE = 0,
    YOGG_SARON_ROOM_ARENA = 1,
    YOGG_SARON_ROOM_STORMWIND = 2,
    YOGG_SARON_ROOM_ICECROWN = 3,
    YOGG_SARON_ROOM_CHAMBER = 4,
    YOGG_SARON_ROOM_BRAIN = 5,
};

YoggSaronRoom YoggSaronRoomOf(Player* player);
bool YoggSaronOnBrainLevel(Player* player);

// Room handling for a landed bot: the dps resolver needs line of sight, which a doorway breaks, so something must walk
// it in.
enum YoggSaronRoomState : uint32
{
    YOGG_SARON_ROOM_STATE_NONE = 0,
    YOGG_SARON_ROOM_STATE_WALKING_IN = 1,
    YOGG_SARON_ROOM_STATE_FIGHTING = 2,
    YOGG_SARON_ROOM_STATE_DOOR_SHUT = 3,
    YOGG_SARON_ROOM_STATE_TO_BRAIN = 4,
    YOGG_SARON_ROOM_STATE_AT_BRAIN = 5,
    // Off the tentacles until healed, see YoggSaronReprisalHold. Has no target, so without its own
    // state it reads as walkingin and the walk-in node marches it back onto one.
    YOGG_SARON_ROOM_STATE_REPRISAL = 6,
    // Done with its share of an Icecrown or Stormwind room, waiting at the door for the rest.
    YOGG_SARON_ROOM_STATE_STAGED = 7,
};

YoggSaronRoomState YoggSaronRoomStateOf(PlayerbotAI* botAI);

// The middle of the room the bot is in, which is the centroid of that room's Influence Tentacle
// summon group. Walking there brings the tentacles into line of sight. False outside the three rooms.
bool YoggSaronRoomMiddle(Player* player, Position& middle);

// Whether the Brain is safe to walk to and hit: every Influence Tentacle in this room dead and the door open. Damaging
// the Brain while one lives is Unit::Kill(who, who) on the attacker.
bool YoggSaronBrainRoomApproachable(PlayerbotAI* botAI);

// Whether Induce Madness is close enough that the bot must start for an exit portal: it strips all 100 Sanity below z
// 300, Insane's removal kills, and no Sanity Well reaches the brain level. Only once the bot is in the brain room.
bool YoggSaronShouldLeaveBrainLevel(PlayerbotAI* botAI);

// Whether the bot has already decided to leave for this Induce Madness. Latched until the cast ends, so
// a bot closing on the portal can't talk itself back to the Brain. Reads the latch only, no sweep.
bool YoggSaronLeavingBrainLevel(Player* bot);

// Cancels Bladestorm, Killing Spree and the like, as far as a player could. A portal's spell is cast by the user,
// untriggered, and those auras fail every untriggered cast. Returns the first spell cancelled, or 0.
uint32 YoggSaronCancelAbilityLock(Player* bot);

// Sanity stacks (63050). 0 with no aura, which is Insane or out of the fight.
uint32 YoggSaronSanityOf(Player* player);

// The spot a bot hits the Brain from once its room is done: behind it for melee, the portal spot of its room for the
// rest. The brain room middle when that spot cannot see the Brain or the room is unknown.
Position YoggSaronBrainSpot(Player* bot);

// A bot's phase 3 station. Healers take the melee spot with the tank (18.5 yd straight behind Yogg, outside his
// knockback). Ranged use a standby spot while a live Crusher is within PHASE_3_RANGED_CRUSHER_CLEARANCE of theirs.
Position const& YoggSaronPhaseThreeSpot(Player* bot);

// Laughing Skulls in gaze range and in the bot's front 180 degrees, the spell's exact test. The distance is rechecked
// after the grid sweep, whose own range test is bounding-radius inclusive.
std::vector<Unit*> GetYoggSaronSkullsInArc(PlayerbotAI* botAI);

// The same skulls without the arc test, for deciding where to stand: a candidate spot is judged on the heading it
// forces, not the bot's current facing.
std::vector<Position> GetYoggSaronSkullsInRange(PlayerbotAI* botAI);

// How far one heading is off another, 0 to pi either way round.
float YoggSaronHeadingGap(float from, float to);

// Skulls in the front half of a heading from (x, y): what Lunatic Gaze's HasInArc(pi) would hit.
uint32 YoggSaronSkullsAhead(std::vector<Position> const& skulls, float x, float y, float heading);

// The heading within ULDUAR_YOGG_SARON_SKULL_TARGET_TURN of `targetBearing` with the fewest skulls ahead,
// nearest the target on a tie, and how many that is.
uint32 YoggSaronAngledSkullHeading(std::vector<Position> const& skulls, float x, float y, float targetBearing,
                                   float& heading);

// Whether a bot at (x, y) attacking (targetX, targetY) has a heading it may keep that leaves every one
// of `skulls` outside its front 180 degrees, the exact filter spell_yogg_saron_lunatic_gaze uses.
bool YoggSaronFacingClearOfSkulls(std::vector<Position> const& skulls, float x, float y, float targetX,
                                  float targetY);

// The raider a Squeeze rescue should be spent on: lowest health first, nearest as the tie-break, in
// Hand of Protection's range, no Forbearance. Never the bot itself (a held paladin bubbles instead). nullptr when none.
Player* YoggSaronSqueezeVictim(PlayerbotAI* botAI);

// One rescuer per victim. True for the bot that takes the claim and false for everyone else until it
// lapses.
bool ClaimYoggSaronSqueezeRescue(PlayerbotAI* botAI, Player* victim);

// Somewhere other than a Crusher Tentacle for a pet to be, nearest first. nullptr when the pet has
// nothing else within reach, which is the only case worth silencing it for.
Unit* YoggSaronPetFallbackTarget(PlayerbotAI* botAI, Creature* pet);

// Phase 3: the leftover Corruptor or Constrictor every pet goes for, or nullptr: a holding Constrictor, then (no
// Thorim) one a beacon heal reaches, then lowest health, then GUID. Skips any with a swinging Crusher in Crush range.
Unit* YoggSaronPhase3PetLeftover(PlayerbotAI* botAI);

// Whether a mark held at a drag or hunter spot would heal this unit, see
// ULDUAR_YOGG_SARON_BEACON_HEAL_RADIUS. Geometry only, the callers check for Thorim.
bool YoggSaronBeaconHealReaches(Unit* unit);

// Record a Brain Link pair, from the spell hook that sees 63803 or 63804 go out. Both ends read the
// same record, so both walk.
void YoggSaronNoteBrainLinkPair(Unit* owner, Unit* partner);

// Who a Brain Linked bot should close on, whichever end of the pair it is, or nullptr when it is not linked, already
// close enough, or underground (the script drops the link past 10 yd of vertical separation).
Player* YoggSaronBrainLinkTarget(PlayerbotAI* botAI);

// Where the portal wave clock stands. ordinal counts waves seen, msToNextWave is the prediction the spread runs on,
// portalsUp is this bot's own sight of the ring: a bot underground sees nothing, which is not a wave ending.
struct YoggSaronPortalWave
{
    bool active = false;
    uint32 ordinal = 0;
    uint32 msToNextWave = 0;
    bool portalsUp = false;
};

YoggSaronPortalWave YoggSaronPortalWaveState(PlayerbotAI* botAI);

// Who goes down the portals, in an order every bot derives identically: melee dps, one healer, then ranged to fill,
// each by GUID, tanks never. Melee first: the room is a 60 s race on foot and it takes them out of the Crush ring.
std::vector<Player*> GetYoggSaronBrainTeam(PlayerbotAI* botAI);

// What this bot should be doing about the next portal wave.
enum YoggSaronPortalIntent : uint32
{
    YOGG_SARON_PORTAL_NOT_TEAM = 0,
    YOGG_SARON_PORTAL_WAITING = 1,
    YOGG_SARON_PORTAL_SPREADING = 2,
    YOGG_SARON_PORTAL_HOLDING = 3,
    YOGG_SARON_PORTAL_LATE = 4,
};

// Nearest-first, latched for the wave so it does not churn as bots move.
YoggSaronPortalIntent YoggSaronPortalPlan(PlayerbotAI* botAI, Position& spot);

// A brain team bot already walking to, standing on, or late for its portal. Anything that would pull it elsewhere
// stands down.
bool YoggSaronPortalWalkPending(PlayerbotAI* botAI);

// A brain team bot already on its portal spot waiting for the wave. Nothing else keeps it there: the portal node only
// walks on spreading or late, so reach would take it back to its target. Slot and distance first.
bool YoggSaronHoldingPortalSpot(PlayerbotAI* botAI);

// Damage dealer in an illusion room off the tentacles until healed. Latched in the tick, see
// ULDUAR_YOGG_SARON_REPRISAL_STOP_PCT; this only reads it.
bool YoggSaronReprisalHold(PlayerbotAI* botAI);

// Nearest living group healer in the bot's own illusion room, or null.
Player* YoggSaronRoomHealer(PlayerbotAI* botAI);

// What the interrupt trigger and action kick: Guardians casting Dark Volley, Immortal Guardians channelling Drain
// Life, marks first, most time left first, none under KICK_MIN_LEFT_MS. A held-back kicker gets only marks.
std::vector<Unit*> GetYoggSaronInterruptCasters(PlayerbotAI* botAI);

// A Guardian a beacon marks or a bot is dragging. A read of the drag claims, no sweep.
bool YoggSaronMarkCaster(Unit* caster);

// This bot is one of the kickers held back for the marks while a beacon lasts.
bool YoggSaronMarkKicker(Player* bot);

// Interrupts this bot can aim, in the order the action tries them, empty for a class with none. Avenger's Shield is
// left out: it picks its own target and cannot be pointed at a named Guardian.
std::vector<char const*> YoggSaronInterruptSpells(Player* bot);
bool YoggSaronCanInterrupt(Player* bot);

// The same list for one caster. An Immortal Guardian's immunity set takes silences but not interrupts,
// so Strangulate, Silencing Shot and Arcane Torrent would only burn their cooldowns on one.
std::vector<char const*> YoggSaronInterruptSpellsFor(Player* bot, Unit* caster);

// Whether this bot can kick the caster with this spell right now. CanCastSpell alone passes a pet's spell
// on cooldown and counts out of range as castable.
bool YoggSaronInterruptReady(PlayerbotAI* botAI, char const* spell, Unit* caster);

// Whether another bot's kick already holds this cast, and taking it for this bot. One kick per cast:
// Drain Life comes round every ~14.5 s per Guardian and the raid has about one kick a second.
bool YoggSaronInterruptClaimedByOther(Player* bot, Unit* caster);
void ClaimYoggSaronInterrupt(Player* bot, Unit* caster);

// Crushers mid Diminish Power channel, not in its 1.5 s cast: 64148 breaks the channel on a taken melee swing or
// melee-class spell, and only once it is channelling. A Judgement during the cast burns a 10 s cooldown for nothing.
std::vector<Unit*> GetYoggSaronChannellingCrushers(PlayerbotAI* botAI);

// Judgements in the order tried, empty for non-paladins. All three share one cooldown and every Judgement is melee
// class, so any breaks the channel. Range is 10 yd plus both combat reaches, ~19.5 yd from the tentacle's centre.
std::vector<char const*> YoggSaronJudgementSpells(Player* bot);

// Guardians whose death nova this bot should leave. Ranged and healers count one both about to die and chasing them; a
// Fervor holder every one in reach. Melee stand in novas by design; they count one only if it kills them.
std::vector<Unit*> GetYoggSaronNovaThreats(PlayerbotAI* botAI, float radius);

// What the next Shadow Nova is worth against `target`: the unresisted figure per low Guardian within the blast, at most
// two, doubled under Fervor. 0 when none is close to detonating. Counts melee too, unlike GetYoggSaronNovaThreats.
uint32 YoggSaronNovaDamageOn(Unit* target);

// Whether that kills `target` outright. False for a bot the nova only bruises, so the shortest cooldown is not wasted
// on it.
bool YoggSaronNovaWouldHurt(Unit* target);

// How long a button must last to still be up when the nova lands, or 0 when the bot needs none. `now` asks at the 10%
// gate (own button, healer external); the 20% gate is Divine Sacrifice's alone. Shared so trigger and action agree.
uint32 YoggSaronNovaNeedMs(Unit* target, bool now);

// Whether a living Guardian is close enough for its death nova to reach this bot. The nova nodes gate on this, not the
// phase: Guardians phase 1 leaves alive keep their death explosion and nothing in the core despawns them.
bool YoggSaronNovaNearby(PlayerbotAI* botAI);

// Whether the nova cooldowns are still being saved: all of phase 1, where nothing else is worth a three-minute
// cooldown, then only while a leftover Guardian is in reach, since phase 2 has Crush and the Death Rays.
bool YoggSaronNovaHold(PlayerbotAI* botAI);

// How many melee and tanks on the stack the next nova would take that far down, which is what decides
// whether the one raid-wide cooldown is worth spending.
uint32 YoggSaronNovaHurtCount(PlayerbotAI* botAI);

// The one paladin carrying Shadow Resistance Aura: lowest GUID of the bot paladins that know Aura Mastery. Prayer of
// Shadow Protection already gives the same 130 on exclusive aura 143 (highest wins), so the slot pays only if doubled.
bool IsYoggSaronShadowAuraPaladin(Player* bot);

// Whether this bot should be left alone to hold that aura, which is what keeps its own buff strategy
// from trading the exclusive slot back on the next GCD.
bool YoggSaronShadowAuraHeld(PlayerbotAI* botAI);

// Whether another paladin should keep off the aura and Aura Mastery: two copies coexist per caster and only the
// strongest works, so a second runner costs the elected one a half second hand-off at the press.
bool YoggSaronShadowAuraOtherHeld(PlayerbotAI* botAI);

// Which raid-wide cover this paladin can give the next nova, or null for none. Shared so the trigger
// and the action cannot pick different buttons.
char const* YoggSaronNovaRaidCover(PlayerbotAI* botAI);

// Whether a battle rez should wait for the handover. It strips every raid buff, so the bot returns on a 13-17% smaller
// bar without Prayer of Shadow Protection, and one nova then kills it from full health.
bool YoggSaronBattleRezHeld(PlayerbotAI* botAI);

// The raider a healer external belongs on: whoever the nova leaves furthest past dead, so a Fervor holder first, then
// the smallest pool. Never the bot itself; nullptr when nobody needs one.
Player* YoggSaronNovaVictim(PlayerbotAI* botAI);

// One external per victim, one raid-wide cooldown per nova. True for the bot that takes the claim, false for the rest
// until it lapses; the window claim stays true for its owner so it can retry across ticks.
bool ClaimYoggSaronNovaExternal(PlayerbotAI* botAI);
bool ClaimYoggSaronNovaWindow(Player* bot);

// The Guardian no dps should touch: the higher-health half of a low pair close enough for one nova to finish the other,
// guid on a tie, so one of a pair is always free to die.
void AppendYoggSaronTargetExclusions(PlayerbotAI* botAI, GuidSet& exclusions);

// Which of two phase 1 Guardians the raid should be on: one inside the leash beats one outside whatever the health (a
// kill out there does nothing for Sara), then lowest health. Shared so kill order and taunt agree across bots.
bool YoggSaronPhase1GuardianPreferred(Unit* candidate, Unit* incumbent);

// The two radii as predicates, so nothing has to remember which 15 is which: a Guardian counts if its nova still
// reaches Sara, and is on the stack if the blast stops at the melee pile instead of the 21.5 yd station.
bool YoggSaronGuardianCountsForSara(Unit* guardian);
bool YoggSaronGuardianOnTheStack(Unit* guardian);

// Inside ULDUAR_YOGG_SARON_P1_ROOM_RADIUS. Plain distance, no sweep, so it goes ahead of the phase read.
bool YoggSaronInPhase1Room(Player* bot);

// Record a Guardian's death, from the spell hook that sees its 65719 go out.
void YoggSaronNoteGuardianDeath(Unit* guardian);

// Whether any Guardian in the bot's instance died in the last `ms`.
bool YoggSaronGuardianDiedWithin(Player* bot, uint32 ms);

// Whether the raid may finish this Guardian where it stands: on the stack or above the park floor, never below the nova
// gap floor while the last nova is under 6 s old or a lower one is in chain range. No park floor without a bot tank.
bool YoggSaronPhase1GuardianKillable(PlayerbotAI* botAI, Unit* guardian);

// The one Guardian every non-tank is on in phase 1, shared per instance (per-bot holds split ranged and melee), or
// null. Held until it dies, stops being killable, or drifts past 15 yd with a killable one on the stack.
Unit* YoggSaronPhase1Focus(PlayerbotAI* botAI);

// True while a Guardian other than the focus sits below the gap floor within 10 yd of the middle or of the focus, where
// AoE kills it on nobody's schedule. Reads the focus without re-picking it.
bool YoggSaronPhase1AoeHold(PlayerbotAI* botAI);

// Guardians alive when Shadow Barrier lands keep the nova gap and the chain rule: their nova still hits for ~25k. No
// park floor, there is no stack left to keep them off.
bool YoggSaronLeftoverGuardianKillable(PlayerbotAI* botAI, Unit* guardian);

// Whether a killable Guardian should take the raid off the one it's on: under the gap floor and lower, so a low one
// does not bleed out unattended. Only under the floor, so two fresh ones trading places do not flip the raid.
bool YoggSaronLowGuardianTakesOver(Unit* candidate, Unit* current);

// After phase 1, true while a Guardian other than the bot's target sits below the gap floor within
// 10 yd of the bot or its target, where AoE would kill it out of turn.
bool YoggSaronLeftoverAoeHold(PlayerbotAI* botAI);

// The Guardian the tank should pull in, or null while every one is already where it should be or walking at a tank. The
// raid's focus first (a tank gets about one taunt per Guardian death), and only one outside the leash.
Unit* YoggSaronPhase1TauntTarget(PlayerbotAI* botAI);

// The Guardian a phase 1 tank should swing at, lowest health first, or null while none is in reach. Nothing else gives
// a tank a target here: the dps resolver stands down for one and the generic picker keeps a stale selection.
Unit* YoggSaronPhase1StackTarget(PlayerbotAI* botAI);

// Whether the raid still has a living bot tank to fetch Guardians with. The reach guard stands melee down on the
// promise that someone brings the target in, so a dead tank would strand the melee half out of combat.
bool YoggSaronBotTankAlive(PlayerbotAI* botAI);

// Where a cloud will be `leadMs` ahead, taken from its own facing and run speed.
Position YoggSaronCloudLead(Creature* cloud, uint32 leadMs = ULDUAR_YOGG_SARON_CLOUD_LEAD_MS);

// Whether a straight walk from the bot to (x, y) stays outside every cloud's summon radius.
bool YoggSaronRouteClearOfClouds(Player* bot, std::vector<Position> const& clouds, float x, float y);

// Whether a walk from the bot to `to` would cross a cloud now, halfway or on arrival, so it should wait (about 6 s).
// False while the bot already stands in one: leaving beats staying. Clouds near `exempt` are ignored.
bool YoggSaronWalkCrossesCloud(Player* bot, Position const& to, Position const* exempt = nullptr,
                               float exemptRadius = 0.0f);

// The window between Sara dying and Yogg emerging: Yogg present without SPELL_SHADOW_BARRIER, which APPEAR casts with
// the knockback. SetVisible(false) leaves him in grid searches, so it reads from the first tick.
struct YoggSaronHandover
{
    bool active = false;
    // Whether the handover owns where this bot stands (melee and tank only), decided once so every caller agrees. Stays
    // true for a hold after the ring lit: the window ends on that tick and a released bot walks back under it.
    bool clearing = false;
};

// A read of the window the encounter's tick latches.
YoggSaronHandover YoggSaronHandoverState(PlayerbotAI* botAI);

// The encounter's tick: latches the phase 1 to 2 handover window and the raid-wide plans.
void YoggSaronTick(PlayerbotAI* botAI);

// The body's knockback ring, which has no world object behind it and so cannot be swept for.
bool YoggSaronInBodyKnockback(Player* player);
bool YoggSaronRouteClearOfBody(Player* bot, float x, float y);

// Whether a straight walk from the bot to (x, y) stays out of every ray's clear radius, and inside one
// never gets closer than the bot already is.
bool YoggSaronRouteClearOfDeathRays(Player* bot, std::vector<Position> const& rays, float x, float y);

// A live Death Ray within its clear radius of a bot on the platform. A short sweep, no phase read.
bool YoggSaronDeathRayNear(PlayerbotAI* botAI);

// A point to cross the room through when the straight line would go over the body. One is enough:
// each leg halves the turn the next one has to make, so the bot walks the arc rather than the chord.
bool YoggSaronBodyDetour(Player* bot, Position const& destination, Position& waypoint);

// Where a walk to a fixed spot should aim this tick: the spot, or the detour waypoint when the straight
// line to it crosses the body. Latch a walk on the spot, never on this: the waypoint moves with the bot.
Position YoggSaronBodyRoute(Player* bot, Position const& spot);

// An Immortal Guardian still worth a taunt or a hit: alive, not Weakened and not being dragged off a
// Shadow Beacon. A Weakened one has no Empowered stacks left and only Titanic Storm can finish it.
bool IsYoggSaronHoldableGuardian(Unit* unit);

// Phase 3 without Thorim: the marked Guardian this bot drags clear of the pile, or nullptr. `claim` also takes one:
// tanks while another holds the pile, ret paladins and dps DKs, hunters, feral druids, by GUID; never one an unmarked
// Guardian hits.
Unit* YoggSaronBeaconDragGuardian(PlayerbotAI* botAI, bool claim);

// Stamps the taunt this bot just cast on the Guardian it drags.
void YoggSaronBeaconTaunted(Player* bot);

// Whether the bot holds a drag claim. A read of the claim map only, no sweep.
bool YoggSaronBeaconDragging(Player* bot);

// Where this bot waits for the next beacon, latched ULDUAR_YOGG_SARON_BEACON_STAGE_LEAD_MS before it's due, or nullptr.
// A hunter waits at its hunter spot. Only until the first mark shows up.
Position const* YoggSaronBeaconStageSpot(Player* bot);

// The drag spot of this bot's own drag, picked when it claimed, and the hunter spot on that side.
Position const& YoggSaronBeaconDragSpot(Player* bot);
Position const& YoggSaronBeaconHunterSpot(Player* bot);

// The healed Guardian this bot dragged, while still on it within ULDUAR_YOGG_SARON_BEACON_RETURN_MS of the release, or
// nullptr. The tank is ~39 yd away and a taunt reaches 30, so the dragger walks it back.
Unit* YoggSaronBeaconReturning(Player* bot);

// Which loose Guardian a tank taunts first, lowest first. 0: a healed one still on the paladin, DK or druid that
// dragged it. Their taunt matched top threat for good, so only a tank's taunt gets it off them. 2: one under a
// hunter's Distracting Shot, which goes back by itself. 1: anything else.
uint8 YoggSaronLooseGuardianTier(Player* bot, Unit* guardian);

// A Guardian the beacon heal is on, or still marked, that is hitting `victim` from the danger line up,
// or nullptr.
Unit* YoggSaronHealedGuardianOn(PlayerbotAI* botAI, Player* victim);

// Who the next external against a healed Guardian goes to: draggers first, then the lowest, humans too. A tank holding
// the pile only under BEACON_TANK_EXTERNAL_PCT. Skips a bot whose button is up and anyone already held.
Player* YoggSaronBeaconExternalVictim(PlayerbotAI* botAI);
bool ClaimYoggSaronBeaconExternal(PlayerbotAI* botAI, Player* victim);

// Holdable Guardians whose victim is this tank, from its own attacker set.
uint32 YoggSaronGuardiansOn(Player* tank);

// Guardians on this tank carrying a beacon heal another Guardian cast, at any health. A returned mark
// carries only its own, so it takes a heal that reached the pile.
uint32 YoggSaronPileHealedOn(Player* tank);

// Phase 3 without Thorim: the Guardian for this warrior's Mortal Strike, beacon heal first, then Drain Life, then
// low enough to cast it, lowest first. Needs melee reach, no healing cut yet, threat to spare.
Unit* YoggSaronMortalStrikeTarget(PlayerbotAI* botAI);

// The summon a warlock's non-combat pet strategy names ("summon felhunter" for nc +felhunter), or nullptr.
char const* YoggSaronWarlockSummon(PlayerbotAI* botAI);

// The tank this bot is taking the pile from, see ULDUAR_YOGG_SARON_TANK_SWAP_PCT, or nullptr. Starts a swap
// when the conditions hold and none is running, and ends one that no longer holds.
Player* YoggSaronTankSwapTake(PlayerbotAI* botAI);

// This bot is the holder of a running swap, so it keeps its area threat and taunts off the pile.
bool YoggSaronTankShedding(PlayerbotAI* botAI);

// A tank under the swap line while another is over the taker line leaves loose Guardians to that one.
bool YoggSaronTankLowHold(Player* bot);

// A phase 3 paladin tank keeps Avenging Wrath while Divine Protection is ready within its 30 s lock.
bool YoggSaronTankWrathHeld(PlayerbotAI* botAI);

bool YoggSaronBeforePhase3(PlayerbotAI* botAI);

// The tank a bot paladin should Lay on Hands, under ULDUAR_YOGG_SARON_TANK_LOH_PCT and in range, lowest
// first, humans too and the paladin itself if it tanks. nullptr when nobody needs it or it can't go out.
Player* YoggSaronTankLayOnHandsTarget(PlayerbotAI* botAI);
bool ClaimYoggSaronTankLayOnHands(PlayerbotAI* botAI, Player* tank);

// A healer in combat during the encounter with no target. Phase 3 gives healers none, and a bot with no
// target always drops to the non-combat engine, which has no mana cooldowns or potions.
bool YoggSaronHealerTargetless(PlayerbotAI* botAI);

// A healed Guardian is on this bot, so an immunity would just hand it to somebody else.
bool YoggSaronBeaconImmunityHeld(PlayerbotAI* botAI);

// A hunter whose drag claim hasn't seen the heal yet. Any damage on its Guardian is threat that keeps it
// there once the taunt ends, and Feign Death or Deterrence would undo the pull.
bool YoggSaronBeaconHunterHold(PlayerbotAI* botAI);

// A druid staged for a beacon, dragging, or walking its Guardian back. Growl needs a bear form, and the cat
// strategy would shift it straight back to Cat Form.
bool YoggSaronBeaconBearHeld(PlayerbotAI* botAI);

// A druid in a bear form, or knowing one and with the mana for it. The bot's spell check skips the cost.
bool YoggSaronBearReady(Player* druid);

// Deafening Roar is due this phase 3: 25-man, fewer than 4 Keepers. Latched by the tick.
bool YoggSaronRoarExpected(PlayerbotAI* botAI);
bool YoggSaronRoarCasting(PlayerbotAI* botAI);

// The bot paladin whose Aura Mastery covers the next roar: off cooldown by then, own Concentration Aura already up,
// lowest GUID. A dragger only if nobody else can, since its 40 yd reach from the drag spot covers almost nobody.
Player* YoggSaronRoarPaladin(PlayerbotAI* botAI);

// This bot is the roar paladin and the roar is under ULDUAR_YOGG_SARON_ROAR_PRESWITCH_MS away or being
// cast, so it keeps Concentration Aura up.
bool YoggSaronRoarAuraHeld(PlayerbotAI* botAI);

// One Aura Mastery per roar. The roar paladin takes it at once, anyone else after the handoff delay, and
// the owner keeps it so a failed press can retry.
bool ClaimYoggSaronRoar(PlayerbotAI* botAI);

// The brain team holds off the Brain, see ULDUAR_YOGG_SARON_BRAIN_HOLD_PCT. Latched per bot in the tick.
// The DoT hold is true under a full hold too.
bool YoggSaronBrainHeld(PlayerbotAI* botAI);
bool YoggSaronBrainDotHeld(PlayerbotAI* botAI);

// Lunatic Gaze is up on Yogg (64163 is a 4 s aura on him, not a channel), and whether a target sits far
// enough round from him that the bot can hit it with Yogg out of its front arc.
bool YoggSaronGazeActive(PlayerbotAI* botAI);
bool YoggSaronOutsideGaze(Player* bot, Unit* yogg, Unit* target);

// Whether a shaman should cast Heroism or Bloodlust now: phase 3, and the raid at its stations or in
// reach of its targets, see ULDUAR_YOGG_SARON_LUST_POSITION_SHARE, or the fallback has run out.
bool YoggSaronPhase3LustReady(PlayerbotAI* botAI);
bool YoggSaronHoldableGuardianWithin(Player* bot, float radius);

// The Immortal Guardians "nearest npcs" holds, in its order. The entry is tested before the line of sight ray, so
// only they get one.
GuidVector GetYoggSaronGuardiansInSight(PlayerbotAI* botAI);

// A holdable Guardian among those whose victim is not a tank, so a taunt is owed. The second form takes a list
// already read.
bool YoggSaronGuardianLoose(PlayerbotAI* botAI);
bool YoggSaronGuardianLoose(PlayerbotAI* botAI, GuidVector const& guardians);

// Whether a non-tank may hit this Guardian: a tank is its victim and leads the bot's threat on it by the start share
// (keep share if already on it). With no tank alive in the group everyone may.
bool YoggSaronGuardianThreatAllows(Player* bot, Unit* guardian, bool holding);

// This tank's threat on the Guardian over the highest non-tank's, pets included. Zero when nobody has
// any, since heal threat is about to land there; float max when only tanks do.
float YoggSaronTankThreatLead(Player* tank, Unit* guardian);

// A Constrictor with somebody squeezed in its seat.
bool YoggSaronConstrictorHolding(Unit* unit);

// Any live Influence Tentacle within `radius` of `centre` (default: the bot), disguise included: a tentacle is
// re-stamped as another entry at spawn and reverts to 33943 only when damaged, so a sweep for 33943 alone misses it.
Unit* YoggSaronLiveIllusionMob(PlayerbotAI* botAI, float radius, Position const* centre = nullptr);

// Every live illusion mob in the bot's own room, nearest first. Same sweep as YoggSaronLiveIllusionMob, which answers
// "is this room clear" and so returns whichever one the grid visit hands back; this one is for walking somewhere.
std::vector<Unit*> GetYoggSaronRoomIllusionMobs(PlayerbotAI* botAI);

// Whether this bot may take this tentacle; a bot done with its share waits at the door instead of piling onto the last
// one. Icecrown: own group, then another once its own is clear. Stormwind: door-side tentacle keeps clearing.
bool YoggSaronIllusionTentacleAllowed(PlayerbotAI* botAI, Unit* tentacle);

// The room isn't clear but nothing left in it is this bot's to take, so it waits at the door.
bool YoggSaronIllusionStaged(PlayerbotAI* botAI);

// Where a staged bot waits. False outside Icecrown and Stormwind.
bool YoggSaronIllusionStageSpot(Player* bot, Position& spot);

// Where the brain team's healer stands in an illusion room: the middle of its live tentacles, where the melee are. The
// room middle is the summon group centroid, so they ring it (36-44 yd in the Chamber, past the 38.5 yd heal range).
bool YoggSaronIllusionHealerStation(PlayerbotAI* botAI, Position& station);

// Whether the Brain is safe to hit: every Influence Tentacle in this room dead. Scoped to the room, not a 200 yd sweep,
// measured from the room's middle so a bot in its doorway does not read the far half as empty.
bool YoggSaronInfluenceTentaclesCleared(PlayerbotAI* botAI);

// Nothing on the platform to kill: no Guardian, Crusher, Constrictor or Corruptor alive.
bool YoggSaronPlatformIdle(PlayerbotAI* botAI);

// Live Crushers as position plus the facing their Crush cone follows. Skips the one hitting the bot and any with no
// victim. Shared by trigger (tight arc) and action (wide arc) so a re-facing tentacle does not restart the dance.
std::vector<Position> GetYoggSaronCrushWedges(PlayerbotAI* botAI, float searchRadius);
bool InYoggSaronCrushWedge(std::vector<Position> const& wedges, float x, float y, float arcDegrees);

// Every live Crusher for ranged and healers, with no victim exemption and no swing gate: an idle one still takes a bot
// that walks in and hits it. Melee only get a stunned one whose stun is about to lift.
std::vector<Position> GetYoggSaronCrusherReaches(PlayerbotAI* botAI, float searchRadius);
bool YoggSaronInCrusherReach(PlayerbotAI* botAI, float radius);

// A Crusher under Shattered Illusion cannot hurt a bot in its reach. True while that holds for STUN_EXIT_LEAD_MS more:
// Induce Madness still casting with more than the lead left, Brain above STUN_BRAIN_FLOOR_PCT (hold guard if held).
bool YoggSaronCrusherStunHolds(PlayerbotAI* botAI, Unit* crusher);

// Whether a forced walk is still closing. MoveTo's `ok` only says a command was issued; the core falls back to a
// straight-line spline, so closing distance is the only evidence of a path. Records are per node and destination.
bool YoggSaronWalkMakingProgress(PlayerbotAI* botAI, char const* node, Position const& destination);

// Drops an unfinished MoveTo booking once the bot has stood still past WALK_STALL_MS. MoveTo refuses the point it
// already issued for MaxWaitForMove (5 s) whether the bot still walks or not. `yogg.walkstep` counts how often.
void YoggSaronReleaseStalledWalk(PlayerbotAI* botAI, char const* node, Position const& destination);

// Window in which a counterable fear can land, for the shared anti-fear component: only Malady of the Mind, cast in P2
// but re-cast from the feared player on removal, so it can outlive P2. Deafening Roar in P3 is a silence.
bool YoggSaronFearWindowActive(PlayerbotAI* botAI);

#endif
