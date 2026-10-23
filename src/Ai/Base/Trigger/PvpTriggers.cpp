/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpTriggers.h"

#include "BattlegroundEY.h"
#include "BattlegroundMgr.h"
#include "BattlegroundWS.h"
#include "GameObject.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "PvpValues.h"
#include "ServerFacade.h"
#include "Timer.h"

bool WsgSupportThreat::IsActive()
{
    Battleground* bg = bot->GetBattleground();
    return bg && bg->GetBgTypeID(true) == BATTLEGROUND_WS && bg->GetStatus() == STATUS_IN_PROGRESS && bot->IsAlive() &&
           !context->GetValue<ObjectGuid>("wsg support target")->Get().IsEmpty();
}

namespace
{
// a game from the random queue records BATTLEGROUND_RB as the player's type: use the rolled map
BattlegroundTypeId RealBgType(Player* bot)
{
    BattlegroundTypeId bgType = bot->GetBattlegroundTypeId();
    if (bgType == BATTLEGROUND_RB && bot->GetBattleground())
        bgType = bot->GetBattleground()->GetBgTypeID(true);
    return bgType;
}
}  // namespace

bool EnemyPlayerNear::IsActive() { return AI_VALUE(Unit*, "enemy player target"); }

bool PlayerHasNoFlag::IsActive()
{
    Battleground* bg = bot->GetBattleground();
    if (bg && bg->GetBgTypeID(true) == BATTLEGROUND_WS)
        return static_cast<BattlegroundWS*>(bg)->GetFlagPickerGUID(bg->GetOtherTeamId(bot->GetTeamId())) !=
               bot->GetGUID();

    return false;
}

bool PlayerIsInBattleground::IsActive() { return botAI->GetBot()->InBattleground(); }

bool BgWaitingTrigger::IsActive()
{
    if (bot->InBattleground())
    {
        if (bot->GetBattleground() && bot->GetBattleground()->GetStatus() == STATUS_WAIT_JOIN)
            return true;
    }

    return false;
}

bool BgActiveTrigger::IsActive()
{
    if (bot->InBattleground())
    {
        if (bot->GetBattleground() && bot->GetBattleground()->GetStatus() == STATUS_IN_PROGRESS)
            return true;
    }

    return false;
}

bool BgInviteActiveTrigger::IsActive()
{
    if (bot->InBattleground() || !bot->InBattlegroundQueue())
    {
        return false;
    }

    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
    {
        BattlegroundQueueTypeId queueTypeId = bot->GetBattlegroundQueueTypeId(i);
        if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
            continue;

        BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);

        GroupQueueInfo ginfo;
        if (bgQueue.GetPlayerGroupInfoData(bot->GetGUID(), &ginfo))
        {
            if (ginfo.IsInvitedToBGInstanceGUID && ginfo.RemoveInviteTime)
            {
                LOG_INFO("playerbots", "Bot {} <{}> ({} {}) : Invited to BG but not in BG",
                         bot->GetGUID().ToString().c_str(), bot->GetName(), bot->GetLevel(),
                         bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H");
                return true;
            }
        }
    }

    return false;
}

bool InsideBGTrigger::IsActive() { return bot->InBattleground() && bot->GetBattleground(); }

bool PlayerIsInBattlegroundWithoutFlag::IsActive()
{
    Battleground* bg = bot->GetBattleground();
    if (bg && bg->GetBgTypeID(true) == BATTLEGROUND_WS)
        return static_cast<BattlegroundWS*>(bg)->GetFlagPickerGUID(bg->GetOtherTeamId(bot->GetTeamId())) !=
               bot->GetGUID();

    return bot->InBattleground();
}

bool PlayerHasFlag::IsActive()
{
    return IsCapturingFlag(bot);
}

