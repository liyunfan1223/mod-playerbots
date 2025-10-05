/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Playerbots.h"

#include "Channel.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "DatabaseLoader.h"
#include "GuildTaskMgr.h"
#include "Metric.h"
#include "PlayerScript.h"
#include "PlayerbotAIConfig.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "cs_playerbots.h"
#include "cmath"
#include "BattleGroundTactics.h"

class PlayerbotsDatabaseScript : public DatabaseScript
{
public:
    PlayerbotsDatabaseScript() : DatabaseScript("PlayerbotsDatabaseScript") {}

    bool OnDatabasesLoading() override
    {
        DatabaseLoader playerbotLoader("server.playerbots");
        playerbotLoader.SetUpdateFlags(sConfigMgr->GetOption<bool>("Playerbots.Updates.EnableDatabases", true)
                                           ? DatabaseLoader::DATABASE_PLAYERBOTS
                                           : 0);
        playerbotLoader.AddDatabase(PlayerbotsDatabase, "Playerbots");

        return playerbotLoader.Load();
    }

    void OnDatabasesKeepAlive() override { PlayerbotsDatabase.KeepAlive(); }

    void OnDatabasesClosing() override { PlayerbotsDatabase.Close(); }

    void OnDatabaseWarnAboutSyncQueries(bool apply) override { PlayerbotsDatabase.WarnAboutSyncQueries(apply); }

    void OnDatabaseSelectIndexLogout(Player* player, uint32& statementIndex, uint32& statementParam) override
    {
        statementIndex = CHAR_UPD_CHAR_OFFLINE;
        statementParam = player->GetGUID().GetCounter();
    }

    void OnDatabaseGetDBRevision(std::string& revision) override
    {
        if (QueryResult resultPlayerbot =
                PlayerbotsDatabase.Query("SELECT date FROM version_db_playerbots ORDER BY date DESC LIMIT 1"))
        {
            Field* fields = resultPlayerbot->Fetch();
            revision = fields[0].Get<std::string>();
        }

        if (revision.empty())
        {
            revision = "Unknown Playerbots Database Revision";
        }
    }
};

