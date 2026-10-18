/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERFREYA_H
#define PLAYERBOTS_ULDENCOUNTERFREYA_H

#include "EncounterHelpers.h"
#include "ObjectGuid.h"
#include "Position.h"
#include "UldData.h"

#include <vector>

class GameObject;
class Player;
class PlayerbotAI;
class Unit;

enum UlduarFreyaIds
{
    NPC_SNAPLASHER = 32916,
    NPC_STORM_LASHER = 32919,
    NPC_DETONATING_LASHER = 32918,
    NPC_ANCIENT_WATER_SPIRIT = 33202,
    NPC_ANCIENT_CONSERVATOR = 33203,
    NPC_HEALTHY_SPORE = 33215,
    NPC_EONARS_GIFT = 33228,
    GOBJECT_NATURE_BOMB = 194902,
    // +8% healing received per stack, 150 stacks on engage: damage on Freya is wasted until the adds strip it.
    SPELL_ATTUNED_TO_NATURE = 62519,

    // Hard mode. 2s cast, raid-wide interrupt with a 10s school lockout on whoever it cuts, nothing to dodge.
    // Hunters are exempt: Steady Shot is PreventionType PACIFY, which never reaches the lockout branch.
    SPELL_FREYA_GROUND_TREMOR_10 = 62437,
    SPELL_FREYA_GROUND_TREMOR_25 = 62859,

    // Unlike Ground Tremor this one can be left: 1.5s cast on a random threat-list target, 8 yd at its feet.
    SPELL_FREYA_SUNBEAM_10 = 62623,
    SPELL_FREYA_SUNBEAM_25 = 62872,

    // Conservator's mark: a 10s aura ticking every 2s, so five 8 yd hits around whoever wears it.
    SPELL_NATURES_FURY_10 = 62589,
    SPELL_NATURES_FURY_25 = 63571,

    // Freya hard mode: Elders left alive permanently empower Freya with an extra ability each.
    // NPC_FREYA comes from core ulduar.h via UldScripts.h.
    NPC_FREYA_IRON_ROOTS = 33088,               // Ironbranch's Iron Roots trap (selectable)
    NPC_FREYA_STRENGTHENED_IRON_ROOTS = 33168,  // Freya's empowered Iron Roots trap (selectable)
    NPC_FREYA_SUN_BEAM = 33170,                 // Freya's Unstable Sun Beam stalker (non-selectable)
    NPC_FREYA_UNSTABLE_SUN_BEAM = 33050,        // Brightleaf's Unstable Sun Beam stalker (non-selectable)
    // HasAura takes an exact id and a 25-man raid only gets the 25-man half (spelldifficulty_dbc 62283 -> 62930,
    // 62861 -> 62438), so both halves have to be listed.
    SPELL_IRON_ROOTS_DAMAGE_10 = 62283,        // DoT on a player trapped by Ironbranch's roots
    SPELL_IRON_ROOTS_DAMAGE_25 = 62930,
    SPELL_IRON_ROOTS_FREYA_DAMAGE_10 = 62861,  // DoT on a player trapped by Freya's roots
    SPELL_IRON_ROOTS_FREYA_DAMAGE_25 = 62438,

    // On allies within 6 yd of a Healthy Spore, immunity to Conservator's Grip. 62541 is the spore's own aura,
    // this is the id that lands on players.
    SPELL_POTENT_PHEROMONES = 64321,
};

// Hard mode: how far out of an Unstable Sun Beam bots step. The pool itself (62865) is only 5 yd.
inline constexpr float ULDUAR_FREYA_UNSTABLE_SUN_BEAM_RADIUS = 12.0f;

// Aim 3 yd past the trigger radius as hysteresis: a one yard margin only gets a ~2 yd step from
// FindNearestPositionClearOfHazards, and the next beam spawn re-triggers it.
inline constexpr float ULDUAR_FREYA_SUN_BEAM_CLEARANCE = 15.0f;

