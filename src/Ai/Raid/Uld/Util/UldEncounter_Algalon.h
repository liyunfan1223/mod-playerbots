/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERALGALON_H
#define PLAYERBOTS_ULDENCOUNTERALGALON_H

#include "EncounterHelpers.h"
#include "ObjectGuid.h"
#include "Position.h"
#include "UldData.h"

#include <array>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

// Algalon the Observer.

enum UlduarAlgalonIds
{
    PB_NPC_ALGALON = 32871,
    PB_NPC_LIVING_CONSTELLATION = 33052,
    PB_NPC_COLLAPSING_STAR = 32955,
    PB_NPC_BLACK_HOLE = 32953,
    PB_NPC_WORM_HOLE = 34099,
    PB_NPC_UNLEASHED_DARK_MATTER = 34097,
    NPC_ALGALON_ASTEROID_TARGET_1 = 33104,
    NPC_ALGALON_ASTEROID_TARGET_2 = 33105,
    SPELL_ALGALON_BIG_BANG = 64443,
    SPELL_ALGALON_BIG_BANG_25 = 64584,
    SPELL_ALGALON_PHASE_PUNCH = 64412,
    // Externals put on the Big Bang soaker, matched by aura so two healers don't double up.
    SPELL_ALGALON_PAIN_SUPPRESSION = 33206,
    SPELL_ALGALON_GUARDIAN_SPIRIT = 47788,
    SPELL_ALGALON_HAND_OF_SACRIFICE = 6940,
    SPELL_ALGALON_DISPERSION = 47585,
    SPELL_ALGALON_WEAKENED_SOUL = 6788,
    SPELL_ALGALON_ICE_BLOCK = 45438,
    SPELL_ALGALON_DIVINE_SHIELD = 642,
    SPELL_ALGALON_CLOAK_OF_SHADOWS = 31224,
    SPELL_ALGALON_ANTI_MAGIC_SHELL = 48707,
    // The star's drain: CountPctFromMaxHealth(1) every 1000 ms from its spawn.
    SPELL_ALGALON_COLLAPSE = 62018,
    SPELL_ALGALON_AURA_MASTERY = 31821,
    SPELL_ALGALON_DIVINE_SACRIFICE = 64205,
    // Procced by the talent on a Divine Sacrifice cast: the raid within 30 yd takes 10/20% less for 6 s, and it
    // stays when the sacrifice is cancelled.
    SPELL_ALGALON_DIVINE_GUARDIAN = 70940,
    SPELL_ALGALON_DIVINE_GUARDIAN_RANK_1 = 53527,
    SPELL_ALGALON_DIVINE_GUARDIAN_RANK_2 = 53530,
};

// He casts Ascend the moment he leaves a 47 yd disc around home or drops under Z 410.
inline constexpr float ULDUAR_ALGALON_ROOM_RADIUS = 47.0f;
inline constexpr float ULDUAR_ALGALON_ROOM_Z_MIN = 410.0f;
inline constexpr float ULDUAR_ALGALON_ROOM_Z_MAX = 425.0f;
// Constellations spawn up to 67 yd out from home, well past the room edge.
inline constexpr float ULDUAR_ALGALON_SCAN_RADIUS = 70.0f;

// Black Hole (62168) and Worm Hole (65250) phase anyone inside 6 yd for 10s, no target cap.
inline constexpr float ULDUAR_ALGALON_SHELTER_RADIUS = 6.0f;
// A hider's run ends this far in, clear of the field's edge.
inline constexpr float ULDUAR_ALGALON_SHELTER_IN_RADIUS = ULDUAR_ALGALON_SHELTER_RADIUS - 1.5f;
// The field phases whoever is inside on its next pulse, one a second.
inline constexpr int32 ULDUAR_ALGALON_FIELD_PULSE_MS = 1000;
// Big Bang's 64445 strips every phase a second after the hit and the field pulses every second, so a
// bot still inside it then is phased again for 10s. Exits clear it with room to spare.
inline constexpr float ULDUAR_ALGALON_HOLE_EXIT_RADIUS = 8.0f;
// A walk whose straight line passes this close to a hole clips its field and phases the walker into
// the void Dark Matter. A blocked walk detours via a point this far round the hole.
inline constexpr float ULDUAR_ALGALON_HOLE_PATH_CLEARANCE = 7.5f;
inline constexpr float ULDUAR_ALGALON_HOLE_DETOUR_RADIUS = 9.5f;
// Just outside the 6 yd field: the last path rule before a dodge or exit gives up on paths altogether,
// and what the walk guard holds a generic mover to.
inline constexpr float ULDUAR_ALGALON_FIELD_PATH_CLEARANCE = 6.3f;
// A generic mover may not stop closer than this to a hole: the bot would stand on the field's edge.
inline constexpr float ULDUAR_ALGALON_FIELD_STAND_CLEARANCE = 7.5f;

// A constellation rests 3-3.5 yd off its victim and is eaten within about 6.5 yd of a hole's centre, so the handler
// waits this far past the hole, just outside the 7.5 yd that phases a player; clamped, never under the minimum.
inline constexpr float ULDUAR_ALGALON_BLACK_HOLE_KITE_OFFSET = 8.0f;
inline constexpr float ULDUAR_ALGALON_KITE_MIN_OFFSET = 7.8f;
// Held ones trail right behind, so the handler waits this far out on their side for their Arcane Barrage, 2 s
// standing still every 5 s, then goes round ahead of them. Past the wait it goes on the first one casting.
inline constexpr float ULDUAR_ALGALON_KITE_STAGE_OFFSET = 11.0f;
inline constexpr uint32 ULDUAR_ALGALON_KITE_WAIT_MS = 6000;
// Parked this long with one resting beside it and outside the field, the pass missed: stage again. A walk to
// the stage or far spot still under way after the stall time was blocked or pushed off: the kite starts over.
inline constexpr uint32 ULDUAR_ALGALON_KITE_RETRY_MS = 3000;
inline constexpr uint32 ULDUAR_ALGALON_KITE_STALL_MS = 12000;
inline constexpr float ULDUAR_ALGALON_KITE_BESIDE = 5.0f;
inline constexpr float ULDUAR_ALGALON_CONSTELLATION_EAT_RADIUS = 6.5f;
// Higher above the floor than this, a hole can't take one (npc_living_constellation::SpellHit): on its way down
// after waking, or stuck there.
inline constexpr float ULDUAR_ALGALON_CONSTELLATION_LOW_Z = 5.0f;
// A spot clamped into the room or onto another field is tried this far either side.
inline constexpr float ULDUAR_ALGALON_KITE_SWING = static_cast<float>(M_PI) / 6.0f;
// A loose one is taunted from the far side of a hole when that spot is within taunt reach of it.
inline constexpr float ULDUAR_ALGALON_TAUNT_REACH = 28.0f;
// The swap tanks' walk from the hold spot in the cast. The only hole within it is never kited.
inline constexpr float ULDUAR_ALGALON_TANK_SHELTER_REACH = 35.0f;
// Park points and dodges stay this far inside the room edge.
inline constexpr float ULDUAR_ALGALON_ROOM_CLAMP = 44.0f;

