/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Playerbots.h"
#include "BattleGroundTactics.h"
#include "BattlefieldScript.h"
#include "Channel.h"
#include "CheckMountStateAction.h"
#include "Config.h"
#include "BuiltInConfig.h"
#include "DBUpdater.h"
#include "DatabaseEnv.h"
#include "PlayerbotsDatabase.h"
#include <mysqld_error.h>
#include "AllMapScript.h"
#include "GatherNodeMgr.h"
#include "GlobalScript.h"
#include "GuildTaskMgr.h"
#include "PlayerScript.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotCommandScript.h"
#include "PlayerbotGuildMgr.h"
#include "PlayerbotSpellRepository.h"
#include "PlayerbotWorldThreadProcessor.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "ServerScript.h"
#include "SessionScript.h"
#include "WorldScript.h"
#include "cmath"

class PlayerbotsDatabaseScript : public DatabaseScript
{
public:
    PlayerbotsDatabaseScript() : DatabaseScript("PlayerbotsDatabaseScript") {}

    bool OnModuleDatabasesLoading() override
    {
        std::string const dbString = sConfigMgr->GetOption<std::string>("PlayerbotsDatabaseInfo", "");
        if (dbString.empty())
        {
            LOG_ERROR("server.playerbots", "Playerbots database is not specified in configuration file");
            return false;
        }

        uint8 const workerThreads = sConfigMgr->GetOption<uint8>("PlayerbotsDatabase.WorkerThreads", 1);
        uint8 const synchThreads = sConfigMgr->GetOption<uint8>("PlayerbotsDatabase.SynchThreads", 2);
        PlayerbotsDatabase.SetConnectionInfo(dbString, workerThreads, synchThreads);

        bool const updatesEnabled = sConfigMgr->GetOption<bool>("Playerbots.Updates.EnableDatabases", true);
        if (updatesEnabled && !DBUpdaterUtil::CheckExecutable())
            return false;

        uint32 error = PlayerbotsDatabase.Open();
        if (error == ER_BAD_DB_ERROR && updatesEnabled)
        {
            // Database missing: create it through the mysql CLI and connect again
            if (!ModuleDBUpdater::Create(PlayerbotsDatabase))
                return false;

            error = PlayerbotsDatabase.Open();
        }

        if (error)
        {
            LOG_ERROR("server.playerbots", "Cannot connect to the playerbots database, error {}", error);
            return false;
        }

        if (updatesEnabled)
        {
            DBUpdaterInfo const info = {
                "Playerbots",
                BuiltInConfig::GetSourceDirectory() + "/modules/mod-playerbots",
                BuiltInConfig::GetSourceDirectory() + "/modules/mod-playerbots/data/sql/playerbots/base/",
                "db_playerbot"
            };

            if (!ModuleDBUpdater::Populate(PlayerbotsDatabase, info))
            {
                LOG_ERROR("server.playerbots", "Could not populate the playerbots database, see log for details.");
                return false;
            }

            if (!ModuleDBUpdater::Update(PlayerbotsDatabase, info))
            {
                LOG_ERROR("server.playerbots", "Could not update the playerbots database, see log for details.");
                return false;
            }
        }

        if (!PlayerbotsDatabase.PrepareStatements())
        {
            LOG_ERROR("server.playerbots", "Could not prepare statements of the playerbots database, see log for details.");
            return false;
        }

        return true;
    }

    void OnModuleDatabasesKeepAlive() override { PlayerbotsDatabase.KeepAlive(); }

    void OnModuleDatabasesClosing() override { PlayerbotsDatabase.Close(); }

    void OnDatabaseWarnAboutSyncQueries(bool apply) override { PlayerbotsDatabase.WarnAboutSyncQueries(apply); }

    void OnDatabaseGetDBRevision(std::map<std::string, std::string>& revisions) override
    {
        std::string revision;
        if (QueryResult resultPlayerbot =
                PlayerbotsDatabase.Query("SELECT date FROM version_db_playerbots ORDER BY date DESC LIMIT 1"))
        {
            Field* fields = resultPlayerbot->Fetch();
            revision = fields[0].Get<std::string>();
        }

        if (revision.empty())
            revision = "Unknown Playerbots Database Revision";

        revisions["Playerbots"] = revision;
    }
};

