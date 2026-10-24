/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERTWINS_H
#define PLAYERBOTS_SWPENCOUNTERTWINS_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPShared.h"
#include <array>
#include <optional>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

namespace SwpHelpers
{

struct EredarTwinsIncomingConflagrationState
{
    ObjectGuid targetGuid = ObjectGuid::Empty;
    uint32 delayMs = 0;
    uint32 expireMs = 0;
};

struct EredarTwinsBlazeTargetState
{
    ObjectGuid targetGuid = ObjectGuid::Empty;
    uint32 startMs = 0;
};

// Which branch of the Alythess tank assignment was taken (for the raid announcement).
enum class AlythessTankSource : uint8
{
    Unresolved,
    MainTankPaladin,  // (1) The main tank is a Paladin;
    PaladinTank,      // (2) the Paladin tank with highest max HP; or
    MainTankFallback, // (3) no Paladin tank at all, so the non-Paladin main tank
};

struct EredarTwinsTankAssignment
{
    ObjectGuid alythessTankGuid = ObjectGuid::Empty;
    AlythessTankSource source = AlythessTankSource::Unresolved;
    uint32 announcementMs = 0;
};

// Used to measure if a bot is on the balcony; deliberately a little below the actual balcony Z.
inline constexpr float EREDAR_TWINS_BALCONY_Z = 50.0f;

inline constexpr uint32 EREDAR_TWINS_BLAZE_CACHE_INTERVAL_MS = 200;
// The Blaze trap GO casts 45246, dealing damage in a 3y radius; extra 1.5y is avoidance buffer.
inline constexpr float BLAZE_DANGER_RADIUS = 4.5f;
inline constexpr float BLAZE_SEARCH_RADIUS = 30.0f;

// Grace period for the tanks to build threat before the rest of the raid opens fire.
inline constexpr uint32 EREDAR_TWINS_DPS_HOLD_MS = 8 * IN_MILLISECONDS;
// DPS cooldowns are held until Sacrolash is at this percent. Eredar Twins is a threat-sensitive
// fight due to Sacrolash dropping threat on tanks and Alythess targeting Conflagration based on
// Sacrolash's threat table.
inline constexpr float EREDAR_TWINS_MAX_DPS_HP_PERCENT = 80.0f;
// Don't exceed this percentage of the tank's threat. For Sacrolash, the limit is based on the
// second highest tank (unless there is only one Sacrolash tank).
inline constexpr float SACROLASH_THREAT_HOLD_RATIO = 0.8f;
inline constexpr float ALYTHESS_THREAT_HOLD_RATIO = 0.9f;

inline constexpr float CONFLAGRATION_SAFE_DISTANCE = 10.0f;
// Bots wait 300ms to react to Conflagration (to make the action look less artificial).
inline constexpr uint32 CONFLAGRATION_DELAY_MS = 300;
// Conflagration is a 3.5s cast, and Blaze is a 2.5s cast. For Conflagration, the bot needs to hold
// position through the impact, so some time is added to account for the projectile's travel time.
inline constexpr uint32 CONFLAGRATION_WINDOW_MS = 5500;
inline constexpr uint32 BLAZE_TARGET_WINDOW_MS = 2500;

inline constexpr uint8 FLAME_TOUCHED_PROTECT_STACKS = 5;
inline constexpr int32 FLAME_SEAR_PROTECT_WINDOW_MS = 2 * IN_MILLISECONDS;

inline Position const ALYTHESS_START_POSITION = { 1819.180f, 625.539f, 33.4038f };
inline std::array const ALYTHESS_TANK_POSITIONS = {
    Position{ 1816.830f, 620.792f, 33.404f },
    Position{ 1824.211f, 625.169f, 33.404f },
    Position{ 1818.701f, 631.196f, 33.404f },
    Position{ 1829.375f, 631.110f, 33.404f },
    Position{ 1830.007f, 620.924f, 33.404f }
};

// Phase 1 positions (ranged up top, melee on Sacrolash).
inline Position const SACROLASH_TANK_POSITION  =             { 1804.255f, 630.193f, 33.404f };
inline Position const EREDAR_TWINS_P1_RANGED_POSITION =      { 1808.076f, 603.460f, 51.684f };
inline Position const EREDAR_TWINS_MELEE_CONFLAG_POSITION =  { 1812.842f, 611.147f, 33.404f };
inline Position const EREDAR_TWINS_RANGED_CONFLAG_POSITION = { 1801.133f, 584.456f, 50.696f };

// Phase 2 positions (everybody stack behind Alythess).
inline Position const EREDAR_TWINS_P2_MELEE_POSITION =       { 1814.327f, 625.645f, 33.404f };
inline Position const EREDAR_TWINS_P2_RANGED_POSITION =      { 1805.587f, 625.653f, 33.404f };

Position GetAlythessTankPosition(Unit* alythess, uint8 index);
Position GetEredarTwinsP2MeleePosition(Unit* alythess);
Position GetEredarTwinsP2RangedPosition(Unit* alythess);
void ResolveEredarTwinsTankAssignment(Player* bot);
Player* GetAlythessTank(Player* bot);
bool IsAlythessTank(Player* bot);
AlythessTankSource GetAlythessTankSource(Player* bot);
Player* GetSacrolashTank(Player* bot, uint8 index);
bool IsAnySacrolashTank(Player* bot);
bool ShouldHoldAlythessThreat(Player* bot, Unit* alythess);
bool ShouldHoldSacrolashThreat(Player* bot, Unit* sacrolash);
std::vector<Position> FindEredarTwinsBlazePositions(Player* bot);
bool IsAlythessTankPositionSafe(PlayerbotAI* botAI, Position const& position);
bool ShouldAdvanceAlythessTankPosition(Unit* alythess, Player* bot, ObjectGuid& lastBlazeGuid);
void RecordIncomingEredarTwinsConflagrationTarget(Player* target);
Player* GetEredarTwinsConflagrationTarget(Player* bot);
void RecordEredarTwinsBlazeTarget(Player* target);
Player* GetEredarTwinsBlazeTarget(Player* bot);

// Shared encounter state

// The conflagration and Blaze targets are cleared mid-fight when they expire, so each field is
// optional rather than the whole entry being erased.
struct EredarTwinsEncounterState
{
    std::optional<EredarTwinsIncomingConflagrationState> incomingConflagration;
    std::optional<EredarTwinsBlazeTargetState> blazeTarget;
    std::optional<uint32> dpsHoldStartMs;
    std::optional<EredarTwinsTankAssignment> tankAssignment;
};

EredarTwinsEncounterState& EredarTwinsState(uint32 instanceId);
EredarTwinsEncounterState* FindEredarTwinsState(uint32 instanceId);
bool EredarTwinsResetInstance(uint32 instanceId);

}

#endif
