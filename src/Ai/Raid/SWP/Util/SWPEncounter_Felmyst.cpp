/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Felmyst.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SWPShared.h"
#include <algorithm>
#include <cmath>
#include <list>
#include <mutex>

using namespace EncounterHelpers;

namespace SwpHelpers
{

// Note: Felmyst's CombatReach is 10.0f

namespace
{

std::mutex felmystStateMutex;
std::unordered_map<uint32, FelmystEncounterState> felmystStates;

FelmystDemonicVaporGuids const& GetCachedDemonicVapors(Player* bot)
{
    static FelmystDemonicVaporGuids const none;
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return none;

    return botAI->GetAiObjectContext()
        ->GetValue<FelmystDemonicVaporGuids>("felmyst demonic vapors")->RefGet();
}

Creature* GetLivingCreature(Player* bot, ObjectGuid const& guid)
{
    Creature* creature = ObjectAccessor::GetCreature(*bot, guid);
    return creature && creature->IsAlive() ? creature : nullptr;
}

void ResetDemonicVaporFlightState(uint32 instanceId)
{
    FelmystEncounterState* state = FindFelmystState(instanceId);
    if (!state)
        return;

    state->demonicVaporRegionIndices.clear();
    state->demonicVaporUsedRegionMask = 0;
    state->demonicVaporFirstRegionIndex = 0;
}

void ResetDemonicVaporFlightStateIfGrounded(Player* bot)
{
    constexpr float searchRadius = 250.0f;
    Creature* felmyst = bot->FindNearestCreature(Id(SwpNpcs::NPC_FELMYST), searchRadius);
    if (!felmyst || !felmyst->IsFlying())
        ResetDemonicVaporFlightState(bot->GetInstanceId());
}

bool TryGetGroundStackCenter(
    Player* bot, Unit* felmyst, FelmystGroundStack stack, float& positionX, float& positionY)
{
    if (!felmyst)
        return false;

    switch (stack)
    {
        case FelmystGroundStack::Melee:
        {
            constexpr float behindDistance = 12.5f;
            float const behindAngle = Position::NormalizeOrientation(
                GetFelmystFrontAngle(bot, felmyst) + M_PI);
            positionX = felmyst->GetPositionX() + behindDistance * std::cos(behindAngle);
            positionY = felmyst->GetPositionY() + behindDistance * std::sin(behindAngle);
            return true;
        }
        case FelmystGroundStack::Left:
        case FelmystGroundStack::Right:
        {
            constexpr float sideDistance = 24.0f;
            float const frontAngle = GetFelmystFrontAngle(bot, felmyst);
            float const sideAngle = frontAngle +
                (stack == FelmystGroundStack::Left ? M_PI_2 : -M_PI_2);
            positionX = felmyst->GetPositionX() + std::cos(sideAngle) * sideDistance;
            positionY = felmyst->GetPositionY() + std::sin(sideAngle) * sideDistance;
            return true;
        }
        default:
            return false;
    }
}

FogLocation GetFogLocationFromLanePointIndex(uint8 laneIndex, bool useLeftPoint)
{
    switch (laneIndex)
    {
        case 0:
            return useLeftPoint ? FogLocation::LeftTop : FogLocation::RightTop;
        case 1:
            return useLeftPoint ? FogLocation::LeftMiddle : FogLocation::RightMiddle;
        case 2:
            return useLeftPoint ? FogLocation::LeftBottom : FogLocation::RightBottom;
        default:
            return FogLocation::None;
    }
}

FogLane GetFogLaneFromLocation(FogLocation location)
{
    switch (location)
    {
        case FogLocation::LeftTop:
        case FogLocation::RightTop:
            return FogLane::Top;

        case FogLocation::LeftMiddle:
        case FogLocation::RightMiddle:
            return FogLane::Middle;

        case FogLocation::LeftBottom:
        case FogLocation::RightBottom:
            return FogLane::Bottom;

        default:
            return FogLane::None;
    }
}

bool IsFogSideLocation(FogLocation location)
{
    return location == FogLocation::LeftSide || location == FogLocation::RightSide;
}

bool IsPastFogThreshold(Player* bot, FogLane dangerLane)
{
    if (dangerLane == FogLane::None)
        return false;

    uint8 const laneIndex = static_cast<uint8>(dangerLane);
    if (laneIndex >= FOG_SAFE_THRESHOLDS.size())
        return false;

    FogSafeThreshold const& threshold = FOG_SAFE_THRESHOLDS[laneIndex];
    Position const& a = threshold.a;
    Position const& b = threshold.b;

    float const cross = (b.GetPositionX() - a.GetPositionX()) *
        (bot->GetPositionY() - a.GetPositionY()) -
        (b.GetPositionY() - a.GetPositionY()) *
        (bot->GetPositionX() - a.GetPositionX());

    bool const botIsLeft = cross > 0.0f;
    return botIsLeft == threshold.safeSideIsNorth;
}

FogLocation GetFogLocationFromPosition(float positionX, float positionY, float matchDistance)
{
    float bestDistance = matchDistance;
    FogLocation bestLocation = FogLocation::None;

    float const leftSideDistance = std::hypot(
        positionX - FOG_LEFT_SIDE.GetPositionX(), positionY - FOG_LEFT_SIDE.GetPositionY());

    if (leftSideDistance <= bestDistance)
    {
        bestDistance = leftSideDistance;
        bestLocation = FogLocation::LeftSide;
    }

    float const rightSideDistance = std::hypot(
        positionX - FOG_RIGHT_SIDE.GetPositionX(), positionY - FOG_RIGHT_SIDE.GetPositionY());

    if (rightSideDistance <= bestDistance)
    {
        bestDistance = rightSideDistance;
        bestLocation = FogLocation::RightSide;
    }

    for (uint8 laneIndex = 0; laneIndex < FOG_LEFT_LANES.size(); ++laneIndex)
    {
        float const leftDistance = std::hypot(
            positionX - FOG_LEFT_LANES[laneIndex].GetPositionX(),
            positionY - FOG_LEFT_LANES[laneIndex].GetPositionY());

        if (leftDistance <= bestDistance)
        {
            bestDistance = leftDistance;
            bestLocation = GetFogLocationFromLanePointIndex(laneIndex, true);
        }

        float const rightDistance = std::hypot(
            positionX - FOG_RIGHT_LANES[laneIndex].GetPositionX(),
            positionY - FOG_RIGHT_LANES[laneIndex].GetPositionY());

        if (rightDistance <= bestDistance)
        {
            bestDistance = rightDistance;
            bestLocation = GetFogLocationFromLanePointIndex(laneIndex, false);
        }
    }

    return bestLocation;
}

bool TryGetFlightDestination(Unit* felmyst, Position& destination)
{
    if (!felmyst)
        return false;

    float destinationX = 0.0f;
    float destinationY = 0.0f;
    float destinationZ = 0.0f;

    if (!felmyst->GetMotionMaster()->GetDestination(destinationX, destinationY, destinationZ))
        return false;

    destination = Position{ destinationX, destinationY, destinationZ };
    return true;
}

bool IsSweeping(Unit* felmyst)
{
    return felmyst && felmyst->HasAura(Id(SwpSpells::SPELL_FELMYST_SPEED_BURST));
}

FogLocation GetCurrentFogLocation(Unit* felmyst)
{
    if (!felmyst)
        return FogLocation::None;

    return GetFogLocationFromPosition(
        felmyst->GetPositionX(), felmyst->GetPositionY(), FELMYST_LOCATION_MATCH_DISTANCE);
}

FogLocation GetDestinationFogLocation(Unit* felmyst)
{
    Position destination;
    if (!TryGetFlightDestination(felmyst, destination))
        return FogLocation::None;

    return GetFogLocationFromPosition(
        destination.GetPositionX(), destination.GetPositionY(), FELMYST_LOCATION_MATCH_DISTANCE);
}

bool IsNearLandingPosition(Position const& destination)
{
    bool const nearRight = destination.GetExactDist2d(FELMYST_RIGHT_LANDING_POSITION) <=
        FELMYST_LOCATION_MATCH_DISTANCE;
    bool const nearLeft = destination.GetExactDist2d(FELMYST_LEFT_LANDING_POSITION) <=
        FELMYST_LOCATION_MATCH_DISTANCE;

    return nearRight || nearLeft;
}

FogLane GetNearestDemonicVaporLane(Player* bot)
{
    FogLane bestLane = FogLane::Middle;
    float bestDistance = std::numeric_limits<float>::max();

    for (uint8 laneIndex = 0; laneIndex < DEMONIC_VAPOR_LANE_REFERENCES.size(); ++laneIndex)
    {
        Position const& reference = DEMONIC_VAPOR_LANE_REFERENCES[laneIndex];
        float const distance = bot->GetExactDist2d(reference);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestLane = static_cast<FogLane>(laneIndex);
        }
    }

