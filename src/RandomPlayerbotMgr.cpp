/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RandomPlayerbotMgr.h"
#include "AccountMgr.h"
#include "AiFactory.h"
#include "ArenaTeamMgr.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "CellImpl.h"
#include "Define.h"
#include "FleeManager.h"
#include "GameTime.h"
#include "GuildMgr.h"
#include "GuildTaskMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "LFGMgr.h"
#include "MapMgr.h"
#include "PerformanceMonitor.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "PlayerbotCommandServer.h"
#include "PlayerbotFactory.h"
#include "Random.h"
#include "ServerFacade.h"
#include "ChannelMgr.h"
#include "Unit.h"
#include "World.h"

#include <cstdlib>
#include <iomanip>
#include <boost/thread/thread.hpp>

void PrintStatsThread()
{
    sRandomPlayerbotMgr->PrintStats();
}

void activatePrintStatsThread()
{
    boost::thread t(PrintStatsThread);
    t.detach();
}

void CheckBgQueueThread()
{
    sRandomPlayerbotMgr->CheckBgQueue();
}

void activateCheckBgQueueThread()
{
    boost::thread t(CheckBgQueueThread);
    t.detach();
}

void CheckLfgQueueThread()
{
    sRandomPlayerbotMgr->CheckLfgQueue();
}

void activateCheckLfgQueueThread()
{
    boost::thread t(CheckLfgQueueThread);
    t.detach();
}

void CheckPlayersThread()
{
    sRandomPlayerbotMgr->CheckPlayers();
}

void activateCheckPlayersThread()
{
    boost::thread t(CheckPlayersThread);
    t.detach();
}

RandomPlayerbotMgr::RandomPlayerbotMgr() : PlayerbotHolder(), processTicks(0), totalPmo(nullptr)
{
    playersLevel = sPlayerbotAIConfig->randombotStartingLevel;

    if (sPlayerbotAIConfig->enabled || sPlayerbotAIConfig->randomBotAutologin)
    {
        sPlayerbotCommandServer->Start();
        PrepareTeleportCache();
    }

    for (uint8 i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        for (uint8 j = BATTLEGROUND_QUEUE_AV; j < MAX_BATTLEGROUND_QUEUE_TYPES; ++j)
        {
            BgPlayers[j][i][TEAM_ALLIANCE] = 0;
            BgPlayers[j][i][TEAM_HORDE] = 0;
            BgBots[j][i][TEAM_ALLIANCE] = 0;
            BgBots[j][i][TEAM_HORDE] = 0;
            ArenaBots[j][i][TEAM_ALLIANCE][TEAM_ALLIANCE] = 0;
            ArenaBots[j][i][TEAM_ALLIANCE][TEAM_HORDE] = 0;
            ArenaBots[j][i][TEAM_HORDE][TEAM_ALLIANCE] = 0;
            ArenaBots[j][i][TEAM_HORDE][TEAM_HORDE] = 0;
            NeedBots[j][i][TEAM_ALLIANCE] = false;
            NeedBots[j][i][TEAM_HORDE] = false;
        }
    }

    BgCheckTimer = 0;
    LfgCheckTimer = 0;
    PlayersCheckTimer = 0;
}

RandomPlayerbotMgr::~RandomPlayerbotMgr()
{
}

uint32 RandomPlayerbotMgr::GetMaxAllowedBotCount()
{
    return GetEventValue(0, "bot_count");
}

void RandomPlayerbotMgr::LogPlayerLocation()
{
    activeBots = 0;

    try
    {
        sPlayerbotAIConfig->openLog("player_location.csv", "w");

        if (sPlayerbotAIConfig->randomBotAutologin)
        {
            for (auto i : GetAllBots())
            {
                Player* bot = i.second;
                if (!bot)
                    continue;

                std::ostringstream out;
                out << sPlayerbotAIConfig->GetTimestampStr() << "+00,";
                out << "RND"
                    << ",";
                out << bot->GetName() << ",";
                out << std::fixed << std::setprecision(2);
                WorldPosition(bot).printWKT(out);
                out << bot->GetOrientation() << ",";
                out << std::to_string(bot->getRace()) << ",";
                out << std::to_string(bot->getClass()) << ",";
                out << bot->GetMapId() << ",";
                out << bot->getLevel() << ",";
                out << bot->GetHealth() << ",";
                out << bot->GetPowerPct(bot->getPowerType()) << ",";
                out << bot->GetMoney() << ",";

                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
                {
                    out << std::to_string(uint8(botAI->GetGrouperType())) << ",";
                    out << std::to_string(uint8(botAI->GetGuilderType())) << ",";
                    out << (botAI->AllowActivity(ALL_ACTIVITY) ? "active" : "inactive") << ",";
                    out << (botAI->IsActive() ? "active" : "delay") << ",";
                    out << botAI->HandleRemoteCommand("state") << ",";

                    if (botAI->AllowActivity(ALL_ACTIVITY))
                        activeBots++;
                }
                else
                {
                    out << 0 << "," << 0 << ",err,err,err,";
                }

                out << (bot->IsInCombat() ? "combat" : "safe") << ",";
                out << (bot->isDead() ? (bot->GetCorpse() ? "ghost" : "dead") : "alive");

                sPlayerbotAIConfig->log("player_location.csv", out.str().c_str());
            }

            for (auto i : GetPlayers())
            {
                Player* bot = i;
                if (!bot)
                    continue;

                std::ostringstream out;
                out << sPlayerbotAIConfig->GetTimestampStr() << "+00,";
                out << "PLR" << ",";
                out << bot->GetName() << ",";
                out << std::fixed << std::setprecision(2);
                WorldPosition(bot).printWKT(out);
                out << bot->GetOrientation() << ",";
                out << std::to_string(bot->getRace()) << ",";
                out << std::to_string(bot->getClass()) << ",";
                out << bot->GetMapId() << ",";
                out << bot->getLevel() << ",";
                out << bot->GetHealth() << ",";
                out << bot->GetPowerPct(bot->getPowerType()) << ",";
                out << bot->GetMoney() << ",";

                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
                {
                    out << std::to_string(uint8(botAI->GetGrouperType())) << ",";
                    out << std::to_string(uint8(botAI->GetGuilderType())) << ",";
                    out << (botAI->AllowActivity(ALL_ACTIVITY) ? "active" : "inactive") << ",";
                    out << (botAI->IsActive() ? "active" : "delay") << ",";
                    out << botAI->HandleRemoteCommand("state") << ",";

                    if (botAI->AllowActivity(ALL_ACTIVITY))
                        activeBots++;
                }
                else
                {
                    out << 0 << "," << 0 << ",player,player,player,";
                }

                out << (bot->IsInCombat() ? "combat" : "safe") << ",";
                out << (bot->isDead() ? (bot->GetCorpse() ? "ghost" : "dead") : "alive");

                sPlayerbotAIConfig->log("player_location.csv", out.str().c_str());
            }
        }
    }
    catch (...)
    {
        return;
        // This is to prevent some thread-unsafeness. Crashes would happen if bots get added or removed.
        // We really don't care here. Just skip a log. Making this thread-safe is not worth the effort.
    }
}

void RandomPlayerbotMgr::UpdateAIInternal(uint32 elapsed, bool /*minimal*/)
{
    if (totalPmo)
        totalPmo->finish();

    totalPmo = sPerformanceMonitor->start(PERF_MON_TOTAL, "RandomPlayerbotMgr::FullTick");

    if (!sPlayerbotAIConfig->randomBotAutologin || !sPlayerbotAIConfig->enabled)
        return;

    uint32 maxAllowedBotCount = GetEventValue(0, "bot_count");
    if (!maxAllowedBotCount || (maxAllowedBotCount < sPlayerbotAIConfig->minRandomBots || maxAllowedBotCount > sPlayerbotAIConfig->maxRandomBots))
    {
        maxAllowedBotCount = urand(sPlayerbotAIConfig->minRandomBots, sPlayerbotAIConfig->maxRandomBots);
        SetEventValue(0, "bot_count", maxAllowedBotCount, urand(sPlayerbotAIConfig->randomBotCountChangeMinInterval, sPlayerbotAIConfig->randomBotCountChangeMaxInterval));
    }

    GetBots();
    std::list<uint32> availableBots = currentBots;
    uint32 availableBotCount = availableBots.size();
    uint32 onlineBotCount = playerBots.size();

    uint32 onlineBotFocus = 75;
    if (onlineBotCount < (uint32)(sPlayerbotAIConfig->minRandomBots * 90 / 100))
        onlineBotFocus = 25;

    SetNextCheckDelay(sPlayerbotAIConfig->randomBotUpdateInterval * (onlineBotFocus + 25) * 10);

    PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_TOTAL, onlineBotCount < maxAllowedBotCount ? "RandomPlayerbotMgr::Login" : "RandomPlayerbotMgr::UpdateAIInternal");

    if (availableBotCount < maxAllowedBotCount)
    {
        AddRandomBots();
    }

    if (sPlayerbotAIConfig->syncLevelWithPlayers && !players.empty())
    {
        if (time(nullptr) > (PlayersCheckTimer + 60))
            activateCheckPlayersThread();
    }
    
    if (sPlayerbotAIConfig->randomBotJoinBG/* && !players.empty()*/)
    {
        if (time(nullptr) > (BgCheckTimer + 30))
            activateCheckBgQueueThread();
    }

    uint32 updateBots = sPlayerbotAIConfig->randomBotsPerInterval * onlineBotFocus / 100;
    uint32 maxNewBots = onlineBotCount < maxAllowedBotCount ? maxAllowedBotCount - onlineBotCount : 0;
    uint32 loginBots = std::min(sPlayerbotAIConfig->randomBotsPerInterval - updateBots, maxNewBots);

    if (!availableBots.empty())
    {
        // Update bots
        for (auto bot : availableBots)
        {
            if (!GetPlayerBot(bot))
                continue;

            if (ProcessBot(bot))
            {
               updateBots--;
            }

            if (!updateBots)
                break;
        }

        if (loginBots)
        {
            loginBots += updateBots;
            loginBots = std::min(loginBots, maxNewBots);

            LOG_INFO("playerbots", "{} new bots", loginBots);

            //Log in bots
            for (auto bot : availableBots)
            {
                if (GetPlayerBot(bot))
                    continue;

                if (ProcessBot(bot))
                {
                    loginBots--;
                }

                if (!loginBots)
                    break;
            }
        }
    }

    if (pmo)
        pmo->finish();

    if (sPlayerbotAIConfig->hasLog("player_location.csv"))
    {
        LogPlayerLocation();
    }
}