class PlayerbotsPlayerScript : public PlayerScript
{
public:
    PlayerbotsPlayerScript() : PlayerScript("PlayerbotsPlayerScript", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_AFTER_UPDATE,
        PLAYERHOOK_ON_CHAT,
        PLAYERHOOK_ON_CHAT_WITH_CHANNEL,
        PLAYERHOOK_ON_CHAT_WITH_GROUP,
        PLAYERHOOK_ON_BEFORE_CRITERIA_PROGRESS,
        PLAYERHOOK_ON_BEFORE_ACHI_COMPLETE,
        PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT,
        PLAYERHOOK_ON_GIVE_EXP,
        PLAYERHOOK_ON_BEFORE_TELEPORT
    }) {}

    void OnPlayerLogin(Player* player) override
    {
        if (!player->GetSession()->IsBot())
        {
            sPlayerbotsMgr->AddPlayerbotData(player, false);
            sRandomPlayerbotMgr->OnPlayerLogin(player);

            // Before modifying the following messages, please make sure it does not violate the AGPLv3.0 license
            // especially if you are distributing a repack or hosting a public server
            // e.g. you can replace the URL with your own repository,
            // but it should be publicly accessible and include all modifications you've made
            if (sPlayerbotAIConfig->enabled)
            {
                ChatHandler(player->GetSession()).SendSysMessage(
                    "|cff00ff00This server runs with |cff00ccffmod-playerbots|r "
                    "|cffcccccchttps://github.com/liyunfan1223/mod-playerbots|r");
            }

            if (sPlayerbotAIConfig->enabled || sPlayerbotAIConfig->randomBotAutologin)
            {
                std::string roundedTime =
                    std::to_string(std::ceil((sPlayerbotAIConfig->maxRandomBots * 0.11 / 60) * 10) / 10.0);
                roundedTime = roundedTime.substr(0, roundedTime.find('.') + 2);

                ChatHandler(player->GetSession()).SendSysMessage(
                    "|cff00ff00Playerbots:|r bot initialization at server startup takes about '"
                    + roundedTime + "' minutes.");
            }
        }
    }

    bool OnPlayerBeforeTeleport(Player* player, uint32 mapid, float /*x*/, float /*y*/, float /*z*/, float /*orientation*/, uint32 /*options*/, Unit* /*target*/) override
    {
        // Only apply to bots to prevent affecting real players
        if (!player || !player->GetSession()->IsBot())
            return true;

        // If changing maps, proactively clean visibility references to prevent
        // stale pointers in other players' visibility maps during the teleport.
        // This fixes a race condition where:
        // 1. Bot A teleports and its visible objects start getting cleaned up
        // 2. Bot B is simultaneously updating visibility and tries to access objects in Bot A's old visibility map
        // 3. Those objects may already be freed, causing a segmentation fault
        if (player->GetMapId() != mapid && player->IsInWorld())
        {
            player->GetObjectVisibilityContainer().CleanVisibilityReferences();
        }

        return true;  // Allow teleport to continue
    }

    void OnPlayerAfterUpdate(Player* player, uint32 diff) override
    {
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
        {
            botAI->UpdateAI(diff);
        }

        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            playerbotMgr->UpdateAI(diff);
        }
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Player* receiver) override
    {
        if (type == CHAT_MSG_WHISPER)
        {
            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(receiver))
            {
                botAI->HandleCommand(type, msg, player);

                return false;
            }
        }

        return true;
    }

    void OnPlayerChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Group* group) override
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (Player* member = itr->GetSource())
            {
                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(member))
                {
                    botAI->HandleCommand(type, msg, player);
                }
            }
        }
    }

    void OnPlayerChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg) override
    {
        if (type == CHAT_MSG_GUILD)
        {
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
            {
                for (PlayerBotMap::const_iterator it = playerbotMgr->GetPlayerBotsBegin();
                     it != playerbotMgr->GetPlayerBotsEnd(); ++it)
                {
                    if (Player* const bot = it->second)
                    {
                        if (bot->GetGuildId() == player->GetGuildId())
                        {
                            GET_PLAYERBOT_AI(bot)->HandleCommand(type, msg, player);
                        }
                    }
                }
            }
        }
    }

    void OnPlayerChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Channel* channel) override
    {
        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            if (channel->GetFlags() & 0x18)
            {
                playerbotMgr->HandleCommand(type, msg);
            }
        }

        sRandomPlayerbotMgr->HandleCommand(type, msg, player);
    }

    bool OnPlayerBeforeAchievementComplete(Player* player, AchievementEntry const* achievement) override
    {
        if (sRandomPlayerbotMgr->IsRandomBot(player) && (achievement->flags == 256 || achievement->flags == 768))
        {
            return false;
        }

        return true;
    }

    void OnPlayerGiveXP(Player* player, uint32& amount, Unit* /*victim*/, uint8 /*xpSource*/) override
    {
        // early return
        if (sPlayerbotAIConfig->randomBotXPRate == 1.0 || !player)
            return;

        // no XP multiplier, when player is no bot.
        if (!player->GetSession()->IsBot() || !sRandomPlayerbotMgr->IsRandomBot(player))
            return;

        // no XP multiplier, when bot is in a group with a real player.
        if (Group* group = player->GetGroup())
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* member = gref->GetSource();
                if (!member)
                {
                    continue;
                }

                if (!member->GetSession()->IsBot())
                {
                    return;
                }
            }
        }

        // otherwise apply bot XP multiplier.
        amount = static_cast<uint32>(std::round(static_cast<float>(amount) * sPlayerbotAIConfig->randomBotXPRate));
    }
};

class PlayerbotsMiscScript : public MiscScript
{
public:
    PlayerbotsMiscScript() : MiscScript("PlayerbotsMiscScript", {MISCHOOK_ON_DESTRUCT_PLAYER}) {}

    void OnDestructPlayer(Player* player) override
    {
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
        {
            delete botAI;
        }

        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            delete playerbotMgr;
        }
    }
};

class PlayerbotsServerScript : public ServerScript
{
public:
    PlayerbotsServerScript() : ServerScript("PlayerbotsServerScript", {
        SERVERHOOK_CAN_PACKET_RECEIVE
    }) {}

    void OnPacketReceived(WorldSession* session, WorldPacket const& packet) override
    {
        if (Player* player = session->GetPlayer())
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                playerbotMgr->HandleMasterIncomingPacket(packet);
    }
};

class PlayerbotsWorldScript : public WorldScript
{
public:
    PlayerbotsWorldScript() : WorldScript("PlayerbotsWorldScript", {
        WORLDHOOK_ON_BEFORE_WORLD_INITIALIZED
    }) {}

