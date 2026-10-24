/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_EOEDATA_H
#define PLAYERBOTS_EOEDATA_H

#include "Common.h"

#include <cmath>
#include <utility>

enum EyeOfEternityIDs
{
    NPC_MALYGOS                         = 28859,
    NPC_POWER_SPARK                     = 30084,
    NPC_NEXUS_LORD                      = 30245,
    NPC_SCION_OF_ETERNITY               = 30249,
    NPC_WYRMREST_SKYTALON               = 30161,
    NPC_ARCANE_OVERLOAD                 = 30282,
    NPC_SURGE_OF_POWER                  = 30334,
    NPC_STATIC_FIELD                    = 30592,
    NPC_HOVER_DISK                      = 30248,

    // Nexus Lord self-cast, the one buff worth a spellsteal in P2.
    SPELL_HASTE                         = 57060,

    // Boss hazards
    SPELL_ARCANE_OVERLOAD_AURA          = 56432,    // ticks on the P2 bubble, re-granting the protection
    SPELL_ARCANE_OVERLOAD_PROTECTION    = 56438,    // -50% damage taken, granted inside the bubble
    SPELL_SURGE_OF_POWER_P2             = 56505,    // P2 beam

    // Skytalon abilities
    SPELL_FLAME_SPIKE                   = 56091,
    SPELL_ENGULF_IN_FLAMES              = 56092,
    SPELL_REVIVIFY                      = 57090,
    SPELL_LIFE_BURST                    = 57143,
    SPELL_FLAME_SHIELD                  = 57108,
};

inline constexpr uint32 EOE_MAP_ID = 616;
inline constexpr uint8 EOE_RAID_SIZE_10MAN = 10;
inline constexpr uint8 EOE_RAID_SIZE_25MAN = 25;
// Mirrors DATA_MALYGOS from the core's eye_of_eternity.h, script headers aren't on the module include path.
inline constexpr uint32 EOE_DATA_MALYGOS = 0;
// Guid slots boss_malygos.cpp fills with the P3 surge victims, 3s before the beam.
// Mirrored for the same reason as EOE_DATA_MALYGOS.
inline constexpr int32 EOE_DATA_FIRST_SURGE_TARGET_GUID = 14;
inline constexpr uint8 EOE_NUM_MAX_SURGE_TARGETS = 3;

inline constexpr float EOE_TWO_PI = 2.0f * static_cast<float>(M_PI);
inline constexpr float EOE_DIRECTION_EPSILON = 0.01f;

// Drops a latch left over from an earlier pull in the same instance. Far longer than any encounter,
// so it never expires mid-fight.
inline constexpr uint32 EOE_LATCH_STALE_MS = 5 * MINUTE * IN_MILLISECONDS;

// Platform centre and floor (CenterPos in the core script).
inline constexpr std::pair<float, float> MALYGOS_CENTER_POSITION = {754.395f, 1301.27f};
inline constexpr float MALYGOS_PLATFORM_Z = 266.10f;
// The four bearings from centre Malygos can land on, mirrored from the core's FourSidesPos.
inline constexpr float MALYGOS_LANDING_ANGLES[] = {-2.3729f, 0.8117f, 2.3467f, -0.7783f};
inline constexpr uint8 MALYGOS_LANDING_ANGLE_COUNT = 4;

// P2 and transition anti-fall ring around the platform centre, and how far inside it a drifting bot is put back.
inline constexpr float MALYGOS_ANTIFALL_RADIUS = 30.0f;
inline constexpr float MALYGOS_ANTIFALL_INSET = 3.0f;

// P1 hold spots: signed distance from centre along the landing bearing, positive towards Malygos.
// The tank spot has to stay under 47.5, where the floor steps up a yard.
inline constexpr float MALYGOS_MAINTANK_OFFSET = 46.0f;
inline constexpr float MALYGOS_STACK_OFFSET = 12.0f;
inline constexpr float MALYGOS_HUNTER_OFFSET = -14.0f;
inline constexpr float MALYGOS_P1_POSITION_TOLERANCE = 5.0f;
inline constexpr float MALYGOS_MELEE_HOLD_DISTANCE = 15.0f;

// Centre to centre, no bounding radii: the script's IsWithinDist3d takes the Position overload.
inline constexpr float POWER_SPARK_BUFF_RADIUS = 12.0f;
// DK grip spot, valid in [3.7, 5.2]: below that grip can't reach a spark on his bearing, above it
// POWER_SPARK_GRIP_SAFE_BOSS_DISTANCE fails. Keeps the stack inside the spark corpse buff (55849).
inline constexpr float POWER_SPARK_GRIP_OFFSET = 4.5f;
// Tighter than the P1 tolerance, the grip only lands right if the DK is actually on the spot.
inline constexpr float POWER_SPARK_GRIP_TOLERANCE = 2.0f;
inline constexpr float POWER_SPARK_GRIP_ENGAGE_RADIUS = 45.0f;
inline constexpr float POWER_SPARK_GRIP_SAFE_BOSS_DISTANCE = POWER_SPARK_BUFF_RADIUS + 6.0f;
// Slack on melee reach before a melee bot lets go of a spark it is already hitting.
inline constexpr float POWER_SPARK_MELEE_STICKY = 3.0f;
// How close a spark has to be before a DK spends a rune snaring it. A gripped spark lands at his
// feet, so this keeps Chains of Ice on the one he pulled rather than one crossing to Malygos.
inline constexpr float POWER_SPARK_SNARE_RADIUS = 15.0f;