uint32 RandomPlayerbotMgr::AddRandomBots()
{
    uint32 maxAllowedBotCount = GetEventValue(0, "bot_count");

    if (currentBots.size() < maxAllowedBotCount)
    {
        maxAllowedBotCount -= currentBots.size();
        maxAllowedBotCount = std::min(sPlayerbotAIConfig->randomBotsPerInterval, maxAllowedBotCount);

        for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotAccounts.begin(); i != sPlayerbotAIConfig->randomBotAccounts.end(); i++)
        {
            uint32 accountId = *i;
            if (sPlayerbotAIConfig->enableRotation) {
                uint32 limit = std::min((uint32)sPlayerbotAIConfig->randomBotAccounts.size(), sPlayerbotAIConfig->rotationPoolSize / 10 + 1);
                uint32 index = urand(0, limit);
                accountId = sPlayerbotAIConfig->randomBotAccounts[index];
            }
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARS_BY_ACCOUNT_ID);
            stmt->SetData(0, accountId);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);
            if (!result)
                continue;

            do
            {
                Field* fields = result->Fetch();
                ObjectGuid::LowType guid = fields[0].Get<uint32>();

                if (GetEventValue(guid, "add"))
                    continue;

                if (GetEventValue(guid, "logout"))
                    continue;

                if (GetPlayerBot(guid))
                    continue;

                if (std::find(currentBots.begin(), currentBots.end(), guid) != currentBots.end())
                    continue;
                
                uint32 add_time = sPlayerbotAIConfig->enableRotation ? 
                    urand(sPlayerbotAIConfig->minRandomBotInWorldTime, sPlayerbotAIConfig->maxRandomBotInWorldTime) :
                    sPlayerbotAIConfig->randomBotInWorldWithRotaionDisabled;
                    
                SetEventValue(guid, "add", 1, add_time);
                SetEventValue(guid, "logout", 0, 0);
                currentBots.push_back(guid);

                maxAllowedBotCount--;
                if (!maxAllowedBotCount)
                    break;

            } while (result->NextRow());

            if (!maxAllowedBotCount)
                break;
        }

        if (maxAllowedBotCount)
            LOG_ERROR("playerbots", "Not enough random bot accounts available. Need {} more!!", ceil(maxAllowedBotCount / 10));
    }

    return currentBots.size();
}

void RandomPlayerbotMgr::LoadBattleMastersCache()
{
    BattleMastersCache.clear();

    LOG_INFO("playerbots", "---------------------------------------");
    LOG_INFO("playerbots", "          Loading BattleMasters Cache  ");
    LOG_INFO("playerbots", "---------------------------------------");

    QueryResult result = WorldDatabase.Query("SELECT `entry`,`bg_template` FROM `battlemaster_entry`");

    uint32 count = 0;

    if (!result)
    {
        return;
    }

    do
    {
        ++count;

        Field* fields = result->Fetch();

        uint32 entry = fields[0].Get<uint32>();
        uint32 bgTypeId = fields[1].Get<uint32>();

        CreatureTemplate const* bmaster = sObjectMgr->GetCreatureTemplate(entry);
        if (!bmaster)
            continue;

        FactionTemplateEntry const* bmFaction = sFactionTemplateStore.LookupEntry(bmaster->faction);
        uint32 bmFactionId = bmFaction->faction;
        FactionEntry const* bmParentFaction = sFactionStore.LookupEntry(bmFactionId);
        uint32 bmParentTeam = bmParentFaction->team;
        TeamId bmTeam = TEAM_NEUTRAL;
        if (bmParentTeam == 891)
            bmTeam = TEAM_ALLIANCE;

        if (bmFactionId == 189)
            bmTeam = TEAM_ALLIANCE;

        if (bmParentTeam == 892)
            bmTeam = TEAM_HORDE;

        if (bmFactionId == 66)
            bmTeam = TEAM_HORDE;

        BattleMastersCache[bmTeam][BattlegroundTypeId(bgTypeId)].insert(BattleMastersCache[bmTeam][BattlegroundTypeId(bgTypeId)].end(), entry);
        LOG_DEBUG("playerbots", "Cached Battmemaster #{} for BG Type {} ({})", entry, bgTypeId, bmTeam == TEAM_ALLIANCE ? "Alliance" : bmTeam == TEAM_HORDE ? "Horde" : "Neutral");

    } while (result->NextRow());

    LOG_INFO("playerbots", ">> Loaded {} battlemaster entries", count);
}

void RandomPlayerbotMgr::CheckBgQueue()
{
    if (!BgCheckTimer)
        BgCheckTimer = time(nullptr);

    uint32 count = 0;
    uint32 visual_count = 0;

    uint32 check_time = count > 0 ? 120 : 30;
    if (time(nullptr) < (BgCheckTimer + check_time))
    {
        return;
    }
    else
    {
        BgCheckTimer = time(nullptr);
    }

    LOG_INFO("playerbots", "Checking BG Queue...");

    for (uint32 i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        for (uint32 j = BATTLEGROUND_QUEUE_AV; j < MAX_BATTLEGROUND_QUEUE_TYPES; ++j)
        {
            BgPlayers[j][i][TEAM_ALLIANCE] = 0;
            BgPlayers[j][i][TEAM_HORDE] = 0;
            BgBots[j][i][TEAM_ALLIANCE] = 0;
            BgBots[j][i][TEAM_HORDE] = 0;
            ArenaBots[j][i][TEAM_ALLIANCE][TEAM_ALLIANCE] = 0;
            ArenaBots[j][i][TEAM_ALLIANCE][TEAM_HORDE] = 0;
            ArenaBots[j][i][TEAM_HORDE][TEAM_ALLIANCE] = 0;
            ArenaBots[j][i][TEAM_HORDE][TEAM_HORDE] = 0;
            NeedBots[j][i][TEAM_ALLIANCE] = false;
            NeedBots[j][i][TEAM_HORDE] = false;
        }
    }

    for (Player* player : players)
    {
        if (!player->InBattlegroundQueue())
            continue;

        if (player->InBattleground() && player->GetBattleground()->GetStatus() == STATUS_WAIT_LEAVE)
            continue;

        for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        {
            BattlegroundQueueTypeId queueTypeId = player->GetBattlegroundQueueTypeId(i);
            if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
                continue;

            TeamId teamId = player->GetTeamId();

            BattlegroundTypeId bgTypeId = sBattlegroundMgr->BGTemplateId(queueTypeId);
            Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
            uint32 mapId = bg->GetMapId();
            PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, player->getLevel());
            if (!pvpDiff)
                continue;

            BattlegroundBracketId bracketId = pvpDiff->GetBracketId();

            if (uint8 arenaType = BattlegroundMgr::BGArenaType(queueTypeId))
            {
                BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
                GroupQueueInfo ginfo;
                TeamId tempT = teamId;

                if (bgQueue.GetPlayerGroupInfoData(player->GetGUID(), &ginfo))
                {
                    if (ginfo.IsRated)
                    {
                        for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
                        {
                            uint32 arena_team_id = player->GetArenaTeamId(arena_slot);
                            ArenaTeam* arenateam = sArenaTeamMgr->GetArenaTeamById(arena_team_id);
                            if (!arenateam)
                                continue;

                            if (arenateam->GetType() != arenaType)
                                continue;

                            Rating[queueTypeId][bracketId][1] = arenateam->GetRating();
                        }
                    }

                    teamId = ginfo.IsRated ? TEAM_HORDE : TEAM_ALLIANCE;
                }

                if (player->InArena())
                {
                    if (player->GetBattleground()->isRated() && (ginfo.IsRated && ginfo.ArenaTeamId && ginfo.ArenaTeamRating && ginfo.OpponentsTeamRating))
                        teamId = TEAM_HORDE;
                    else
                        teamId = TEAM_ALLIANCE;
                }

                ArenaBots[queueTypeId][bracketId][teamId][tempT]++;
            }

            if (GET_PLAYERBOT_AI(player))
                BgBots[queueTypeId][bracketId][teamId]++;
            else
                BgPlayers[queueTypeId][bracketId][teamId]++;

            if (!player->IsInvitedForBattlegroundInstance() && (!player->InBattleground() || (player->GetBattleground() && player->GetBattleground()->GetBgTypeID() != BattlegroundMgr::BGTemplateId(queueTypeId))))
            {
                if (BattlegroundMgr::BGArenaType(queueTypeId))
                {
                    NeedBots[queueTypeId][bracketId][teamId] = true;
                }
                else
                {
                    NeedBots[queueTypeId][bracketId][TEAM_ALLIANCE] = true;
                    NeedBots[queueTypeId][bracketId][TEAM_HORDE] = true;
                }
            }
        }
    }

    for (PlayerBotMap::iterator i = playerBots.begin(); i != playerBots.end(); ++i)
    {
        Player* bot = i->second;
        if (!bot || !bot->IsInWorld())
            continue;

        if (!bot->InBattlegroundQueue())
            continue;

        if (!IsRandomBot(bot))
            continue;

        if (bot->InBattleground() && bot->GetBattleground()->GetStatus() == STATUS_WAIT_LEAVE)
            continue;

        for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        {
            BattlegroundQueueTypeId queueTypeId = bot->GetBattlegroundQueueTypeId(i);
            if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
                continue;

            TeamId teamId = bot->GetTeamId();

            BattlegroundTypeId bgTypeId = sBattlegroundMgr->BGTemplateId(queueTypeId);
            Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
            uint32 mapId = bg->GetMapId();
            PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->getLevel());
            if (!pvpDiff)
                continue;

            BattlegroundBracketId bracketId = pvpDiff->GetBracketId();

            uint8 arenaType = BattlegroundMgr::BGArenaType(queueTypeId);
            if (arenaType)
            {
                BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
                GroupQueueInfo ginfo;
                TeamId tempT = teamId;
                if (bgQueue.GetPlayerGroupInfoData(bot->GetGUID(), &ginfo))
                    teamId = ginfo.IsRated ? TEAM_HORDE : TEAM_ALLIANCE;

                if (bot->InArena())
                {
                    if (bot->GetBattleground()->isRated() && (ginfo.IsRated && ginfo.ArenaTeamId && ginfo.ArenaTeamRating && ginfo.OpponentsTeamRating))
                        teamId = TEAM_HORDE;
                    else
                        teamId = TEAM_ALLIANCE;
                }

                ArenaBots[queueTypeId][bracketId][teamId][tempT]++;
            }

            BgBots[queueTypeId][bracketId][teamId]++;
        }
    }

    for (uint8 i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        for (uint8 j = BATTLEGROUND_QUEUE_AV; j < MAX_BATTLEGROUND_QUEUE_TYPES; ++j)
        {
            BattlegroundQueueTypeId queueTypeId = BattlegroundQueueTypeId(j);

            if ((BgPlayers[j][i][TEAM_ALLIANCE] + BgBots[j][i][TEAM_ALLIANCE] + BgPlayers[j][i][TEAM_HORDE] + BgBots[j][i][TEAM_HORDE]) == 0)
                continue;

            if (uint8 type = BattlegroundMgr::BGArenaType(queueTypeId))
            {
                LOG_INFO("playerbots", "ARENA:{} {}: Player (Skirmish:{}, Rated:{}) B (Skirmish:{}, Rated:{}) Total (Skirmish:{} Rated:{})",
                    type == ARENA_TYPE_2v2 ? "2v2" : type == ARENA_TYPE_3v3 ? "3v3" : "5v5", i == 0 ? "10-19" : i == 1 ? "20-29" : i == 2 ? "30-39" : i == 3 ? "40-49" :
                    i == 4 ? "50-59" : (i == 5 && MAX_BATTLEGROUND_BRACKETS == 6) ? "60" : (i == 5 && MAX_BATTLEGROUND_BRACKETS == 7) ? "60-69" :
                    i == 6 ? (i == 6 && MAX_BATTLEGROUND_BRACKETS == 16) ? "70-79" : "70" : "80",
                    BgPlayers[j][i][TEAM_ALLIANCE], BgPlayers[j][i][TEAM_HORDE], BgBots[j][i][TEAM_ALLIANCE], BgBots[j][i][TEAM_HORDE],
                    BgPlayers[j][i][TEAM_ALLIANCE] + BgBots[j][i][TEAM_ALLIANCE], BgPlayers[j][i][TEAM_HORDE] + BgBots[j][i][TEAM_HORDE]);

                continue;
            }

            BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
            std::string _bgType;
            switch (bgTypeId)
            {
                case BATTLEGROUND_AV:
                    _bgType = "AV";
                    break;
                case BATTLEGROUND_WS:
                    _bgType = "WSG";
                    break;
                case BATTLEGROUND_AB:
                    _bgType = "AB";
                    break;
                case BATTLEGROUND_EY:
                    _bgType = "EotS";
                    break;
                case BATTLEGROUND_RB:
                    _bgType = "Random";
                    break;
                case BATTLEGROUND_SA:
                    _bgType = "SotA";
                    break;
                case BATTLEGROUND_IC:
                    _bgType = "IoC";
                    break;
                default:
                    _bgType = "Other";
                    break;
            }

            LOG_INFO("playerbots", "BG:{} {}: Player ({}:{}) Bot ({}:{}) Total (A:{} H:{})",
                _bgType, i == 0 ? "10-19" : i == 1 ? "20-29" : i == 2 ? "30-39" : i == 3 ? "40-49" : i == 4 ? "50-59" : (i == 5 && MAX_BATTLEGROUND_BRACKETS == 6) ? "60" :
                (i == 5 && MAX_BATTLEGROUND_BRACKETS == 7) ? "60-69" : i == 6 ? (i == 6 && MAX_BATTLEGROUND_BRACKETS == 16) ? "70-79" : "70" : "80",
                BgPlayers[j][i][TEAM_ALLIANCE], BgPlayers[j][i][TEAM_HORDE], BgBots[j][i][TEAM_ALLIANCE], BgBots[j][i][TEAM_HORDE],
                BgPlayers[j][i][TEAM_ALLIANCE] + BgBots[j][i][TEAM_ALLIANCE], BgPlayers[j][i][TEAM_HORDE] + BgBots[j][i][TEAM_HORDE]);
        }
    }

    LOG_INFO("playerbots", "BG Queue check finished");
}