    void OnBeforeWorldInitialized() override
    {
        // Before modifying the following messages, please make sure it does not violate the AGPLv3.0 license
        // especially if you are distributing a repack or hosting a public server
        // e.g. you can replace the URL with your own repository,
        // but it should be publicly accessible and include all modifications you've made
        LOG_INFO("server.loading", "╔══════════════════════════════════════════════════════════╗");
        LOG_INFO("server.loading", "║                                                          ║");
        LOG_INFO("server.loading", "║              AzerothCore Playerbots Module               ║");
        LOG_INFO("server.loading", "║                                                          ║");
        LOG_INFO("server.loading", "╟──────────────────────────────────────────────────────────╢");
        LOG_INFO("server.loading", "║     mod-playerbots is a community-driven open-source     ║");
        LOG_INFO("server.loading", "║  project based on AzerothCore, licensed under AGPLv3.0   ║");
        LOG_INFO("server.loading", "╟──────────────────────────────────────────────────────────╢");
        LOG_INFO("server.loading", "║      https://github.com/liyunfan1223/mod-playerbots      ║");
        LOG_INFO("server.loading", "╚══════════════════════════════════════════════════════════╝");

        uint32 oldMSTime = getMSTime();

        LOG_INFO("server.loading", " ");
        LOG_INFO("server.loading", "Load Playerbots Config...");

        sPlayerbotAIConfig->Initialize();

        LOG_INFO("server.loading", ">> Loaded playerbots config in {} ms", GetMSTimeDiffToNow(oldMSTime));
        LOG_INFO("server.loading", " ");
    }
};

class PlayerbotsScript : public PlayerbotScript
{
public:
    PlayerbotsScript() : PlayerbotScript("PlayerbotsScript") {}

    bool OnPlayerbotCheckLFGQueue(lfg::Lfg5Guids const& guidsList) override
    {
        bool nonBotFound = false;
        for (ObjectGuid const& guid : guidsList.guids)
        {
            Player* player = ObjectAccessor::FindPlayer(guid);
            if (guid.IsGroup() || (player && !GET_PLAYERBOT_AI(player)))
            {
                nonBotFound = true;
                break;
            }
        }

        return nonBotFound;
    }

    void OnPlayerbotCheckKillTask(Player* player, Unit* victim) override
    {
        if (player)
            sGuildTaskMgr->CheckKillTask(player, victim);
    }

    void OnPlayerbotCheckPetitionAccount(Player* player, bool& found) override
    {
        if (found && GET_PLAYERBOT_AI(player))
            found = false;
    }

    bool OnPlayerbotCheckUpdatesToSend(Player* player) override
    {
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
            return botAI->IsRealPlayer();

        return true;
    }

    void OnPlayerbotPacketSent(Player* player, WorldPacket const* packet) override
    {
        if (!player)
            return;

        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
        {
            botAI->HandleBotOutgoingPacket(*packet);
        }
        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            playerbotMgr->HandleMasterOutgoingPacket(*packet);
        }
    }

    void OnPlayerbotUpdate(uint32 diff) override
    {
        sRandomPlayerbotMgr->UpdateAI(diff);
        sRandomPlayerbotMgr->UpdateSessions();
    }

    void OnPlayerbotUpdateSessions(Player* player) override
    {
        if (player)
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                playerbotMgr->UpdateSessions();
    }

    void OnPlayerbotLogout(Player* player) override
    {
        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || botAI->IsRealPlayer())
            {
                playerbotMgr->LogoutAllBots();
            }
        }

        sRandomPlayerbotMgr->OnPlayerLogout(player);
    }

    void OnPlayerbotLogoutBots() override
    {
        LOG_INFO("playerbots", "Logging out all bots...");
        sRandomPlayerbotMgr->LogoutAllBots();
    }
};

class PlayerBotsBGScript : public BGScript
{
public:
    PlayerBotsBGScript() : BGScript("PlayerBotsBGScript") {}

    void OnBattlegroundStart(Battleground* bg) override
    {
        BGStrategyData data;

        switch (bg->GetBgTypeID())
        {
            case BATTLEGROUND_WS:
                data.allianceStrategy = urand(0, WS_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, WS_STRATEGY_MAX - 1);
                break;
            case BATTLEGROUND_AB:
                data.allianceStrategy = urand(0, AB_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, AB_STRATEGY_MAX - 1);
                break;
            case BATTLEGROUND_AV:
                data.allianceStrategy = urand(0, AV_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, AV_STRATEGY_MAX - 1);
                break;
            case BATTLEGROUND_EY:
                data.allianceStrategy = urand(0, EY_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, EY_STRATEGY_MAX - 1);
                break;
            default:
                break;
        }

        bgStrategies[bg->GetInstanceID()] = data;
    }

    void OnBattlegroundEnd(Battleground* bg, TeamId /*winnerTeam*/) override { bgStrategies.erase(bg->GetInstanceID()); }
};

void AddPlayerbotsScripts()
{
    new PlayerbotsDatabaseScript();
    new PlayerbotsPlayerScript();
    new PlayerbotsMiscScript();
    new PlayerbotsServerScript();
    new PlayerbotsWorldScript();
    new PlayerbotsScript();
    new PlayerBotsBGScript();

    AddSC_playerbots_commandscript();
}