class PlayerbotsPlayerScript : public PlayerScript
{
public:
    PlayerbotsPlayerScript() : PlayerScript("PlayerbotsPlayerScript", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_BEFORE_LOGOUT,
        PLAYERHOOK_ON_AFTER_UPDATE,
        PLAYERHOOK_ON_CREATURE_KILL_CREDIT,
        PLAYERHOOK_ON_BEFORE_PETITION_SIGN,
        PLAYERHOOK_ON_BEFORE_CRITERIA_PROGRESS,
        PLAYERHOOK_ON_BEFORE_ACHI_COMPLETE,
        PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_GROUP_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_GUILD_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_CHANNEL_CHAT,
        PLAYERHOOK_ON_GIVE_EXP,
        PLAYERHOOK_ON_BEFORE_TELEPORT
    }) {}

    void OnPlayerLogin(Player* player) override
    {
        if (!player->GetSession()->IsHeadless())
        {
            PlayerbotsMgr::instance().AddPlayerbotData(player, false);
            sRandomPlayerbotMgr.OnPlayerLogin(player);

            // Before modifying the following messages, please make sure it does not violate the GNU GPLv2
            // license especially if you are distributing a repack or hosting a public server
            // e.g. you can replace the URL with your own repository,
            // but it should be publicly accessible and include all modifications you've made
            if (sPlayerbotAIConfig.Enabled)
            {
                ChatHandler(player->GetSession()).SendSysMessage(
                    "|cff00ff00This server runs with |cff00ccffmod-playerbots|r "
                    "|cffcccccchttps://github.com/mod-playerbots/mod-playerbots|r");
            }

            if (sPlayerbotAIConfig.Enabled || sPlayerbotAIConfig.RandomBotAutologin)
            {
                std::string maxAllowedBotCount = std::to_string(sRandomPlayerbotMgr.GetMaxAllowedBotCount());

                ChatHandler(player->GetSession()).SendSysMessage(
                    "|cff00ff00Playerbots:|r The server is configured with " + maxAllowedBotCount + " bots.");
            }
        }
    }

    void OnPlayerBeforeLogout(Player* player) override
    {
        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            PlayerbotAI* botAI = PlayerbotsMgr::instance().GetPlayerbotAI(player);

            if (botAI == nullptr || IsSelfBot(player))
                playerbotMgr->LogoutAllBots();
        }

        sRandomPlayerbotMgr.OnPlayerLogout(player);
    }

    void OnPlayerCreatureKillCredit(Player* player, Creature* killed) override
    {
        GuildTaskMgr::instance().CheckKillTask(player, killed);
    }

    void OnPlayerBeforePetitionSign(Player* player, ObjectGuid /*petitionGuid*/, bool& alreadySignedByAccount) override
    {
        if (!alreadySignedByAccount)
            return;

        // bots share accounts, so the same-account rule must not stop them from signing each other
        if (PlayerbotsMgr::instance().GetPlayerbotAI(player) != nullptr)
            alreadySignedByAccount = false;
    }

    bool OnPlayerBeforeTeleport(Player* /*player*/, uint32 /*mapid*/, float /*x*/, float /*y*/, float /*z*/,
                                float /*orientation*/, uint32 /*options*/, Unit* /*target*/) override
    {
        /* for now commmented out until proven its actually required
        * havent seen any proof CleanVisibilityReferences() is needed

        // If the player is not safe to touch, do nothing
        if (!player)
            return true;

        // If same map or not in world do nothing
        if (!player->IsInWorld() || player->GetMapId() == mapid)
            return true;

        // If this is a SelfBot, do nothing
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (!botAI || IsSelfBot(player))
            return true;

        // Cross-map bot teleport: defer visibility reference cleanup.
        // CleanVisibilityReferences() erases this bot's GUID from other objects' visibility containers.
        // This is intentionally done via the event queue (instead of directly here) because erasing
        // from other players' visibility maps inside the teleport call stack can hit unsafe re-entrancy
        // or iterator invalidation while visibility updates are in progress
        ObjectGuid guid = player->GetGUID();
        player->m_Events.AddEventAtOffset(
            [guid, mapid]()
            {
                // do nothing, if the player is not safe to touch
                Player* p = ObjectAccessor::FindPlayer(guid);
                if (!p || !p->IsInWorld() || p->IsDuringRemoveFromWorld())
                    return;

                // do nothing if we are already on the target map
                if (p->GetMapId() == mapid)
                    return;

                p->GetObjectVisibilityContainer().CleanVisibilityReferences();
            },
            Milliseconds(0));

        */

        return true;
    }

    void OnPlayerAfterUpdate(Player* player, uint32 diff) override
    {
        PlayerbotAI* const botAI = PlayerbotsMgr::instance().GetPlayerbotAI(player);

        if (botAI != nullptr)
        {
            botAI->UpdateAI(diff);
        }

        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            playerbotMgr->UpdateAI(diff);
        }
    }

    using PlayerScript::OnPlayerCanUseChat;  // keep the base overloads visible

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver) override
    {
        if (type != CHAT_MSG_WHISPER || lang == LANG_ADDON)
        {
            return true;
        }

        PlayerbotAI* const botAI = PlayerbotsMgr::instance().GetPlayerbotAI(receiver);

        if (botAI == nullptr)
        {
            return true;
        }

        botAI->HandleCommand(type, msg, player);

        // hotfix; otherwise the server will crash when whispering logout
        // https://github.com/mod-playerbots/mod-playerbots/pull/1838
        // TODO: find the root cause and solve it. (does not happen in party chat)
        if (msg == "logout")
            return false;

        return true;
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group) override
    {
        // Addon traffic (DBM, Carbonite, ...) is no bot command; its text used to trigger item commands like trade.
        if (lang == LANG_ADDON)
            return true;

        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* const member = itr->GetSource();

            if (member == nullptr)
                continue;

            PlayerbotAI* const botAI = PlayerbotsMgr::instance().GetPlayerbotAI(member);

            if (botAI == nullptr)
                continue;

            botAI->HandleCommand(type, msg, player);
        }

        return true;
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* /*guild*/) override
    {
        if (type != CHAT_MSG_GUILD || lang == LANG_ADDON)
            return true;

        PlayerbotMgr* playerbotMgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);

        if (playerbotMgr == nullptr)
            return true;

        for (PlayerBotMap::const_iterator it = playerbotMgr->GetPlayerBotsBegin(); it != playerbotMgr->GetPlayerBotsEnd(); ++it)
        {
            Player* const bot = it->second;

            if (bot == nullptr)
                continue;

            if (bot->GetGuildId() != player->GetGuildId())
                continue;

            PlayerbotsMgr::instance().GetPlayerbotAI(bot)->HandleCommand(type, msg, player);
        }

        return true;
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Channel* channel) override
    {
        PlayerbotMgr* const playerbotMgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);

        if (playerbotMgr != nullptr && channel->GetFlags() & 0x18)
            playerbotMgr->HandleCommand(type, msg);

        sRandomPlayerbotMgr.HandleCommand(type, msg, player);

        return true;
    }

    bool OnPlayerBeforeAchievementComplete(Player* player, AchievementEntry const* achievement) override
    {
        if ((sRandomPlayerbotMgr.IsRandomBot(player) || sRandomPlayerbotMgr.IsAddclassBot(player)) &&
            (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL)))
        {
            return false;
        }

        return true;
    }

    void OnPlayerGiveXP(Player* player, uint32& amount, Unit* /*victim*/, uint8 /*xpSource*/) override
    {
        // early return
        if (sPlayerbotAIConfig.RandomBotXPRate == 1.0 || !player)
            return;

        // no XP multiplier, when player is no bot.
        if (!player->GetSession()->IsHeadless() || !sRandomPlayerbotMgr.IsRandomBot(player))
            return;

        // no XP multiplier, when bot is in a group with a real player.
        if (Group* group = player->GetGroup())
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* member = gref->GetSource();
                if (!member)
                    continue;

                if (!member->GetSession()->IsHeadless())
                    return;
            }
        }

        // otherwise apply bot XP multiplier.
        amount = static_cast<uint32>(std::round(static_cast<float>(amount) * sPlayerbotAIConfig.RandomBotXPRate));
    }
};

