/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 * Ported/adapted from VincentVanclef/mod-playerbots, revision d7cc409c2e145bc4fa5b64bfc9539dab4cf11a43.
 */

#include "RTGBattlegroundObjectiveBrain.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

#include "Battleground.h"
#include "BattlegroundEY.h"
#include "BattlegroundWS.h"
#include "GameObject.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "Timer.h"

namespace
{
struct TowerObjective
{
    uint32 trigger;
    Position position;
};

std::array<TowerObjective, EY_POINTS_MAX> const TowerObjectives = {
    {{AT_FEL_REAVER_POINT, Position(2044.173f, 1727.503f, 1189.505f)},
     {AT_BLOOD_ELF_POINT, Position(2048.277f, 1395.093f, 1194.255f)},
     {AT_DRAENEI_RUINS_POINT, Position(2286.245f, 1404.683f, 1196.991f)},
     {AT_MAGE_TOWER_POINT, Position(2284.720f, 1728.457f, 1189.153f)}}};

BattlegroundTypeId RealType(Battleground* bg)
{
    return bg->GetBgTypeID() == BATTLEGROUND_RB ? bg->GetBgTypeID(true) : bg->GetBgTypeID();
}

PositionInfo Destination(Position const& position, uint32 mapId)
{
    return PositionInfo(position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(), mapId);
}

PositionInfo FlagRoom(TeamId team, uint32 mapId)
{
    uint32 const trigger = team == TEAM_ALLIANCE ? BG_WS_TRIGGER_ALLIANCE_FLAG_SPAWN : BG_WS_TRIGGER_HORDE_FLAG_SPAWN;
    if (AreaTrigger const* area = sObjectMgr->GetAreaTrigger(trigger))
        return PositionInfo(area->x, area->y, area->z, area->map);

    return team == TEAM_ALLIANCE ? PositionInfo(1539.219f, 1481.747f, 352.458f, mapId)
                                 : PositionInfo(915.958f, 1433.925f, 346.193f, mapId);
}

PositionInfo TowerPosition(uint32 node, uint32 mapId, bool turnIn)
{
    if (turnIn)
        if (AreaTrigger const* area = sObjectMgr->GetAreaTrigger(TowerObjectives[node].trigger))
            return PositionInfo(area->x, area->y, area->z, area->map);

    return Destination(TowerObjectives[node].position, mapId);
}

bool IsCarrier(Player* bot, Battleground* bg)
{
    if (RealType(bg) == BATTLEGROUND_WS)
        return bg->GetFlagPickerGUID(bg->GetOtherTeamId(bot->GetBgTeamId())) == bot->GetGUID() ||
               bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG);

    return bg->GetFlagPickerGUID() == bot->GetGUID() || bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL);
}

bool UsableTarget(Player* bot, Unit* target)
{
    return target && target != bot && target->IsAlive() && target->GetMap() == bot->GetMap() &&
           bot->CanSeeOrDetect(target, false, false, true);
}

BgObjectiveAssignment UnitObjective(BgObjectiveRole role, Unit* target, bool emergency = false)
{
    BgObjectiveAssignment result;
    result.role = role;
    result.targetGuid = target->GetGUID();
    result.destination = Destination(target->GetPosition(), target->GetMapId());
    result.emergency = emergency;
    return result;
}

uint32 State(Player* bot, Battleground* bg, TeamId team)
{
    uint32 state = IsCarrier(bot, bg) ? 1u : 0u;
    if (RealType(bg) == BATTLEGROUND_WS)
    {
        BattlegroundWS* ws = static_cast<BattlegroundWS*>(bg);
        return state | (uint32(ws->GetFlagState(team)) << 1) |
               (uint32(ws->GetFlagState(bg->GetOtherTeamId(team))) << 4);
    }

    BattlegroundEY* eye = static_cast<BattlegroundEY*>(bg);
    for (uint32 node = 0; node < EY_POINTS_MAX; ++node)
        state |= uint32(eye->GetCapturePointInfo(node)._ownerTeamId) << (1 + 2 * node);
    return state | (uint32(eye->GetFlagState()) << 10);
}
}  // namespace