// Phase Punch is a 45s aura refreshed every 15.5s and the 5th stack phases the tank out. Swapping at
// 4 leaves ~17s of margin before either tank is stuck unable to take him back.
inline constexpr uint8 ULDUAR_ALGALON_PHASE_PUNCH_SWAP_STACKS = 4;
// The Phase Punch queued behind the cast lands on the soaker while everyone else is still phased, and a
// 5th stack then leaves nobody unphased, which resets him. So a holder deep in stacks hands him over first.
inline constexpr uint8 ULDUAR_ALGALON_EARLY_SWAP_STACKS = 3;
inline constexpr uint8 ULDUAR_ALGALON_EARLY_SWAP_PARTNER_STACKS = 1;
inline constexpr uint32 ULDUAR_ALGALON_EARLY_SWAP_SECONDS = 15;
// A holder with no physical defensive left hands him over before the cast to a partner who has one, as long
// as the partner can still take the Phase Punch behind it.
inline constexpr uint8 ULDUAR_ALGALON_HANDOVER_PARTNER_STACKS = 2;

// First cast 90s after the intro ends, then every 90.5s. The prediction is re-latched on every cast.
inline constexpr uint32 ULDUAR_ALGALON_FIRST_BIG_BANG_MS = 90000;
inline constexpr uint32 ULDUAR_ALGALON_BIG_BANG_INTERVAL_MS = 90500;
inline constexpr uint32 ULDUAR_ALGALON_BIG_BANG_CAST_MS = 8000;
// How early the raid needs a hole standing, and keeps the last one out of the kite.
inline constexpr uint32 ULDUAR_ALGALON_SHELTER_WINDOW_SECONDS = 30;
// Ice Block, Divine Shield, Cloak of Shadows and Anti-Magic Shell stay unspent this long before a cast and
// while phased: magic immunity strips the hole's phase aura (62168, arcane) or keeps it off.
inline constexpr uint32 ULDUAR_ALGALON_COOLDOWN_HOLD_SECONDS = 20;
// Every constellation goes on the handler before the cast: from its start they keep their victim 9.5 s and
// follow it into a hole, closing it on whoever is still walking there.
inline constexpr uint32 ULDUAR_ALGALON_GATHER_LEAD_SECONDS = 10;
// Hiders keep fighting until their walk plus this margin is all that's left of the cast. The phase only
// has to cover the hit, and 64445 brings everyone back a second after it, so hiding early buys nothing.
inline constexpr int32 ULDUAR_ALGALON_HIDE_MARGIN_MS = 2500;
// A bot only thinks again once its cast ends, so a cast running past its hide slack holds the run back.
inline constexpr int32 ULDUAR_ALGALON_HIDE_CAST_SLACK_MS = 250;
// Nitro Boosts only on a run longer than this.
inline constexpr float ULDUAR_ALGALON_NITRO_MIN_WALK = 10.0f;
// Cast remaining when the soaker's defensive goes up, so it also covers his swings until the raid is back.
inline constexpr int32 ULDUAR_ALGALON_SOAK_DEFENSIVE_MS = 3000;
// Dispersion only lasts 6s. Up this long before the hit it covers the hit and every star dying under it.
inline constexpr int32 ULDUAR_ALGALON_SOAK_DISPERSION_MS = 5400;
// An external needs the priest to still reach a hole after the GCD.
inline constexpr int32 ULDUAR_ALGALON_EXTERNAL_MIN_REMAINING_MS = 4000;
// The hit takes the soaker to ~20% and the raid is back a second later, so the healers' first cast
// after it decides whether he lives. They get this long to land it.
inline constexpr uint32 ULDUAR_ALGALON_RESCUE_WINDOW_MS = 4000;
inline constexpr float ULDUAR_ALGALON_RESCUE_HP_PCT = 60.0f;
inline constexpr float ULDUAR_ALGALON_RESCUE_LAY_ON_HANDS_PCT = 30.0f;
// A healer still has one global cooldown to spend on the soaker before its walk to the hole.
inline constexpr int32 ULDUAR_ALGALON_PREP_GCD_MS = 1500;
// A bot in the field this long before the hit is phased by then: one field pulse and some slack. A tank that
// can't make it stays out.
inline constexpr int32 ULDUAR_ALGALON_LAST_IN_MARGIN_MS = 1500;
// The handler going last leaves this much earlier, or one 250 ms tick reads it as caught out first.
inline constexpr int32 ULDUAR_ALGALON_HOLD_LEAVE_LEAD_MS = 500;
// After the cast he holds still and his Quantum Strike and Phase Punch wait this long (EVENT_RESUME_UPDATING).
inline constexpr uint32 ULDUAR_ALGALON_BIG_BANG_STILL_MS = 3000;
// His swings run 8-23k and Quantum Strike ~10k on a ~44k tank, so from 60% one burst kills it in 1.3 s.
inline constexpr float ULDUAR_ALGALON_TANK_DEFENSIVE_PCT = 60.0f;
inline constexpr float ULDUAR_ALGALON_TANK_LAY_ON_HANDS_PCT = 25.0f;
// Tanks die 2-6 s after a star explosion or a meteor, with the healers on the raid: through this window the
// holder keeps a damage cutter up, only under the percentage so none is wasted on a healthy tank.
inline constexpr uint32 ULDUAR_ALGALON_TANK_WINDOW_LEAD_MS = 1000;
inline constexpr float ULDUAR_ALGALON_TANK_WINDOW_PCT = 80.0f;
inline constexpr uint32 ULDUAR_ALGALON_TANK_WINDOW_TAIL_MS = 5000;
// After one bot casts a cutter, the aura takes a moment to show on the tank; nobody else casts in that time.
inline constexpr uint32 ULDUAR_ALGALON_TANK_DEFENSIVE_CLAIM_MS = 1000;
// Hand of Sacrifice and Divine Sacrifice move damage onto the paladin.
inline constexpr float ULDUAR_ALGALON_SACRIFICE_MIN_PCT = 50.0f;
// A star this low counts as exploding whatever its slope.
inline constexpr float ULDUAR_ALGALON_EXPLOSION_LOW_PCT = 3.0f;
// Aura Mastery and Divine Guardian last 6 s, so they go up this close to a star's forecast death, or with it
// this low. The lead allows for the slope running about a second late on a paced kill.
inline constexpr uint32 ULDUAR_ALGALON_EXPLOSION_COVER_LEAD_MS = 3500;
inline constexpr float ULDUAR_ALGALON_EXPLOSION_COVER_HP_PCT = 5.0f;
// A star still alive once its cover ran out gets another: a meteor blackout stalled one kill 10 s.
inline constexpr uint32 ULDUAR_ALGALON_EXPLOSION_COVER_MS = 6000;
// A cover only with this many under the explosion's line: a single one has its own button or a healthstone, and
// a cover spent on one or none is missing for the star that catches most of the raid.
inline constexpr uint32 ULDUAR_ALGALON_EXPLOSION_COVER_MIN_UNDER = 2;
// A bot that wouldn't survive the explosion presses its own button this close to it. Covered, the top roll
// counts at 0.8: Divine Guardian is a flat -20%, and under Aura Mastery the worst hit is 79% of it.
inline constexpr uint32 ULDUAR_ALGALON_EXPLOSION_PERSONAL_LEAD_MS = 1500;
inline constexpr float ULDUAR_ALGALON_COVERED_EXPLOSION_FACTOR = 0.8f;
// Stars dying this close after the first count as one hit to survive.
inline constexpr uint32 ULDUAR_ALGALON_EXPLOSION_STACK_MS = 3000;
inline constexpr uint32 ULDUAR_ALGALON_EXPLOSION_STACK_MAX = 3;
// The least a healthstone or healing potion heals, for whether one lifts a bot over the line.
inline constexpr uint32 ULDUAR_ALGALON_EXPLOSION_CONSUMABLE_HEAL = 3500;
// Above this, the class nodes leave a bot's personal buttons to the explosion node.
inline constexpr float ULDUAR_ALGALON_BUTTON_HOLD_PCT = 35.0f;
// A paladin aura's reach.
inline constexpr float ULDUAR_ALGALON_AURA_RANGE = 40.0f;
// Healers pick by health %, and every explosion puts clothies below a tank losing ~15k/s, so the pair
// gets healers of its own. Never more than the raid keeps: the star window waits for the raid to top up.
inline constexpr uint8 ULDUAR_ALGALON_TANK_HEALERS_10 = 1;
inline constexpr uint8 ULDUAR_ALGALON_TANK_HEALERS_25 = 2;
// Main hand, off hand and Quantum Strike can land inside 1.5 s, so under this only the fastest heal will do.
inline constexpr float ULDUAR_ALGALON_TANK_CRITICAL_PCT = 50.0f;
// The tank he's hitting gets the fast heals from here: a main hand swing plus Quantum Strike is 26-31k
// on a ~45k tank, so one global spent on anything else at 65% loses it.
inline constexpr float ULDUAR_ALGALON_HOLDER_URGENT_PCT = 70.0f;
inline constexpr float ULDUAR_ALGALON_TANK_HEALER_SELF_PCT = 50.0f;
inline constexpr float ULDUAR_ALGALON_HOLDER_TOPOFF_PCT = 95.0f;
inline constexpr float ULDUAR_ALGALON_OFFTANK_TOPOFF_PCT = 85.0f;
// The partner only goes ahead of a holder below its top-off when it's this much lower.
inline constexpr float ULDUAR_ALGALON_OFFTANK_MARGIN_PCT = 15.0f;
// Run speed for walk estimates made for someone else, without their speed to hand.
inline constexpr float ULDUAR_ALGALON_RUN_SPEED = 7.0f;
// A hole further than this can't be reached in the cast, so the bot walks towards it beforehand, to this
// far from it, from this long before the cast.
inline constexpr float ULDUAR_ALGALON_FAR_HOLE_YARDS = 35.0f;
inline constexpr float ULDUAR_ALGALON_NEAR_HOLE_STANDOFF = 30.0f;
inline constexpr uint32 ULDUAR_ALGALON_FAR_HOLE_SECONDS = 15;