class PlayerbotsMiscScript : public MiscScript
{
public:
    PlayerbotsMiscScript() : MiscScript("PlayerbotsMiscScript", {MISCHOOK_ON_DESTRUCT_PLAYER}) {}

    void OnDestructPlayer(Player* player) override
    {
        PlayerbotAI* botAI = PlayerbotsMgr::instance().GetPlayerbotAI(player);

        if (botAI != nullptr)
            delete botAI;

        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
            delete playerbotMgr;
    }
};

class PlayerbotsServerScript : public ServerScript
{
public:
    PlayerbotsServerScript() : ServerScript("PlayerbotsServerScript", {
        SERVERHOOK_ON_PACKET_SENT,
        SERVERHOOK_CAN_PACKET_RECEIVE
    }) {}

    void OnPacketSent(WorldSession* session, WorldPacket const& packet) override
    {
        Player* player = session->GetPlayer();

        if (player == nullptr)
            return;

        PlayerbotAI* botAI = PlayerbotsMgr::instance().GetPlayerbotAI(player);

        if (botAI != nullptr)
            botAI->HandleBotOutgoingPacket(packet);

        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
            playerbotMgr->HandleMasterOutgoingPacket(packet);
    }

    bool CanPacketReceive(WorldSession* session, WorldPacket const& packet) override
    {
        if (Player* player = session->GetPlayer())
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                playerbotMgr->HandleMasterIncomingPacket(packet);

        return true;
    }
};