    return bestLane;
}

uint8 GetDemonicVaporAllowedSides(Player* bot)
{
    float const centerDistance = bot->GetExactDist2d(FELMYST_CENTER_GROUND_REFERENCE);
    float const rightDistance = bot->GetExactDist2d(FELMYST_RIGHT_LANDING_POSITION);
    float const leftDistance = bot->GetExactDist2d(FELMYST_LEFT_LANDING_POSITION);

    if (centerDistance <= rightDistance && centerDistance <= leftDistance)
        return DEMONIC_VAPOR_LEFT_SIDE | DEMONIC_VAPOR_RIGHT_SIDE;

    return rightDistance <= leftDistance ?
        DEMONIC_VAPOR_LEFT_SIDE : DEMONIC_VAPOR_RIGHT_SIDE;
}

uint8 GetDemonicVaporAnchorMask(uint8 anchorIndex)
{
    if (anchorIndex >= DEMONIC_VAPOR_KITE_ANCHORS.size())
        return 0;

    return static_cast<uint8>(1u << anchorIndex);
}

uint8 FlipVaporSide(uint8 sideMask)
{
    return sideMask == DEMONIC_VAPOR_LEFT_SIDE ? DEMONIC_VAPOR_RIGHT_SIDE : DEMONIC_VAPOR_LEFT_SIDE;
}

uint8 SelectPreferredDemonicVaporSide(Player* bot, FogLane lane, uint8 allowedSides)
{
    if (allowedSides == DEMONIC_VAPOR_LEFT_SIDE || allowedSides == DEMONIC_VAPOR_RIGHT_SIDE)
        return allowedSides;

    float bestLeftDistance = std::numeric_limits<float>::max();
    float bestRightDistance = std::numeric_limits<float>::max();

    for (uint8 anchorIndex = 0;
         anchorIndex < DEMONIC_VAPOR_KITE_ANCHORS.size();
         ++anchorIndex)
    {
        DemonicVaporAnchor const& anchor =
            DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex];
        if (anchor.lane != lane)
            continue;

        Position const& anchorPosition = DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex].position;
        float const distance = bot->GetExactDist2d(anchorPosition);

        if (anchor.sideMask == DEMONIC_VAPOR_LEFT_SIDE)
            bestLeftDistance = distance;
        else if (anchor.sideMask == DEMONIC_VAPOR_RIGHT_SIDE)
            bestRightDistance = distance;
    }

    return bestLeftDistance <= bestRightDistance ?
        DEMONIC_VAPOR_LEFT_SIDE : DEMONIC_VAPOR_RIGHT_SIDE;
}

