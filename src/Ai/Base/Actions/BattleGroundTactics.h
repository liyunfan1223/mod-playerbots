/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BATTLEGROUNDTACTICS_H
#define PLAYERBOTS_BATTLEGROUNDTACTICS_H

#include "AiObjectContext.h"
#include "BattlegroundAV.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "WsgRouteState.h"

class ChatHandler;
class Battleground;
class PlayerbotAI;
struct Position;
class PositionInfo;

#define SPELL_CAPTURE_BANNER 21651

enum WSBotStrategy : uint8
{
    WS_STRATEGY_BALANCED      = 0,
    WS_STRATEGY_OFFENSIVE     = 1,
    WS_STRATEGY_DEFENSIVE     = 2,
    WS_STRATEGY_MAX           = 3,
};

constexpr uint8 WSG_DEFENDER_ROLES_BALANCED = 3;
constexpr uint8 WSG_DEFENDER_ROLES_OFFENSIVE = 2;
constexpr uint8 WSG_DEFENDER_ROLES_DEFENSIVE = 5;
constexpr uint8 WSG_ATTACKER_ROLES_BALANCED = 3;
constexpr uint8 WSG_ATTACKER_ROLES_OFFENSIVE = 5;
constexpr uint8 WSG_ATTACKER_ROLES_DEFENSIVE = 2;

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

struct BGStrategyData
{
    uint8 allianceStrategy = 0;
    uint8 hordeStrategy = 0;
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
    static void SetBotStrategies(uint32 instanceId, BGStrategyData const& data);
    static void ClearBotStrategies(uint32 instanceId);
    static uint8 GetWsgDefenderCount(Battleground* bg, TeamId teamId);
    static bool ShouldYieldWsgTactics(PlayerbotAI* botAI);

    BGTactics(PlayerbotAI* botAI, std::string const name = "bg tactics")
        : MovementAction(botAI, name),
          _wsgState(context->GetValue<WsgRouteState>("wsg route")->RefGet()),
          _wsgPath(_wsgState._wsgPath),
          _wsgApproachPath(_wsgState._wsgApproachPath),
          _wsgMidfieldPath(_wsgState._wsgMidfieldPath),
          _wsgMidfieldGoal(_wsgState._wsgMidfieldGoal),
          _wsgMidfieldReached(_wsgState._wsgMidfieldReached),
          _wsgAvoidPath(_wsgState._wsgAvoidPath),
          _wsgCompletedPath(_wsgState._wsgCompletedPath),
          _wsgCompletedReverse(_wsgState._wsgCompletedReverse),
          _wsgInstanceId(_wsgState._wsgInstanceId),
          _wsgJumpNextPoint(_wsgState._wsgJumpNextPoint),
          _wsgGraveyardDeparture(_wsgState._wsgGraveyardDeparture),
          _wsgNextPoint(_wsgState._wsgNextPoint),
          _wsgApproachMs(_wsgState._wsgApproachMs),
          _wsgAvoidMs(_wsgState._wsgAvoidMs),
          _wsgProgressMs(_wsgState._wsgProgressMs),
          _wsgLastProgressCheckMs(_wsgState._wsgLastProgressCheckMs),
          _wsgGoalX(_wsgState._wsgGoalX),
          _wsgGoalY(_wsgState._wsgGoalY),
          _wsgProgressDistance(_wsgState._wsgProgressDistance),
          _wsgStalledSamples(_wsgState._wsgStalledSamples),
          _wsgReverse(_wsgState._wsgReverse),
          _wsgApproachReverse(_wsgState._wsgApproachReverse),
          _wsgCarrying(_wsgState._wsgCarrying)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    bool ExecuteWsgRoute();
    bool CanStartWsgRouteFallback();
    bool RefreshWsgObjective();
    bool protectFC();
    bool ClearObsoleteWsgFollow();
    void SuspendWsgRouteProgress();
    void ClearWsgRoute();

private:
    enum class WsgJumpResult
    {
        NotApplicable,
        Blocked,
        Moved
    };