inline constexpr float BUBBLE_SEARCH_RADIUS = 60.0f;
// Fraction of its protection radius an Arcane Overload bubble loses per tick, and the floor below
// which it is not worth crossing the platform for.
inline constexpr float BUBBLE_SHRINK_PER_TICK = 0.02f;
inline constexpr float BUBBLE_MIN_USABLE_FACTOR = 0.35f;

inline constexpr float DISK_APPROACH_REACH = 3.0f;
inline constexpr float DISK_APPROACH_TOLERANCE = 2.0f;
inline constexpr float DISK_MAX_WALK_DISTANCE = 40.0f;
// Highest a disk may hover over the platform before its rider steps off.
inline constexpr float DISK_DISMOUNT_MAX_HEIGHT = 5.0f;

// How far the P2 Surge of Power focus is looked for.
inline constexpr float EOE_SURGE_SEARCH_RADIUS = 100.0f;
inline constexpr float SURGE_BEAM_CLEAR_DISTANCE = 12.0f;
inline constexpr float SURGE_BEAM_SIDESTEP = 6.0f;

// Arcane Pulse (57432) radius, the P3 flight never gets closer to the boss than this.
inline constexpr float ARCANE_PULSE_RADIUS = 30.0f;
// Radius of the P3 stack ring, and its fixed height and heading. Malygos' live Z is no good: he
// opens the phase 70y up and sinks to his fight position seconds later.
inline constexpr float DRAKE_STACK_RADIUS = ARCANE_PULSE_RADIUS + 15.0f;
inline constexpr float MALYGOS_P3_BOSS_Z = MALYGOS_PLATFORM_Z - 5.0f;
inline constexpr float DRAKE_STACK_ANGLE = -static_cast<float>(M_PI_2);
// How far off the stack point a drake drifts before it re-flies. Loose on purpose: it is what stops
// the flight piling onto one coordinate.
inline constexpr float DRAKE_STACK_TOLERANCE = 10.0f;
inline constexpr uint32 DRAKE_STACK_RECALC_MS = 300;
// How far around the ring a drake covers in one hop, kept short so the chord clears Arcane Pulse.
inline constexpr float DRAKE_APPROACH_ARC = static_cast<float>(M_PI) / 3.0f;
inline constexpr uint8 DRAKE_RING_HEADINGS = 24;
inline constexpr float DRAKE_DESTINATION_EPSILON = 2.0f;

// Drake abilities reach 60y; leave headroom for drakes parked on the far side of the stack.
inline constexpr float DRAKE_ATTACK_RANGE = 55.0f;
inline constexpr float DRAKE_FORMUP_RADIUS = 20.0f;
inline constexpr float DRAKE_FORMUP_SPREAD = 15.0f;

inline constexpr uint8 DRAKE_HEALERS_25MAN = 5;
inline constexpr uint8 DRAKE_HEALERS_10MAN = 2;
// Combo points a healer banks before Life Burst, for the +50% healing buff rather than the heal.
inline constexpr uint8 DRAKE_LIFE_BURST_COMBO = 5;
inline constexpr uint8 DRAKE_ENGULF_COMBO = 3;
// Bank a fixated drake dumps into Engulf instead of saving it for the shield. One point only
// refreshes the stack for 6 s, less than the rebuild cycle, two carry it 10 s.
inline constexpr uint8 DRAKE_ENGULF_SURGE_COMBO = 2;
// Life Burst's self buff at full combo, and how little may be left before a healer renews it.
inline constexpr uint32 DRAKE_LIFE_BURST_BUFF_MS = 25000;
inline constexpr uint32 DRAKE_LIFE_BURST_REFRESH_MS = 5000;
inline constexpr uint32 DRAKE_BURST_STAGGER_MS = 1500;
// Drake health at which a burst is wanted, and at which the whole corps goes and the stagger is off.
inline constexpr uint8 DRAKE_BURST_HEALTH_PCT = 90;
inline constexpr uint8 DRAKE_BURST_EMERGENCY_PCT = 30;
inline constexpr uint32 DRAKE_LIFE_BURST_HEAL = 5000;
// A Life Burst plus a Flame Shield. Below this a capped healer stops casting and lets the bar fill.
inline constexpr uint32 DRAKE_HOLD_ENERGY_FLOOR = 75;

// The beam has to be covered across [DELAY, END]; nothing before it counts.
inline constexpr uint32 SURGE_BEAM_DELAY_MS = 3000;
inline constexpr uint32 SURGE_BEAM_END_MS = 6000;
// One repeat of EVENT_SPELL_PH3_SURGE_OF_POWER. A drake still flagged this long after its fixate
// began was picked again, since back-to-back picks leave no gap in the slots to spot.
inline constexpr uint32 SURGE_CYCLE_MS = 7000;
inline constexpr uint32 DRAKE_SHIELD_BASE_MS = 1000;
inline constexpr uint32 DRAKE_SHIELD_MS_PER_COMBO = 1000;
// Above this the bank is worth more as a finisher, so the shield waits for the rotation to spend it.
inline constexpr uint8 DRAKE_SHIELD_MAX_COMBO = 3;
// What a fixated drake rebuilds to. Two would cover the whole beam, but the energy bar can't fund
// them, and a shield that never goes up eats the full surge.
inline constexpr uint8 DRAKE_SHIELD_RESERVE_COMBO = 1;
inline constexpr uint32 DRAKE_FIXATE_GAP_MS = 2000;

// Radius a drake has to clear of a Static Field, and the wider radius the stack point has to clear
// because drakes park anywhere within DRAKE_STACK_TOLERANCE of it.
inline constexpr float STATIC_FIELD_SAFE_RADIUS = 32.0f;
inline constexpr float STATIC_FIELD_CLEARANCE = STATIC_FIELD_SAFE_RADIUS + DRAKE_STACK_TOLERANCE;

#endif