bool RTGBattlegroundObjectiveBrain::IsEnabled(Player* bot)
{
    if (!sPlayerbotAIConfig.BgObjectiveBrain || !bot || !bot->IsAlive() || !bot->InBattleground() || bot->InArena())
        return false;

    Battleground* bg = bot->GetBattleground();
    return bg && bg->GetStatus() == STATUS_IN_PROGRESS &&
           (RealType(bg) == BATTLEGROUND_WS || RealType(bg) == BATTLEGROUND_EY);
}

void RTGBattlegroundObjectiveBrain::Reset()
{
    _assignment = {};
    _instanceId = 0;
    _mapId = 0;
    _team = TEAM_NEUTRAL;
    _lastCheck = 0;
}

void RTGBattlegroundObjectiveBrain::SampleTeam(Player* bot, Battleground* bg, TeamId team)
{
    _teamSize = 0;
    _teamSlot = 0;
    for (auto const& [guid, participant] : bg->GetPlayers())
    {
        if (!participant || participant->GetBgTeamId() != team)
            continue;
        ++_teamSize;
        if (participant->IsAlive() && GET_PLAYERBOT_AI(participant) && guid < bot->GetGUID())
            ++_teamSlot;
    }
    _teamSize = std::max(1u, _teamSize);
}

BgObjectiveAssignment RTGBattlegroundObjectiveBrain::SelectWsg(PlayerbotAI* botAI, BattlegroundWS* bg, TeamId team)
{
    Player* bot = botAI->GetBot();
    TeamId const enemyTeam = bg->GetOtherTeamId(team);
    BgObjectiveAssignment result;
    if (IsCarrier(bot, bg))
    {
        bool const ownFlagHome = bg->GetFlagState(team) == BG_WS_FLAG_STATE_ON_BASE;
        result.role = ownFlagHome ? BgObjectiveRole::CarrierTurnIn : BgObjectiveRole::CarrierHold;
        result.destination = FlagRoom(team, bg->GetMapId());
        if (!ownFlagHome)
            result.destination = team == TEAM_ALLIANCE ? PositionInfo(1540.286f, 1476.026f, 352.692f, bg->GetMapId())
                                                       : PositionInfo(926.142f, 1460.641f, 346.116f, bg->GetMapId());
        result.emergency = true;
        return result;
    }

    if (bg->GetFlagState(team) == BG_WS_FLAG_STATE_ON_GROUND)
        if (GameObject* flag = botAI->GetGameObject(bg->GetDroppedFlagGUID(team));
            flag && flag->isSpawned() && bot->GetDistance(flag) < 85.0f)
        {
            result.role = BgObjectiveRole::ReturnFlag;
            result.targetGuid = flag->GetGUID();
            result.destination = Destination(flag->GetPosition(), bg->GetMapId());
            result.emergency = true;
            return result;
        }

    Unit* enemyCarrier = botAI->GetAiObjectContext()->GetValue<Unit*>("enemy flag carrier")->Get();
    Unit* friendlyCarrier = botAI->GetAiObjectContext()->GetValue<Unit*>("team flag carrier")->Get();
    bool const hunter = _teamSlot % 5 == 2 || _teamSlot % 5 == 3;
    if (UsableTarget(bot, enemyCarrier) && (hunter || bot->GetDistance(enemyCarrier) < 70.0f))
        return UnitObjective(BgObjectiveRole::AttackCarrier, enemyCarrier, true);

    if (_teamSize > 2 && _teamSlot == 1 && UsableTarget(bot, friendlyCarrier))
        return UnitObjective(BgObjectiveRole::EscortCarrier, friendlyCarrier);

    if (_teamSize >= 5 && _teamSlot % 10 == 4 && bg->GetFlagState(team) == BG_WS_FLAG_STATE_ON_BASE)
    {
        result.role = BgObjectiveRole::DefendFlag;
        result.destination = FlagRoom(team, bg->GetMapId());
        return result;
    }

    result.role = BgObjectiveRole::CaptureFlag;
    result.destination = FlagRoom(enemyTeam, bg->GetMapId());
    if (bg->GetFlagState(enemyTeam) == BG_WS_FLAG_STATE_ON_GROUND)
        if (GameObject* flag = botAI->GetGameObject(bg->GetDroppedFlagGUID(enemyTeam)); flag && flag->isSpawned())
            result.destination = Destination(flag->GetPosition(), bg->GetMapId());

    if (bg->GetFlagState(enemyTeam) == BG_WS_FLAG_STATE_ON_PLAYER)
    {
        result.role = BgObjectiveRole::Pressure;
        result.destination = team == TEAM_ALLIANCE ? PositionInfo(1016.42f, 1402.33f, 341.352f, bg->GetMapId())
                                                   : PositionInfo(1443.55f, 1533.40f, 343.148f, bg->GetMapId());
    }

    if (bg->GetFlagState(team) == BG_WS_FLAG_STATE_ON_PLAYER && !UsableTarget(bot, enemyCarrier) && hunter)
    {
        result.role = BgObjectiveRole::Pressure;
        result.destination = team == TEAM_ALLIANCE ? PositionInfo(1016.42f, 1402.33f, 341.352f, bg->GetMapId())
                                                   : PositionInfo(1443.55f, 1533.40f, 343.148f, bg->GetMapId());
    }
    return result;
}