void RandomPlayerbotMgr::CheckLfgQueue()
{
    if (!LfgCheckTimer || time(nullptr) > (LfgCheckTimer + 30))
        LfgCheckTimer = time(nullptr);

    LOG_INFO("playerbots", "Checking LFG Queue...");

    // Clear LFG list
    LfgDungeons[TEAM_ALLIANCE].clear();
    LfgDungeons[TEAM_HORDE].clear();

    for (std::vector<Player*>::iterator i = players.begin(); i != players.end(); ++i)
    {
        Player* player = *i;
        if (!player || !player->IsInWorld())
            continue;

        Group* group = player->GetGroup();
        ObjectGuid guid = group ? group->GetGUID() : player->GetGUID();

        lfg::LfgState gState = sLFGMgr->GetState(guid);
        if (gState != lfg::LFG_STATE_NONE && gState < lfg::LFG_STATE_DUNGEON)
        {
            lfg::LfgDungeonSet const& dList = sLFGMgr->GetSelectedDungeons(player->GetGUID());
            for (lfg::LfgDungeonSet::const_iterator itr = dList.begin(); itr != dList.end(); ++itr)
            {
                lfg::LFGDungeonData const* dungeon = sLFGMgr->GetLFGDungeon(*itr);
                if (!dungeon)
                    continue;

                LfgDungeons[player->GetTeamId()].push_back(dungeon->id);
            }
        }
    }

    LOG_INFO("playerbots", "LFG Queue check finished");
}

void RandomPlayerbotMgr::CheckPlayers()
{
    if (!PlayersCheckTimer || time(nullptr) > (PlayersCheckTimer + 60))
        PlayersCheckTimer = time(nullptr);

    LOG_INFO("playerbots", "Checking Players...");

    if (!playersLevel)
        playersLevel = sPlayerbotAIConfig->randombotStartingLevel;

    for (std::vector<Player*>::iterator i = players.begin(); i != players.end(); ++i)
    {
        Player* player = *i;

        if (player->IsGameMaster())
            continue;

        //if (player->GetSession()->GetSecurity() > SEC_PLAYER)
        //    continue;

        if (player->getLevel() > playersLevel)
            playersLevel = player->getLevel() + 3;
    }

    LOG_INFO("playerbots", "Max player level is {}, max bot level set to {}", playersLevel - 3, playersLevel);
}

void RandomPlayerbotMgr::ScheduleRandomize(uint32 bot, uint32 time)
{
    SetEventValue(bot, "randomize", 1, time);
    //SetEventValue(bot, "logout", 1, time + 30 + urand(sPlayerbotAIConfig->randomBotUpdateInterval, sPlayerbotAIConfig->randomBotUpdateInterval * 3));
}

void RandomPlayerbotMgr::ScheduleTeleport(uint32 bot, uint32 time)
{
    if (!time)
        time = 60 + urand(sPlayerbotAIConfig->randomBotUpdateInterval, sPlayerbotAIConfig->randomBotUpdateInterval * 3);

    SetEventValue(bot, "teleport", 1, time);
}

void RandomPlayerbotMgr::ScheduleChangeStrategy(uint32 bot, uint32 time)
{
    if (!time)
        time = urand(sPlayerbotAIConfig->minRandomBotChangeStrategyTime, sPlayerbotAIConfig->maxRandomBotChangeStrategyTime);

    SetEventValue(bot, "change_strategy", 1, time);
}