std::array<FogLane, 3> GetDemonicVaporLanePriority(FogLane lane)
{
    switch (lane)
    {
        case FogLane::Top:
            return {{ FogLane::Top, FogLane::Middle, FogLane::Bottom }};
        case FogLane::Bottom:
            return {{ FogLane::Bottom, FogLane::Middle, FogLane::Top }};
        default:
            return {{ FogLane::Middle, FogLane::Top, FogLane::Bottom }};
    }
}

void PushUniqueDemonicVaporAnchor(std::vector<uint8>& anchorIndices, uint8 anchorIndex)
{
    if (std::find(anchorIndices.begin(), anchorIndices.end(), anchorIndex) == anchorIndices.end())
        anchorIndices.push_back(anchorIndex);
}

void AppendDemonicVaporAnchorsForSide(
    std::vector<uint8>& anchorIndices, FogLane preferredLane, uint8 sideMask)
{
    auto const lanePriority = GetDemonicVaporLanePriority(preferredLane);
    for (FogLane lane : lanePriority)
    {
        for (uint8 anchorIndex = 0; anchorIndex < DEMONIC_VAPOR_KITE_ANCHORS.size();
             ++anchorIndex)
        {
            DemonicVaporAnchor const& anchor = DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex];
            if (anchor.sideMask == sideMask && anchor.lane == lane)
                PushUniqueDemonicVaporAnchor(anchorIndices, anchorIndex);
        }
    }
}

float GetMinDistanceToOtherPlayers(Player* bot, float x, float y)
{
    float minDistance = std::numeric_limits<float>::max();
    Map::PlayerList const& players = bot->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        Player* member = it->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != SWP_MAP_ID)
            continue;

        float const distance = std::hypot(x - member->GetPositionX(), y - member->GetPositionY());
        if (distance < minDistance)
            minDistance = distance;
    }

    return minDistance;
}

float GetMinDistanceToHazards(float x, float y, std::vector<Creature*> const& hazards)
{
    float minDistance = std::numeric_limits<float>::max();
    for (Creature* hazard : hazards)
    {
        if (!hazard)
            continue;

        float const distance = std::hypot(x - hazard->GetPositionX(), y - hazard->GetPositionY());
        if (distance < minDistance)
            minDistance = distance;
    }

    return minDistance;
}

bool IsDemonicVaporPathSafe(
    Player* bot, Position const& start, Position const& target,
    std::vector<Creature*> const& hazards)
{
    constexpr float pathStepSize = 2.0f;
    constexpr float playerPathClearance = 7.0f;
    constexpr float hazardPathClearance = 10.0f;
    float const totalDistance = start.GetExactDist2d(target);
    if (totalDistance <= 0.0f)
        return true;

    Map::PlayerList const& players = bot->GetMap()->GetPlayers();
    uint32 const stepCount = static_cast<uint32>(totalDistance / pathStepSize) + 1;
    for (uint32 step = 0; step <= stepCount; ++step)
    {
        float const t =
            std::min(static_cast<float>(step * pathStepSize) / totalDistance, 1.0f);
        float const checkX =
            start.GetPositionX() + (target.GetPositionX() - start.GetPositionX()) * t;
        float const checkY =
            start.GetPositionY() + (target.GetPositionY() - start.GetPositionY()) * t;

        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* member = it->GetSource();
            if (!member || member == bot || !member->IsAlive() || member->GetMapId() != SWP_MAP_ID)
                continue;

            if (std::hypot(checkX - member->GetPositionX(), checkY - member->GetPositionY()) <
                playerPathClearance)
            {
                return false;
            }
        }

        for (Creature* hazard : hazards)
        {
            if (!hazard)
                continue;

            if (std::hypot(checkX - hazard->GetPositionX(), checkY - hazard->GetPositionY()) <
                hazardPathClearance)
            {
                return false;
            }
        }
    }

    return true;
}

bool TryGetDemonicVaporAnchorDestination(
    Player* bot, uint8 anchorIndex, std::vector<Creature*> const& hazards,
    bool requireSafePath, bool requireSafeEndpoint, Position& destination)
{
    if (anchorIndex >= DEMONIC_VAPOR_KITE_ANCHORS.size())
        return false;

    constexpr float minPlayerEndpointClearance = 8.0f;
    constexpr float minHazardEndpointClearance = 12.0f;

    float destinationX = DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex].position.GetPositionX();
    float destinationY = DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex].position.GetPositionY();
    float destinationZ = DEMONIC_VAPOR_KITE_ANCHORS[anchorIndex].position.GetPositionZ();

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            destinationX, destinationY, destinationZ, true))
    {
        return false;
    }

    float const minPlayerDistance = GetMinDistanceToOtherPlayers(bot, destinationX, destinationY);
    float const minHazardDistance = GetMinDistanceToHazards(destinationX, destinationY, hazards);

    if (requireSafeEndpoint &&
        (minPlayerDistance < minPlayerEndpointClearance ||
         minHazardDistance < minHazardEndpointClearance))
    {
        return false;
    }

    Position const candidate(destinationX, destinationY, destinationZ, bot->GetOrientation());
    Position const origin(
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetOrientation());
    if (requireSafePath && !IsDemonicVaporPathSafe(bot, origin, candidate, hazards))
        return false;

    destination = candidate;
    return true;
}

bool TryGetFelmystDemonicVaporStepDestination(
    Player* bot, Position const& anchorDestination, Position& destination)
{
    constexpr float stepDistance = 10.0f;
    float const distanceToAnchor = bot->GetExactDist2d(anchorDestination);
    if (distanceToAnchor <= stepDistance)
    {
        destination = anchorDestination;
        return true;
    }

    float const directionX =
        (anchorDestination.GetPositionX() - bot->GetPositionX()) / distanceToAnchor;
    float const directionY =
        (anchorDestination.GetPositionY() - bot->GetPositionY()) / distanceToAnchor;

    float destinationX = bot->GetPositionX() + directionX * stepDistance;
    float destinationY = bot->GetPositionY() + directionY * stepDistance;
    float destinationZ = bot->GetMapWaterOrGroundLevel(
        destinationX, destinationY, anchorDestination.GetPositionZ());

    if (destinationZ <= INVALID_HEIGHT)
        destinationZ = bot->GetPositionZ();

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            destinationX, destinationY, destinationZ, true))
    {
        return false;
    }

    if (bot->GetExactDist2d(destinationX, destinationY) <= 0.01f)
        return false;

    destination = Position(destinationX, destinationY, destinationZ, bot->GetOrientation());

    return true;
}