BgObjectiveAssignment RTGBattlegroundObjectiveBrain::SelectEots(PlayerbotAI* botAI, BattlegroundEY* bg, TeamId team)
{
    Player* bot = botAI->GetBot();
    BgObjectiveAssignment result;
    std::array<uint32, EY_POINTS_MAX> pressure = {};
    // Sample hostile pressure once per assignment refresh, not once per tower or AI tick.
    for (auto const& reference : bg->GetBgMap()->GetPlayers())
    {
        Player* player = reference.GetSource();
        if (!player || !player->IsAlive() || player->GetBgTeamId() == team)
            continue;
        for (uint32 node = 0; node < EY_POINTS_MAX; ++node)
            if (player->GetExactDist2d(TowerObjectives[node].position) < 72.0f)
                ++pressure[node];
    }

    uint32 owned = 0;
    uint32 bestOwned = EY_POINTS_MAX;
    float bestDistance = std::numeric_limits<float>::max();
    for (uint32 node = 0; node < EY_POINTS_MAX; ++node)
        if (bg->GetCapturePointInfo(node).IsUnderControl(team))
        {
            ++owned;
            PositionInfo const position = TowerPosition(node, bg->GetMapId(), true);
            float const distance = bot->GetExactDist(position.x, position.y, position.z);
            if (distance < bestDistance)
            {
                bestDistance = distance;
                bestOwned = node;
            }
        }

    bool const carrier = IsCarrier(bot, bg);
    if (carrier && bestOwned < EY_POINTS_MAX)
    {
        result.role = BgObjectiveRole::CarrierTurnIn;
        result.node = bestOwned;
        result.destination = TowerPosition(bestOwned, bg->GetMapId(), true);
        result.emergency = true;
        return result;
    }

    Unit* enemyCarrier = botAI->GetAiObjectContext()->GetValue<Unit*>("enemy flag carrier")->Get();
    if (!carrier && UsableTarget(bot, enemyCarrier) && bot->GetDistance(enemyCarrier) < 115.0f)
        return UnitObjective(BgObjectiveRole::AttackCarrier, enemyCarrier, true);

    uint32 const firstHome = team == TEAM_HORDE ? POINT_FEL_REAVER : POINT_DRAENEI_RUINS;
    uint32 const secondHome = team == TEAM_HORDE ? POINT_BLOOD_ELF : POINT_MAGE_TOWER;
    uint32 const homeOwned = uint32(bg->GetCapturePointInfo(firstHome).IsUnderControl(team)) +
                             uint32(bg->GetCapturePointInfo(secondHome).IsUnderControl(team));
    GameObject* flag = botAI->GetGameObject(bg->GetDroppedFlagGUID());
    if (!flag || !flag->isSpawned())
        flag = bg->GetBGObject(BG_EY_OBJECT_FLAG_NETHERSTORM);
    uint32 const runnerCount = std::max(1u, _teamSize / 5);
    if (!carrier && owned > 0 && homeOwned > 0 && _teamSlot < runnerCount && flag && flag->isSpawned())
    {
        result.role = BgObjectiveRole::CaptureFlag;
        result.targetGuid = flag->GetGUID();
        result.destination = Destination(flag->GetPosition(), bg->GetMapId());
        return result;
    }

    Unit* friendlyCarrier = botAI->GetAiObjectContext()->GetValue<Unit*>("team flag carrier")->Get();
    Unit* enemy = botAI->GetAiObjectContext()->GetValue<Unit*>("enemy player target")->Get();
    if (!carrier && owned >= 2 && _teamSize >= 5 && _teamSlot == runnerCount && UsableTarget(bot, friendlyCarrier) &&
        UsableTarget(bot, enemy) && enemy->GetDistance(friendlyCarrier) < 45.0f)
        return UnitObjective(BgObjectiveRole::EscortCarrier, friendlyCarrier, true);

    if (!carrier && owned >= 2 && _teamSlot % 5 == 4)
        for (uint32 offset = 0; offset < EY_POINTS_MAX; ++offset)
        {
            uint32 const node = (_teamSlot + offset) % EY_POINTS_MAX;
            if (bg->GetCapturePointInfo(node).IsUnderControl(team) && pressure[node] > 0)
            {
                result.role = BgObjectiveRole::DefendTower;
                result.node = node;
                result.destination = TowerPosition(node, bg->GetMapId(), false);
                return result;
            }
        }

    uint32 chosen = EY_POINTS_MAX;
    float bestScore = std::numeric_limits<float>::max();
    uint32 const preferredHome = _teamSlot % 2 == 0 ? firstHome : secondHome;
    bool const awayPressure = homeOwned == 1 && _teamSize >= 5 && _teamSlot % 5 == 3;
    for (uint32 offset = 0; offset < EY_POINTS_MAX; ++offset)
    {
        uint32 const node = (_teamSlot + offset) % EY_POINTS_MAX;
        if (bg->GetCapturePointInfo(node).IsUnderControl(team))
            continue;
        bool const home = node == firstHome || node == secondHome;
        float score = bot->GetExactDist2d(TowerObjectives[node].position) + float(pressure[node]) * 18.0f;
        if (homeOwned < 2 && home && !awayPressure)
            score -= 250.0f;
        if (awayPressure && !home)
            score -= 250.0f;
        if (node == preferredHome)
            score -= 90.0f;
        if (node == (_teamSlot % EY_POINTS_MAX))
            score -= 70.0f;
        if (score < bestScore)
        {
            bestScore = score;
            chosen = node;
        }
    }
    if (chosen < EY_POINTS_MAX)
    {
        result.role = BgObjectiveRole::CaptureTower;
        result.node = chosen;
        result.destination = TowerPosition(chosen, bg->GetMapId(), false);
        result.emergency = carrier;
        return result;
    }

    result.role = BgObjectiveRole::Pressure;
    result.destination = PositionInfo(2175.0f, 1569.0f, 1159.0f, bg->GetMapId());
    return result;
}