// A star dying in this window round the hit explodes on the soaker alone; 64445 strips the phase a second
// after the hit. No sooner: the tanks hold him until the cast starts, and Dispersion lasts 6 s.
inline constexpr int32 ULDUAR_ALGALON_UNDER_LEAD_MS = 4000;
inline constexpr int32 ULDUAR_ALGALON_UNDER_TAIL_MS = 600;
// With a star under, every hider goes in this much earlier, handler and tanks too, stretched up to the cap so
// all are in a pulse and a half before the earliest such death.
inline constexpr int32 ULDUAR_ALGALON_UNDER_HIDE_EXTRA_MS = 3000;
inline constexpr int32 ULDUAR_ALGALON_UNDER_HIDE_STRETCH_MS = 4000;
inline constexpr int32 ULDUAR_ALGALON_UNDER_DEATH_LEAD_MS = 1500;
// A new star joins the plan only while the team's work on it ends this long before the cast.
inline constexpr int32 ULDUAR_ALGALON_UNDER_FINISH_MS = 5000;
// Between these two before the cast, a planned star whose work overruns the cast for the late hold goes back to
// pacing, which can still kill it first. After that pacing couldn't make it either.
inline constexpr int32 ULDUAR_ALGALON_UNDER_RELEASE_FROM_MS = 15000;
inline constexpr int32 ULDUAR_ALGALON_UNDER_RELEASE_TO_MS = 8000;
inline constexpr uint32 ULDUAR_ALGALON_UNDER_LATE_HOLD_MS = 1000;
// A soaker missing this long ends the plan; less is a phase or a re-election.
inline constexpr uint32 ULDUAR_ALGALON_UNDER_SOAKER_GRACE_MS = 2000;
// The plan holds until the strip is past, and phased bots keep clear of an under-star its last seconds.
inline constexpr uint32 ULDUAR_ALGALON_UNDER_KEEP_MS = 1200;
inline constexpr int32 ULDUAR_ALGALON_UNDER_DYING_MS = 3000;
// The team's speed until it shows one, and its walk and start per star. The bulk starts once the work left
// plus this slack reaches the cast, so a low star sits exposed to splash no longer.
inline constexpr float ULDUAR_ALGALON_UNDER_TEAM_PCT_PER_SEC = 6.0f;
inline constexpr int32 ULDUAR_ALGALON_UNDER_SWITCH_MS = 3000;
inline constexpr int32 ULDUAR_ALGALON_UNDER_BULK_SLACK_MS = 35000;
// With a star under, a ring hider waits this close to its hole before the cast, so it is in by the window.
inline constexpr float ULDUAR_ALGALON_UNDER_HOLE_REACH = 22.0f;

