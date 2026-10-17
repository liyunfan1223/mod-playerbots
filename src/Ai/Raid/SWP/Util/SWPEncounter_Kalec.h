/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERKALEC_H
#define PLAYERBOTS_SWPENCOUNTERKALEC_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPShared.h"
#include <array>
#include <limits>
#include <unordered_map>

class Group;
class Player;
class PlayerbotAI;

namespace SwpHelpers
{

inline constexpr uint8 KALECGOS_INVALID_GROUP = std::numeric_limits<uint8>::max();
inline constexpr uint8 KALECGOS_TANK_COUNT = 3;
// Non-tank bots are sorted into this many groups, with dispellers and healers as evenly distributed
// as possible.
inline constexpr uint8 KALECGOS_GROUP_COUNT = 4;

// How long assist tanks hold off on attacking after the pull.
inline constexpr uint32 KALECGOS_PULL_THREAT_SUPPRESSION_MS = 5 * IN_MILLISECONDS;
// Curse of Boundless Agony doubles its tick damage every 5 ticks and, when removed by dispel or
// expiration, bounces to another player. Hold off on dispelling while damage is low.
inline constexpr int32 KALECGOS_DISPEL_REMAINING_MS = 15 * IN_MILLISECONDS;

// Rifts remain active for 10s after spawning.
inline constexpr uint32 SPECTRAL_RIFT_ACTIVE_WINDOW_MS = 10 * IN_MILLISECONDS;
// Used to determine if Exhaustion will expire on a bot before the rift expires. Reduced to account
// for AI tick delay + potential latency. Signed because it is compared against an aura duration.
inline constexpr int32 SPECTRAL_RIFT_ENTRY_WINDOW_MS =
    static_cast<int32>(SPECTRAL_RIFT_ACTIVE_WINDOW_MS) - 200;
inline constexpr float SPECTRAL_RIFT_SEARCH_RADIUS = 75.0f;
inline constexpr uint32 SPECTRAL_RIFT_CACHE_INTERVAL_MS = 200;
// Approximate Z-coordinate of the Spectral Realm, used to force teleport bots down if they are
// struggling with gravity.
inline constexpr float SPECTRAL_REALM_Z = -74.5f;

inline Position const KALECGOS_TANK_POSITION =           { 1703.584f, 895.626f, 53.076f };
inline Position const KALECGOS_INITIAL_RANGED_POSITION = { 1704.634f, 938.080f, 53.076f };

bool IsExhausted(Player* bot);
bool IsInSpectralRealm(Player* bot);
bool IsKalecgosDecurser(Player* bot);
void EnsureKalecgosRaidAssignments(Player* bot);
Player* FindKalecgosDesignatedTank(Player* player);
Player* GetKalecgosDesignatedTank(Player* player);
ObjectGuid FindKalecgosSpectralRiftGuid(Player* bot);
bool ShouldEnterKalecgosPortal(Player* bot);
void RecordSpectralBlastTarget(Player* player, PlayerbotAI* announcerAI);
void RecordSpectralRealmEnter(Player* player);

// Shared encounter state

struct KalecgosEncounterState
{
    uint32 encounterStartMs = 0;
    uint32 activeRiftOpenedMs = 0;
    uint8 activeRiftGroup = KALECGOS_INVALID_GROUP;
    ObjectGuid blastedPlayerGuid = ObjectGuid::Empty;
    ObjectGuid currentTankGuid = ObjectGuid::Empty;
    ObjectGuid activeRiftOutgoingTankGuid = ObjectGuid::Empty;
    bool surfaceHealthAnnounced = false;
    bool spectralHealthAnnounced = false;
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankAssignmentGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };
    std::array<ObjectGuid, KALECGOS_TANK_COUNT> tankPortalRotationGuids = {
        ObjectGuid::Empty, ObjectGuid::Empty, ObjectGuid::Empty };
    std::unordered_map<ObjectGuid, uint8> playerToGroup;
};

KalecgosEncounterState& KalecgosState(uint32 instanceId);
KalecgosEncounterState* FindKalecgosState(uint32 instanceId);
bool KalecgosResetInstance(uint32 instanceId);

}

#endif
