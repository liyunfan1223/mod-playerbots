/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

/*
 * PvP portions adapted from VincentVanclef/mod-playerbots, commit d7cc409c2e145bc4fa5b64bfc9539dab4cf11a43.
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may
 * redistribute it and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PvpTriggers.h"
#include "BattleGroundTactics.h"
#include "BattlegroundEY.h"
#include "BattlegroundMgr.h"
#include "BattlegroundWS.h"
#include "Playerbots.h"
#include "ServerFacade.h"

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

bool ImprovedBgTeams(Player* bot)
{
    return bot && bot->InBattleground() && !bot->InArena() &&
           (sPlayerbotAIConfig.PvpTacticsImprovements || sPlayerbotAIConfig.BgObjectiveBrain);
}

TeamId BgTeam(Player* bot) { return ImprovedBgTeams(bot) ? bot->GetBgTeamId() : bot->GetTeamId(); }
}  // namespace

bool EnemyPlayerNear::IsActive() { return AI_VALUE(Unit*, "enemy player target"); }

bool PlayerHasNoFlag::IsActive()
{
    if (!bot->GetBattleground())
        return false;

    if (ImprovedBgTeams(bot) && bot->InBattleground() && bot->GetBattleground())
        return RealBgType(bot) == BATTLEGROUND_WS && !PlayerHasFlag::IsCarryingFlag(bot);

    if (botAI->GetBot()->InBattleground())
    {
        if (RealBgType(botAI->GetBot()) == BattlegroundTypeId::BATTLEGROUND_WS)
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
    if (!bot->GetBattleground())
        return false;

    if (ImprovedBgTeams(bot))
        return bot->InBattleground() && bot->GetBattleground() && !PlayerHasFlag::IsCarryingFlag(bot);

    if (botAI->GetBot()->InBattleground())
    {
        if (RealBgType(botAI->GetBot()) == BattlegroundTypeId::BATTLEGROUND_WS)
        {
            BattlegroundWS* bg = (BattlegroundWS*)botAI->GetBot()->GetBattleground();
            if (!(bg->GetFlagState(bg->GetOtherTeamId(bot->GetTeamId())) == BG_WS_FLAG_STATE_ON_PLAYER))
                return true;

            if (bot->GetGUID() == bg->GetFlagPickerGUID(TEAM_ALLIANCE) ||
                bot->GetGUID() == bg->GetFlagPickerGUID(TEAM_HORDE))
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

bool PlayerHasFlag::IsCarryingFlag(Player* bot)
{
    if (!bot || !bot->InBattleground())
        return false;

    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    if (RealBgType(bot) == BATTLEGROUND_WS)
    {
        BattlegroundWS* warsong = static_cast<BattlegroundWS*>(bg);
        ObjectGuid const alliancePicker = warsong->GetFlagPickerGUID(TEAM_ALLIANCE);
        ObjectGuid const hordePicker = warsong->GetFlagPickerGUID(TEAM_HORDE);
        if (alliancePicker == bot->GetGUID() || hordePicker == bot->GetGUID())
            return true;

        return (alliancePicker.IsEmpty() && bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG)) ||
               (hordePicker.IsEmpty() && bot->HasAura(BG_WS_SPELL_WARSONG_FLAG));
    }

    if (RealBgType(bot) == BATTLEGROUND_EY)
    {
        ObjectGuid const picker = static_cast<BattlegroundEY*>(bg)->GetFlagPickerGUID();
        return picker.IsEmpty() ? bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL) : picker == bot->GetGUID();
    }

    return false;
}

bool PlayerHasFlag::IsCapturingFlag(Player* bot)
{
    if (!bot || !bot->InBattleground() || !bot->GetBattleground())
        return false;

    if (ImprovedBgTeams(bot) && RealBgType(bot) == BATTLEGROUND_WS)
    {
        if (!IsCarryingFlag(bot))
            return false;

        BattlegroundWS* bg = static_cast<BattlegroundWS*>(bot->GetBattleground());
        TeamId const team = BgTeam(bot);
        if (team != TEAM_ALLIANCE && team != TEAM_HORDE)
            return false;

        if (!bg->GetFlagPickerGUID(team).IsEmpty())
        {
            uint32 const ownFlagObject = team == TEAM_HORDE ? BG_WS_OBJECT_H_FLAG : BG_WS_OBJECT_A_FLAG;
            if (GameObject* flag = bg->GetBGObject(ownFlagObject))
                return bot->GetDistance(flag) > 36.0f;
        }

        return true;
    }

    if (bot->InBattleground())
    {
        if (RealBgType(bot) == BATTLEGROUND_WS)
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
            if (ImprovedBgTeams(bot) ? IsCarryingFlag(bot) : bot->GetGUID() == bg->GetFlagPickerGUID())
            {
                // Count how many bases the bot's team owns
                uint32 controlledBases = 0;
                for (uint8 point = 0; point < EY_POINTS_MAX; ++point)
                {
                    if (bg->GetCapturePointInfo(point)._ownerTeamId == BgTeam(bot))
                        controlledBases++;
                }

                // If no bases are controlled, bot should go aggressive
                if (controlledBases == 0)
                    return false; // bot has flag but no place to take it

                // Otherwise, return false and stay defensive / move to base
                return true;
            }
        }

        return false;
    }

    return false;
}

bool TeamHasFlag::IsActive()
{
    if (!botAI->GetBot()->InBattleground() || !bot->GetBattleground())
        return false;

    if (RealBgType(botAI->GetBot()) != BattlegroundTypeId::BATTLEGROUND_WS)
        return false;

    BattlegroundWS* bg = (BattlegroundWS*)botAI->GetBot()->GetBattleground();

    ObjectGuid botGuid = bot->GetGUID();
    TeamId teamId = BgTeam(bot);
    if (teamId != TEAM_ALLIANCE && teamId != TEAM_HORDE)
        return false;
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
    if (!bot->GetBattleground())
        return false;

    if (botAI->GetBot()->InBattleground())
    {
        if (RealBgType(botAI->GetBot()) == BattlegroundTypeId::BATTLEGROUND_WS)
        {
            BattlegroundWS* bg = (BattlegroundWS*)botAI->GetBot()->GetBattleground();

            if (BgTeam(bot) == TEAM_HORDE)
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

    if (!carrier || !ServerFacade::instance().IsDistanceLessOrEqualThan(ServerFacade::instance().GetDistance2d(bot, carrier), 100.f))
        return false;

    // Check if there is another enemy player target closer than the FC
    Unit* nearbyEnemy = AI_VALUE(Unit*, "enemy player target");

    if (nearbyEnemy)
    {
        float distToFC = ServerFacade::instance().GetDistance2d(bot, carrier);
        float distToEnemy = ServerFacade::instance().GetDistance2d(bot, nearbyEnemy);

        // If the other enemy is significantly closer, don't pursue FC
        if (distToEnemy + 15.0f < distToFC) // Add small buffer
            return false;
    }

    return true;
}

bool TeamFlagCarrierNear::IsActive()
{
    if (RealBgType(bot) == BATTLEGROUND_WS)
    {
        BattlegroundWS* bg = dynamic_cast<BattlegroundWS*>(bot->GetBattleground());
        if (bg)
        {
            bool bothFlagsNotAtBase =
                bg->GetFlagState(TEAM_ALLIANCE) != BG_WS_FLAG_STATE_ON_BASE &&
                bg->GetFlagState(TEAM_HORDE) != BG_WS_FLAG_STATE_ON_BASE;

            if (bothFlagsNotAtBase)
                return false;
        }
    }

    Unit* carrier = AI_VALUE(Unit*, "team flag carrier");
    float const escortRange =
        sPlayerbotAIConfig.PvpTacticsImprovements && bot->InBattleground() && !bot->InArena() ? 35.0f : 200.0f;
    return carrier && ServerFacade::instance().IsDistanceLessOrEqualThan(
                          ServerFacade::instance().GetDistance2d(bot, carrier), escortRange);
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