bool PlayerHasFlag::IsCapturingFlag(Player* bot)
{
    if (bot->InBattleground())
    {
        Battleground* battleground = bot->GetBattleground();
        if (battleground && battleground->GetBgTypeID(true) == BATTLEGROUND_WS)
        {
            BattlegroundWS* bg = (BattlegroundWS*)bot->GetBattleground();
            // bot is horde and has ally flag
            if (bot->GetGUID() == bg->GetFlagPickerGUID(TEAM_ALLIANCE))
            {
                if (bg->GetFlagPickerGUID(TEAM_HORDE))  // enemy has flag too
                {
                    if (GameObject* go = bg->GetBGObject(BG_WS_OBJECT_H_FLAG))
                    {
                        // only indicate capturing if signicant distance from own flag
                        // (otherwise allow bot to defend itself)
                        return bot->GetDistance(go) > 36.0f;
                    }
                }
                return true;  // enemy doesnt have flag so we can cap immediately
            }
            // bot is ally and has horde flag
            if (bot->GetGUID() == bg->GetFlagPickerGUID(TEAM_HORDE))
            {
                if (bg->GetFlagPickerGUID(TEAM_ALLIANCE))  // enemy has flag too
                {
                    if (GameObject* go = bg->GetBGObject(BG_WS_OBJECT_A_FLAG))
                    {
                        // only indicate capturing if signicant distance from own flag
                        // (otherwise allow bot to defend itself)
                        return bot->GetDistance(go) > 36.0f;
                    }
                }
                return true;  // enemy doesnt have flag so we can cap immediately
            }
            return false;  // bot doesn't have flag
        }

        if (RealBgType(bot) == BATTLEGROUND_EY)
        {
            BattlegroundEY* bg = (BattlegroundEY*)bot->GetBattleground();

            // Check if bot has the flag
            if (bot->GetGUID() == bg->GetFlagPickerGUID())
            {
                // Count how many bases the bot's team owns
                uint32 controlledBases = 0;
                for (uint8 point = 0; point < EY_POINTS_MAX; ++point)
                {
                    if (bg->GetCapturePointInfo(point)._ownerTeamId == bot->GetTeamId())
                        controlledBases++;
                }

                // If no bases are controlled, bot should go aggressive
                if (controlledBases == 0)
                    return false; // bot has flag but no place to take it

                // Otherwise, return false and stay defensive / move to base
                return bot->GetGUID() == bg->GetFlagPickerGUID();
            }
        }

        return false;
    }

    return false;
}

bool TeamHasFlag::IsActive()
{
    if (!botAI->GetBot()->InBattleground())
        return false;

    Battleground* battleground = bot->GetBattleground();
    if (!battleground || battleground->GetBgTypeID(true) != BATTLEGROUND_WS)
        return false;

    BattlegroundWS* bg = static_cast<BattlegroundWS*>(battleground);

    ObjectGuid botGuid = bot->GetGUID();
    TeamId teamId = bot->GetTeamId();
    TeamId enemyTeamId = bg->GetOtherTeamId(teamId);

    // If the bot is carrying any flag, don't activate
    if (botGuid == bg->GetFlagPickerGUID(TEAM_ALLIANCE) || botGuid == bg->GetFlagPickerGUID(TEAM_HORDE))
        return false;

    // Check: Own team has enemy flag, enemy team does NOT have your flag
    bool ownTeamHasFlag = bg->GetFlagState(enemyTeamId) == BG_WS_FLAG_STATE_ON_PLAYER;
    bool enemyTeamHasFlag = bg->GetFlagState(teamId) == BG_WS_FLAG_STATE_ON_PLAYER;

    return ownTeamHasFlag && !enemyTeamHasFlag;
}