bool RTGBattlegroundObjectiveBrain::IsValid(PlayerbotAI* botAI, Battleground* bg, TeamId team) const
{
    if (!_assignment.destination.valueSet)
        return false;
    if (_assignment.role == BgObjectiveRole::AttackCarrier || _assignment.role == BgObjectiveRole::EscortCarrier)
    {
        Unit* target = botAI->GetUnit(_assignment.targetGuid);
        if (!UsableTarget(botAI->GetBot(), target))
            return false;
        if (RealType(bg) == BATTLEGROUND_EY)
            return bg->GetFlagPickerGUID() == _assignment.targetGuid;
        TeamId const flagTeam = _assignment.role == BgObjectiveRole::AttackCarrier ? team : bg->GetOtherTeamId(team);
        return bg->GetFlagPickerGUID(flagTeam) == _assignment.targetGuid;
    }
    if (_assignment.role == BgObjectiveRole::ReturnFlag)
    {
        GameObject* flag = botAI->GetGameObject(_assignment.targetGuid);
        return static_cast<BattlegroundWS*>(bg)->GetFlagState(team) == BG_WS_FLAG_STATE_ON_GROUND && flag &&
               flag->isSpawned();
    }
    if (_assignment.role == BgObjectiveRole::CaptureTower)
        return !static_cast<BattlegroundEY*>(bg)->GetCapturePointInfo(_assignment.node).IsUnderControl(team);
    if (_assignment.role == BgObjectiveRole::DefendTower)
        return static_cast<BattlegroundEY*>(bg)->GetCapturePointInfo(_assignment.node).IsUnderControl(team);
    if (_assignment.role == BgObjectiveRole::CaptureFlag && !_assignment.targetGuid.IsEmpty())
    {
        GameObject* flag = botAI->GetGameObject(_assignment.targetGuid);
        return flag && flag->isSpawned();
    }
    return true;
}

