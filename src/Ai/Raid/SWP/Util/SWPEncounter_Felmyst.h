/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPENCOUNTERFELMYST_H
#define PLAYERBOTS_SWPENCOUNTERFELMYST_H

#include "ObjectGuid.h"
#include "Position.h"
#include "SWPShared.h"
#include <array>
#include <limits>
#include <unordered_map>
#include <vector>

class Creature;
class Player;
class Unit;

namespace SwpHelpers
{

enum class FogLane : uint8
{
    None = std::numeric_limits<uint8>::max(),
    Top = 0,
    Middle = 1,
    Bottom = 2,
};

enum class FogPhase : uint8
{
    None,
    Windup,
    Sweep,
    Recovery,
};

enum class FogLocation : uint8
{
    None,
    LeftSide,
    RightSide,
    LeftTop,
    LeftMiddle,
    LeftBottom,
    RightTop,
    RightMiddle,
    RightBottom,
};

enum class FelmystGroundStack : uint8
{
    None = std::numeric_limits<uint8>::max(),
    Melee = 0,
    Left = 1,
    Right = 2,
};

struct IncomingEncapsulateState
{
    ObjectGuid targetGuid = ObjectGuid::Empty;
    uint32 delayMs = 0;
    uint32 expireMs = 0;
    bool auraObserved = false;
};

struct DemonicVaporAnchor
{
    Position position;
    FogLane lane;
    uint8 sideMask;
};

struct FogOfCorruptionState
{
    FogLane lane = FogLane::None;
    FogPhase phase = FogPhase::None;
    uint32 expireMs = 0;
};

struct FogPassState
{
    FogLocation lastDestinationLocation = FogLocation::None;
    FogLane lastCompletedLane = FogLane::None;
    FogLane armedSweepLane = FogLane::None;
    uint8 completedPassCount = 0;
    uint32 thirdPassWindowExpireMs = 0;
};

struct FogSafeThreshold
{
    Position a, b;
    bool safeSideIsNorth;  // true = safe side has higher X (north), false = lower X (south)
};

struct FelmystDemonicVaporGuids
{
    GuidVector heads;
    GuidVector trails;
};

inline constexpr uint32 DEMONIC_VAPOR_CACHE_INTERVAL_MS = 200;
inline constexpr float DEMONIC_VAPOR_SEARCH_RADIUS = 100.0f;
// A head is treated as its summoner's only within this distance of them, edge to edge.
inline constexpr float DEMONIC_VAPOR_SUMMONER_RADIUS = 50.0f;

// How close to the center of a ground stack a bot stops, center to center.
inline constexpr float FELMYST_RANGED_GROUP_RADIUS = 2.0f;
// Bots wait this long to react to Encapsulate (to make the action look less artificial).
inline constexpr uint32 ENCAPSULATE_DELAY_MS = 300;
// How close ranged have to be to a player charmed by Fog to attack.
inline constexpr float FELMYST_CHARMED_TARGET_RANGE = 30.0f;
// How long after landing following a flight phase does the raid keeps holding DPS.
inline constexpr uint32 FELMYST_GROUNDED_DPS_WAIT_MS = 3 * IN_MILLISECONDS;
// How close Felmyst must be to a specified position to be considered there.
inline constexpr float FELMYST_LOCATION_MATCH_DISTANCE = 2.0f;
// How far a fog destination must be from any vapor. Vapor has a 3y radius and no real buffer is
// needed since this distance is for a non-moving bot, but the visual looks much wider than 3y
// so a bit of leeway allows for some more realism as players would not stand so close.
inline constexpr float FOG_DESTINATION_VAPOR_CLEARANCE = 8.0f;

inline Position const FOG_LEFT_SIDE =  { 1469.064f, 729.585f, 59.824f, 4.677f };
inline Position const FOG_RIGHT_SIDE = { 1458.556f, 502.200f, 59.900f, 1.606f };

inline Position const FELMYST_LEFT_LANDING_POSITION =   { 1476.770f, 665.094f, 20.642f };
inline Position const FELMYST_RIGHT_LANDING_POSITION =  { 1469.930f, 557.009f, 22.632f };
inline Position const FELMYST_CENTER_GROUND_REFERENCE = { 1473.350f, 611.052f, 21.637f };

inline Position const FOG_CRATE_STUCK_POSITION =    { 1484.443f, 591.337f, 23.391f };
inline Position const FOG_CRATE_TELEPORT_POSITION = { 1482.181f, 591.253f, 24.545f };

inline std::array const FELMYST_TANK_POSITIONS = {
    Position{ 1460.145f, 598.290f, 21.869f },
    Position{ 1480.587f, 636.805f, 21.713f },
    Position{ 1479.524f, 584.069f, 23.231f },
};

inline std::array const FOG_LEFT_LANES = {
    Position{ 1494.745f, 704.000f, 50.085f, 4.747f },
    Position{ 1469.923f, 703.239f, 50.086f, 4.747f },
    Position{ 1446.515f, 701.518f, 50.085f, 4.747f },
};

inline std::array const FOG_RIGHT_LANES = {
    Position{ 1492.820f, 515.668f, 50.083f, 1.449f },
    Position{ 1466.732f, 515.595f, 50.572f, 1.449f },
    Position{ 1441.640f, 520.520f, 50.083f, 1.449f },
};

// Note that WoW coordinates are rotated 90° from real-life coordinates.
inline std::array const FOG_SAFE_THRESHOLDS = {
    FogSafeThreshold{ // Top lane safe threshold (west→east: safe = south)
        Position{ 1470.122f, 660.345f, 20.462f },
        Position{ 1470.358f, 560.042f, 22.635f },
        false,
    },
    FogSafeThreshold{ // Middle lane safe threshold (west→east: safe = north)
        Position{ 1498.880f, 675.159f, 22.511f },
        Position{ 1497.864f, 546.197f, 26.351f },
        true,
    },
    FogSafeThreshold{ // Bottom lane safe threshold (west→east: safe = north)
        Position{ 1477.381f, 659.824f, 21.051f },
        Position{ 1477.397f, 555.516f, 23.968f },
        true,
    }
};

inline float const FELMYST_LEFT_LANDING_Y = FELMYST_LEFT_LANDING_POSITION.GetPositionY();
inline float const FELMYST_LEFT_LANDING_Z = FELMYST_LEFT_LANDING_POSITION.GetPositionZ();
inline float const FELMYST_RIGHT_LANDING_Y = FELMYST_RIGHT_LANDING_POSITION.GetPositionY();
inline float const FELMYST_RIGHT_LANDING_Z = FELMYST_RIGHT_LANDING_POSITION.GetPositionZ();

inline constexpr uint8 DEMONIC_VAPOR_LEFT_SIDE = 0x1;
inline constexpr uint8 DEMONIC_VAPOR_RIGHT_SIDE = 0x2;

inline std::array const DEMONIC_VAPOR_KITE_ANCHORS = {
    DemonicVaporAnchor{
        Position{ 1492.820f, FELMYST_RIGHT_LANDING_Y, FELMYST_RIGHT_LANDING_Z },
        FogLane::Top, DEMONIC_VAPOR_RIGHT_SIDE,
    },
    DemonicVaporAnchor{
        Position{ 1494.745f, FELMYST_LEFT_LANDING_Y, FELMYST_LEFT_LANDING_Z },
        FogLane::Top, DEMONIC_VAPOR_LEFT_SIDE,
    },
    DemonicVaporAnchor{
        Position{ 1466.732f, FELMYST_RIGHT_LANDING_Y, FELMYST_RIGHT_LANDING_Z },
        FogLane::Middle, DEMONIC_VAPOR_RIGHT_SIDE,
    },
    DemonicVaporAnchor{
        Position{ 1469.923f, FELMYST_LEFT_LANDING_Y, FELMYST_LEFT_LANDING_Z },
        FogLane::Middle, DEMONIC_VAPOR_LEFT_SIDE,
    },
    DemonicVaporAnchor{
        Position{ 1441.640f, FELMYST_RIGHT_LANDING_Y, FELMYST_RIGHT_LANDING_Z },
        FogLane::Bottom, DEMONIC_VAPOR_RIGHT_SIDE,
    },
    DemonicVaporAnchor{
        Position{ 1446.515f, FELMYST_LEFT_LANDING_Y, FELMYST_LEFT_LANDING_Z },
        FogLane::Bottom, DEMONIC_VAPOR_LEFT_SIDE,
    }
};

inline std::array const DEMONIC_VAPOR_LANE_REFERENCES = {
    Position{ 1493.783f, 609.834f, 50.084f },
    Position{ 1468.328f, 609.417f, 50.329f },
    Position{ 1444.078f, 611.019f, 50.084f },
};

Position const& GetFelmystMainTankGroundPosition(Player* bot);
bool TryGetFelmystGroundStackPosition(
    Player* bot, Unit* felmyst, FelmystGroundStack stack, Position& position);
FelmystGroundStack GetClosestFelmystGroundStack(Player* bot, Unit* felmyst, Unit* unit);
float GetFelmystFrontAngle(Player* bot, Unit* felmyst);
bool TryGetFelmystRangedPosition(Player* bot, Unit* felmyst, Position& position);
FelmystDemonicVaporGuids FindFelmystDemonicVaporGuids(Player* bot);
Creature* GetFelmystDemonicVaporSummonedBy(Player* bot, Player* summoner);
bool IsFelmystDemonicVaporHeadNearBot(Player* bot);
std::vector<Creature*> GetDemonicVaporHazards(Player* bot);
void ClearFelmystDemonicVaporKiteState(Player* bot);
bool TryGetFelmystDemonicVaporKiteDestination(Player* bot, Position& destination);
bool TryGetFelmystFogCrossingDestination(Player* bot, FogLane dangerLane, Position& destination);
bool TryGetFelmystLandingApproachDestination(
    Player* bot, FogLane lastCompletedLane, Unit* felmyst, Position& destination);
bool IsFelmystLanding(Unit* felmyst);
bool IsFelmystAirPhaseTargetSuppressed(Unit* felmyst);
bool TryGetFelmystPostThirdPassWindow(Unit* felmyst, FogLane& lane);
bool IsFelmystFogMovementSuppressed(Unit* felmyst);
bool IsFelmystFogActiveForBot(Player* bot, Unit* felmyst);
bool TryGetFelmystFogOfCorruptionStageState(Unit* felmyst, FogOfCorruptionState& state);
bool TryGetActiveFogOfCorruptionState(Player* bot, Unit* felmyst, FogOfCorruptionState& state);
void RecordFelmystIncomingEncapsulateTarget(Player* target, uint32 durationMs = 3000);
Player* GetFelmystEncapsulateTarget(Player* bot);
bool ShouldMoveAwayFromFelmystEncapsulateTarget(
    Player* bot, Unit* felmyst, Player* encapsulateTarget);
bool DidEncapsulateOccurThisGroundPhase(Player* bot);
Player* GetFelmystGasNovaDispelTarget(Player* bot);
Player* GetFelmystCharmedTarget(Player* bot, Unit* felmyst);
Player* GetFelmystFlightLeader(Player* player);

// Shared encounter state

struct FelmystEncounterState
{
    IncomingEncapsulateState incomingEncapsulate;
    bool encapsulateOccurredThisGroundPhase = false;
    std::unordered_map<ObjectGuid, uint8> demonicVaporRegionIndices;
    uint8 demonicVaporUsedRegionMask = 0;
    uint8 demonicVaporFirstRegionIndex = 0;
    FogOfCorruptionState fogOfCorruption;
    FogPassState fogPass;
    uint32 landingDpsWaitStartMs = 0;
    uint32 landingTouchdownMs = 0;
    ObjectGuid flightLeaderGuid = ObjectGuid::Empty;
};

FelmystEncounterState& FelmystState(uint32 instanceId);
FelmystEncounterState* FindFelmystState(uint32 instanceId);
bool FelmystResetInstance(uint32 instanceId);

}

#endif
