/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BATTLEGROUNDTACTICSACTION_H
#define _PLAYERBOT_BATTLEGROUNDTACTICSACTION_H

#include "BattlegroundAV.h"
#include "MovementActions.h"

class ChatHandler;
class Battleground;
class PlayerbotAI;
struct Position;

#define SPELL_CAPTURE_BANNER 21651

enum WSBotStrategy : uint8
{
    WS_STRATEGY_BALANCED      = 0,
    WS_STRATEGY_OFFENSIVE     = 1,
    WS_STRATEGY_DEFENSIVE     = 2,
    WS_STRATEGY_MAX           = 3,
};

enum ABBotStrategy : uint8
{
    AB_STRATEGY_BALANCED      = 0,
    AB_STRATEGY_OFFENSIVE     = 1,
    AB_STRATEGY_DEFENSIVE     = 2,
    AB_STRATEGY_MAX           = 3,
};

enum AVBotStrategy : uint8
{
    AV_STRATEGY_BALANCED      = 0,
    AV_STRATEGY_OFFENSIVE     = 1,
    AV_STRATEGY_DEFENSIVE     = 2,
    AV_STRATEGY_MAX           = 3,
};

enum EYBotStrategy : uint8
{
    EY_STRATEGY_BALANCED      = 0,
    EY_STRATEGY_FRONT_FOCUS   = 1,
    EY_STRATEGY_BACK_FOCUS    = 2,
    EY_STRATEGY_FLAG_FOCUS    = 3,
    EY_STRATEGY_MAX           = 4
};

typedef void (*BattleBotWaypointFunc)();

struct BGStrategyData
{
    uint8 allianceStrategy = 0;
    uint8 hordeStrategy = 0;
};

extern std::unordered_map<uint32, BGStrategyData> bgStrategies;

struct BattleBotWaypoint
{
    BattleBotWaypoint(float x_, float y_, float z_, BattleBotWaypointFunc func) : x(x_), y(y_), z(z_), pFunc(func){};

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    BattleBotWaypointFunc pFunc = nullptr;
};

struct AVNodePositionData
{
    Position pos;
    float maxRadius;
};

// Added to fix bot stuck at objectives
static std::unordered_map<uint8, AVNodePositionData> AVNodeMovementTargets = {
    {BG_AV_NODES_FIRSTAID_STATION, {Position(640.364f, -36.535f, 45.625f), 15.0f}},
    {BG_AV_NODES_STORMPIKE_GRAVE, {Position(665.598f, -292.976f, 30.291f), 15.0f}},
    {BG_AV_NODES_STONEHEART_GRAVE, {Position(76.108f, -399.602f, 45.730f), 15.0f}},
    {BG_AV_NODES_SNOWFALL_GRAVE, {Position(-201.298f, -119.661f, 78.291f), 15.0f}},
    {BG_AV_NODES_ICEBLOOD_GRAVE, {Position(-617.858f, -400.654f, 59.692f), 15.0f}},
    {BG_AV_NODES_FROSTWOLF_GRAVE, {Position(-1083.803f, -341.520f, 55.304f), 15.0f}},
    {BG_AV_NODES_FROSTWOLF_HUT, {Position(-1405.678f, -309.108f, 89.377f, 0.392f), 10.0f}},
    {BG_AV_NODES_DUNBALDAR_SOUTH, {Position(556.551f, -77.240f, 51.931f), 0.0f}},
    {BG_AV_NODES_DUNBALDAR_NORTH, {Position(670.664f, -142.031f, 63.666f), 0.0f}},
    {BG_AV_NODES_ICEWING_BUNKER, {Position(200.310f, -361.232f, 56.387f), 0.0f}},
    {BG_AV_NODES_STONEHEART_BUNKER, {Position(-156.302f, -440.032f, 40.403f), 0.0f}},
    {BG_AV_NODES_ICEBLOOD_TOWER, {Position(-569.702f, -265.362f, 75.009f), 0.0f}},
    {BG_AV_NODES_TOWER_POINT, {Position(-767.439f, -360.200f, 90.895f), 0.0f}},
    {BG_AV_NODES_FROSTWOLF_ETOWER, {Position(-1303.737f, -314.070f, 113.868f), 0.0f}},
    {BG_AV_NODES_FROSTWOLF_WTOWER, {Position(-1300.648f, -267.356f, 114.151f), 0.0f}},
};

typedef std::vector<BattleBotWaypoint> BattleBotPath;

extern std::vector<BattleBotPath*> const vPaths_WS;
extern std::vector<BattleBotPath*> const vPaths_AB;
extern std::vector<BattleBotPath*> const vPaths_AV;
extern std::vector<BattleBotPath*> const vPaths_EY;
extern std::vector<BattleBotPath*> const vPaths_IC;

class BGTactics : public MovementAction
{
public:
    static bool HandleConsoleCommand(ChatHandler* handler, char const* args);
    uint8 static GetBotStrategyForTeam(Battleground* bg, TeamId teamId);

    BGTactics(PlayerbotAI* botAI, std::string const name = "bg tactics") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;

private:
    static std::string const HandleConsoleCommandPrivate(WorldSession* session, char const* args);
    bool moveToStart(bool force = false);
    bool selectObjective(bool reset = false);
    bool moveToObjective(bool ignoreDist);
    bool selectObjectiveWp(std::vector<BattleBotPath*> const& vPaths);
    bool moveToObjectiveWp(BattleBotPath* const& currentPath, uint32 currentPoint, bool reverse = false);
    bool startNewPathBegin(std::vector<BattleBotPath*> const& vPaths);
    bool startNewPathFree(std::vector<BattleBotPath*> const& vPaths);
    bool resetObjective();
    bool wsJumpDown();
    bool eyJumpDown();
    bool atFlag(std::vector<BattleBotPath*> const& vPaths, std::vector<uint32> const& vFlagIds);
    bool flagTaken();
    bool teamFlagTaken();
    bool protectFC();
    bool useBuff();
    uint32 getPlayersInArea(TeamId teamId, Position point, float range, bool combat = true);
    bool IsLockedInsideKeep();
};

class ArenaTactics : public MovementAction
{
public:
    ArenaTactics(PlayerbotAI* botAI, std::string const name = "arena tactics") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;

private:
    bool moveToCenter(Battleground* bg);
};

#endif