bool RandomPlayerbotMgr::ProcessBot(uint32 bot)
{
    ObjectGuid botGUID = ObjectGuid::Create<HighGuid::Player>(bot);

    Player* player = GetPlayerBot(botGUID);
    PlayerbotAI* botAI = player ? GET_PLAYERBOT_AI(player) : nullptr;

    uint32 isValid = GetEventValue(bot, "add");
    if (!isValid)
    {
		if (!player || !player->GetGroup())
		{
            if (player)
                LOG_INFO("playerbots", "Bot #{} {}:{} <{}>: log out", bot, IsAlliance(player->getRace()) ? "A" : "H", player->getLevel(), player->GetName().c_str());
            else
                LOG_INFO("playerbots", "Bot #{}: log out", bot);

			SetEventValue(bot, "add", 0, 0);
			currentBots.erase(std::remove(currentBots.begin(), currentBots.end(), bot), currentBots.end());

			if (player)
                LogoutPlayerBot(botGUID);
		}

        return false;
    }

    uint32 isLogginIn = GetEventValue(bot, "login");
    if (isLogginIn)
        return false;

    if (!player)
    {
        AddPlayerBot(botGUID, 0);
        SetEventValue(bot, "login", 1, sPlayerbotAIConfig->randomBotUpdateInterval);

        uint32 randomTime = urand(sPlayerbotAIConfig->minRandomBotReviveTime, sPlayerbotAIConfig->maxRandomBotReviveTime);
        SetEventValue(bot, "update", 1, randomTime);

        // do not randomize or teleport immediately after server start (prevent lagging)
        if (!GetEventValue(bot, "randomize")) {
            randomTime = urand(sPlayerbotAIConfig->randomBotUpdateInterval * 5, sPlayerbotAIConfig->randomBotUpdateInterval * 20);
            ScheduleRandomize(bot, randomTime);
        }
        if (!GetEventValue(bot, "teleport")) {
            randomTime = urand(sPlayerbotAIConfig->randomBotUpdateInterval * 5, sPlayerbotAIConfig->randomBotUpdateInterval * 20);
            ScheduleTeleport(bot, randomTime);
        }
        return true;
    }

    SetEventValue(bot, "login", 0, 0);

    if (player->GetGroup() || player->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return false;

    uint32 update = GetEventValue(bot, "update");
    if (!update)
    {
        if (botAI)
            botAI->GetAiObjectContext()->GetValue<bool>("random bot update")->Set(true);

        bool update = true;
        if (botAI)
        {
            //botAI->GetAiObjectContext()->GetValue<bool>("random bot update")->Set(true);
            if (!sRandomPlayerbotMgr->IsRandomBot(player))
                update = false;

            if (player->GetGroup() && botAI->GetGroupMaster())
            {
                PlayerbotAI* groupMasterBotAI = GET_PLAYERBOT_AI(botAI->GetGroupMaster());
                if (!groupMasterBotAI || groupMasterBotAI->IsRealPlayer())
                {
                    update = false;
                }
            }

            // if (botAI->HasPlayerNearby(sPlayerbotAIConfig->grindDistance))
            //     update = false;
        }

        if (update)
            ProcessBot(player);

        uint32 randomTime = urand(sPlayerbotAIConfig->minRandomBotReviveTime, sPlayerbotAIConfig->maxRandomBotReviveTime);
        SetEventValue(bot, "update", 1, randomTime);

        return true;
    }

    uint32 logout = GetEventValue(bot, "logout");
    if (player && !logout && !isValid)
    {
        LOG_INFO("playerbots", "Bot #{} {}:{} <{}>: log out", bot, IsAlliance(player->getRace()) ? "A" : "H", player->getLevel(), player->GetName().c_str());
        LogoutPlayerBot(botGUID);
        currentBots.remove(bot);
        SetEventValue(bot, "logout", 1, urand(sPlayerbotAIConfig->minRandomBotInWorldTime, sPlayerbotAIConfig->maxRandomBotInWorldTime));
        return true;
    }

    return false;
}

bool RandomPlayerbotMgr::ProcessBot(Player* player)
{
    uint32 bot = player->GetGUID().GetCounter();

    if (player->InBattleground())
        return false;

    if (player->InBattlegroundQueue())
        return false;

    //if (player->isDead())
        //return false;
    if (player->isDead())
    {
        if (!GetEventValue(bot, "dead"))
        {
            uint32 randomTime = urand(sPlayerbotAIConfig->minRandomBotReviveTime, sPlayerbotAIConfig->maxRandomBotReviveTime);
            // LOG_INFO("playerbots", "Mark bot {} as dead, will be revived in {}s.", player->GetName().c_str(), randomTime);
            SetEventValue(bot, "dead", 1, sPlayerbotAIConfig->maxRandomBotInWorldTime);
            SetEventValue(bot, "revive", 1, randomTime);
            return false;
        }

        if (!GetEventValue(bot, "revive"))
        {
			Revive(player);
			return true;
        }

        return false;
    }

    Group* group = player->GetGroup();
    if (group && !group->isLFGGroup() && IsRandomBot(group->GetLeader())) {
        player->RemoveFromGroup();
        LOG_INFO("playerbots", "Bot {} remove from group since leader is random bot.", player->GetName().c_str());
    }
    //GET_PLAYERBOT_AI(player)->GetAiObjectContext()->GetValue<bool>("random bot update")->Set(false);

    // bool randomiser = true;
    // if (player->GetGuildId())
    // {
    //     if (Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId()))
    //     {
    //         if (guild->GetLeaderGUID() == player->GetGUID())
    //         {
    //             for (std::vector<Player*>::iterator i = players.begin(); i != players.end(); ++i)
    //                 sGuildTaskMgr->Update(*i, player);
    //         }

    //         uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(guild->GetLeaderGUID());
    //         if (!sPlayerbotAIConfig->IsInRandomAccountList(accountId))
    //         {
    //             uint8 rank = player->GetRank();
    //             randomiser = rank < 4 ? false : true;
    //         }
    //     }
    // }

    uint32 randomize = GetEventValue(bot, "randomize");
    if (!randomize)
    {
        Randomize(player);
        LOG_INFO("playerbots", "Bot #{} {}:{} <{}>: randomized", bot, player->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", player->getLevel(), player->GetName());
        uint32 randomTime = urand(sPlayerbotAIConfig->minRandomBotRandomizeTime, sPlayerbotAIConfig->maxRandomBotRandomizeTime);
        ScheduleRandomize(bot, randomTime);
        return true;
    }

    // enable random teleport logic if no auto traveling enabled
    // if (!sPlayerbotAIConfig->autoDoQuests)
    // {
    uint32 teleport = GetEventValue(bot, "teleport");
    if (!teleport)
    {
        LOG_INFO("playerbots", "Bot #{} <{}>: teleport for level and refresh", bot, player->GetName());
        Refresh(player);
        RandomTeleportForLevel(player);
        uint32 time = urand(sPlayerbotAIConfig->minRandomBotTeleportInterval, sPlayerbotAIConfig->maxRandomBotTeleportInterval);
        ScheduleTeleport(bot, time);
        return true;
    }
    // }

    // uint32 changeStrategy = GetEventValue(bot, "change_strategy");
    // if (!changeStrategy)
    // {
    //     LOG_INFO("playerbots", "Changing strategy for bot  #{} <{}>", bot, player->GetName().c_str());
    //     ChangeStrategy(player);
    //     return true;
    // }

    return false;
}

void RandomPlayerbotMgr::Revive(Player* player)
{
    uint32 bot = player->GetGUID().GetCounter();

    // LOG_INFO("playerbots", "Bot {} revived", player->GetName().c_str());
    SetEventValue(bot, "dead", 0, 0);
    SetEventValue(bot, "revive", 0, 0);


    Refresh(player);
    RandomTeleportGrindForLevel(player);
}

void RandomPlayerbotMgr::RandomTeleport(Player* bot, std::vector<WorldLocation>& locs, bool hearth)
{
    if (bot->IsBeingTeleported())
        return;

    if (bot->InBattleground())
        return;

    if (bot->InBattlegroundQueue())
        return;

    // if (bot->getLevel() < 5)
    //     return;

    // if (sPlayerbotAIConfig->randomBotRpgChance < 0)
    //     return;

    if (locs.empty())
    {
        LOG_DEBUG("playerbots", "Cannot teleport bot {} - no locations available", bot->GetName().c_str());
        return;
    }

    std::vector<WorldPosition> tlocs;
    for (auto& loc : locs)
        tlocs.push_back(WorldPosition(loc));
    // Do not teleport to maps disabled in config
    tlocs.erase(std::remove_if(tlocs.begin(), tlocs.end(), [bot](WorldPosition l)
    {
        std::vector<uint32>::iterator i = find(sPlayerbotAIConfig->randomBotMaps.begin(), sPlayerbotAIConfig->randomBotMaps.end(), l.getMapId());
        return i == sPlayerbotAIConfig->randomBotMaps.end();
    }), tlocs.end());
    if (tlocs.empty())
    {
        LOG_DEBUG("playerbots", "Cannot teleport bot {} - all locations removed by filter", bot->GetName().c_str());
        return;
    }

    if (tlocs.empty())
    {
        LOG_DEBUG("playerbots", "Cannot teleport bot {} - no locations available", bot->GetName().c_str());
        return;
    }

    PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "RandomTeleportByLocations");

    uint32 index = 0;
    for (uint32 i = 0; i < tlocs.size(); i++)
    {
        for (uint8 attemtps = 0; attemtps < 3; ++attemtps)
        {
            WorldLocation loc = tlocs[urand(0, tlocs.size() - 1)];

            float x = loc.GetPositionX(); // + (attemtps > 0 ? urand(0, sPlayerbotAIConfig->grindDistance) - sPlayerbotAIConfig->grindDistance / 2 : 0);
            float y = loc.GetPositionY(); // + (attemtps > 0 ? urand(0, sPlayerbotAIConfig->grindDistance) - sPlayerbotAIConfig->grindDistance / 2 : 0);
            float z = loc.GetPositionZ();

            Map* map = sMapMgr->FindMap(loc.GetMapId(), 0);
            if (!map)
                continue;
            
            AreaTableEntry const* zone = sAreaTableStore.LookupEntry(map->GetZoneId(bot->GetPhaseMask(), x, y, z));
            if (!zone)
                continue;

            // Do not teleport to enemy zones if level is low
            if (zone->team == 4 && bot->GetTeamId() == TEAM_ALLIANCE)
                continue;

            if (zone->team == 2 && bot->GetTeamId() == TEAM_HORDE)
                continue;

            if (map->IsInWater(bot->GetPhaseMask(), x, y, z, bot->GetCollisionHeight()))
                continue;

            float ground = map->GetHeight(x, y, z + 0.5f);
            if (ground <= INVALID_HEIGHT)
                continue;

            z = 0.05f + ground;

            LOG_INFO("playerbots", "Random teleporting bot {} to {} {},{},{} ({}/{} locations)",
                bot->GetName().c_str(), zone->area_name[0], x, y, z, attemtps, tlocs.size());

            if (hearth)
            {
                bot->SetHomebind(loc, zone->ID);
            }

            bot->GetMotionMaster()->Clear();
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
            if (botAI)
                botAI->Reset(true);
            bot->TeleportTo(loc.GetMapId(), x, y, z, 0);
            bot->SendMovementFlagUpdate();

            if (pmo)
                pmo->finish();

            return;
        }
    }

    if (pmo)
        pmo->finish();

    LOG_ERROR("playerbots", "Cannot teleport bot {} - no locations available", bot->GetName().c_str());
}

void RandomPlayerbotMgr::PrepareTeleportCache()
{
    uint8 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
    if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    LOG_INFO("playerbots", "Preparing random teleport caches for {} levels...", maxLevel);

    QueryResult results = WorldDatabase.Query(
    "SELECT "
		    "g.map, "
		    "position_x, "
		    "position_y, "
		    "position_z, "
		    "t.minlevel "
        "FROM "
		    "(SELECT "
		    "map, "
		    "MIN( c.guid ) guid, "
		    "t.entry "
		    "FROM "
				"creature c "
				"INNER JOIN creature_template t ON c.id1 = t.entry "
		    "WHERE "
				"t.npcflag = 0 "
				"AND t.lootid != 0 "
				"AND t.unit_flags != 768 "
				"AND t.maxlevel - t.minlevel < 3 "
				"AND map IN ({}) "
				"AND c.id1 != 32820 "
				"AND c.spawntimesecs < 1000 "
				"AND t.faction != 188 "
		    "GROUP BY "
				"map, "
				"ROUND( position_x / 500 ), "
				"ROUND( position_y / 500 ), "
				"ROUND( position_z / 50), "
				"t.entry "
		    "HAVING "
				"count(*) > 10) AS g "
		    "INNER JOIN creature c ON g.guid = c.guid "
		    "INNER JOIN creature_template t on c.id1 = t.entry "
        "ORDER BY "
	        "t.minlevel;", sPlayerbotAIConfig->randomBotMapsAsString.c_str());
    uint32 collected_locs = 0;
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint16 mapId = fields[0].Get<uint16>();
            float x = fields[1].Get<float>();
            float y = fields[2].Get<float>();
            float z = fields[3].Get<float>();
            uint32 level = fields[4].Get<uint32>();
            WorldLocation loc(mapId, x, y, z, 0);
            collected_locs++;
            for (int32 l = (int32)level - (int32)sPlayerbotAIConfig->randomBotTeleHigherLevel; l <= (int32)level + (int32)sPlayerbotAIConfig->randomBotTeleLowerLevel; l++) {
                if (l < 1 || l > maxLevel) {
                    continue;
                }
                locsPerLevelCache[(uint8)l].push_back(loc);
            }
        } while (results->NextRow());
    }
    LOG_INFO("playerbots", "{} locations for level collected.", collected_locs);
    
    results = WorldDatabase.Query(
    "SELECT "
		    "map, "
		    "position_x, "
		    "position_y, "
		    "position_z, "
            "orientation, "
		    "t.minlevel "
        "FROM "
		    "creature c "
		    "INNER JOIN creature_template t on c.id1 = t.entry "
        "WHERE "
            "t.npcflag & 131072 "
            "AND t.npcflag != 135298 "
            "AND t.minlevel != 55 "
            "AND t.minlevel != 65 "
            "AND t.faction != 35 "
            "AND t.faction != 474 "
            "AND t.faction != 69 "
            "AND map IN ({}) "
        "ORDER BY "
	        "t.minlevel;", sPlayerbotAIConfig->randomBotMapsAsString.c_str());
    collected_locs = 0;
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint16 mapId = fields[0].Get<uint16>();
            float x = fields[1].Get<float>();
            float y = fields[2].Get<float>();
            float z = fields[3].Get<float>();
            float orient = fields[4].Get<float>();
            uint32 level = fields[5].Get<uint32>();
            WorldLocation loc(mapId, x + cos(orient) * 10.0f, y + sin(orient) * 10.0f, z, orient + M_PI);
            collected_locs++;
            for (int32 l = 1; l <= maxLevel; l++) {
                if (l <= 60 && level >= 60) {
                    continue;
                }
                if (l <= 70 && level >= 70) {
                    continue;
                }
                if (l >= 70 && level >= 60 && level <= 70) {
                    continue;
                }
                if (l >= 30 && level <= 30) {
                    continue;
                }
                bankerLocsPerLevelCache[(uint8)l].push_back(loc);
            }
        } while (results->NextRow());
    }
    LOG_INFO("playerbots", "{} banker locations for level collected.", collected_locs);

    // temporary only use locsPerLevelCache, so disable rpgLocsCacheLevel cache

    // LOG_INFO("playerbots", "Preparing RPG teleport caches for {} factions...", sFactionTemplateStore.GetNumRows());
    // results = WorldDatabase.Query("SELECT map, position_x, position_y, position_z, r.race, r.minl, r.maxl FROM creature c INNER JOIN playerbots_rpg_races r ON c.id1 = r.entry "
    //     "WHERE r.race < 15");
    // if (results)
    // {
    //     do
    //     {
    //         Field* fields = results->Fetch();
    //         uint16 mapId = fields[0].Get<uint16>();
    //         float x = fields[1].Get<float>();
    //         float y = fields[2].Get<float>();
    //         float z = fields[3].Get<float>();
    //         uint32 race = fields[4].Get<uint32>();
    //         uint32 minl = fields[5].Get<uint32>();
    //         uint32 maxl = fields[6].Get<uint32>();

    //         for (uint32 level = 1; level < sPlayerbotAIConfig->randomBotMaxLevel + 1; level++)
    //         {
    //             if (level > maxl || level < minl)
    //                 continue;

    //             WorldLocation loc(mapId, x, y, z, 0);
    //             for (uint32 r = 1; r < MAX_RACES; r++)
    //             {
    //                 if (race == r || race == 0)
    //                     rpgLocsCacheLevel[r][level].push_back(loc);
    //             }
    //         }
    //     } while (results->NextRow());
    // }
}