// Ceiling on the escape latch. Long enough to walk the clearance at 7 yd/s, short enough that a bot
// rooted mid-dodge hands the tick back instead of holding it for the rest of the wave.
inline constexpr uint32 ULDUAR_FREYA_SUN_BEAM_LATCH_MS = 3000;

// Sunbeam lands where its target is when the cast ends (SelectSpellTargets runs from Spell::cast), so only the bots
// around the target can leave. Trigger at AVOID, aim at CLEAR, same split as the Nature Bomb.
inline constexpr float ULDUAR_FREYA_SUNBEAM_AVOID_RADIUS = 11.0f;
inline constexpr float ULDUAR_FREYA_SUNBEAM_CLEAR_RADIUS = 13.0f;

// The 1.5s cast. Any longer holds the bot still after the beam has already landed.
inline constexpr uint32 ULDUAR_FREYA_SUNBEAM_LATCH_MS = 1500;

// Each trio member revives 11s after its own death unless all three are down, so they have to die together.
// Wide on purpose: the band only covers the last tenth of the wave, about 6s of raid damage.
inline constexpr float ULDUAR_FREYA_TRIO_SYNC_WINDOW_PCT = 30.0f;    // below this the trio outranks other adds
inline constexpr float ULDUAR_FREYA_TRIO_FLOOR_RELEASE_PCT = 15.0f;  // all members below: free burn to the finish
inline constexpr float ULDUAR_FREYA_TRIO_HARD_FLOOR_PCT = 10.0f;     // never cross while a sibling is still high

// Potent Pheromones (64321) radius around a Healthy Spore, the only counter to the raid-wide Conservator's Grip.
inline constexpr float ULDUAR_FREYA_SPORE_RADIUS = 6.0f;

// Raid members that make a spore full, so the back line spreads instead of piling onto the nearest. Six puts
// the melee spore over the line from the start: both of this wave's big hits are 8 yd wide, a spore is 6.
inline constexpr uint32 ULDUAR_FREYA_SPORE_CROWD = 6;

// One Nature's Fury tick around the carrier: EffectRadiusIndex 14 on the triggered 63570.
inline constexpr float ULDUAR_FREYA_NATURES_FURY_RADIUS = 8.0f;

// Where the carrier aims. Past the radius, since landing on the edge gets a 2 yd step that combat movement
// undoes before the next tick.
inline constexpr float ULDUAR_FREYA_NATURES_FURY_CLEAR = 11.0f;

// Ceiling on the bail latch, about the walk to the next spore at 7 yd/s. The mark itself runs 10s.
inline constexpr uint32 ULDUAR_FREYA_NATURES_FURY_LATCH_MS = 3000;

// Where a bot stops: inside the aura, clear of the spore's collision. The centre is unreachable and MoveTo would
// retry it forever. The trigger stands down a yard further out as hysteresis.
inline constexpr float ULDUAR_FREYA_SPORE_STAND_RANGE = 4.0f;

// Spores spawn 20 yd from the Conservator, plus room for the boss having been dragged part way to one.
inline constexpr float ULDUAR_FREYA_SPORE_SEARCH_RADIUS = 40.0f;

// Detonate fires on death, not on a timer. 62598 (10-man) and 62937 (25-man) both carry EffectRadiusIndex 18.
inline constexpr float ULDUAR_FREYA_DETONATE_RADIUS = 15.0f;

// Melee never chase a lasher past this. Tight on purpose: "nearby" means the lasher came to the melee group.
inline constexpr float ULDUAR_FREYA_MELEE_LASHER_RANGE = 12.0f;

// Margin before the add tank moves between two near-equal trio members. Its own damage is what closes
// the gap, so without this it would swap every few ticks and lose swing timers to nothing.
inline constexpr float ULDUAR_FREYA_TANK_TRIO_SWITCH_PCT = 5.0f;

// Nature Bomb (64587): 10 yd in both raid sizes, at the target's feet. No margin: the last-resort escape only
// leaves the circles it's standing in, since a volley of seven to ten often leaves nowhere that clears them all.
inline constexpr float ULDUAR_FREYA_NATURE_BOMB_BLAST_RADIUS = 10.0f;