// Each star death is Black Hole Explosion on everyone unphased, 64122 up to 16,912 and 65108 up to 21,525.
// Collapse drains 1% max health a second, so four ignored stars explode within seconds of each other.
inline constexpr uint32 ULDUAR_ALGALON_STAR_PACING_GAP_MS = 8000;
inline constexpr uint32 ULDUAR_ALGALON_STAR_EXPLOSION_MAX_10 = 16913;
inline constexpr uint32 ULDUAR_ALGALON_STAR_EXPLOSION_MAX_25 = 21526;
// One gap plus a kill, per star still to go off.
inline constexpr uint32 ULDUAR_ALGALON_STAR_SERIAL_SECONDS = 16;
// The black hole lands where the star dies and phases everyone in its field.
inline constexpr float ULDUAR_ALGALON_STAR_CLEAR_RADIUS = 7.0f;
// A hole under a swap tank phases it and sets him loose; one inside his melee ring phases the melee.
inline constexpr float ULDUAR_ALGALON_STAR_TANK_CLEARANCE = 9.0f;
inline constexpr float ULDUAR_ALGALON_STAR_BOSS_CLEARANCE = 16.0f;
// A star this low dies within seconds whoever is on it, and everyone in its field is phased.
inline constexpr float ULDUAR_ALGALON_STAR_DYING_HP_PCT = 5.0f;
inline constexpr float ULDUAR_ALGALON_STAR_DYING_RADIUS = 8.0f;
// Inside this the generic movers are held until it dies, or reach walks a bot that stepped off straight back.
inline constexpr float ULDUAR_ALGALON_STAR_DYING_HOLD_RADIUS = 12.0f;
// A ranged team member further than this from the focus star walks to this far from it.
inline constexpr float ULDUAR_ALGALON_STAR_TEAM_REACH = 28.0f;
inline constexpr float ULDUAR_ALGALON_STAR_TEAM_STANDOFF = 25.0f;
inline constexpr uint8 ULDUAR_ALGALON_STAR_TEAM_10 = 2;
inline constexpr uint8 ULDUAR_ALGALON_STAR_TEAM_25 = 3;
// Splash that would also land on a star this close to the bot or its target is held; 15 is glyphed Pestilence.
inline constexpr float ULDUAR_ALGALON_SPLASH_RADIUS = 15.0f;
// Round a target, splash reaches this far past its combat reach: Pestilence landed on stars 19.5 yd from his centre.
inline constexpr float ULDUAR_ALGALON_SPLASH_PAST_REACH = 11.0f;
// Ground AoE sits there 8-10 s and stars wander ~1.7 yd/s, so it's held while one is this close to where it lands.
inline constexpr float ULDUAR_ALGALON_GROUND_SPLASH_RADIUS = 25.0f;

// Star death forecast. Splash near him adds to the drain, so a star's health isn't its seconds left; the
// slope of its health log is.
inline constexpr uint32 ULDUAR_ALGALON_STAR_LOG_MS = 4000;
inline constexpr uint32 ULDUAR_ALGALON_STAR_LOG_STEP_MS = 500;
// Less log than this reads as the drain: a slope over one or two samples is noise.
inline constexpr uint32 ULDUAR_ALGALON_STAR_LOG_MIN_MS = 2000;
inline constexpr float ULDUAR_ALGALON_STAR_DRAIN_PCT_PER_SEC = 1.0f;
// Keep-or-drop reads a dropped star as dying at its own slope up to this: casts in flight, channels, DoTs.
inline constexpr float ULDUAR_ALGALON_STAR_DROPPED_PCT_PER_SEC = 5.0f;
// On top of the walk, the first 2 s on a star only match the drain: target switch, first casts, DoTs ramping.
inline constexpr uint32 ULDUAR_ALGALON_STAR_KILL_START_MS = 2000;
// The team's kill speed over a whole kill, slowest and fastest.
inline constexpr float ULDUAR_ALGALON_STAR_KILL_SLOW_PCT_PER_SEC = 5.5f;
inline constexpr float ULDUAR_ALGALON_STAR_KILL_FAST_PCT_PER_SEC = 9.0f;
// A focus losing less than this is still the walk in, not the kill.
inline constexpr float ULDUAR_ALGALON_STAR_KILL_MEASURED_PCT_PER_SEC = 3.5f;
// The fastest 4 s slope worth planning for: a kill's last second can run faster, but only for that second.
inline constexpr float ULDUAR_ALGALON_STAR_KILL_PEAK_PCT_PER_SEC = 12.0f;
// A star whose own death would land badly is only acted on this close to it: a fresh wave's four all die
// together and would read as due at the spawn, and the serial deadline already paces them.
inline constexpr uint32 ULDUAR_ALGALON_STAR_DUE_HORIZON_MS = 30000;
// A kill already running only stops for an explosion this close, so it doesn't flip on a tick of noise.
inline constexpr uint32 ULDUAR_ALGALON_STAR_KEEP_GAP_MS = 6000;
// No explosion from just before the cast until the raid is back out of the void.
inline constexpr uint32 ULDUAR_ALGALON_STAR_BIG_BANG_LEAD_MS = 1000;
inline constexpr uint32 ULDUAR_ALGALON_STAR_BIG_BANG_TAIL_MS = 3000;
// Nor around a meteor's impact: it takes 6-11k off everyone, and the healers need seconds to undo it.
inline constexpr uint32 ULDUAR_ALGALON_COSMIC_SMASH_IMPACT_MS = 4800;
inline constexpr uint32 ULDUAR_ALGALON_STAR_IMPACT_LEAD_MS = 4000;
inline constexpr uint32 ULDUAR_ALGALON_STAR_IMPACT_TAIL_MS = 5000;
// Meteors fall on a fixed clock (Repeat(25s + 500ms)): the first lands 30.9 s after the intro ends.
inline constexpr uint32 ULDUAR_ALGALON_FIRST_SMASH_IMPACT_MS = 30900;
inline constexpr uint32 ULDUAR_ALGALON_COSMIC_SMASH_INTERVAL_MS = 25500;
// Predicted impacts the star plan looks ahead to.
inline constexpr uint8 ULDUAR_ALGALON_SMASH_FORECAST = 3;
// Forecast to die this soon, a star is a hazard whatever its health: at 3.4%/s the 5% line is 1.5 s.
inline constexpr uint32 ULDUAR_ALGALON_STAR_DYING_MS = 4000;