void RandomPlayerbotMgr::RandomTeleportForLevel(Player* bot)
{
    if (bot->InBattleground())
        return;

    uint32 level = bot->getLevel();
    uint8 race = bot->getRace();
    LOG_INFO("playerbots", "Random teleporting bot {} for level {} ({} locations available)", bot->GetName().c_str(), bot->GetLevel(), locsPerLevelCache[level].size());
    if (urand(0, 100) < sPlayerbotAIConfig->probTeleToBankers * 100) {
        RandomTeleport(bot, bankerLocsPerLevelCache[level], true);
    } else {
        RandomTeleport(bot, locsPerLevelCache[level]);
    }
}

void RandomPlayerbotMgr::RandomTeleportGrindForLevel(Player* bot)
{
    if (bot->InBattleground())
        return;

    uint32 level = bot->getLevel();
    uint8 race = bot->getRace();
    LOG_INFO("playerbots", "Random teleporting bot {} for level {} ({} locations available)", bot->GetName().c_str(), bot->GetLevel(), locsPerLevelCache[level].size());

    RandomTeleport(bot, locsPerLevelCache[level]);
}

void RandomPlayerbotMgr::RandomTeleport(Player* bot)
{
    if (bot->InBattleground())
        return;

    PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "RandomTeleport");
    std::vector<WorldLocation> locs;

    std::list<Unit*> targets;
    float range = sPlayerbotAIConfig->randomBotTeleportDistance;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, range);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);

    if (!targets.empty())
    {
        for (Unit* unit : targets)
        {
            bot->UpdatePosition(*unit);
            FleeManager manager(bot, sPlayerbotAIConfig->sightDistance, 0, true);
            float rx, ry, rz;
            if (manager.CalculateDestination(&rx, &ry, &rz))
            {
                WorldLocation loc(bot->GetMapId(), rx, ry, rz);
                locs.push_back(loc);
            }
        }
    }
    else
    {
        RandomTeleportForLevel(bot);
    }

    if (pmo)
        pmo->finish();

    Refresh(bot);
}

void RandomPlayerbotMgr::Randomize(Player* bot)
{
    if (bot->InBattleground())
        return;

    if (bot->getLevel() < 3 || (bot->getLevel() < 56 && bot->getClass() == CLASS_DEATH_KNIGHT)) {
        RandomizeFirst(bot);
    }
    else if (bot->getLevel() < sPlayerbotAIConfig->randomBotMaxLevel || !sPlayerbotAIConfig->downgradeMaxLevelBot) {
        IncreaseLevel(bot);
    }
    else {
        RandomizeFirst(bot);
    }
    RandomTeleportForLevel(bot);
}

void RandomPlayerbotMgr::IncreaseLevel(Player* bot)
{
	uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
	if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
		maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

	PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "IncreaseLevel");
	uint32 lastLevel = GetValue(bot, "level");
	uint8 level = bot->getLevel() + 1;
    if (level > maxLevel) {
        level = maxLevel;
    }
	if (lastLevel != level)
	{
        PlayerbotFactory factory(bot, level);
        factory.Randomize(true);
	}

    if (pmo)
        pmo->finish();
}

void RandomPlayerbotMgr::RandomizeFirst(Player* bot)
{
	uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
	if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
		maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    // if lvl sync is enabled, max level is limited by online players lvl
    if (sPlayerbotAIConfig->syncLevelWithPlayers)
        maxLevel = std::max(sPlayerbotAIConfig->randomBotMinLevel, std::min(playersLevel, sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)));

	PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "RandomizeFirst");
    
    uint32 level;

    if (sPlayerbotAIConfig->downgradeMaxLevelBot && bot->GetLevel() == sPlayerbotAIConfig->randomBotMaxLevel) {
        if (bot->getClass() == CLASS_DEATH_KNIGHT) {
            level = sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL);
        } else {
            level = sPlayerbotAIConfig->randomBotMinLevel;
        }
    } else {
        level = urand(sPlayerbotAIConfig->randomBotMinLevel, maxLevel);
        if (urand(0, 100) < 100 * sPlayerbotAIConfig->randomBotMaxLevelChance)
            level = maxLevel;

        if (bot->getClass() == CLASS_DEATH_KNIGHT)
            level = urand(std::max(sPlayerbotAIConfig->randomBotMinLevel, sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL)), std::max(sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL), maxLevel));
    }

    if (sPlayerbotAIConfig->disableRandomLevels) {
        level = bot->getClass() == CLASS_DEATH_KNIGHT ? 
            std::max(sPlayerbotAIConfig->randombotStartingLevel, sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL)) : 
            sPlayerbotAIConfig->randombotStartingLevel;
    }

    SetValue(bot, "level", level);

    PlayerbotFactory factory(bot, level);
    factory.Randomize(false);

    uint32 randomTime = urand(sPlayerbotAIConfig->minRandomBotRandomizeTime, sPlayerbotAIConfig->maxRandomBotRandomizeTime);
    uint32 inworldTime = urand(sPlayerbotAIConfig->minRandomBotInWorldTime, sPlayerbotAIConfig->maxRandomBotInWorldTime);

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_RANDOM_BOTS);
    stmt->SetData(0, randomTime);
    stmt->SetData(1, "bot_delete");
    stmt->SetData(2, bot->GetGUID().GetCounter());
    PlayerbotsDatabase.Execute(stmt);

    stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_RANDOM_BOTS);
    stmt->SetData(0, inworldTime);
    stmt->SetData(1, "logout");
    stmt->SetData(2, bot->GetGUID().GetCounter());
    PlayerbotsDatabase.Execute(stmt);

    // teleport to a random inn for bot level
    if (GET_PLAYERBOT_AI(bot))
        GET_PLAYERBOT_AI(bot)->Reset(true);

    if (bot->GetGroup())
        bot->RemoveFromGroup();

	if (pmo)
        pmo->finish();
}

void RandomPlayerbotMgr::RandomizeMin(Player* bot)
{
	PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "RandomizeMin");
    
    uint32 level = sPlayerbotAIConfig->randomBotMinLevel;

    SetValue(bot, "level", level);

    PlayerbotFactory factory(bot, level);
    factory.Randomize(false);

    uint32 randomTime = urand(sPlayerbotAIConfig->minRandomBotRandomizeTime, sPlayerbotAIConfig->maxRandomBotRandomizeTime);
    uint32 inworldTime = urand(sPlayerbotAIConfig->minRandomBotInWorldTime, sPlayerbotAIConfig->maxRandomBotInWorldTime);

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_RANDOM_BOTS);
    stmt->SetData(0, randomTime);
    stmt->SetData(1, "bot_delete");
    stmt->SetData(2, bot->GetGUID().GetCounter());
    PlayerbotsDatabase.Execute(stmt);

    stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_RANDOM_BOTS);
    stmt->SetData(0, inworldTime);
    stmt->SetData(1, "logout");
    stmt->SetData(2, bot->GetGUID().GetCounter());
    PlayerbotsDatabase.Execute(stmt);

    // teleport to a random inn for bot level
    if (GET_PLAYERBOT_AI(bot))
        GET_PLAYERBOT_AI(bot)->Reset(true);

    if (bot->GetGroup())
        bot->RemoveFromGroup();

	if (pmo)
        pmo->finish();
}