// The extra yard covers the bot's own reach, so it doesn't clip the blast's edge while holding still.
inline constexpr float ULDUAR_FREYA_NATURE_BOMB_AVOID_RADIUS = 11.0f;

// Where the escape aims, deliberately past the trigger radius: landing on the boundary would re-fire
// the node every tick as combat movement pulls the bot back toward its target.
inline constexpr float ULDUAR_FREYA_NATURE_BOMB_CLEAR_RADIUS = 13.0f;

// Has to outlast the fuse, which is 6s, not the 11s the timer reads: boss_freya_nature_bomb::UpdateAI fires at
// _explodeTimer >= 11000 but the branch under it snaps the timer from 5000 straight to 10000.
inline constexpr uint32 ULDUAR_FREYA_NATURE_BOMB_LATCH_MS = 7000;

// Ceiling on the main tank's reposition. Long enough to walk the clearance at 7 yd/s, short enough
// that a tank stopped on the way hands the tick back instead of holding Freya still for a whole volley.
inline constexpr uint32 ULDUAR_FREYA_TANK_BOMB_LATCH_MS = 3000;

// Where the main tank holds Freya, checked on the navmesh with the bots' path filter.
extern const Position ULDUAR_FREYA_TANK_ANCHOR;

// How far the tank may take her from the anchor. Every bomb escape aims away from the back line, so without a
// leash the drift compounds across the room.
inline constexpr float ULDUAR_FREYA_TANK_LEASH = 20.0f;

// Ceiling on the walk home. A re-issued MoveTo clears the motion master out from under the walk it repeats.
inline constexpr uint32 ULDUAR_FREYA_TANK_HOLD_LATCH_MS = 3000;

// How far the escapes look for hazards. Freya's come in clusters (a bomb per player, ten lashers, overlapping
// beams), so the bot has to see the whole cluster or it walks out of one and into the next.
inline constexpr float ULDUAR_FREYA_HAZARD_SEARCH_RADIUS = 30.0f;

// Ring and bearing steps of the main tank's bomb sweep.
inline constexpr float ULDUAR_FREYA_SWEEP_STEP = 2.0f;
inline constexpr float ULDUAR_FREYA_SWEEP_ANGLE_STEP = static_cast<float>(M_PI) / 8.0f;

// The root creature is summoned on the bot it traps, so it's always this close.
inline constexpr float ULDUAR_FREYA_IRON_ROOTS_SEARCH_RADIUS = 10.0f;

// Lasher wave: one camp, the lashers come to it and get AoE'd. Under FINISH_PCT they're picked off one by one
// so the blasts stagger. PACK_RADIUS / MIN_COUNT are MediumAoeTrigger's 8 yd / 3, PACK_CLEAR one past Detonate.
inline constexpr float ULDUAR_FREYA_LASHER_PACK_RADIUS = 8.0f;
inline constexpr float ULDUAR_FREYA_LASHER_PACK_CLEAR = 16.0f;
inline constexpr uint32 ULDUAR_FREYA_LASHER_PACK_MIN_COUNT = 3;
inline constexpr float ULDUAR_FREYA_LASHER_FINISH_PCT = 20.0f;

// Frost Nova is a 10 yd sphere centred on the caster, so this is also how close the lashers have to
// have closed on the mage for it to reach anything.
inline constexpr float ULDUAR_FREYA_FROST_NOVA_RADIUS = 10.0f;
inline constexpr uint32 ULDUAR_FREYA_FROST_NOVA_MIN_LASHERS = 2;

// Frost Trap goes down at the hunter's feet once a lasher is this close, so it's armed when the lasher arrives.
// Lashers have no creature_immunities row, so the -50% from 13810 lands: 8.0 yd/s down to 4.0, under a player's 7.0.
inline constexpr float ULDUAR_FREYA_FROST_TRAP_ARM_RANGE = 20.0f;