// Cosmic Smash: full damage inside 6 yd, doubled dmg/dist to 10, dmg/dist past that. Impact is ~4.8s
// after the marker lands, one marker on 10-man, three on 25-man.
inline constexpr float ULDUAR_ALGALON_COSMIC_SMASH_TRIGGER_RADIUS = 15.0f;
inline constexpr float ULDUAR_ALGALON_COSMIC_SMASH_CLEARANCE = 15.0f;
inline constexpr float ULDUAR_ALGALON_COSMIC_SMASH_MIN_CLEARANCE = 10.0f;
// It lands up to this much after the 4.8 s.
inline constexpr int32 ULDUAR_ALGALON_COSMIC_SMASH_IMPACT_SPREAD_MS = 300;
// A hide walk that has the bot inside the minimum clearance of a meteor as it lands, before the bot is phased, goes
// round the impact through a point this far off it.
inline constexpr float ULDUAR_ALGALON_SMASH_DETOUR_RADIUS = 12.0f;
// A hider that can be phased this much before a meteor lands sets off then, so a tick or two late still makes it.
inline constexpr int32 ULDUAR_ALGALON_SMASH_IN_LEAD_MS = 500;
// Inside this the generic movers are held until the marker expires.
inline constexpr float ULDUAR_ALGALON_COSMIC_SMASH_HOLD_RADIUS = 19.0f;
inline constexpr float ULDUAR_ALGALON_DODGE_SEARCH_RADIUS = 24.0f;
// The marker creatures live 10s but the meteor lands ~4.8s in; past this they mark nothing.
inline constexpr uint32 ULDUAR_ALGALON_MARKER_LIFETIME_MS = 5500;
// Ring searches: a full turn of bearings at the angle step, rings this far apart.
inline constexpr float ULDUAR_ALGALON_SWEEP_STEP = 2.0f;
inline constexpr float ULDUAR_ALGALON_SWEEP_ANGLE_STEP = static_cast<float>(M_PI) / 8.0f;
inline constexpr uint8 ULDUAR_ALGALON_SWEEP_BEARINGS = 16;
// Challenging Shout and Challenging Roar only reach this far round the tank.
inline constexpr float ULDUAR_ALGALON_AREA_TAUNT_RANGE = 10.0f;

// Rings at 19, 25 and 31 yd from the hold spot, each alternating sides so a short raid still spreads. Every
// slot is at least 8 yd off the worm hole spots, and healers stay within 22 yd of the tank slot.
inline constexpr uint8 ULDUAR_ALGALON_HEALER_SLOTS = 4;
inline constexpr uint8 ULDUAR_ALGALON_RANGED_INNER_SLOTS = 6;
inline constexpr uint8 ULDUAR_ALGALON_RANGED_OUTER_SLOTS = 8;
inline constexpr uint8 ULDUAR_ALGALON_TOTAL_SLOTS =
    ULDUAR_ALGALON_HEALER_SLOTS + ULDUAR_ALGALON_RANGED_INNER_SLOTS + ULDUAR_ALGALON_RANGED_OUTER_SLOTS;
inline constexpr float ULDUAR_ALGALON_SLOT_TOLERANCE = 2.0f;
// The spots the walk out never puts in his reach; a spot made up during the fight keeps this from him.
inline constexpr float ULDUAR_ALGALON_FORMATION_CLEARANCE = 13.5f;
// The holder this close to the tank slot has him where he belongs.
inline constexpr float ULDUAR_ALGALON_HOLDER_ON_SPOT_RADIUS = 3.0f;
// With him anywhere else, the other swap tank waits this far from him towards the tank slot.
inline constexpr float ULDUAR_ALGALON_OFFTANK_STEP = 7.0f;
inline constexpr uint8 ULDUAR_ALGALON_MELEE_STAGE_SPOTS = 10;
// Melee go in once a swap tank holds him on the tank slot, or this long after the intro if he holds him
// somewhere else, and after the second wait whoever holds him, so a pull nobody picks up doesn't idle them.
inline constexpr uint32 ULDUAR_ALGALON_MELEE_RELEASE_MS = 8000;
inline constexpr uint32 ULDUAR_ALGALON_MELEE_RELEASE_ANYWAY_MS = 20000;
// Threat redirects go on the tank he attacks for this long after the intro. Tricks reaches 20 yd and
// Misdirection 100; CanCastSpell lets an out of range cast through.
inline constexpr uint32 ULDUAR_ALGALON_OPENER_MS = 20000;
inline constexpr float ULDUAR_ALGALON_TRICKS_RANGE = 20.0f;
inline constexpr float ULDUAR_ALGALON_MISDIRECTION_RANGE = 100.0f;
// The pull's first holder keeps him on the pickup slot this long, facing him, before the walk out.
inline constexpr uint32 ULDUAR_ALGALON_PULL_HOLD_MS = 3000;
// A human tank is told to taunt this often while a swap or a pickup is theirs to make.
inline constexpr uint32 ULDUAR_ALGALON_TANK_CALL_MS = 5000;
// Humans are told to step into a hole with this much of the Big Bang cast left.
inline constexpr int32 ULDUAR_ALGALON_HIDE_CALL_MS = 3000;
// A human soaker is told to press his button with this much of the cast left, a second ahead of a bot.
inline constexpr int32 ULDUAR_ALGALON_HUMAN_SOAK_CALL_MS = 4000;
// How far a slot buried under a hole may be stepped aside before the formation gives up on it.
inline constexpr float ULDUAR_ALGALON_SLOT_DISPLACE_RADIUS = 12.0f;

// Per instance, not per bot: one room scan serves the whole raid.
inline constexpr uint32 ULDUAR_ALGALON_STATE_TICK_MS = 250;
// A walk standing still this long was stopped short and has to be issued again.
inline constexpr uint32 ULDUAR_ALGALON_STALL_MS = 500;

// His home and landing spot.
extern const Position ULDUAR_ALGALON_ROOM_CENTER;
// Both swap tanks wait 8 yd from his landing spot, inside his reach, so the pickup taunt lands the moment
// he turns selectable.
extern const Position ULDUAR_ALGALON_PICKUP_SLOT;
extern const Position ULDUAR_ALGALON_PULL_OFFTANK_SLOT;
// The holder stands on the tank slot and he stops short of it, about on the hold spot; the other swap
// tank's slot is in his reach wherever he stops.
extern const Position ULDUAR_ALGALON_TANK_SLOT;
extern const Position ULDUAR_ALGALON_HOLD_SPOT;
extern const Position ULDUAR_ALGALON_OFFTANK_SLOT;
extern const std::array<Position, ULDUAR_ALGALON_TOTAL_SLOTS> ULDUAR_ALGALON_RING_SLOTS;
extern const std::array<Position, ULDUAR_ALGALON_MELEE_STAGE_SPOTS> ULDUAR_ALGALON_MELEE_STAGE_SLOTS;

// Written as its number into algalon.phase.
enum class AlgalonPhase : uint32
{
    Idle = 0,
    Intro = 1,
    One = 2,
    Two = 3,
    Won = 4,
};

// How far the handler is in walking held constellations through a hole.
enum class AlgalonKiteStage : uint8
{
    Stage,
    Wait,
    Cross,
    Parked,
};

struct AlgalonKite
{
    AlgalonKiteStage stage = AlgalonKiteStage::Stage;
    uint32 sinceMs = 0;
    // Latched per stage, or the walk would turn as the constellations shift.
    Position spot;
    bool hasSpot = false;
    // The loose one the far spot was picked for, taunted from there.
    ObjectGuid looseTarget;
};