bool EnemyTeamHasFlag::IsActive()
{
    if (botAI->GetBot()->InBattleground())
    {
        Battleground* battleground = bot->GetBattleground();
        if (battleground && battleground->GetBgTypeID(true) == BATTLEGROUND_WS)
        {
            BattlegroundWS* bg = (BattlegroundWS*)botAI->GetBot()->GetBattleground();

            if (bot->GetTeamId() == TEAM_HORDE)
            {
                if (!bg->GetFlagPickerGUID(TEAM_HORDE).IsEmpty())
                    return true;
            }
            else
            {
                if (!bg->GetFlagPickerGUID(TEAM_ALLIANCE).IsEmpty())
                    return true;
            }
        }

        return false;
    }

    return false;
}

bool EnemyFlagCarrierNear::IsActive()
{
    Unit* carrier = AI_VALUE(Unit*, "enemy flag carrier");

    Battleground* bg = bot->GetBattleground();
    if (bg && bg->GetBgTypeID(true) == BATTLEGROUND_WS)
        return CanRespondToWsgCarrier(botAI, carrier);

    if (!carrier || !ServerFacade::instance().IsDistanceLessOrEqualThan(
                        ServerFacade::instance().GetDistance2d(bot, carrier), 100.f))
        return false;

    // Check if there is another enemy player target closer than the FC
    Unit* nearbyEnemy = AI_VALUE(Unit*, "enemy player target");
    if (nearbyEnemy)
    {
        float distToFC = ServerFacade::instance().GetDistance2d(bot, carrier);
        float distToEnemy = ServerFacade::instance().GetDistance2d(bot, nearbyEnemy);
        if (distToEnemy + 15.0f < distToFC)
            return false;
    }

    return true;
}

bool WsgEscortSeparated::IsActive() { return ShouldRegroupWsgEscort(botAI); }

