/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WSGROUTESTATE_H
#define PLAYERBOTS_WSGROUTESTATE_H

#include <vector>

#include "Position.h"
#include "Value.h"

class PlayerbotAI;

typedef void (*BattleBotWaypointFunc)();

struct BattleBotWaypoint
{
    BattleBotWaypoint(float x_, float y_, float z_, BattleBotWaypointFunc func) : x(x_), y(y_), z(z_), pFunc(func) {};

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    BattleBotWaypointFunc pFunc = nullptr;
};

typedef std::vector<BattleBotWaypoint> BattleBotPath;

struct WsgRouteState
{
    BattleBotPath* _wsgPath = nullptr;
    BattleBotPath* _wsgApproachPath = nullptr;
    BattleBotPath* _wsgMidfieldPath = nullptr;
    Position _wsgMidfieldGoal;
    bool _wsgMidfieldReached = false;
    BattleBotPath* _wsgAvoidPath = nullptr;
    BattleBotPath* _wsgCompletedPath = nullptr;
    bool _wsgCompletedReverse = false;
    uint32 _wsgInstanceId = 0;
    uint32 _wsgJumpNextPoint = 0;
    bool _wsgGraveyardDeparture = false;
    uint32 _wsgNextPoint = 0;
    uint32 _wsgApproachMs = 0;
    uint32 _wsgAvoidMs = 0;
    uint32 _wsgProgressMs = 0;
    uint32 _wsgLastProgressCheckMs = 0;
    float _wsgGoalX = 0.0f;
    float _wsgGoalY = 0.0f;
    float _wsgProgressDistance = 0.0f;
    uint8 _wsgStalledSamples = 0;
    bool _wsgReverse = false;
    bool _wsgApproachReverse = false;
    bool _wsgCarrying = false;
    bool _wsgRouteBlocked = false;
};

class WsgRouteStateValue : public ManualSetValue<WsgRouteState>
{
public:
    WsgRouteStateValue(PlayerbotAI* botAI) : ManualSetValue<WsgRouteState>(botAI, {}, "wsg route") {}
};

#endif