class PlayerbotsSessionScript : public SessionScript
{
public:
    PlayerbotsSessionScript() : SessionScript("PlayerbotsSessionScript", {
        SESSIONHOOK_ON_UPDATE
    }) {}

    // Runs on the world thread for every real session: drains the packet queues of the bots it owns
    void OnSessionUpdate(WorldSession* session, uint32 /*diff*/) override
    {
        if (Player* player = session->GetPlayer())
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                playerbotMgr->UpdateSessions();
    }
};

class PlayerbotsAllMapScript : public AllMapScript
{
public:
    PlayerbotsAllMapScript() : AllMapScript("PlayerbotsAllMapScript", {
        ALLMAPHOOK_CAN_SEND_OBJECT_UPDATES_TO_PLAYER
    }) {}

    // Bots have no client to render object updates; only self bots still need them
    bool CanSendObjectUpdatesToPlayer(Map* /*map*/, Player* player) override
    {
        PlayerbotAI* botAI = PlayerbotsMgr::instance().GetPlayerbotAI(player);

        if (botAI == nullptr)
            return true;

        return IsSelfBot(player);
    }
};

class PlayerbotsGlobalScript : public GlobalScript
{
public:
    PlayerbotsGlobalScript() : GlobalScript("PlayerbotsGlobalScript", {
        GLOBALHOOK_CAN_CREATE_LFG_PROPOSAL
    }) {}

    // Never form a dungeon group made only of bots
    bool CanCreateLfgProposal(lfg::Lfg5Guids const& guids) override
    {
        for (ObjectGuid const& guid : guids.guids)
        {
            Player* player = ObjectAccessor::FindPlayer(guid);

            if (guid.IsGroup() || IsRealPlayer(player) || IsSelfBot(player))
                return true;
        }

        return false;
    }
};

class PlayerbotsWorldScript : public WorldScript
{
public:
    PlayerbotsWorldScript() : WorldScript("PlayerbotsWorldScript", {
        WORLDHOOK_ON_BEFORE_WORLD_INITIALIZED,
        WORLDHOOK_ON_UPDATE,
        WORLDHOOK_ON_SHUTDOWN
    }) {}

