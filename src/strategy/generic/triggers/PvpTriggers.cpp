/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PvpTriggers.h"

#include "BattlegroundEY.h"
#include "BattlegroundMgr.h"
#include "BattlegroundWS.h"
#include "Playerbots.h"
#include "ServerFacade.h"

bool EnemyPlayerNear::IsActive() { return AI_VALUE(Unit*, "enemy player target"); }

bool PlayerHasNoFlag::IsActive()
{
    if (botAI->GetBot()->InBattleground())
    {
        if (botAI->GetBot()->GetBattlegroundTypeId() == BattlegroundTypeId::BATTLEGROUND_WS)
        {
            BattlegroundWS* bg = (BattlegroundWS*)botAI->GetBot()->GetBattleground();
            if (!(bg->GetFlagState(bg->GetOtherTeamId(bot->GetTeamId())) == BG_WS_FLAG_STATE_ON_PLAYER))
                return true;

            if (bot->GetGUID() == bg->GetFlagPickerGUID(TEAM_ALLIANCE) ||
                bot->GetGUID() == bg->GetFlagPickerGUID(TEAM_HORDE))
            {
                return false;
            }
            return true;
        }
        return false;
    }

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
    if (botAI->GetBot()->InBattleground())
    {
        if (botAI->GetBot()->GetBattlegroundTypeId() == BattlegroundTypeId::BATTLEGROUND_WS)
        {
            BattlegroundWS* bg = (BattlegroundWS*)botAI->GetBot()->GetBattleground();
            if (!(bg->GetFlagState(bg->GetOtherTeamId(bot->GetTeamId())) == BG_WS_FLAG_STATE_ON_PLAYER))
                return true;

            if (bot->GetGUID() == bg->GetFlagPickerGUID(TEAM_ALLIANCE) ||
                bot->GetGUID() == bg->GetFlagPickerGUID(TEAM_ALLIANCE))
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool PlayerHasFlag::IsActive()
{
    return IsCapturingFlag(bot);
}

bool PlayerHasFlag::IsCapturingFlag(Player* bot)
{
    if (bot->InBattleground())
    {
        if (bot->GetBattlegroundTypeId() == BATTLEGROUND_WS)
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

        if (bot->GetBattlegroundTypeId() == BATTLEGROUND_EY)
        {
            // TODO we should probably add similiar logic as WSG to allow combat
            // when bot has flag but no bases are available to take it to
            BattlegroundEY* bg = (BattlegroundEY*)bot->GetBattleground();
            return bot->GetGUID() == bg->GetFlagPickerGUID();
        }

        return false;
    }

    return false;
}

bool TeamHasFlag::IsActive()
{
    if (botAI->GetBot()->InBattleground())
    {
        if (botAI->GetBot()->GetBattlegroundTypeId() == BattlegroundTypeId::BATTLEGROUND_WS)
        {
            BattlegroundWS* bg = (BattlegroundWS*)botAI->GetBot()->GetBattleground();

            if (bot->GetGUID() == bg->GetFlagPickerGUID(TEAM_ALLIANCE) ||
                bot->GetGUID() == bg->GetFlagPickerGUID(TEAM_HORDE))
            {
                return false;
            }

            if (bg->GetFlagState(bg->GetOtherTeamId(bot->GetTeamId())) == BG_WS_FLAG_STATE_ON_PLAYER)
                return true;
        }

        return false;
    }

    return false;
}

bool EnemyTeamHasFlag::IsActive()
{
    if (botAI->GetBot()->InBattleground())
    {
        if (botAI->GetBot()->GetBattlegroundTypeId() == BattlegroundTypeId::BATTLEGROUND_WS)
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
    return carrier && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, carrier), 200.f);
}

bool TeamFlagCarrierNear::IsActive()
{
    Unit* carrier = AI_VALUE(Unit*, "team flag carrier");
    return carrier && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, carrier), 200.f);
}

bool PlayerWantsInBattlegroundTrigger::IsActive()
{
    if (bot->InBattleground())
        return false;

    if (bot->GetBattleground() && bot->GetBattleground()->GetStatus() == STATUS_WAIT_JOIN)
        return false;

    if (bot->GetBattleground() && bot->GetBattleground()->GetStatus() == STATUS_IN_PROGRESS)
        return false;

    if (!bot->CanJoinToBattleground())
        return false;

    return true;
}

bool VehicleNearTrigger::IsActive()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles");
    return npcs.size();
}

bool InVehicleTrigger::IsActive() { return botAI->IsInVehicle(); }