void RandomPlayerbotMgr::Clear(Player* bot)
{
    PlayerbotFactory factory(bot, bot->GetLevel());
    factory.ClearEverything();
}

uint32 RandomPlayerbotMgr::GetZoneLevel(uint16 mapId, float teleX, float teleY, float teleZ)
{
	uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    uint32 level = 0;
    QueryResult results = WorldDatabase.Query("SELECT AVG(t.minlevel) minlevel, AVG(t.maxlevel) maxlevel FROM creature c "
            "INNER JOIN creature_template t ON c.id1 = t.entry WHERE map = {} AND minlevel > 1 AND ABS(position_x - {}) < {} AND ABS(position_y - {}) < {}",
            mapId, teleX, sPlayerbotAIConfig->randomBotTeleportDistance / 2, teleY, sPlayerbotAIConfig->randomBotTeleportDistance / 2);

    if (results)
    {
        Field* fields = results->Fetch();
        uint8 minLevel = fields[0].Get<uint8>();
        uint8 maxLevel = fields[1].Get<uint8>();
        level = urand(minLevel, maxLevel);
        if (level > maxLevel)
            level = maxLevel;
    }
    else
    {
        level = urand(1, maxLevel);
    }

    return level;
}

void RandomPlayerbotMgr::Refresh(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return;

    if (bot->isDead())
    {
        bot->ResurrectPlayer(1.0f);
        bot->SpawnCorpseBones();
        botAI->ResetStrategies(false);
    }

    // if (sPlayerbotAIConfig->disableRandomLevels)
    //     return;

    if (bot->InBattleground())
        return;

    LOG_INFO("playerbots", "Refreshing bot {} <{}>", bot->GetGUID().ToString().c_str(), bot->GetName().c_str());

    PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "Refresh");

    botAI->Reset();

    bot->DurabilityRepairAll(false, 1.0f, false);
	bot->SetFullHealth();
	bot->SetPvP(true);

    PlayerbotFactory factory(bot, bot->getLevel(), ITEM_QUALITY_RARE);
    factory.Refresh();

    if (bot->GetMaxPower(POWER_MANA) > 0)
        bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));

    if (bot->GetMaxPower(POWER_ENERGY) > 0)
        bot->SetPower(POWER_ENERGY, bot->GetMaxPower(POWER_ENERGY));

    uint32 money = bot->GetMoney();
    bot->SetMoney(money + 500 * sqrt(urand(1, bot->getLevel() * 5)));

    if (bot->GetGroup())
        bot->RemoveFromGroup();

    if (pmo)
        pmo->finish();
}

bool RandomPlayerbotMgr::IsRandomBot(Player* bot)
{
    if (bot && GET_PLAYERBOT_AI(bot))
    {
        if (GET_PLAYERBOT_AI(bot)->IsRealPlayer())
            return false;
    }
    if (bot)
    {
        return IsRandomBot(bot->GetGUID().GetCounter());
    }

    return false;
}

bool RandomPlayerbotMgr::IsRandomBot(ObjectGuid::LowType bot)
{
    ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(bot);
    if (!sPlayerbotAIConfig->IsInRandomAccountList(sCharacterCache->GetCharacterAccountIdByGuid(guid)))
        return false;
    if (std::find(currentBots.begin(), currentBots.end(), bot) != currentBots.end())
        return true;
    return false;
}

void RandomPlayerbotMgr::GetBots()
{
    if (!currentBots.empty())
        return;

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_RANDOM_BOTS_BY_OWNER_AND_EVENT);
    stmt->SetData(0, 0);
    stmt->SetData(1, "add");
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 bot = fields[0].Get<uint32>();
            if (GetEventValue(bot, "add"))
                currentBots.push_back(bot);
        }
        while (result->NextRow());
    }
}

std::vector<uint32> RandomPlayerbotMgr::GetBgBots(uint32 bracket)
{
    //if (!currentBgBots.empty()) return currentBgBots;

    std::vector<uint32> BgBots;

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_RANDOM_BOTS_BY_EVENT_AND_VALUE);
    stmt->SetData(0, "bg");
    stmt->SetData(1, bracket);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 bot = fields[0].Get<uint32>();
            BgBots.push_back(bot);
        }
        while (result->NextRow());
    }

    return std::move(BgBots);
}

uint32 RandomPlayerbotMgr::GetEventValue(uint32 bot, std::string const event)
{
    // load all events at once on first event load
    if (eventCache[bot].empty())
    {
        PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_RANDOM_BOTS_BY_OWNER_AND_BOT);
        stmt->SetData(0, 0);
        stmt->SetData(1, bot);
        if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
        {
            do
            {
                Field* fields = result->Fetch();
                std::string const eventName = fields[0].Get<std::string>();

                CachedEvent e;
                e.value = fields[1].Get<uint32>();
                e.lastChangeTime = fields[2].Get<uint32>();
                e.validIn = fields[3].Get<uint32>();
                e.data = fields[4].Get<std::string>();
                eventCache[bot][eventName] = std::move(e);
            }
            while (result->NextRow());
        }
    }

    CachedEvent& e = eventCache[bot][event];
    /*if (e.IsEmpty())
    {
        QueryResult results = PlayerbotsDatabase.Query("SELECT `value`, `time`, validIn, `data` FROM playerbots_random_bots WHERE owner = 0 AND bot = {} AND event = {}",
                bot, event.c_str());

        if (results)
        {
            Field* fields = results->Fetch();
            e.value = fields[0].Get<uint32>();
            e.lastChangeTime = fields[1].Get<uint32>();
            e.validIn = fields[2].Get<uint32>();
            e.data = fields[3].Get<std::string>();
        }
    }
    */

    if ((time(0) - e.lastChangeTime) >= e.validIn && event != "specNo" && event != "specLink")
        e.value = 0;

    return e.value;
}

std::string const RandomPlayerbotMgr::GetEventData(uint32 bot, std::string const event)
{
    std::string data = "";
    if (GetEventValue(bot, event))
    {
        CachedEvent e = eventCache[bot][event];
        data = e.data;
    }

    return data;
}

uint32 RandomPlayerbotMgr::SetEventValue(uint32 bot, std::string const event, uint32 value, uint32 validIn, std::string const data)
{
    PlayerbotsDatabaseTransaction trans = PlayerbotsDatabase.BeginTransaction();

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_RANDOM_BOTS_BY_OWNER_AND_EVENT);
    stmt->SetData(0, 0);
    stmt->SetData(1, bot);
    stmt->SetData(2, event.c_str());
    trans->Append(stmt);

    if (value)
    {
        stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_RANDOM_BOTS);
        stmt->SetData(0, 0);
        stmt->SetData(1, bot);
        stmt->SetData(2, static_cast<uint32>(GameTime::GetGameTime().count()));
        stmt->SetData(3, validIn);
        stmt->SetData(4, event.c_str());
        stmt->SetData(5, value);
        if (data != "")
        {
            stmt->SetData(6, data.c_str());
        }
        else
        {
            stmt->SetData(6);
        }
        trans->Append(stmt);
    }

    PlayerbotsDatabase.CommitTransaction(trans);

    CachedEvent e(value, (uint32)time(nullptr), validIn, data);
    eventCache[bot][event] = std::move(e);
    return value;
}

uint32 RandomPlayerbotMgr::GetValue(uint32 bot, std::string const type)
{
    return GetEventValue(bot, type);
}

uint32 RandomPlayerbotMgr::GetValue(Player* bot, std::string const type)
{
    return GetValue(bot->GetGUID().GetCounter(), type);
}

std::string const RandomPlayerbotMgr::GetData(uint32 bot, std::string const type)
{
    return GetEventData(bot, type);
}

void RandomPlayerbotMgr::SetValue(uint32 bot, std::string const type, uint32 value, std::string const data)
{
    SetEventValue(bot, type, value, sPlayerbotAIConfig->maxRandomBotInWorldTime, data);
}

void RandomPlayerbotMgr::SetValue(Player* bot, std::string const type, uint32 value, std::string const data)
{
    SetValue(bot->GetGUID().GetCounter(), type, value, data);
}