// A lasher this low is about to blow, typically with just the walk out of its 15 yd blast left. Higher costs
// more damage than it saves.
inline constexpr float ULDUAR_FREYA_LASHER_BAIL_PCT = 15.0f;

// Holds the step-out's exit. Lashers die one at a time, so re-derived each tick the exit shifts a yard and every
// MoveTo resets the walk instead of finishing it.
inline constexpr uint32 ULDUAR_FREYA_LASHER_BAIL_LATCH_MS = 3000;

// How far the back line steps from each lasher under LASHER_BAIL_PCT, not from the pack centre (they spread
// ~17 yd). 18 clears the 15 yd blast and still shoots.
inline constexpr float ULDUAR_FREYA_LASHER_CAMP_STANDOFF = 18.0f;

// Eonar's Gift heals Freya 30-60% if it lives 12s. Five ranged kill it well inside that and leave the rest on
// the lasher pack.
inline constexpr uint32 ULDUAR_FREYA_GIFT_SHARE = 5;

// If the share has not finished the Gift by now, every ranged bot joins it. Leaves about 3s of margin
// on the 12s.
inline constexpr uint32 ULDUAR_FREYA_GIFT_SHARE_MS = 5000;

// How tight the camp holds. Ranged are pulled in harder because the ball has to fit inside one AoE;
// healers get the slack, since they also have to stay in range of the melee group and the tanks.
inline constexpr float ULDUAR_FREYA_RANGED_CAMP_TOLERANCE = 10.0f;
inline constexpr float ULDUAR_FREYA_HEALER_CAMP_TOLERANCE = 15.0f;

// Bots stop this short of the camp spot on the bearing they came in on, so the back line forms a ring with no
// shared state. Stops the pile, can't beat an 8 yd splash on its own.
inline constexpr float ULDUAR_FREYA_RANGED_CAMP_SPACING = 6.0f;

// Hardened Bark is at 99 stacks 15s into a trio wave, and one swing then kills anyone but a tank. Aggro flips at
// 110% in melee, so a damage dealer this close to the tank's threat moves to a sibling until the tank is ahead.
inline constexpr float ULDUAR_FREYA_SNAPLASHER_THREAT_SHED_PCT = 80.0f;
inline constexpr float ULDUAR_FREYA_SNAPLASHER_THREAT_RETURN_PCT = 60.0f;

// The tank's head start on a fresh Snaplasher. A ratio can't cover the opener: the tank's first hit is a few
// thousand threat and one global of raid damage is several times that.
inline constexpr uint32 ULDUAR_FREYA_SNAPLASHER_TANK_LEAD_MS = 5000;

// Freya. Everything the encounter needs from one grid pass, so the priority action, the tank action
// and both multipliers cannot disagree about what is up.
struct FreyaWaveState
{
    Unit* eonarsGift = nullptr;
    Unit* conservator = nullptr;
    Unit* snaplasher = nullptr;
    Unit* stormLasher = nullptr;
    Unit* waterSpirit = nullptr;
    std::vector<Unit*> detonatingLashers;

    std::vector<Unit*> LivingTrio() const;

    // Any living member below the sync window: the raid must finish this trio before it touches
    // anything else, or the members already low revive.
    bool TrioLocked() const;

    // Every living member at or below the release threshold: the last seconds, nothing pulls a bot away and no
    // member is held back.
    bool TrioReleased() const;
};

void GatherFreyaWaveState(PlayerbotAI* botAI, FreyaWaveState& state);

// Per-bot cache for lookups every Freya node repeats each tick. Keyed on getMSTime(), holds guids so a despawn
// drops out. Never across ticks: a queued action can run up to Playerbots.ExpireActionTime later.
class FreyaScan
{
public:
    explicit FreyaScan(PlayerbotAI* botAI) : botAI(botAI) {}

    // "find target" freya. Null until she has this bot on her threat list.
    Unit* Boss();

    // "possible targets no los", exactly as the value returns it.
    GuidVector const& PossibleTargets();

    // Healthy Spores and both Sun Beam stalkers, as "nearest npcs" returns them. One visit for all three, since the
    // spore pick and the hazard sweep both read them in the same tick.
    GuidVector const& Stalkers();

