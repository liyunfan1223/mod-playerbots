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

#include "cs_playerbots.h"
#include "Channel.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "DatabaseLoader.h"
#include "GuildTaskMgr.h"
#include "Metric.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"

class PlayerbotsDatabaseScript : public DatabaseScript
{
    public:
        PlayerbotsDatabaseScript() : DatabaseScript("PlayerbotsDatabaseScript") { }

        bool OnDatabasesLoading() override
        {
            DatabaseLoader playerbotLoader("server.playerbots");
            playerbotLoader.SetUpdateFlags(sConfigMgr->GetOption<bool>("Playerbots.Updates.EnableDatabases", true) ? DatabaseLoader::DATABASE_PLAYERBOTS : 0);
            playerbotLoader.AddDatabase(PlayerbotsDatabase, "Playerbots");

            return playerbotLoader.Load();
        }

        void OnDatabasesKeepAlive() override
        {
            PlayerbotsDatabase.KeepAlive();
        }

        void OnDatabasesClosing() override
        {
            PlayerbotsDatabase.Close();
        }

        void OnDatabaseWarnAboutSyncQueries(bool apply) override
        {
            PlayerbotsDatabase.WarnAboutSyncQueries(apply);
        }

        void OnDatabaseSelectIndexLogout(Player* player, uint32& statementIndex, uint32& statementParam) override
        {
            statementIndex = CHAR_UPD_CHAR_ONLINE;
            statementParam = player->GetGUID().GetCounter();
        }

        void OnDatabaseGetDBRevision(std::string& revision) override
        {
            if (QueryResult resultPlayerbot = PlayerbotsDatabase.Query("SELECT date FROM version_db_playerbots ORDER BY date DESC LIMIT 1"))
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

class PlayerbotsMetricScript : public MetricScript
{
    public:
        PlayerbotsMetricScript() : MetricScript("PlayerbotsMetricScript") { }

        void OnMetricLogging() override
        {
            if (sMetric->IsEnabled())
            {
                sMetric->LogValue("db_queue_playerbots", uint64(PlayerbotsDatabase.QueueSize()), {});
            }
        }
};

class PlayerbotsPlayerScript : public PlayerScript
{
    public:
        PlayerbotsPlayerScript() : PlayerScript("PlayerbotsPlayerScript") { }

        void OnLogin(Player* player) override
        {
            if (!player->GetSession()->IsBot())
            {
                sPlayerbotsMgr->AddPlayerbotData(player, false);
                sRandomPlayerbotMgr->OnPlayerLogin(player);
            }
        }

        void OnAfterUpdate(Player* player, uint32 diff) override
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

        bool CanPlayerUseChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Player* receiver) override
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

        void OnChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Group* group) override
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

        void OnChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg) override
        {
            if (type == CHAT_MSG_GUILD)
            {
                if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                {
                    for (PlayerBotMap::const_iterator it = playerbotMgr->GetPlayerBotsBegin(); it != playerbotMgr->GetPlayerBotsEnd(); ++it)
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

        void OnChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Channel* channel) override
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

        bool OnBeforeCriteriaProgress(Player* player, AchievementCriteriaEntry const* /*criteria*/) override
        {
            if (sRandomPlayerbotMgr->IsRandomBot(player))
            {
                return false;
            }
            return true;
        }

        bool OnBeforeAchiComplete(Player* player, AchievementEntry const* /*achievement*/) override
        {
            if (sRandomPlayerbotMgr->IsRandomBot(player))
            {
                return false;
            }
            return true;
        }
};

class PlayerbotsMiscScript : public MiscScript
{
    public:
        PlayerbotsMiscScript() : MiscScript("PlayerbotsMiscScript") { }

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
        PlayerbotsServerScript() : ServerScript("PlayerbotsServerScript") { }

        void OnPacketReceived(WorldSession* session, WorldPacket& packet) override
        {
            if (Player* player = session->GetPlayer())
                if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                    playerbotMgr->HandleMasterIncomingPacket(packet);
        }
};

class PlayerbotsWorldScript : public WorldScript
{
    public:
        PlayerbotsWorldScript() : WorldScript("PlayerbotsWorldScript") { }

        void OnBeforeWorldInitialized() override
        {
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
        PlayerbotsScript() : PlayerbotScript("PlayerbotsScript") { }

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
            else if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
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
            sRandomPlayerbotMgr->LogoutAllBots();
        }
};

void AddPlayerbotsScripts()
{
    new PlayerbotsDatabaseScript();
    new PlayerbotsMetricScript();
    new PlayerbotsPlayerScript();
    new PlayerbotsMiscScript();
    new PlayerbotsServerScript();
    new PlayerbotsWorldScript();
    new PlayerbotsScript();

    AddSC_playerbots_commandscript();
}