    void OnBeforeWorldInitialized() override
    {
        // Before modifying the following messages, please make sure it does not violate the GNU GPLv2
        // license especially if you are distributing a repack or hosting a public server
        // e.g. you can replace the URL with your own repository,
        // but it should be publicly accessible and include all modifications you've made
        LOG_INFO("server.loading", "╔══════════════════════════════════════════════════════════╗");
        LOG_INFO("server.loading", "║                                                          ║");
        LOG_INFO("server.loading", "║              AzerothCore Playerbots Module               ║");
        LOG_INFO("server.loading", "║                                                          ║");
        LOG_INFO("server.loading", "╟──────────────────────────────────────────────────────────╢");
        LOG_INFO("server.loading", "║     mod-playerbots is a community-driven open-source     ║");
        LOG_INFO("server.loading", "║  project based on AzerothCore, licensed under GNU GPLv2  ║");
        LOG_INFO("server.loading", "╟──────────────────────────────────────────────────────────╢");
        LOG_INFO("server.loading", "║     https://github.com/mod-playerbots/mod-playerbots     ║");
        LOG_INFO("server.loading", "╚══════════════════════════════════════════════════════════╝");

        uint32 oldMSTime = getMSTime();

        LOG_INFO("server.loading", " ");
        LOG_INFO("server.loading", "Load Playerbots Config...");

        sPlayerbotAIConfig.Initialize();

        LOG_INFO("server.loading", ">> Loaded playerbots config in {} ms", GetMSTimeDiffToNow(oldMSTime));
        LOG_INFO("server.loading", " ");

        PlayerbotSpellRepository::Instance().Initialize();
        CheckMountStateAction::LoadPreferredMounts();

        LOG_INFO("server.loading", "Loading gather node index...");
        GatherNodeMgr::instance().Load();

        LOG_INFO("server.loading", "Playerbots World Thread Processor initialized");
    }

    void OnUpdate(uint32 diff) override
    {
        PlayerbotHolder::UpdatePendingLogins();  // Headless sessions whose login holder is in flight
        sRandomPlayerbotMgr.UpdateSessions();  // Per-bot packet queues, world thread only
        PlayerbotWorldThreadProcessor::instance().Update(diff);
        sRandomPlayerbotMgr.UpdateAI(diff);  // World thread only
    }

    // Runs before the sessions are kicked on server shutdown
    void OnShutdown() override
    {
        LOG_INFO("playerbots", "Logging out all bots...");
        sRandomPlayerbotMgr.LogoutAllBots();
        PlayerbotHolder::ClearPendingLogins();
    }
};

class PlayerBotsBGScript : public BGScript
{
public:
    PlayerBotsBGScript() : BGScript("PlayerBotsBGScript") {}

    void OnBattlegroundStart(Battleground* bg) override
    {
        BGStrategyData data;

        BattlegroundTypeId bgType = bg->GetBgTypeID();
        if (bgType == BATTLEGROUND_RB)  // a random-queue game: the rolled map
            bgType = bg->GetBgTypeID(true);

        switch (bgType)
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

// Workaround for missing InitEnabledHooksIfNeeded for new BattlefieldScript in ScriptMgr
class PlayerbotsBattlefieldScript : public BattlefieldScript
{
public:
    PlayerbotsBattlefieldScript() : BattlefieldScript("PlayerbotsBattlefieldScript") { }
};

void AddPlayerbotsSecureLoginScripts();
void AddPlayerbotsSelfBotAfkScripts();

void AddSC_MagtheridonBotScripts();
void AddSC_TempestKeepBotScripts();
void AddSC_HyjalBotScripts();
void AddSC_IcecrownBotScripts();
void AddSC_RubySanctumBotScripts();
void AddSC_randombot_level_mgr();

void AddPlayerbotsScripts()
{
    new PlayerbotsBattlefieldScript();
    new PlayerbotsDatabaseScript();
    new PlayerbotsPlayerScript();
    new PlayerbotsMiscScript();
    new PlayerbotsServerScript();
    new PlayerbotsSessionScript();
    new PlayerbotsAllMapScript();
    new PlayerbotsGlobalScript();
    new PlayerbotsWorldScript();
    new PlayerBotsBGScript();
    AddPlayerbotsSecureLoginScripts();
    AddPlayerbotsSelfBotAfkScripts();
    AddPlayerbotsCommandscripts();
    PlayerBotsGuildValidationScript();
    AddSC_MagtheridonBotScripts();
    AddSC_TempestKeepBotScripts();
    AddSC_HyjalBotScripts();
    AddSC_IcecrownBotScripts();
    AddSC_RubySanctumBotScripts();
    AddSC_randombot_level_mgr();
}