// One creature from the room scan. Positions stay valid for phased bots, which can't see the room.
struct AlgalonScanUnit
{
    ObjectGuid guid;
    Position position;
    ObjectGuid victim;
    float healthPct = 0.0f;
    bool active = false;
    // Stars only: health lost per second over the log, never under the drain. `lossMeasured` once the log
    // covers ULDUAR_ALGALON_STAR_LOG_MIN_MS.
    float lossPctPerSec = ULDUAR_ALGALON_STAR_DRAIN_PCT_PER_SEC;
    bool lossMeasured = false;
    // Stars only, at the scan: health, the drain per tick and the time to the next tick.
    uint32 health = 0;
    // Stars only: what the DoTs on it still deal, its own drain left out.
    uint32 pendingHp = 0;
    uint32 tickHp = 0;
    int32 nextTickMs = 0;
    // Planned to die under the coming Big Bang, and its band for that, from this scan: (underLo, underHi].
    bool under = false;
    uint32 underLo = 0;
    uint32 underHi = 0;
    // An under-star in its last seconds: phased bots keep clear, since it dies where they'd stand at the strip.
    bool underDying = false;
};

struct AlgalonEncounterState
{
    // Slots are held, not re-derived, so one death doesn't renumber everyone behind the corpse.
    std::unordered_map<ObjectGuid, uint8> slotAssignments{};
    // Latched per cast so a hole appearing closer can't turn a bot around mid run.
    std::unordered_map<ObjectGuid, ObjectGuid> shelterAssignments{};
    std::unordered_map<ObjectGuid, ObjectGuid> kiteHoleAssignments{};

    AlgalonPhase phase{};
    uint32 bigBang{};
    ObjectGuid bigBangSoaker{};
    ObjectGuid bigBangBackup{};
    // A bot with Dispersion back for the next hit, latched outside the cast. Set, the tanks hide and keep their
    // buttons for themselves; empty, a tank soaks and its buttons are reserved.
    ObjectGuid disperser{};
    // The paladin running Shadow Resistance Aura while stars live, so its Aura Mastery doubles it.
    ObjectGuid shadowAuraPaladin{};
    bool urgent{};
    uint32 holeCount{};
    ObjectGuid focusStar{};
    GuidUnorderedSet starTeam{};
    ObjectGuid handler{};
    // The two tanks that trade him on Phase Punch, latched for the pull: two bot tanks when the raid has
    // them, else the group's main and first assist tank, one of whom may be a human.
    GuidUnorderedSet swapPair{};
    // Bot healers on the swap pair alone, latched for the pull like the pair.
    GuidUnorderedSet tankHealers{};
    uint32 tankHealerSlots{};

    ObjectGuid boss;
    bool engaged = false;
    bool engagedSeen = false;
    bool meleeReleased = false;
    uint32 introEndMs = 0;
    // The pull's first swap tank to hold him, and when.
    ObjectGuid firstHolder;
    uint32 firstHoldMs = 0;
    uint32 lastTankCallMs = 0;
    bool bigBangHideCalled = false;
    bool humanSoakCalled = false;
    uint32 nextBigBangMs = 0;
    uint32 bigBangCount = 0;
    bool bigBangCasting = false;
    // The soaker is cleared with the cast, and the second after the hit is when he needs the raid.
    uint32 lastBigBangEndMs = 0;
    ObjectGuid lastSoaker;
    // Per hole, hiders still out during the cast, for the handler that goes in last.
    std::unordered_map<ObjectGuid, uint8> hidersOut;
    uint32 tankCutClaimMs = 0;
    uint32 layOnHandsClaimMs = 0;
    // The star the last explosion cover went up for, and when.
    ObjectGuid coverStar;
    uint32 coverClaimMs = 0;
    // Per bot, the star it last pressed a personal button for.
    std::unordered_map<ObjectGuid, ObjectGuid> personalStars;
    // Bots whose Nitro Boosts were ready at the cast start: their hide walk is timed boosted.
    std::unordered_set<ObjectGuid> nitroRunners;
    // Stars planned to die under the coming or running cast, frozen through it, and the window (absolute ms).
    std::unordered_set<ObjectGuid> underStars;
    uint32 underFromMs = 0;
    uint32 underToMs = 0;
    // The cast count the plan was made under: a cast starting begins the next plan from scratch.
    uint32 underPlanCycle = 0;
    // Planned stars handed back to pacing, never planned again.
    std::unordered_set<ObjectGuid> underReleased;
    // Per planned star, since when its work has overrun the cast in the release stretch.
    std::unordered_map<ObjectGuid, uint32> underLateMs;
    // Since when no disperser has been latched while stars were planned.
    uint32 underNoSoakMs = 0;
    // The under-star the team is taking down, kept until it is done: re-picked each tick, the team would keep
    // switching and only match the drain.
    ObjectGuid bulkStar;
    // With no hole at all, the star killed before the cast to make one.
    ObjectGuid shelterStar;
    // The bulk's own speed, over a span on one star from past its first casts, and the cast count it was taken
    // under. A paced kill holds nothing back and runs faster.
    float bulkPctPerSec = 0.0f;
    uint32 bulkMeasuredCycle = 0;
    uint32 focusSinceMs = 0;
    ObjectGuid bulkFrom;
    uint32 bulkFromMs = 0;
    float bulkFromPct = 0.0f;
    uint32 lastSmashImpactMs = 0;

    std::vector<AlgalonScanUnit> stars;
    std::vector<AlgalonScanUnit> holes;
    std::vector<AlgalonScanUnit> constellations;
    std::vector<AlgalonScanUnit> markers;
    std::vector<AlgalonScanUnit> darkMatter;
    std::unordered_map<ObjectGuid, uint32> markerSeenMs;
    // (ms, health %) per star, one sample per ULDUAR_ALGALON_STAR_LOG_STEP_MS. Kept across a drop: its slope
    // is how fast the star still dies, from DoTs and casts in flight or the drain alone.
    std::unordered_map<ObjectGuid, std::vector<std::pair<uint32, float>>> starHealthLog;
    // (ms, health %) when the team left a star, kept for the log's span: that long its slope still has the burst.
    std::unordered_map<ObjectGuid, std::pair<uint32, float>> starLeft;
    // The speed the team last showed on its focus, capped at the peak.
    float teamKillPctPerSec = 0.0f;
    // The next meteor not yet marked.
    uint32 nextSmashImpactMs = 0;
    // Opened at a deadline, the kill keeps the deadline's looser position test.
    bool focusByDeadline = false;
    // Per handler, reset whenever its kite hole is latched.
    std::unordered_map<ObjectGuid, AlgalonKite> kites;
    bool wormHolesSeen = false;

    uint32 lastStarDeathMs = 0;
    uint32 lastTickMs = 0;
};