    // Where the back line gathers: the anchor bot. The trigger, isUseful and Execute all ask for it in one tick.
    Position const& LasherCamp();

private:
    PlayerbotAI* botAI;

    uint32 bossAtMs = 0;
    ObjectGuid boss;
    uint32 targetsAtMs = 0;
    GuidVector targets;
    uint32 stalkersAtMs = 0;
    GuidVector stalkers;
    uint32 campAtMs = 0;
    Position camp;
};

// This bot's FreyaScan, held by the "freya scan" value.
FreyaScan& GetFreyaScan(PlayerbotAI* botAI);

// First living unit with this entry in the scan's target list: the same list and the same test as
// GetUldUnitByEntry, off one sweep a tick instead of one per call.
Unit* GetFreyaScanUnitByEntry(PlayerbotAI* botAI, uint32 entry);

// Freya via a visit that tests her entry first. The Ground Tremor nodes also run on trash and in other fights,
// where nothing else builds the full target list.
Unit* GetFreyaBossByEntry(PlayerbotAI* botAI);

// Whether damage on this trio member has to stop so the three converge. Backstop for damage targeting can't
// steer (a swing mid-animation, a ticking DoT). GetFreyaTrioAssignment normally moves bots off first.
bool FreyaTrioSyncSuppress(FreyaWaveState const& state, Unit* target);

// Greedy split over remaining health, recomputed each tick: every bot reaches the same split, no shared state.
// Suppressed members drop out, so the floor moves bots to a sibling instead of idling them.
Unit* GetFreyaTrioAssignment(PlayerbotAI* botAI, FreyaWaveState const& state);

// Main tank: Freya. Add tank: Snaplasher, Conservator, highest-health trio member, a lasher on top of it, Freya.
// Highest health since tank damage is invisible to GetFreyaTrioAssignment, so it helps the three converge.
Unit* GetFreyaTankTarget(PlayerbotAI* botAI, FreyaWaveState const& state, Unit* currentTarget);

// Nearest living lasher in range, sticky on currentTarget with a switch margin. Range is also the leash, so a
// lasher that retargets can't tow the bot across the room.
Unit* GetFreyaLocalLasherTarget(PlayerbotAI* botAI, FreyaWaveState const& state, Unit* currentTarget, float range);

// The lasher every ranged bot should be on. Lowest health, GUID breaking ties: raid-wide agreement with
// no shared state, and self-stabilising, since the add being focused stays the lowest.
Unit* GetFreyaRangedLasherFocus(FreyaWaveState const& state);

// The spore the Conservator is parked on. Keyed off the Conservator, never the caller, so the dragging tank and
// the melee agree on it without talking.
Unit* GetFreyaConservatorSpore(PlayerbotAI* botAI, Unit* conservator);

// Melee get the parked spore, ranged and healers the nearest one with room. Trigger and action must both use
// this, or a bot wakes for one spore and walks to another.
Unit* GetFreyaTargetSpore(PlayerbotAI* botAI);

// Every living Healthy Spore the bot can see.
std::vector<Unit*> GetFreyaSpores(PlayerbotAI* botAI);

// Living raid members within radius of a point, except one (normally the bot asking).
uint32 CountFreyaRaidNear(PlayerbotAI* botAI, Position const& centre, float radius, Player* except);

// Nearest spore with nobody else in the splash, so the carrier keeps Potent Pheromones. Null when none is free,
// and the bail settles for open floor and the Grip.
Unit* GetFreyaNaturesFuryShelter(PlayerbotAI* botAI);

// True while any bot in the group that counts as ranged DPS is alive. Eonar's Gift is a ranged job,
// but a melee-only raid still has to kill it or Freya heals 30-60%.
bool FreyaHasLivingRangedDps(PlayerbotAI* botAI);

// Every live Nature Bomb near the bot. GameObjects, not creatures: the bomb NPC is banished and never
// shows up in the npc value lists.
std::vector<Position> GetFreyaNatureBombPositions(Player* bot, float searchRadius);