Position ClosestPointOnSegment(Position const& p, Position const& segA, Position const& segB)
{
    float const abX = segB.GetPositionX() - segA.GetPositionX();
    float const abY = segB.GetPositionY() - segA.GetPositionY();
    float const lenSq = abX * abX + abY * abY;
    if (lenSq <= 0.0f)
        return segA;

    float const t = std::clamp(
        ((p.GetPositionX() - segA.GetPositionX()) * abX +
         (p.GetPositionY() - segA.GetPositionY()) * abY) / lenSq, 0.0f, 1.0f);

    return Position(
        segA.GetPositionX() + t * abX, segA.GetPositionY() + t * abY, segA.GetPositionZ());
}

} // end anonymous namespace

FelmystDemonicVaporGuids FindFelmystDemonicVaporGuids(Player* bot)
{
    FelmystDemonicVaporGuids guids;

    auto const addGuids = [bot](uint32 entry, GuidVector& out)
    {
        std::list<Creature*> creatures;
        bot->GetCreatureListWithEntryInGrid(creatures, entry, DEMONIC_VAPOR_SEARCH_RADIUS);
        for (Creature* creature : creatures)
        {
            if (creature && creature->IsAlive())
                out.push_back(creature->GetGUID());
        }
    };

    addGuids(Id(SwpNpcs::NPC_DEMONIC_VAPOR), guids.heads);
    addGuids(Id(SwpNpcs::NPC_DEMONIC_VAPOR_TRAIL), guids.trails);
    return guids;
}

std::vector<Creature*> GetDemonicVaporHazards(Player* bot)
{
    FelmystDemonicVaporGuids const& guids = GetCachedDemonicVapors(bot);

    std::vector<Creature*> hazards;
    hazards.reserve(guids.heads.size() + guids.trails.size());

    for (ObjectGuid const& guid : guids.heads)
    {
        Creature* head = GetLivingCreature(bot, guid);
        if (head && head->GetSummonerGUID() != bot->GetGUID())
            hazards.push_back(head);
    }

    for (ObjectGuid const& guid : guids.trails)
    {
        if (Creature* trail = GetLivingCreature(bot, guid))
            hazards.push_back(trail);
    }

    return hazards;
}

namespace
{

bool TryFindSpotPastFogThreshold(
    Player* bot, FogLane lane, Position const& projectFrom, Position& destination)
{
    if (lane == FogLane::None)
        return false;

    uint8 const laneIndex = static_cast<uint8>(lane);
    if (laneIndex >= FOG_SAFE_THRESHOLDS.size())
        return false;

    Position const bestProjection = ClosestPointOnSegment(
        projectFrom, FOG_SAFE_THRESHOLDS[laneIndex].a, FOG_SAFE_THRESHOLDS[laneIndex].b);

    FogSafeThreshold const& threshold = FOG_SAFE_THRESHOLDS[laneIndex];

    float const perpX = threshold.safeSideIsNorth ?
        -(threshold.b.GetPositionY() - threshold.a.GetPositionY()) :
         (threshold.b.GetPositionY() - threshold.a.GetPositionY());
    float const perpY = threshold.safeSideIsNorth ?
        (threshold.b.GetPositionX() - threshold.a.GetPositionX()) :
        -(threshold.b.GetPositionX() - threshold.a.GetPositionX());

    float const segmentLength = std::hypot(perpX, perpY);
    if (segmentLength <= 0.0f)
        return false;

    float const unitX = perpX / segmentLength;
    float const unitY = perpY / segmentLength;

    float const alongX = (threshold.b.GetPositionX() - threshold.a.GetPositionX()) / segmentLength;
    float const alongY = (threshold.b.GetPositionY() - threshold.a.GetPositionY()) / segmentLength;

    auto const clearancePastThreshold = [&threshold, unitX, unitY](float x, float y)
    {
        return (x - threshold.a.GetPositionX()) * unitX +
            (y - threshold.a.GetPositionY()) * unitY;
    };

    std::vector<Creature*> const hazards = GetDemonicVaporHazards(bot);
    auto const isClearOfVapor = [&hazards](float x, float y)
    {
        for (Creature* hazard : hazards)
        {
            if (hazard && hazard->GetExactDist2d(x, y) < FOG_DESTINATION_VAPOR_CLEARANCE)
                return false;
        }

        return true;
    };

    constexpr float minThresholdClearance = 3.0f;
    constexpr float maxClearance = 30.0f;
    constexpr float clearanceStep = 3.0f;
    constexpr float maxLateralOffset = 20.0f;
    constexpr float lateralStep = 5.0f;

    struct FogCandidate
    {
        float x;
        float y;
        float distance;
    };

    std::vector<FogCandidate> candidates;
    float const projectionAlong =
        (bestProjection.GetPositionX() - threshold.a.GetPositionX()) * alongX +
        (bestProjection.GetPositionY() - threshold.a.GetPositionY()) * alongY;

    constexpr uint32 clearanceStepCount =
        static_cast<uint32>((maxClearance - minThresholdClearance) / clearanceStep);
    constexpr uint32 lateralStepCount =
        static_cast<uint32>(2.0f * maxLateralOffset / lateralStep);

    for (uint32 clearanceIndex = 0; clearanceIndex <= clearanceStepCount; ++clearanceIndex)
    {
        float const clearance =
            minThresholdClearance + static_cast<float>(clearanceIndex) * clearanceStep;

        for (uint32 lateralIndex = 0; lateralIndex <= lateralStepCount; ++lateralIndex)
        {
            float const lateral =
                -maxLateralOffset + static_cast<float>(lateralIndex) * lateralStep;

            float const along = projectionAlong + lateral;
            if (along < 0.0f || along > segmentLength)
                continue;

            float const x = bestProjection.GetPositionX() + unitX * clearance + alongX * lateral;
            float const y = bestProjection.GetPositionY() + unitY * clearance + alongY * lateral;
            candidates.push_back({ x, y, projectFrom.GetExactDist2d(x, y) });
        }
    }

    std::sort(candidates.begin(), candidates.end(),
        [](FogCandidate const& first, FogCandidate const& second)
        {
            return first.distance < second.distance;
        });

    for (FogCandidate const& candidate : candidates)
    {
        if (!isClearOfVapor(candidate.x, candidate.y))
            continue;

        float x = candidate.x;
        float y = candidate.y;
        float z = bot->GetMapWaterOrGroundLevel(x, y, bot->GetPositionZ());
        if (z <= INVALID_HEIGHT)
            z = bot->GetPositionZ();

        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), x, y, z, true))
        {
            continue;
        }

        if (clearancePastThreshold(x, y) < minThresholdClearance || !isClearOfVapor(x, y))
            continue;

        destination = Position(x, y, z);
        return true;
    }

    return false;
}

} // end anonymous namespace 2