// Instance lookup, never "find target" or "nearest npcs": both are phase filtered, and a bot inside a
// hole would lose him and every node with him.
Unit* GetAlgalon(PlayerbotAI* botAI);
bool AlgalonInRoom(Player* bot);
// Alive and the bot is in his room, pull or not. The formation settles on this before the pull.
bool AlgalonPresent(PlayerbotAI* botAI);
// In combat and not beaten.
bool AlgalonEngaged(PlayerbotAI* botAI);
bool AlgalonInPhaseOne(PlayerbotAI* botAI);
bool IsAlgalonPhased(Unit const* unit);
// The first ULDUAR_ALGALON_OPENER_MS after the intro.
bool AlgalonInOpener(PlayerbotAI* botAI);
// The swap tank holding him. Never the pickup tank during the intro: he can take the other one, and a
// Misdirection already on the pickup tank then pulls him off.
Player* GetAlgalonOpenerTank(PlayerbotAI* botAI);
// "misdirection" or "tricks of the trade" for the classes that redirect threat, else nullptr.
char const* AlgalonThreatRedirectSpell(Player* bot);

// Runs from the definition tick, at most once per ULDUAR_ALGALON_STATE_TICK_MS per instance.
void AlgalonTickEncounterState(PlayerbotAI* botAI);

bool AlgalonBigBangCasting(PlayerbotAI* botAI);
// -1 when he isn't casting it.
int32 AlgalonBigBangRemainingMs(PlayerbotAI* botAI);
// Off the predicted clock; false while casting.
bool AlgalonBigBangWithin(PlayerbotAI* botAI, uint32 seconds);
// The cast the instance tick has latched a soaker for. Roles read this, never the live cast: in the
// tick's first 250ms of a cast nobody is the soaker yet, and every bot would run for a hole.
bool AlgalonBigBangLatched(Player* bot);
// Until the next predicted hit, cast included; 0 while he casts it. INT32_MAX until the intro ends, when
// nothing is predicted and nothing should be held for it.
int32 AlgalonMsToBigBangHit(PlayerbotAI* botAI);
// The last cast's soaker, alive, back in the room and still low, while the healers' window to reach him
// after the hit is open; else nullptr.
Player* GetAlgalonRescueTarget(PlayerbotAI* botAI);
// What this bot can put on the soaker: an external before the hit, a heal over time or shield before it
// hides, an instant heal once it is back. nullptr when it has nothing castable or already put it up.
char const* GetAlgalonSoakerExternal(PlayerbotAI* botAI, Player* soaker);
char const* GetAlgalonSoakerPrep(PlayerbotAI* botAI, Player* soaker);
char const* GetAlgalonRescueSpell(PlayerbotAI* botAI, Player* soaker);

// Engaged and on the tank healer list. False after a kill or wipe and off the map.
bool IsAlgalonTankHealer(Player* bot);
struct AlgalonTankHealPick
{
    Player* target = nullptr;
    // Low enough that only the fastest heal will do.
    bool critical = false;
};
// Who this tank healer heals next: the holder, the partner, or itself.
AlgalonTankHealPick GetAlgalonTankHealPick(PlayerbotAI* botAI);
char const* GetAlgalonTankHealSpell(PlayerbotAI* botAI, AlgalonTankHealPick const& pick);
// The swap tank he isn't hitting, unless this paladin's Beacon is already on it.
Player* GetAlgalonBeaconTarget(PlayerbotAI* botAI);

// A disperser is latched for the next hit: the tanks hide and keep their buttons for him.
bool AlgalonDispersionSoak(Player* bot);
// The cooldown casting it now would start, talents included.
int32 AlgalonCooldownMs(Player* bot, uint32 spellId);

// A damage cutter or Lay on Hands this bot puts on the holder now. Cheapest first and one at a time, so each bot
// asks whether it's the first provider able to.
struct AlgalonTankCut
{
    Player* target = nullptr;
    uint32 spellId = 0;
    // "window" (raid damage due or just landed), "low" or "loh".
    char const* why = nullptr;
};
AlgalonTankCut GetAlgalonTankCut(PlayerbotAI* botAI);
void ClaimAlgalonTankCut(Player* bot, AlgalonTankCut const& cut);

// "aura mastery" or "divine sacrifice" when this paladin covers the star about to explode, else nullptr. In the
// cast, only for a star dying before those under its line are in a field.
char const* GetAlgalonExplosionCover(PlayerbotAI* botAI);
void ClaimAlgalonExplosionCover(Player* bot, char const* cover);
// A star dying under the running cast within `radius`: where a phased bot would come back inside a new hole.
bool AlgalonUnderStarNear(Player* bot, float radius);
// This bot's own button against the star about to die, when the hit would kill it, else a healthstone or a
// healing potion ("healthstone"/"healing potion" actions); empty otherwise. In the cast, only against a star dying
// before the bot is in a field, and never an immunity: those keep the field's phase off.
struct AlgalonPersonalPick
{
    uint32 spellId = 0;
    char const* consumable = nullptr;
    explicit operator bool() const { return spellId || consumable; }
};
AlgalonPersonalPick GetAlgalonExplosionPersonal(PlayerbotAI* botAI);
void ClaimAlgalonExplosionPersonal(Player* bot);
// Runs Shadow Resistance Aura while stars live.
bool IsAlgalonShadowAuraPaladin(Player* bot);

// What this bot does about the current Big Bang.
enum class AlgalonHideRole : uint8
{
    None,
    Soak,
    Backup,
    Run,
    In,
    Exit,
    NoShelter,
    Wait,
    Hold,
};
AlgalonHideRole GetAlgalonHideRole(Player* bot);
// The Run branch alone, without latching a shelter, for rule predicates.
bool AlgalonShouldRunForShelter(Player* bot);
// How long this bot can still fight before it has to run for its hole; INT32_MAX when it has no hole to run
// to, doesn't hide, or is a tank that can't reach one in time.
int32 AlgalonHideSlackMs(Player* bot);
// Uses the bot's Nitro Boosts on its hide run when they were ready at the cast start and the walk is long enough.
// Without them, a hunter's Aspect of the Cheetah or a rogue's Sprint on a run that would come in late.
void AlgalonBoostHideRun(Player* bot, float walk);
// The soaker and the backup.
bool AlgalonStaysOut(Player* bot);
// A tank still out with no hole it can reach before the hit, which takes it on its own button.
bool AlgalonTankCaughtOut(Player* bot);
// A constellation is chasing this bot, so it hides last: they follow it in and close its hole.
bool AlgalonHoldsConstellation(Player* bot);
// By spell id: CanCastSpell refuses during a channel, and a shadow priest is nearly always in Mind Flay.
bool AlgalonCanDisperse(Player* bot);

