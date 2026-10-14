/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FLEEMANAGER_H
#define PLAYERBOTS_FLEEMANAGER_H

#include "Common.h"
#include "TravelMgr.h"
#include <vector>

class Player;
class PlayerbotAI;

class FleePoint
{
public:
    FleePoint(float x, float y, float z) : x(x), y(y), z(z), sumDistance(0.0f), minDistance(0.0f)
    {
    }

    float x;
    float y;
    float z;

    float sumDistance;
    float minDistance;
};

class FleeManager
{
public:
    FleeManager(Player* bot, float maxAllowedDistance, float followAngle, bool forceMaxDistance = false,
                WorldPosition startPosition = WorldPosition());

    bool CalculateDestination(float* rx, float* ry, float* rz);
    bool isUseful();

private:
    void calculatePossibleDestinations(std::vector<FleePoint*>& points);
    void calculateDistanceToCreatures(FleePoint* point);
    void cleanup(std::vector<FleePoint*>& points);
    FleePoint* selectOptimalDestination(std::vector<FleePoint*>& points);
    bool isBetterThan(FleePoint* point, FleePoint* other);

    Player* bot;
    float maxAllowedDistance;
    bool forceMaxDistance;
    WorldPosition startPosition;
};

#endif