bool TryGetFelmystFogCrossingDestination(Player* bot, FogLane dangerLane, Position& destination)
{
    Position const projectFrom(
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());

    return TryFindSpotPastFogThreshold(bot, dangerLane, projectFrom, destination);
}

bool TryGetFelmystLandingApproachDestination(
    Player* bot, FogLane lastCompletedLane, Unit* felmyst, Position& destination)
{
    if (!felmyst)
        return false;

    Position const projectFrom(
        felmyst->GetPositionX(), felmyst->GetPositionY(), felmyst->GetPositionZ());

    return TryFindSpotPastFogThreshold(bot, lastCompletedLane, projectFrom, destination);
}

Position const& GetFelmystMainTankGroundPosition(Player* bot)
{
    Position const* bestPosition = &FELMYST_TANK_POSITIONS[0];
    float bestDistance = std::numeric_limits<float>::max();

    for (Position const& position : FELMYST_TANK_POSITIONS)
    {
        float const distance = bot->GetExactDist2d(position);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestPosition = &position;
        }
    }

    return *bestPosition;
}

bool TryGetFelmystGroundStackPosition(
    Player* bot, Unit* felmyst, FelmystGroundStack stack, Position& position)
{
    float destinationX = 0.0f;
    float destinationY = 0.0f;

    if (!TryGetGroundStackCenter(bot, felmyst, stack, destinationX, destinationY))
        return false;

    float destinationZ = bot->GetMapWaterOrGroundLevel(
        destinationX, destinationY, bot->GetPositionZ());

    if (destinationZ <= INVALID_HEIGHT)
        destinationZ = bot->GetPositionZ();

    bot->GetMap()->CheckCollisionAndGetValidCoords(
        bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        destinationX, destinationY, destinationZ, false);

    position = Position{ destinationX, destinationY, destinationZ };
    return true;
}

FelmystGroundStack GetClosestFelmystGroundStack(Player* bot, Unit* felmyst, Unit* unit)
{
    if (!unit || !felmyst)
        return FelmystGroundStack::None;

    FelmystGroundStack bestStack = FelmystGroundStack::None;
    float bestDistance = std::numeric_limits<float>::max();
    for (FelmystGroundStack stack : {
             FelmystGroundStack::Melee, FelmystGroundStack::Left, FelmystGroundStack::Right })
    {
        float stackX = 0.0f;
        float stackY = 0.0f;
        if (!TryGetGroundStackCenter(bot, felmyst, stack, stackX, stackY))
            continue;

        float const stackDistance = unit->GetExactDist2d(stackX, stackY);
        if (stackDistance < bestDistance)
        {
            bestDistance = stackDistance;
            bestStack = stack;
        }
    }

    return bestStack;
}

float GetFelmystFrontAngle(Player* bot, Unit* felmyst)
{
    Position const& defaultTankPosition = GetFelmystMainTankGroundPosition(bot);
    float frontX = defaultTankPosition.GetPositionX();
    float frontY = defaultTankPosition.GetPositionY();

    Player* mainTank = GetGroupMainTank(bot);
    if (mainTank && mainTank->IsAlive() && mainTank->GetMapId() == felmyst->GetMapId())
    {
        frontX = mainTank->GetPositionX();
        frontY = mainTank->GetPositionY();
    }
    else if (Unit* victim = felmyst->GetVictim())
    {
        frontX = victim->GetPositionX();
        frontY = victim->GetPositionY();
    }

    return std::atan2(frontY - felmyst->GetPositionY(), frontX - felmyst->GetPositionX());
}