    static std::string const HandleConsoleCommandPrivate(WorldSession* session, char const* args);
    bool moveToStart(bool force = false);
    bool selectObjective(bool reset = false);

protected:
    bool moveToObjective(bool ignoreDist);
    bool selectObjectiveWp(std::vector<BattleBotPath*> const& vPaths);

private:
    bool moveToObjectiveWp(BattleBotPath* const& currentPath, uint32 currentPoint, bool reverse = false);
    bool MoveToWsgWaypoint();
    void ValidateWsgRoute();
    void UpdateWsgRouteLane();
    PositionInfo GetWsgRoutingGoal(PositionInfo goal) const;
    void ClearWsgLane();
    void CheckWsgRouteProgress();
    void ClearWsgJump();
    bool IsUnsafeWsgCliffApproach(float x, float y, float z) const;
    bool startNewPathBegin(std::vector<BattleBotPath*> const& vPaths);
    bool startNewPathFree(std::vector<BattleBotPath*> const& vPaths);
    bool resetObjective();
    WsgJumpResult WsJumpDown();
    bool eyJumpDown();
    bool atFlag(std::vector<BattleBotPath*> const& vPaths, std::vector<uint32> const& vFlagIds);
    bool flagTaken();
    bool teamFlagTaken();
    bool useBuff();
    uint32 getPlayersInArea(TeamId teamId, Position point, float range, bool combat = true);
    bool IsLockedInsideKeep();

protected:
    WsgRouteState& _wsgState;
    BattleBotPath*& _wsgPath;
    BattleBotPath*& _wsgApproachPath;
    BattleBotPath*& _wsgMidfieldPath;
    Position& _wsgMidfieldGoal;
    bool& _wsgMidfieldReached;
    BattleBotPath*& _wsgAvoidPath;
    BattleBotPath*& _wsgCompletedPath;
    bool& _wsgCompletedReverse;
    uint32& _wsgInstanceId;
    uint32& _wsgJumpNextPoint;
    bool& _wsgGraveyardDeparture;
    uint32& _wsgNextPoint;
    uint32& _wsgApproachMs;
    uint32& _wsgAvoidMs;
    uint32& _wsgProgressMs;
    uint32& _wsgLastProgressCheckMs;
    float& _wsgGoalX;
    float& _wsgGoalY;
    float& _wsgProgressDistance;
    uint8& _wsgStalledSamples;
    bool& _wsgReverse;
    bool& _wsgApproachReverse;
    bool& _wsgCarrying;
};

class WsgObjectiveRefreshAction : public BGTactics
{
public:
    WsgObjectiveRefreshAction(PlayerbotAI* botAI) : BGTactics(botAI, "refresh wsg objective") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class WsgEscortFollowAction : public BGTactics
{
public:
    WsgEscortFollowAction(PlayerbotAI* botAI) : BGTactics(botAI, "protect fc") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    uint8 _escortIndex = 0;
    bool _hasEscortIndex = false;
};

class WsgObjectiveRouteAction : public BGTactics
{
public:
    WsgObjectiveRouteAction(PlayerbotAI* botAI, std::string const name = "bg wsg route") : BGTactics(botAI, name) {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class WsgObjectiveApproachAction : public WsgObjectiveRouteAction
{
public:
    WsgObjectiveApproachAction(PlayerbotAI* botAI) : WsgObjectiveRouteAction(botAI, "bg wsg approach") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class WsgObjectiveCorridorAction : public WsgObjectiveRouteAction
{
public:
    WsgObjectiveCorridorAction(PlayerbotAI* botAI) : WsgObjectiveRouteAction(botAI, "bg wsg corridor") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class WsgObjectiveDirectAction : public WsgObjectiveRouteAction
{
public:
    WsgObjectiveDirectAction(PlayerbotAI* botAI) : WsgObjectiveRouteAction(botAI, "bg wsg direct") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class WsgSupportAttackAction : public AttackAction
{
public:
    WsgSupportAttackAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack wsg support") {}
    Unit* GetTarget() override;
    bool isUseful() override;
};

class WsgRegroupAction : public DropTargetAction
{
public:
    WsgRegroupAction(PlayerbotAI* botAI) : DropTargetAction(botAI) {}
    std::string const getName() override { return "wsg regroup"; }
    bool isUseful() override;
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