bool RandomPlayerbotMgr::HandlePlayerbotConsoleCommand(ChatHandler* handler, char const* args)
{
    if (!sPlayerbotAIConfig->enabled)
    {
        LOG_ERROR("playerbots", "Playerbots system is currently disabled!");
        return false;
    }

    if (!args || !*args)
    {
        LOG_ERROR("playerbots", "Usage: rndbot stats/update/reset/init/refresh/add/remove");
        return false;
    }

    std::string const cmd = args;

    if (cmd == "reset")
    {
        PlayerbotsDatabase.Execute(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_RANDOM_BOTS));
        sRandomPlayerbotMgr->eventCache.clear();
        LOG_INFO("playerbots", "Random bots were reset for all players. Please restart the Server.");
        return true;
    }

    if (cmd == "stats")
    {
        activatePrintStatsThread();
        return true;
    }

    if (cmd == "reload")
    {
        sPlayerbotAIConfig->Initialize();
        return true;
    }

    if (cmd == "update")
    {
        sRandomPlayerbotMgr->UpdateAIInternal(0);
        return true;
    }

    std::map<std::string, ConsoleCommandHandler> handlers;
    // handlers["initmin"] = &RandomPlayerbotMgr::RandomizeMin;
    handlers["init"] = &RandomPlayerbotMgr::RandomizeFirst;
    handlers["clear"] = &RandomPlayerbotMgr::Clear;
    handlers["levelup"] = handlers["level"] = &RandomPlayerbotMgr::IncreaseLevel;
    handlers["refresh"] = &RandomPlayerbotMgr::Refresh;
    handlers["teleport"] = &RandomPlayerbotMgr::RandomTeleportForLevel;
    // handlers["rpg"] = &RandomPlayerbotMgr::RandomTeleportForRpg;
    handlers["revive"] = &RandomPlayerbotMgr::Revive;
    handlers["grind"] = &RandomPlayerbotMgr::RandomTeleport;
    handlers["change_strategy"] = &RandomPlayerbotMgr::ChangeStrategy;

    for (std::map<std::string, ConsoleCommandHandler>::iterator j = handlers.begin(); j != handlers.end(); ++j)
    {
        std::string const prefix = j->first;
        if (cmd.find(prefix) != 0)
            continue;

        std::string const name = cmd.size() > prefix.size() + 1 ? cmd.substr(1 + prefix.size()) : "%";

        std::vector<uint32> botIds;
        for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotAccounts.begin(); i != sPlayerbotAIConfig->randomBotAccounts.end(); ++i)
        {
            uint32 account = *i;
            if (QueryResult results = CharacterDatabase.Query("SELECT guid FROM characters WHERE account = {} AND name like '{}'", account, name.c_str()))
            {
                do
                {
                    Field* fields = results->Fetch();

                    uint32 botId = fields[0].Get<uint32>();
                    ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(botId);
                    if (!sRandomPlayerbotMgr->IsRandomBot(guid.GetCounter())) {
                        continue;
                    }
                    Player* bot = ObjectAccessor::FindPlayer(guid);
                    if (!bot)
                        continue;

                    botIds.push_back(botId);
                } while (results->NextRow());
			}
        }

        if (botIds.empty())
        {
            LOG_INFO("playerbots", "Nothing to do");
            return false;
        }

        uint32 processed = 0;
        for (std::vector<uint32>::iterator i = botIds.begin(); i != botIds.end(); ++i)
        {
            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(*i);
            Player* bot = ObjectAccessor::FindPlayer(guid);
            if (!bot)
                continue;

            LOG_INFO("playerbots", "[{}/{}] Processing command {} for bot {}", processed++, botIds.size(), cmd.c_str(), bot->GetName().c_str());

            ConsoleCommandHandler handler = j->second;
            (sRandomPlayerbotMgr->*handler)(bot);
        }

        return true;
    }

    // std::vector<std::string> messages = sRandomPlayerbotMgr->HandlePlayerbotCommand(args);
    // for (std::vector<std::string>::iterator i = messages.begin(); i != messages.end(); ++i)
    // {
    //     LOG_INFO("playerbots", "{}", i->c_str());
    // }
    return true;
}

void RandomPlayerbotMgr::HandleCommand(uint32 type, std::string const text, Player* fromPlayer, std::string channelName)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;

        if (!channelName.empty())
        {
            if (ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId()))
            {
                Channel* chn = cMgr->GetChannel(channelName, bot);
                if (!chn)
                    continue;
            }
        }

        GET_PLAYERBOT_AI(bot)->HandleCommand(type, text, fromPlayer);
    }
}

void RandomPlayerbotMgr::OnPlayerLogout(Player* player)
{
    DisablePlayerBot(player->GetGUID());

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI && player == botAI->GetMaster())
        {
            botAI->SetMaster(nullptr);
            if (!bot->InBattleground())
            {
                botAI->ResetStrategies();
            }
        }
    }

    std::vector<Player*>::iterator i = std::find(players.begin(), players.end(), player);
    if (i != players.end())
        players.erase(i);
}

void RandomPlayerbotMgr::OnBotLoginInternal(Player * const bot)
{
    LOG_INFO("playerbots", "{}/{} Bot {} logged in", playerBots.size(), sRandomPlayerbotMgr->GetMaxAllowedBotCount(), bot->GetName().c_str());
}

void RandomPlayerbotMgr::OnPlayerLogin(Player* player)
{
    uint32 botsNearby = 0;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (player == bot/* || GET_PLAYERBOT_AI(player)*/) // TEST
            continue;

        Cell playerCell(player->GetPositionX(), player->GetPositionY());
        Cell botCell(bot->GetPositionX(), bot->GetPositionY());

        //if (playerCell == botCell)
            //botsNearby++;

        Group* group = bot->GetGroup();
        if (!group)
            continue;

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
            if (botAI && member == player && (!botAI->GetMaster() || GET_PLAYERBOT_AI(botAI->GetMaster())))
            {
                if (!bot->InBattleground())
                {
                    botAI->SetMaster(player);
                    botAI->ResetStrategies();
                    botAI->TellMaster("Hello");
                }

                break;
            }
        }
    }

    if (botsNearby > 100 && false)
    {
        WorldPosition botPos(player);

        //botPos.GetReachableRandomPointOnGround(player, sPlayerbotAIConfig->reactDistance * 2, true);

        //player->TeleportTo(botPos);
        //player->Relocate(botPos.coord_x, botPos.coord_y, botPos.coord_z, botPos.orientation);

        if (!player->GetFactionTemplateEntry())
        {
            botPos.GetReachableRandomPointOnGround(player, sPlayerbotAIConfig->reactDistance * 2, true);
        }
        else
        {
            std::vector<TravelDestination*> dests = sTravelMgr->getRpgTravelDestinations(player, true, true, 200000.0f);

            do
            {
                RpgTravelDestination* dest = (RpgTravelDestination*)dests[urand(0, dests.size() - 1)];
                CreatureTemplate const* cInfo = dest->GetCreatureTemplate();
                if (!cInfo)
                    continue;

                FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
                ReputationRank reaction = Unit::GetFactionReactionTo(player->GetFactionTemplateEntry(), factionEntry);

                if (reaction > REP_NEUTRAL && dest->nearestPoint(&botPos)->m_mapId == player->GetMapId())
                {
                    botPos = *dest->nearestPoint(&botPos);
                    break;
                }
            } while (true);
        }

        player->TeleportTo(botPos);

        // player->Relocate(botPos.getX(), botPos.getY(), botPos.getZ(), botPos.getO());
    }

    if (IsRandomBot(player))
    {
        ObjectGuid::LowType guid = player->GetGUID().GetCounter();
        SetEventValue(guid, "login", 0, 0);
    }
    else
    {
        players.push_back(player);
        LOG_DEBUG("playerbots", "Including non-random bot player {} into random bot update", player->GetName().c_str());
    }
}

void RandomPlayerbotMgr::OnPlayerLoginError(uint32 bot)
{
    SetEventValue(bot, "add", 0, 0);
    currentBots.erase(std::remove(currentBots.begin(), currentBots.end(), bot), currentBots.end());
}

Player* RandomPlayerbotMgr::GetRandomPlayer()
{
    if (players.empty())
        return nullptr;

    uint32 index = urand(0, players.size() - 1);
    return players[index];
}

void RandomPlayerbotMgr::PrintStats()
{
    LOG_INFO("playerbots", "{} Random Bots online", playerBots.size());

    std::map<uint8, uint32> alliance, horde;
    for (uint32 i = 0; i < 10; ++i)
    {
        alliance[i] = 0;
        horde[i] = 0;
    }

    std::map<uint8, uint32> perRace;
    std::map<uint8, uint32> perClass;
    for (uint8 race = RACE_HUMAN; race < MAX_RACES; ++race)
    {
        perRace[race] = 0;
    }

    for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
    {
        perClass[cls] = 0;
    }

    uint32 dps = 0;
    uint32 heal = 0;
    uint32 tank = 0;
    uint32 active = 0;
    uint32 update = 0;
    uint32 randomize = 0;
    uint32 teleport = 0;
    uint32 changeStrategy = 0;
    uint32 dead = 0;
    uint32 combat = 0;
    uint32 revive = 0;
    uint32 taxi = 0;
    uint32 moving = 0;
    uint32 mounted = 0;
    uint32 stateCount[MAX_TRAVEL_STATE + 1] = { 0 };
    std::vector<std::pair<Quest const*, int32>> questCount;
    for (PlayerBotMap::iterator i = playerBots.begin(); i != playerBots.end(); ++i)
    {
        Player* bot = i->second;
        if (IsAlliance(bot->getRace()))
            ++alliance[bot->getLevel() / 10];
        else
            ++horde[bot->getLevel() / 10];

        ++perRace[bot->getRace()];
        ++perClass[bot->getClass()];

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI->AllowActivity())
            ++active;

        if (botAI->GetAiObjectContext()->GetValue<bool>("random bot update")->Get())
            ++update;

        uint32 botId = bot->GetGUID().GetCounter();
        if (!GetEventValue(botId, "randomize"))
            ++randomize;

        if (!GetEventValue(botId, "teleport"))
            ++teleport;

        if (!GetEventValue(botId, "change_strategy"))
            ++changeStrategy;

        if (bot->isDead())
        {
            ++dead;
            //if (!GetEventValue(botId, "dead"))
                //++revive;
        }

        uint8 spec = AiFactory::GetPlayerSpecTab(bot);
        switch (bot->getClass())
        {
            case CLASS_DRUID:
                if (spec == 2)
                    ++heal;
                else
                    ++dps;
                break;
            case CLASS_PALADIN:
                if (spec == 1)
                    ++tank;
                else if (spec == 0)
                    ++heal;
                else
                    ++dps;
                break;
            case CLASS_PRIEST:
                if (spec != 2)
                    ++heal;
                else
                    ++dps;
                break;
            case CLASS_SHAMAN:
                if (spec == 2)
                    ++heal;
                else
                    ++dps;
                break;
            case CLASS_WARRIOR:
                if (spec == 2)
                    ++tank;
                else
                    ++dps;
                break;
            case CLASS_DEATH_KNIGHT:
                if (spec == 0)
                    tank++;
                else
                    dps++;
                break;
            default:
                ++dps;
                break;
        }

        if (TravelTarget* target = botAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get())
        {
            TravelState state = target->getTravelState();
            stateCount[state]++;

            Quest const* quest;
            if (target->getDestination())
                quest = target->getDestination()->GetQuestTemplate();

            if (quest)
            {
                bool found = false;

                for (auto& q : questCount)
                {
                    if (q.first != quest)
                        continue;

                    q.second++;
                    found = true;
                }

                if (!found)
                    questCount.push_back(std::make_pair(quest, 1));
            }
        }
    }

    LOG_INFO("playerbots", "Bots level:");
	uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
	for (uint8 i = 0; i < 10; ++i)
    {
        if (!alliance[i] && !horde[i])
            continue;

        uint32 from = i*10;
        uint32 to = std::min(from + 9, maxLevel);
        if (!from)
            from = 1;

        LOG_INFO("playerbots", "    {}..{}: {} alliance, {} horde", from, to, alliance[i], horde[i]);
    }

    LOG_INFO("playerbots", "Bots race:");
    for (uint8 race = RACE_HUMAN; race < MAX_RACES; ++race)
    {
        if (perRace[race])
            LOG_INFO("playerbots", "    {}: {}", ChatHelper::FormatRace(race).c_str(), perRace[race]);
    }

    LOG_INFO("playerbots", "Bots class:");
    for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
    {
        if (perClass[cls])
            LOG_INFO("playerbots", "    {}: {}", ChatHelper::FormatClass(cls).c_str(), perClass[cls]);
    }

    LOG_INFO("playerbots", "Bots role:");
    LOG_INFO("playerbots", "    tank: {}, heal: {}, dps: {}", tank, heal, dps);

    LOG_INFO("playerbots", "Bots status:");
    LOG_INFO("playerbots", "    Active: {}", active);
    LOG_INFO("playerbots", "    Moving: {}", moving);

    //LOG_INFO("playerbots", "Bots to:");
    //LOG_INFO("playerbots", "    update: {}", update);
    //LOG_INFO("playerbots", "    randomize: {}", randomize);
    //LOG_INFO("playerbots", "    teleport: {}", teleport);
    //LOG_INFO("playerbots", "    change_strategy: {}", changeStrategy);
    //LOG_INFO("playerbots", "    revive: {}", revive);

    LOG_INFO("playerbots", "    On taxi: {}", taxi);
    LOG_INFO("playerbots", "    On mount: {}", mounted);
    LOG_INFO("playerbots", "    In combat: {}", combat);
    LOG_INFO("playerbots", "    Dead: {}", dead);

    LOG_INFO("playerbots", "Bots questing:");
    LOG_INFO("playerbots", "    Picking quests: {}", stateCount[TRAVEL_STATE_TRAVEL_PICK_UP_QUEST] + stateCount[TRAVEL_STATE_WORK_PICK_UP_QUEST]);
    LOG_INFO("playerbots", "    Doing quests: {}", stateCount[TRAVEL_STATE_TRAVEL_DO_QUEST] + stateCount[TRAVEL_STATE_WORK_DO_QUEST]);
    LOG_INFO("playerbots", "    Completing quests: {}", stateCount[TRAVEL_STATE_TRAVEL_HAND_IN_QUEST] + stateCount[TRAVEL_STATE_WORK_HAND_IN_QUEST]);
    LOG_INFO("playerbots", "    Idling: {}", stateCount[TRAVEL_STATE_IDLE]);

    /*sort(questCount.begin(), questCount.end(), [](std::pair<Quest const*, int32> i, std::pair<Quest const*, int32> j) {return i.second > j.second; });

    LOG_INFO("playerbots", "Bots top quests:");

    uint32 cnt = 0;
    for (auto& quest : questCount)
    {
        LOG_INFO("playerbots", "    [{}]: {} ({})", quest.second, quest.first->GetTitle().c_str(), quest.first->GetQuestLevel());
        cnt++;
        if (cnt > 25)
            break;
    }
    */
}