bool TryGetFelmystRangedPosition(Player* bot, Unit* felmyst, Position& position)
{
    if (!felmyst || bot->GetMapId() != SWP_MAP_ID || !PlayerbotAI::IsRanged(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    bool const botIsHealer = PlayerbotAI::IsHeal(bot);
    ObjectGuid const botGuid = bot->GetGUID();
    uint32 stackIndex = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SWP_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        if (PlayerbotAI::IsHeal(member) == botIsHealer && member->GetGUID() < botGuid)
            ++stackIndex;
    }

    // Healers spread across all three stacks; ranged dps alternates between the two sides
    FelmystGroundStack const stack = botIsHealer ?
        static_cast<FelmystGroundStack>(stackIndex % 3) :
        (stackIndex % 2 == 0 ? FelmystGroundStack::Left : FelmystGroundStack::Right);

    return TryGetFelmystGroundStackPosition(bot, felmyst, stack, position);
}

Creature* GetFelmystDemonicVaporSummonedBy(Player* bot, Player* summoner)
{
    if (!summoner)
        return nullptr;

    for (ObjectGuid const& guid : GetCachedDemonicVapors(bot).heads)
    {
        Creature* head = GetLivingCreature(bot, guid);
        if (head && head->GetSummonerGUID() == summoner->GetGUID() &&
            summoner->GetDistance2d(head) <= DEMONIC_VAPOR_SUMMONER_RADIUS)
        {
            return head;
        }
    }

    return nullptr;
}

bool IsFelmystDemonicVaporHeadNearBot(Player* bot)
{
    constexpr float kiteDistanceThreshold = 15.0f;
    Creature* vapor = GetFelmystDemonicVaporSummonedBy(bot, bot);
    return vapor && bot->GetExactDist2d(vapor) <= kiteDistanceThreshold;
}

bool IsFelmystLanding(Unit* felmyst)
{
    Position destination;
    if (!TryGetFlightDestination(felmyst, destination))
        return false;

    return IsNearLandingPosition(destination);
}

namespace
{

// A pass is recognized by watching Felmyst's flight destination.
FogPassState const* AdvanceFelmystFogPassTracker(Unit* felmyst)
{
    if (!felmyst)
        return nullptr;

    uint32 const instanceId = felmyst->GetInstanceId();
    if (!felmyst->IsFlying() || IsFelmystLanding(felmyst))
    {
        FelmystState(instanceId).fogPass = FogPassState{};
        return nullptr;
    }

    FogPassState& tracker = FelmystState(instanceId).fogPass;

    const FogLocation currentLocation = GetCurrentFogLocation(felmyst);
    const FogLane currentLane = GetFogLaneFromLocation(currentLocation);

    const FogLocation destinationLocation = GetDestinationFogLocation(felmyst);
    const FogLane destinationLane = GetFogLaneFromLocation(destinationLocation);

    const FogLocation previousDestinationLocation = tracker.lastDestinationLocation;
    const FogLane previousDestinationLane = GetFogLaneFromLocation(previousDestinationLocation);

    if (IsSweeping(felmyst))
    {
        FogLane sweepLane = destinationLane;
        if (sweepLane == FogLane::None)
            sweepLane = currentLane;

        tracker.armedSweepLane = sweepLane;
    }

    if (destinationLocation != FogLocation::None &&
        destinationLocation != previousDestinationLocation)
    {
        if (tracker.armedSweepLane != FogLane::None &&
            previousDestinationLane == tracker.armedSweepLane &&
            IsFogSideLocation(destinationLocation))
        {
            ++tracker.completedPassCount;
            tracker.lastCompletedLane = tracker.armedSweepLane;
            tracker.armedSweepLane = FogLane::None;

            constexpr uint32 thirdPassWindowMs = 10000;
            if (tracker.completedPassCount >= 3)
                tracker.thirdPassWindowExpireMs = getMSTime() + thirdPassWindowMs;
        }

        tracker.lastDestinationLocation = destinationLocation;
    }

    return &tracker;
}

bool IsPostThirdPassWindowOpen(FogPassState const& tracker)
{
    return tracker.completedPassCount >= 3 && tracker.lastCompletedLane != FogLane::None &&
        tracker.thirdPassWindowExpireMs > getMSTime();
}

FelmystEncounterState const* GetFelmystAirborneState(Unit* felmyst)
{
    if (!felmyst || !felmyst->IsFlying())
        return nullptr;

    return FindFelmystState(felmyst->GetInstanceId());
}

} // end anonymous namespace

bool TryGetFelmystPostThirdPassWindow(Unit* felmyst, FogLane& lane)
{
    lane = FogLane::None;

    FogPassState const* tracker = AdvanceFelmystFogPassTracker(felmyst);
    if (!tracker || !IsPostThirdPassWindowOpen(*tracker))
        return false;

    lane = tracker->lastCompletedLane;
    return true;
}

bool IsFelmystFogActiveForBot(Player* bot, Unit* felmyst)
{
    FelmystEncounterState const* state = GetFelmystAirborneState(felmyst);
    if (!state)
        return false;

    FogOfCorruptionState const& fog = state->fogOfCorruption;
    if (fog.phase == FogPhase::None || fog.phase == FogPhase::Recovery ||
        fog.lane == FogLane::None)
    {
        return false;
    }

    return !IsPastFogThreshold(bot, fog.lane);
}

bool IsFelmystFogMovementSuppressed(Unit* felmyst)
{
    FelmystEncounterState const* state = GetFelmystAirborneState(felmyst);
    if (!state)
        return false;

    return state->fogOfCorruption.phase != FogPhase::None ||
        IsPostThirdPassWindowOpen(state->fogPass);
}

bool IsFelmystAirPhaseTargetSuppressed(Unit* felmyst)
{
    if (!felmyst || !felmyst->IsFlying())
        return false;

    // HP threshold to preserve melee targeting during the initial airborne pull
    if (felmyst->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return false;

    Position destination;
    if (!TryGetFlightDestination(felmyst, destination))
        return true;

    return !IsNearLandingPosition(destination);
}

void ClearFelmystDemonicVaporKiteState(Player* bot)
{
    uint32 const instanceId = bot->GetInstanceId();
    ObjectGuid const guid = bot->GetGUID();

    if (FelmystEncounterState* state = FindFelmystState(instanceId))
        state->demonicVaporRegionIndices.erase(guid);

    ResetDemonicVaporFlightStateIfGrounded(bot);
}

bool TryGetFelmystDemonicVaporKiteDestination(Player* bot, Position& destination)
{
    uint32 const instanceId = bot->GetInstanceId();
    ObjectGuid const guid = bot->GetGUID();

    ResetDemonicVaporFlightStateIfGrounded(bot);

    if (!IsFelmystDemonicVaporHeadNearBot(bot))
    {
        ClearFelmystDemonicVaporKiteState(bot);
        return false;
    }

    auto& regionIndices = FelmystState(instanceId).demonicVaporRegionIndices;
    uint8& usedRegionMask = FelmystState(instanceId).demonicVaporUsedRegionMask;
    auto const regionItr = regionIndices.find(guid);
    std::vector<uint8> preferredAnchors;

    if (regionItr != regionIndices.end() &&
        regionItr->second < DEMONIC_VAPOR_KITE_ANCHORS.size())
    {
        PushUniqueDemonicVaporAnchor(preferredAnchors, regionItr->second);
        AppendDemonicVaporAnchorsForSide(
            preferredAnchors,
            DEMONIC_VAPOR_KITE_ANCHORS[regionItr->second].lane,
            DEMONIC_VAPOR_KITE_ANCHORS[regionItr->second].sideMask);
    }
    else
    {
        const FogLane preferredLane = GetNearestDemonicVaporLane(bot);
        uint8 const allowedSides = GetDemonicVaporAllowedSides(bot);
        uint8 preferredSide = SelectPreferredDemonicVaporSide(
            bot, preferredLane, allowedSides);

        FelmystEncounterState const* firstRegionState = FindFelmystState(instanceId);
        if (allowedSides == (DEMONIC_VAPOR_LEFT_SIDE | DEMONIC_VAPOR_RIGHT_SIDE) &&
            firstRegionState &&
            firstRegionState->demonicVaporFirstRegionIndex < DEMONIC_VAPOR_KITE_ANCHORS.size())
        {
            uint8 const firstAnchorIndex = firstRegionState->demonicVaporFirstRegionIndex;
            preferredSide = FlipVaporSide(
                DEMONIC_VAPOR_KITE_ANCHORS[firstAnchorIndex].sideMask);
        }

        AppendDemonicVaporAnchorsForSide(preferredAnchors, preferredLane, preferredSide);

        uint8 const alternateSide = FlipVaporSide(preferredSide);
        if (allowedSides & alternateSide)
            AppendDemonicVaporAnchorsForSide(preferredAnchors, preferredLane, alternateSide);
    }

    std::vector<Creature*> const hazards = GetDemonicVaporHazards(bot);
    auto const tryAnchors = [&](bool requireSafePath, bool requireSafeEndpoint)
    {
        for (uint8 anchorIndex : preferredAnchors)
        {
            if (regionItr == regionIndices.end() &&
                (usedRegionMask & GetDemonicVaporAnchorMask(anchorIndex)) != 0)
            {
                continue;
            }

            Position anchorDestination;
            if (!TryGetDemonicVaporAnchorDestination(
                    bot, anchorIndex, hazards, requireSafePath,
                    requireSafeEndpoint, anchorDestination))
            {
                continue;
            }

            if (!TryGetFelmystDemonicVaporStepDestination(bot, anchorDestination, destination))
                continue;

            regionIndices[guid] = anchorIndex;
            usedRegionMask |= GetDemonicVaporAnchorMask(anchorIndex);
            FelmystState(instanceId).demonicVaporFirstRegionIndex = anchorIndex;
            return true;
        }

        return false;
    };

    return tryAnchors(true, true) || tryAnchors(false, true) || tryAnchors(false, false);
}

bool TryGetFelmystFogOfCorruptionStageState(Unit* felmyst, FogOfCorruptionState& state)
{
    state = FogOfCorruptionState();
    uint32 const now = getMSTime();
    constexpr uint32 fogRecoveryGraceMs = 2500;

    if (!felmyst)
        return false;

    uint32 const instanceId = felmyst->GetInstanceId();
    if (!felmyst->IsFlying())
    {
        ResetDemonicVaporFlightState(instanceId);
        FelmystState(instanceId).fogPass = FogPassState{};
        FelmystState(instanceId).fogOfCorruption = FogOfCorruptionState{};
        return false;
    }

    AdvanceFelmystFogPassTracker(felmyst);

    FogOfCorruptionState& tracker = FelmystState(instanceId).fogOfCorruption;
    bool const hasTracker = tracker.phase != FogPhase::None;

    const FogLocation currentLocation = GetCurrentFogLocation(felmyst);
    const FogLocation destinationLocation = GetDestinationFogLocation(felmyst);
    const FogLane currentLane = GetFogLaneFromLocation(currentLocation);

    if (currentLane != FogLane::None)
    {
        constexpr uint32 fogWindupGraceMs = 7000;
        tracker.lane = currentLane;
        tracker.phase = FogPhase::Windup;
        tracker.expireMs = now + fogWindupGraceMs;
        state = tracker;
        return true;
    }

    if (IsSweeping(felmyst))
    {
        if (tracker.lane == FogLane::None)
            return false;

        tracker.phase = FogPhase::Sweep;
        tracker.expireMs = now + fogRecoveryGraceMs;
        state = tracker;
        return true;
    }

    if (hasTracker && tracker.expireMs > now && tracker.lane != FogLane::None &&
        tracker.phase == FogPhase::Windup &&
        !IsFogSideLocation(currentLocation) && !IsFogSideLocation(destinationLocation))
    {
        state = tracker;
        return true;
    }

    if (hasTracker && tracker.expireMs > now && tracker.lane != FogLane::None &&
        (tracker.phase == FogPhase::Sweep || tracker.phase == FogPhase::Recovery ||
         IsFogSideLocation(currentLocation) || IsFogSideLocation(destinationLocation)))
    {
        tracker.phase = FogPhase::Recovery;
        state = tracker;
        return true;
    }

    FelmystState(instanceId).fogOfCorruption = FogOfCorruptionState{};
    return false;
}

bool TryGetActiveFogOfCorruptionState(Player* bot, Unit* felmyst, FogOfCorruptionState& state)
{
    if (!TryGetFelmystFogOfCorruptionStageState(felmyst, state))
        return false;

    if (state.phase == FogPhase::Recovery)
        return false;

    if (IsPastFogThreshold(bot, state.lane))
        return false;

    return state.lane != FogLane::None;
}

void RecordFelmystIncomingEncapsulateTarget(Player* target, uint32 durationMs)
{
    if (!target)
        return;

    uint32 const now = getMSTime();
    IncomingEncapsulateState& state = FelmystState(target->GetInstanceId()).incomingEncapsulate;

    if (state.targetGuid != target->GetGUID())
        state.delayMs = now + ENCAPSULATE_DELAY_MS;

    state.targetGuid = target->GetGUID();
    state.expireMs = now + durationMs;
    state.auraObserved = false;
}

Player* GetFelmystEncapsulateTarget(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    uint32 const now = getMSTime();
    if (FelmystEncounterState* felmystState = FindFelmystState(bot->GetInstanceId()))
    {
        auto& incomingState = felmystState->incomingEncapsulate;
        Player* incomingTarget = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetGUID() != incomingState.targetGuid)
                continue;

            incomingTarget = member;
            break;
        }

        if (incomingTarget && incomingTarget->HasAura(Id(SwpSpells::SPELL_ENCAPSULATE)))
        {
            incomingState.auraObserved = true;
            felmystState->encapsulateOccurredThisGroundPhase = true;
            return incomingTarget;
        }

        if (!incomingTarget || incomingState.auraObserved || incomingState.expireMs <= now)
            felmystState->incomingEncapsulate = IncomingEncapsulateState{};
        else
            return incomingState.delayMs <= now ? incomingTarget : nullptr;
    }

    Player* closestTarget = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !member->HasAura(Id(SwpSpells::SPELL_ENCAPSULATE)))
            continue;

        FelmystState(bot->GetInstanceId()).encapsulateOccurredThisGroundPhase = true;

        float distance = bot->GetExactDist(member);
        if (distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

bool ShouldMoveAwayFromFelmystEncapsulateTarget(
    Player* bot, Unit* felmyst, Player* encapsulateTarget)
{
    if (!felmyst || !encapsulateTarget || encapsulateTarget == bot ||
        felmyst->GetVictim() == bot)
    {
        return false;
    }

    FelmystGroundStack const botStack = GetClosestFelmystGroundStack(bot, felmyst, bot);
    return botStack != FelmystGroundStack::None &&
        botStack == GetClosestFelmystGroundStack(bot, felmyst, encapsulateTarget);
}

bool DidEncapsulateOccurThisGroundPhase(Player* bot)
{
    FelmystEncounterState const* state = FindFelmystState(bot->GetInstanceId());
    return state && state->encapsulateOccurredThisGroundPhase;
}

Player* GetFelmystGasNovaDispelTarget(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* closestTarget = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->HasAura(Id(SwpSpells::SPELL_GAS_NOVA)))
            continue;

        float distance = bot->GetExactDist(member);
        if (distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

Player* GetFelmystCharmedTarget(Player* bot, Unit* felmyst)
{
    if (!felmyst)
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* lowestHpTarget = nullptr;
    uint32 lowestHp = std::numeric_limits<uint32>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != SWP_MAP_ID)
            continue;

        if (!member->HasAura(Id(SwpSpells::SPELL_FOG_OF_CORRUPTION_CHARM)))
            continue;

        if (PlayerbotAI::IsMelee(bot) && !felmyst->IsFlying() && !bot->IsWithinMeleeRange(member))
            continue;

        if (PlayerbotAI::IsRanged(bot) && bot->GetDistance2d(member) > RANGED_ABILITY_REACH)
            continue;

        if (member->GetHealth() < lowestHp)
        {
            lowestHp = member->GetHealth();
            lowestHpTarget = member;
        }
    }

    return lowestHpTarget;
}