bool WsgFlagStateChanged::IsActive()
{
    Battleground* bg = bot->GetBattleground();
    bool isWarsong = bg && bg->GetBgTypeID(true) == BATTLEGROUND_WS;
    if (!isWarsong || bg->GetStatus() != STATUS_IN_PROGRESS || !bot->IsAlive())
    {
        _initialized = false;
        _markerValid = false;
        _lastMarkerSampleMs = 0;
        return false;
    }

    BattlegroundWS* warsong = static_cast<BattlegroundWS*>(bg);
    GameObject* allianceBase = bg->GetBGObject(BG_WS_OBJECT_A_FLAG);
    GameObject* hordeBase = bg->GetBGObject(BG_WS_OBJECT_H_FLAG);
    bool allianceBaseReady = allianceBase && allianceBase->isSpawned() && allianceBase->GetGoState() == GO_STATE_READY;
    bool hordeBaseReady = hordeBase && hordeBase->isSpawned() && hordeBase->GetGoState() == GO_STATE_READY;
    uint8 allianceFlagState = warsong->GetFlagState(TEAM_ALLIANCE);
    uint8 hordeFlagState = warsong->GetFlagState(TEAM_HORDE);
    ObjectGuid alliancePicker = warsong->GetFlagPickerGUID(TEAM_ALLIANCE);
    ObjectGuid hordePicker = warsong->GetFlagPickerGUID(TEAM_HORDE);
    ObjectGuid allianceDropped = warsong->GetDroppedFlagGUID(TEAM_ALLIANCE);
    ObjectGuid hordeDropped = warsong->GetDroppedFlagGUID(TEAM_HORDE);
    WsgTeamAssignment assignment = context->GetValue<WsgTeamAssignment>("wsg team assignment")->Get();

    bool changed =
        !_initialized || _instanceId != bg->GetInstanceID() || _allianceFlagState != allianceFlagState ||
        _hordeFlagState != hordeFlagState || _alliancePicker != alliancePicker || _hordePicker != hordePicker ||
        _allianceDropped != allianceDropped || _hordeDropped != hordeDropped ||
        _allianceBaseReady != allianceBaseReady || _hordeBaseReady != hordeBaseReady || _role != assignment.Role ||
        _defenders != assignment.Defenders || _escorts != assignment.Escorts || _escort != assignment.Escort ||
        _escortIndex != assignment.EscortIndex || _baseDefender != assignment.BaseDefender ||
        _defender != assignment.Defender || _attacker != assignment.Attacker || _returner != assignment.Returner ||
        _committedAttack != assignment.CommittedAttack || _assignmentValid != assignment.Valid;

    // The same X/Y marker is sent to players requesting BG positions. Sample only for
    // assigned returners, and avoid interrupting their route for every small movement.
    if (!_initialized || _instanceId != bg->GetInstanceID())
    {
        _markerValid = false;
        _lastMarkerSampleMs = 0;
    }
    TeamId team = bot->GetTeamId();
    uint8 ownFlagState = team == TEAM_ALLIANCE ? allianceFlagState : hordeFlagState;
    ObjectGuid ownPicker = team == TEAM_ALLIANCE ? alliancePicker : hordePicker;
    bool returner = assignment.Valid && assignment.Returner &&
                    warsong->GetFlagPickerGUID(bg->GetOtherTeamId(team)) != bot->GetGUID();
    bool markerChanged = false;
    if (ownFlagState != BG_WS_FLAG_STATE_ON_PLAYER || !returner || ownPicker != _markerGuid)
    {
        _markerValid = false;
        _lastMarkerSampleMs = 0;
        _markerGuid = ownPicker;
    }
    if (ownFlagState == BG_WS_FLAG_STATE_ON_PLAYER && returner)
    {
        uint32 now = getMSTime();
        constexpr uint32 markerInterval = 5 * IN_MILLISECONDS;
        if (!_lastMarkerSampleMs || now - _lastMarkerSampleMs >= markerInterval)
        {
            _lastMarkerSampleMs = now;
            Player* enemyCarrier = ownPicker.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(bg->GetBgMap(), ownPicker);
            if (enemyCarrier && enemyCarrier->IsAlive() && enemyCarrier->IsInWorld() &&
                enemyCarrier->GetMap() == bot->GetMap() && enemyCarrier->GetTeamId() != team)
            {
                float x = enemyCarrier->GetPositionX();
                float y = enemyCarrier->GetPositionY();
                constexpr float markerReplanDistance = 50.0f;
                float dx = x - _markerX;
                float dy = y - _markerY;
                markerChanged = !_markerValid || dx * dx + dy * dy >= markerReplanDistance * markerReplanDistance;
                if (markerChanged)
                {
                    _markerX = x;
                    _markerY = y;
                    _markerValid = true;
                }
            }
        }
    }

    _initialized = true;
    _instanceId = bg->GetInstanceID();
    _allianceFlagState = allianceFlagState;
    _hordeFlagState = hordeFlagState;
    _alliancePicker = alliancePicker;
    _hordePicker = hordePicker;
    _allianceDropped = allianceDropped;
    _hordeDropped = hordeDropped;
    _allianceBaseReady = allianceBaseReady;
    _hordeBaseReady = hordeBaseReady;
    _role = assignment.Role;
    _defenders = assignment.Defenders;
    _escorts = assignment.Escorts;
    _escort = assignment.Escort;
    _escortIndex = assignment.EscortIndex;
    _baseDefender = assignment.BaseDefender;
    _defender = assignment.Defender;
    _attacker = assignment.Attacker;
    _returner = assignment.Returner;
    _committedAttack = assignment.CommittedAttack;
    _assignmentValid = assignment.Valid;
    return changed || markerChanged;
}

bool PlayerWantsInBattlegroundTrigger::IsActive()
{
    if (bot->InBattleground())
        return false;

    if (bot->GetBattleground() && bot->GetBattleground()->GetStatus() == STATUS_WAIT_JOIN)
        return false;

    if (bot->GetBattleground() && bot->GetBattleground()->GetStatus() == STATUS_IN_PROGRESS)
        return false;

    if (bot->IsDeserter())
        return false;

    return true;
}

bool VehicleNearTrigger::IsActive()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles");
    return npcs.size();
}

bool InVehicleTrigger::IsActive() { return botAI->IsInVehicle(); }