// Every live Sun Beam near the bot. Takes the AI rather than the Player because the beam stalkers are
// non-selectable and only ever appear in the raw nearby-npc list.
std::vector<Position> GetFreyaSunBeamPositions(PlayerbotAI* botAI, float searchRadius);

// Every hazard both escapes route around, each with its clearance. A bot reading only its own kind steps from a
// bomb into a beam and back, and at equal relevance that never settles.
std::vector<EncounterHelpers::HazardCircle> GetFreyaEscapeHazards(PlayerbotAI* botAI, float searchRadius);

// The same list from hazards the caller already holds, so an escape that collected the bombs to test
// its own spot does not collect them again to route around them.
std::vector<EncounterHelpers::HazardCircle> BuildFreyaEscapeHazards(std::vector<Position> const& bombs,
                                                                    std::vector<Position> const& beams,
                                                                    std::vector<Position> const& lashers = {});

// Low lashers an escape has to clear. None for melee and tanks: they keep `freya lasher about to blow`, and
// owing them the whole pile halves their damage.
std::vector<Position> GetFreyaEscapeLasherPositions(PlayerbotAI* botAI, float searchRadius);

// Where the back line gathers: the lowest-GUID living ranged DPS bot here, so everyone picks the same one. A live
// bot keeps the camp on the mesh and in reach of what the raid is shooting.
Player* GetFreyaRangedCampAnchor(PlayerbotAI* botAI);

// This bot's place in the GUID order of the living ranged DPS, so a job can be handed to the first few
// of them without shared state. UINT32_MAX for anyone who is not ranged DPS.
uint32 GetFreyaRangedDpsRank(PlayerbotAI* botAI);

// Where the back line balls up for the AoE: on the anchor bot. Never a computed point, a live bot is always on
// the mesh. Empty when no ranged DPS bot is alive.
Position GetFreyaLasherCampSpot(PlayerbotAI* botAI);

// Whether this damage dealer has to leave the Snaplasher to a tank for now. `shedding` is the caller's own
// memory of the last answer, which is what keeps a bot at the line from swapping target every tick.
bool FreyaShedsSnaplasherThreat(PlayerbotAI* botAI, Unit* snaplasher, uint32 seenForMs, bool shedding);

// Living lashers under maxPct within radius. The health filter is the point: clearing every lasher would push
// melee out of the fight.
std::vector<Position> GetFreyaLowLasherPositions(PlayerbotAI* botAI, FreyaWaveState const& state, float maxPct,
                                                 float radius);

// Lasher with the most lashers around it, lowest GUID on ties. AoeTrigger counts attackers near the current
// target, not the bot, so pointing the raid at the middle of the pile is what makes class AoE fire.
Unit* GetFreyaLasherPackFocus(FreyaWaveState const& state);

// Whether the pack around this point is ready for the staggered finish: at least PACK_MIN_COUNT lashers
// inside PACK_RADIUS and every one of them at or below FINISH_PCT.
bool IsFreyaLasherPackFinishing(FreyaWaveState const& state, Position const& centre);

// The finishing pack within radius of this bot, or nullptr. Measured from the pack's own centre rather
// than from the bot, so a bot standing at the edge of the pile still sees the finish it is feeding.
Unit* GetFreyaFinishingPackNear(PlayerbotAI* botAI, FreyaWaveState const& state, float radius);

// Takes a point rather than a bot: the same count is wanted around a candidate pack focus, and around
// the bot itself for the nova and the trap.
uint32 CountFreyaLashersNear(Position const& centre, FreyaWaveState const& state, float radius);

// Freya is mid-cast on Ground Tremor. Matches both difficulty ids, since 62437 is the 10-man twin.
bool IsFreyaGroundTremorCasting(Unit* boss);

// Whoever Freya's Sunbeam is aimed at, or null when she isn't casting it. Pets count, a pet running into the raid
// mid-cast drags the beam with it.
Unit* GetFreyaSunbeamTarget(Unit* boss);

#endif