// Bots will follow this player during the vapor phase. Return the first eligible assistant, and
// if no eligible assistant is found, return the first eligible bot.
Player* GetFelmystFlightLeader(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
        return nullptr;

    FelmystEncounterState& state = FelmystState(player->GetInstanceId());

    auto const isEligible = [player](Player* member) -> bool
    {
        return member && member->IsAlive() && member->GetMapId() == SWP_MAP_ID &&
            !GetFelmystDemonicVaporSummonedBy(player, member);
    };

    // Keep the then-current flight leader if still eligible to maintain consistency. Notably, if
    // the leader is targeted by vapor and a new leader is assigned, we should not switch back to
    // the original leader after the vapor head is gone, as they will be clear across the map.
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetGUID() == state.flightLeaderGuid)
        {
            if (isEligible(member))
                return member;
            break;
        }
    }

    state.flightLeaderGuid = ObjectGuid::Empty;

    Player* fallbackBot = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!isEligible(member))
            continue;

        if (group->IsAssistant(member->GetGUID()))
        {
            state.flightLeaderGuid = member->GetGUID();
            return member;
        }

        if (!fallbackBot && GET_PLAYERBOT_AI(member))
            fallbackBot = member;
    }

    if (fallbackBot)
        state.flightLeaderGuid = fallbackBot->GetGUID();

    return fallbackBot;
}

// Shared encounter state

FelmystEncounterState& FelmystState(uint32 instanceId)
{
    std::lock_guard lock(felmystStateMutex);
    return felmystStates[instanceId];
}

FelmystEncounterState* FindFelmystState(uint32 instanceId)
{
    std::lock_guard lock(felmystStateMutex);
    auto it = felmystStates.find(instanceId);
    return it != felmystStates.end() ? &it->second : nullptr;
}

bool FelmystResetInstance(uint32 instanceId)
{
    std::lock_guard lock(felmystStateMutex);
    return felmystStates.erase(instanceId) > 0;
}

}