bool RTGBattlegroundObjectiveBrain::Update(PlayerbotAI* botAI, PositionInfo& destination)
{
    Player* bot = botAI->GetBot();
    if (!IsEnabled(bot))
    {
        Reset();
        return false;
    }
    Battleground* bg = bot->GetBattleground();
    TeamId const team = bot->GetBgTeamId();
    if (team != TEAM_ALLIANCE && team != TEAM_HORDE)
        return false;
    bool const sameInstance = _instanceId == bg->GetInstanceID() && _mapId == bot->GetMapId() && _team == team;
    if (!sameInstance)
        Reset();

    uint32 const now = getMSTime();
    uint32 const state = State(bot, bg, team);
    bool const stateChanged = !sameInstance || state != _state;
    uint32 const refresh = std::clamp(sPlayerbotAIConfig.BgObjectiveRecheckMs, 1000u, 10000u);
    if (!stateChanged && _assignment.destination.valueSet && getMSTimeDiff(_lastCheck, now) < refresh)
    {
        destination = _assignment.destination;
        return true;
    }
    SampleTeam(bot, bg, team);
    BgObjectiveAssignment candidate = RealType(bg) == BATTLEGROUND_WS
                                          ? SelectWsg(botAI, static_cast<BattlegroundWS*>(bg), team)
                                          : SelectEots(botAI, static_cast<BattlegroundEY*>(bg), team);
    if (!candidate.destination.valueSet)
        return false;

    uint32 const commit =
        RealType(bg) == BATTLEGROUND_WS ? sPlayerbotAIConfig.BgWsgCommitMs : sPlayerbotAIConfig.BgEotsCommitMs;
    bool const sameObjective = candidate.role == _assignment.role && candidate.node == _assignment.node &&
                               candidate.targetGuid == _assignment.targetGuid;
    if (!sameObjective && !stateChanged && !candidate.emergency && IsValid(botAI, bg, team) &&
        getMSTimeDiff(_assignment.assignedAt, now) < commit)
        candidate = _assignment;
    else
        candidate.assignedAt = sameObjective ? _assignment.assignedAt : now;

    _assignment = candidate;
    _lastCheck = now;
    _instanceId = bg->GetInstanceID();
    _mapId = bot->GetMapId();
    _team = team;
    _state = state;
    destination = _assignment.destination;
    if (sPlayerbotAIConfig.BgDebugObjectiveBrain && !sameObjective)
        LOG_DEBUG("playerbots", "BG objective bot={} instance={} role={} node={} target={}", bot->GetName(),
                  _instanceId, uint32(_assignment.role), _assignment.node, _assignment.targetGuid.ToString());
    return true;
}

bool RTGBattlegroundObjectiveBrain::ShouldHold(Player* bot) const
{
    if (_assignment.role != BgObjectiveRole::CaptureTower && _assignment.role != BgObjectiveRole::DefendTower &&
        _assignment.role != BgObjectiveRole::DefendFlag && _assignment.role != BgObjectiveRole::CarrierHold)
        return false;
    PositionInfo const& position = _assignment.destination;
    return position.valueSet && bot->GetExactDist2d(position.x, position.y) < 7.5f &&
           std::fabs(bot->GetPositionZ() - position.z) < 8.0f;
}
