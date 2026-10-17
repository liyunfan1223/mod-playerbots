/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERKJ_H
#define PLAYERBOTS_SWPENCOUNTERKJ_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPShared.h"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

namespace SwpHelpers
{

struct KiljaedenRangedBotAssignment
{
    ObjectGuid guid;
    uint8 slotIndex = 0;
};

struct KiljaedenArmageddon
{
    Position destination;
    uint32 expireMs = 0;
    float safeDistance = 0.0f;
};

struct KiljaedenDarknessShieldState
{
    bool inDarkness = false;
    bool shieldCastThisDarkness = false;
    uint32 darknessStartMs = 0;
    uint32 lastDarknessCastMsLeft = 0;
};

inline std::array const KILJAEDEN_DRAGON_ORB_ENTRIES = {
    Id(SwpObjects::GO_DRAGON_ORB_1),
    Id(SwpObjects::GO_DRAGON_ORB_2),
    Id(SwpObjects::GO_DRAGON_ORB_3),
    Id(SwpObjects::GO_DRAGON_ORB_4),
};

inline std::array const KILJAEDEN_DRAGON_SPELLS = {
    Id(SwpSpells::SPELL_SHIELD_OF_THE_BLUE),
    Id(SwpSpells::SPELL_DRAGON_BREATH_HASTE),
    Id(SwpSpells::SPELL_DRAGON_BREATH_REVITALIZE),
};

inline constexpr float KILJAEDEN_PHASE3_HP_THRESHOLD = 85.0f;
inline constexpr float KILJAEDEN_PHASE4_HP_THRESHOLD = 55.0f;
inline constexpr float KILJAEDEN_PHASE5_HP_THRESHOLD = 25.0f;

inline constexpr uint32 HAND_CACHE_INTERVAL_MS = 200;
inline constexpr uint32 DRAGON_ORB_CACHE_INTERVAL_MS = 200;

inline constexpr float HAND_SEARCH_RADIUS = 80.0f;
// Hammer of Justice is a single-target spell that counts both combat reaches so its actual range is
// its tooltip range of 10y + 1.5y (player) + 2.5y (Hand) = 14y. Holding a little inside that
// threshold leaves room for the Hand to shift between the range check and the cast landing.
inline constexpr float HAND_HOLY_PALADIN_STANDOFF = 12.0f;
// Timing between stuns for bots to coordinate them. A gate based purely on UNIT_STATE_STUNNED
// still results in spam stuns due to the delay between a spell casting and resolving.
inline constexpr uint32 HAND_CONTROL_CLAIM_MS = 1500;
// Hands cast Shadow Infusion (45772) at or below 20% HP, which makes them permanently immune
// to both stun and silence.
inline constexpr float HAND_CC_IMMUNE_HP_PERCENT = 20.0f;

// Throttle assigned ranged position rebuilds since they should be stable during the encounter.
inline constexpr uint32 KILJAEDEN_RANGED_ASSIGNMENT_REBUILD_INTERVAL_MS = 1000;
inline constexpr uint32 ARMAGEDDON_ASSIGNMENT_REBUILD_INTERVAL_MS = 250;
inline constexpr float KILJAEDEN_RANGED_ARC_ORIENTATION = 0.8f;
inline constexpr float KILJAEDEN_INNER_RANGED_RADIUS = 23.0f;
inline constexpr float KILJAEDEN_OUTER_RANGED_RADIUS = 36.0f;
inline constexpr uint8 KILJAEDEN_INNER_RANGED_SLOT_COUNT = 7;
inline constexpr uint8 KILJAEDEN_OUTER_RANGED_SLOT_COUNT = 11;
inline constexpr uint8 KILJAEDEN_TOTAL_RANGED_SLOT_COUNT =
    KILJAEDEN_INNER_RANGED_SLOT_COUNT + KILJAEDEN_OUTER_RANGED_SLOT_COUNT;
// Only up to two ranged bots may share a slot when an Armageddon forces a reshuffle.
inline constexpr uint8 KILJAEDEN_MAX_BOTS_PER_RANGED_SLOT = 2;
inline constexpr uint32 ARMAGEDDON_HAZARD_DURATION_MS = 10 * IN_MILLISECONDS;
inline constexpr float ARMAGEDDON_SAFE_DISTANCE = 11.0f;

inline constexpr float KILJAEDEN_REFLECTION_SEARCH_RADIUS = 100.0f;
inline constexpr float DRAGON_ORB_SEARCH_RADIUS = 200.0f;
inline constexpr float DRAGON_ORB_IN_USE_HOLD_DISTANCE = 15.0f;
// Shield of the Blue (45848) lasts 5s and Darkness of a Thousand Souls (46605) is an 8s channel, so
// the dragon casts once <4.5s remain.
inline constexpr int32 SHIELD_OF_THE_BLUE_CAST_WINDOW_MS = 4500;
// How far bots with Fire Bloom stay away from the Darkness stack until the Shield cast window.
inline constexpr float FIRE_BLOOM_STANDOFF = 15.0f;
// Breath: Haste and Breath: Revitalize are 13y cones on allies, so the dragon stops a little
// under half that from its target and looks for a cluster of roughly the same to cover at once.
inline constexpr float KILJAEDEN_DRAGON_BREATH_STANDOFF = 6.0f;
inline constexpr float KILJAEDEN_DRAGON_STANDOFF_TOLERANCE = 1.0f;
inline constexpr float KILJAEDEN_DRAGON_CLUSTER_RADIUS = 6.0f;
inline constexpr uint8 KILJAEDEN_DRAGON_MIN_CLUSTER_SIZE = 3;

inline Position const SUNWELL_CENTER_POSITION =     { 1698.450f, 628.030f, 28.199f };
inline Position const KILJAEDEN_TANK_POSITION =     { 1704.729f, 634.891f, 27.787f };
inline Position const KILJAEDEN_S_MELEE_POSITION =  { 1689.487f, 632.119f, 27.823f };
inline Position const KILJAEDEN_E_MELEE_POSITION =  { 1700.542f, 619.589f, 27.786f };
inline Position const KILJAEDEN_DARKNESS_POSITION = { 1709.768f, 642.241f, 27.706f };

GuidVector FindKiljaedenHandGuids(Player* bot);
std::vector<Unit*> GetKiljaedenHands(PlayerbotAI* botAI);
bool IsKiljaedenHandControlClaimed(Unit* hand);
void ClaimKiljaedenHandControl(Unit* hand);
void AddKiljaedenArmageddon(
    uint32 instanceId, Position const& destination, uint32 durationMs, float safeDistance);
bool TryGetKiljaedenNearestArmageddon(Player* bot, KiljaedenArmageddon& armageddon);
void PruneExpiredKiljaedenArmageddons(uint32 instanceId);
bool TryGetKiljaedenRangedSlotPosition(uint8 slotIndex, Position& position);
void EnsureKiljaedenRangedAssignments(Player* bot);
void EnsureKiljaedenRangedArmageddonAssignments(Player* bot);
bool IsKiljaedenCastingDarknessOfAThousandSouls(Unit* kiljaeden);
GuidVector FindKiljaedenDragonOrbGuids(Player* bot);
Player* GetKiljaedenDragonOrbUser(Player* bot);
bool HasUsedKiljaedenDragonOrb(Player* bot);
bool HasKiljaedenDragonAura(Player* bot);
Unit* GetKiljaedenControlledDragon(Player* bot);
bool CastKiljaedenDragonSpell(Unit* dragon, uint32 spellId);
Player* FindBestKiljaedenDragonClusterTarget(Player* bot, Unit* dragon, uint32 spellId);
Player* FindClosestKiljaedenDragonTarget(Player* bot, Unit* dragon, uint32 spellId = 0);
bool HasStaleRootFlag(Player* bot);
bool ReleaseStaleRootFlag(Player* bot);

// Shared encounter state

struct KiljaedenEncounterState
{
    std::vector<KiljaedenArmageddon> armageddons;
    std::unordered_map<ObjectGuid, uint8> rangedAssignments;
    std::unordered_map<ObjectGuid, uint8> rangedArmageddonAssignments;
    std::unordered_map<ObjectGuid, uint32> handControlClaims;
    std::unordered_set<ObjectGuid> dragonOrbUsers;
    std::unordered_set<ObjectGuid> trackedArmageddonTargets;
    uint32 dragonOrbAnnouncementMs = 0;
    uint32 rangedAssignmentRebuildMs = 0;
    uint32 rangedArmageddonRebuildMs = 0;
};

KiljaedenEncounterState& KiljaedenState(uint32 instanceId);
KiljaedenEncounterState* FindKiljaedenState(uint32 instanceId);
bool KiljaedenResetInstance(uint32 instanceId);

}

#endif