Player* GetAlgalonBigBangSoaker(PlayerbotAI* botAI);
Player* GetAlgalonBigBangBackup(PlayerbotAI* botAI);
// The hole this bot runs to for the current cast, latched.
bool GetAlgalonShelter(Player* bot, Position& shelter);
// Where the hide run walks next: the hole, or round a meteor that would land by the bot on the way, before it is
// phased. False when one does and nothing goes round.
bool AlgalonHideRoute(Player* bot, Position const& shelter, Position& next);
// Nearest cached hole within radius, readable while phased.
bool AlgalonHoleNear(Player* bot, float radius, Position* hole = nullptr);

// Whoever Algalon is swinging at. Alternates between the two tanks with the Phase Punch swap.
Player* GetAlgalonBossTank(PlayerbotAI* botAI);
bool IsAlgalonSwapTank(Player* player);
uint8 GetAlgalonPhasePunchStacks(Player* player);
// The swap tank that takes him when his target is nobody who should have him, a bot before a human.
// Before the pull it is the one waiting on the tank slot for the end of the intro.
Player* GetAlgalonPickupTank(PlayerbotAI* botAI);
// Due to taunt him now: he is loose, or the holder is deep enough in Phase Punch to hand him over.
bool AlgalonShouldPickUp(Player* bot);
bool AlgalonShouldSwap(Player* bot);
// A pickup or swap due on this bot. It beats the handler's constellation work, whose kite would hold the walk
// back to him from out of taunt range.
bool AlgalonTakesBossNow(Player* bot);
// A swap tank or the handler not holding him stays off him while a swap tank does. After a taunt both build
// threat at the same rate: the one with the stacks would take him straight back, a third tank from both.
bool AlgalonYieldsBoss(Player* bot);
// The swap tank holding him when the other is dead or phased. Not stacks: the old holder keeps its 4 for the
// aura's 45 s after every swap, so that would be most of the fight.
Player* AlgalonLoneHolder(PlayerbotAI* botAI);
// The swap tank not holding him, busy with an add rather than idle on its slot.
bool AlgalonOffTankWorking(Player* bot);

// Constellations in phase 1, Unleashed Dark Matter in phase 2: a third tank, else the swap tank not
// holding Algalon.
Player* GetAlgalonHandler(PlayerbotAI* botAI);
// Whether this bot is the handler, for a predicate asked on every action: by guid, with no walk over the group.
bool IsAlgalonHandler(Player* bot);
// A loose constellation to taunt first, else one already on the handler while a hole is spare to drag it
// through.
Unit* GetAlgalonHandlerConstellation(Player* bot);
// Where the handler walks to drag its constellations through a spare hole. For a loose one, the far-side spot
// it taunts from.
bool GetAlgalonKiteSpot(Player* bot, Unit* constellation, Position& spot);
// The far-side spot to taunt a loose constellation from, latching it; false when it is out of taunt reach, no
// hole is spare or the raid is gathering for Big Bang.
bool AlgalonKiteBehind(Player* bot, Unit* constellation, Position& spot);
// Read-only, for rule predicates: a far-side spot is latched for this loose constellation.
bool AlgalonKitingBehind(Player* bot, Unit* constellation);
// A Dark Matter on anyone but the handler, the one hurting the most fragile victim first.
Unit* GetAlgalonLooseDarkMatter(Player* bot);

// Ranged DPS assigned to the stars; every ranged DPS joins while no hole stands with a Big Bang due.
bool IsAlgalonStarTeam(Player* bot);
// The star the team may kill right now, or nullptr while the pacing window is shut.
Unit* GetAlgalonFocusStar(PlayerbotAI* botAI);
uint8 AlgalonAliveStarCount(PlayerbotAI* botAI);

bool AlgalonCosmicSmashThreatens(Player* bot);
// The bot stands where a star about to die will put its hole.
bool AlgalonDyingStarThreatens(Player* bot);
bool AlgalonCosmicSmashMarkerNear(PlayerbotAI* botAI, Position const& spot, float radius);
// Close enough to a live marker or a dying star that the generic movers are held until it is gone.
bool AlgalonSmashHoldsGround(Player* bot);
// Every marker, hole and dying star a dodge or an exit must stay clear of.
std::vector<EncounterHelpers::HazardCircle> GetAlgalonDodgeHazards(Player* bot, float markerClearance);
bool AlgalonSpotInRoom(float x, float y);
// A straight walk from the bot to (x, y) would pass within `clearance` of a hole. The hole it is already
// leaving doesn't count.
bool AlgalonPathCrossesHole(Player* bot, float x, float y, float clearance = ULDUAR_ALGALON_HOLE_PATH_CLEARANCE);
// A generic mover stopping at (x, y) would leave the bot standing at a field's edge, or walk it through one.
// A phased bot passes the exit radius: still inside a field when the phase drops, it is phased again.
bool AlgalonWalkEntersField(Player* bot, float x, float y,
                            float standClearance = ULDUAR_ALGALON_FIELD_STAND_CLEARANCE);
// Where to walk next on the way to `to`: `to` itself, or a point round a hole the straight walk would clip.
// False when the way is blocked and nothing goes round.
bool AlgalonWalkAround(Player* bot, Position const& to, Position& next);
// Phased with a hole within its exit radius: walking out is all that counts until it is outside.
bool AlgalonPhasedInField(Player* bot);
// A star other than `except` within `radius` of the spot.
bool AlgalonStarWithin(PlayerbotAI* botAI, Position const& spot, float radius, ObjectGuid const& except);
// Its hole is too far to reach in the cast, so it is walking towards it before Big Bang.
bool AlgalonHeadsForFarHole(Player* bot);

// Ranged and healers ring the hold spot.
bool AlgalonTakesRingSlot(Player* bot);
// A melee bot kept outside his reach until a swap tank holds him.
bool AlgalonMeleeStaged(Player* bot);
bool TryGetAlgalonSlotPosition(uint8 slotIndex, Position& position);
// The spot this bot should stand on, stepped aside when a hole buried it, or a waypoint round a hole in
// the way.
bool TryGetAlgalonSlot(Player* bot, Position& position, bool* waypoint = nullptr);
// The undisplaced slot, for dodges that prefer to land near it.
bool GetAlgalonSlotAnchor(Player* bot, Position& slot);
// Where a meteor dodge prefers to land: a hider's hole when a meteor lands in a Big Bang cast, else its slot.
bool GetAlgalonDodgeAnchor(Player* bot, Position& anchor);
// Big Bang latched and the bot will hide: no gap closer may carry it off.
bool AlgalonHiderInCast(Player* bot);

// Stars, constellations and Dark Matter the bot's own job doesn't cover, and Algalon himself for the
// swap tank yielding him.
void AppendAlgalonTargetExclusions(PlayerbotAI* botAI, GuidSet& exclusions);
// Same rule for damage already on its way: false vetoes a hit on this unit.
bool AlgalonMayDamage(Player* bot, Unit* target);

#endif