double RandomPlayerbotMgr::GetBuyMultiplier(Player* bot)
{
    uint32 id = bot->GetGUID().GetCounter();
    uint32 value = GetEventValue(id, "buymultiplier");
    if (!value)
    {
        value = urand(50, 120);
        uint32 validIn = urand(sPlayerbotAIConfig->minRandomBotsPriceChangeInterval, sPlayerbotAIConfig->maxRandomBotsPriceChangeInterval);
        SetEventValue(id, "buymultiplier", value, validIn);
    }

    return (double)value / 100.0;
}

double RandomPlayerbotMgr::GetSellMultiplier(Player* bot)
{
    uint32 id = bot->GetGUID().GetCounter();
    uint32 value = GetEventValue(id, "sellmultiplier");
    if (!value)
    {
        value = urand(80, 250);
        uint32 validIn = urand(sPlayerbotAIConfig->minRandomBotsPriceChangeInterval, sPlayerbotAIConfig->maxRandomBotsPriceChangeInterval);
        SetEventValue(id, "sellmultiplier", value, validIn);
    }

    return (double)value / 100.0;
}

void RandomPlayerbotMgr::AddTradeDiscount(Player* bot, Player* master, int32 value)
{
    if (!master)
        return;

    uint32 discount = GetTradeDiscount(bot, master);
    int32 result = (int32)discount + value;
    discount = (result < 0 ? 0 : result);

    SetTradeDiscount(bot, master, discount);
}

void RandomPlayerbotMgr::SetTradeDiscount(Player* bot, Player* master, uint32 value)
{
    if (!master)
        return;

    uint32 botId =  bot->GetGUID().GetCounter();
    uint32 masterId =  master->GetGUID().GetCounter();

    std::ostringstream name;
    name << "trade_discount_" << masterId;
    SetEventValue(botId, name.str(), value, sPlayerbotAIConfig->maxRandomBotInWorldTime);
}

uint32 RandomPlayerbotMgr::GetTradeDiscount(Player* bot, Player* master)
{
    if (!master)
        return 0;

    uint32 botId =  bot->GetGUID().GetCounter();
    uint32 masterId = master->GetGUID().GetCounter();

    std::ostringstream name;
    name << "trade_discount_" << masterId;
    return GetEventValue(botId, name.str());
}

std::string const RandomPlayerbotMgr::HandleRemoteCommand(std::string const request)
{
    std::string::const_iterator pos = std::find(request.begin(), request.end(), ',');
    if (pos == request.end())
    {
        std::ostringstream out;
        out << "invalid request: " << request;
        return out.str();
    }

    std::string const command = std::string(request.begin(), pos);
    ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(atoi(std::string(pos + 1, request.end()).c_str()));
    Player* bot = GetPlayerBot(guid);
    if (!bot)
        return "invalid guid";

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return "invalid guid";

    return botAI->HandleRemoteCommand(command);
}

void RandomPlayerbotMgr::ChangeStrategy(Player* player)
{
    uint32 bot = player->GetGUID().GetCounter();

    if (frand(0.f, 100.f) > sPlayerbotAIConfig->randomBotRpgChance)
    {
        LOG_INFO("playerbots", "Bot #{} <{}>: sent to grind spot", bot, player->GetName().c_str());
        ScheduleTeleport(bot, 30);
    }
    else
    {
        LOG_INFO("playerbots", "Changing strategy for bot #{} <{}> to RPG", bot, player->GetName().c_str());
        LOG_INFO("playerbots", "Bot #{} <{}>: sent to inn", bot, player->GetName().c_str());
        RandomTeleportForLevel(player);
        SetEventValue(bot, "teleport", 1, sPlayerbotAIConfig->maxRandomBotInWorldTime);
    }

    ScheduleChangeStrategy(bot);
}

void RandomPlayerbotMgr::ChangeStrategyOnce(Player* player)
{
    uint32 bot = player->GetGUID().GetCounter();

    if (frand(0.f, 100.f) > sPlayerbotAIConfig->randomBotRpgChance) // select grind / pvp
    {
        LOG_INFO("playerbots", "Bot #{} <{}>: sent to grind spot", bot, player->GetName().c_str());
        RandomTeleportForLevel(player);
        Refresh(player);
    }
    else
    {
        LOG_INFO("playerbots", "Bot #{} <{}>: sent to inn", bot, player->GetName().c_str());
        RandomTeleportForLevel(player);
    }
}

void RandomPlayerbotMgr::RandomTeleportForRpg(Player* bot)
{
    uint32 race = bot->getRace();
    uint32 level = bot->GetLevel();
    LOG_INFO("playerbots", "Random teleporting bot {} for RPG ({} locations available)", bot->GetName().c_str(), rpgLocsCacheLevel[race].size());
    RandomTeleport(bot, rpgLocsCacheLevel[race][level], true);
}

void RandomPlayerbotMgr::Remove(Player* bot)
{
    ObjectGuid owner = bot->GetGUID();

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_RANDOM_BOTS_BY_OWNER);
    stmt->SetData(0, 0);
    stmt->SetData(1, owner.GetCounter());
    PlayerbotsDatabase.Execute(stmt);

    eventCache[owner.GetCounter()].clear();

    LogoutPlayerBot(owner);
}

CreatureData const* RandomPlayerbotMgr::GetCreatureDataByEntry(uint32 entry)
{
    if (entry != 0)
    {
        for (auto const& itr : sObjectMgr->GetAllCreatureData())
            if (itr.second.id1 == entry)
                return &itr.second;
    }

    return nullptr;
}

ObjectGuid const RandomPlayerbotMgr::GetBattleMasterGUID(Player* bot, BattlegroundTypeId bgTypeId)
{
    ObjectGuid battleMasterGUID = ObjectGuid::Empty;

    TeamId team = bot->GetTeamId();
    std::vector<uint32> Bms;

    for (auto i = std::begin(BattleMastersCache[team][bgTypeId]); i != std::end(BattleMastersCache[team][bgTypeId]); ++i)
    {
        Bms.insert(Bms.end(), *i);
    }

    for (auto i = std::begin(BattleMastersCache[TEAM_NEUTRAL][bgTypeId]); i != std::end(BattleMastersCache[TEAM_NEUTRAL][bgTypeId]); ++i)
    {
        Bms.insert(Bms.end(), *i);
    }

    if (Bms.empty())
        return battleMasterGUID;

    float dist1 = FLT_MAX;

    for (auto i = begin(Bms); i != end(Bms); ++i)
    {
        CreatureData const* data = sRandomPlayerbotMgr->GetCreatureDataByEntry(*i);
        if (!data)
            continue;

        Unit* Bm = PlayerbotAI::GetUnit(data);
        if (!Bm)
            continue;

        if (bot->GetMapId() != Bm->GetMapId())
            continue;

        // return first available guid on map if queue from anywhere
        if (!BattlegroundMgr::IsArenaType(bgTypeId))
        {
            battleMasterGUID = Bm->GetGUID();
            break;
        }

        AreaTableEntry const* zone = sAreaTableStore.LookupEntry(Bm->GetZoneId());
        if (!zone)
            continue;

        if (zone->team == 4 && bot->GetTeamId() == TEAM_ALLIANCE)
            continue;

        if (zone->team == 2 && bot->GetTeamId() == TEAM_HORDE)
            continue;

        if (Bm->getDeathState() == DeathState::Dead)
            continue;

        float dist2 = sServerFacade->GetDistance2d(bot, data->posX, data->posY);
        if (dist2 < dist1)
        {
            dist1 = dist2;
            battleMasterGUID = Bm->GetGUID();
        }
    }

    return battleMasterGUID;
}
