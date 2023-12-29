/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CharacterCache.h"
#include "CharacterPackets.h"
#include "Common.h"
#include "Define.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotMgr.h"
#include "PlayerbotSecurity.h"
#include "Playerbots.h"
#include "PlayerbotDbStore.h"
#include "PlayerbotFactory.h"
#include "SharedDefines.h"
#include "WorldSession.h"
#include "ChannelMgr.h"
#include <cstdio>
#include <cstring>
#include <string>

PlayerbotHolder::PlayerbotHolder() : PlayerbotAIBase(false)
{
}

class PlayerbotLoginQueryHolder : public LoginQueryHolder
{
    private:
        uint32 masterAccountId;
        PlayerbotHolder* playerbotHolder;

    public:
        PlayerbotLoginQueryHolder(PlayerbotHolder* playerbotHolder, uint32 masterAccount, uint32 accountId, ObjectGuid guid)
            : LoginQueryHolder(accountId, guid), masterAccountId(masterAccount), playerbotHolder(playerbotHolder) { }

        uint32 GetMasterAccountId() const { return masterAccountId; }
        PlayerbotHolder* GetPlayerbotHolder() { return playerbotHolder; }
};

void PlayerbotHolder::AddPlayerBot(ObjectGuid playerGuid, uint32 masterAccountId)
{
    // has bot already been added?
    Player* bot = ObjectAccessor::FindConnectedPlayer(playerGuid);
    if (bot && bot->IsInWorld())
        return;

    uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(playerGuid);
    if (!accountId)
        return;

    std::shared_ptr<PlayerbotLoginQueryHolder> holder = std::make_shared<PlayerbotLoginQueryHolder>(this, masterAccountId, accountId, playerGuid);
    if (!holder->Initialize())
    {
        return;
    }

    if (WorldSession* masterSession = sWorld->FindSession(masterAccountId))
    {
        masterSession->AddQueryHolderCallback(CharacterDatabase.DelayQueryHolder(holder)).AfterComplete([this](SQLQueryHolderBase const& holder)
        {
            HandlePlayerBotLoginCallback(static_cast<PlayerbotLoginQueryHolder const&>(holder));
        });
    }
    else
    {
        sWorld->AddQueryHolderCallback(CharacterDatabase.DelayQueryHolder(holder)).AfterComplete([this](SQLQueryHolderBase const& holder)
        {
            HandlePlayerBotLoginCallback(static_cast<PlayerbotLoginQueryHolder const&>(holder));
        });
    }
}

void PlayerbotHolder::HandlePlayerBotLoginCallback(PlayerbotLoginQueryHolder const& holder)
{
    uint32 botAccountId = holder.GetAccountId();

    WorldSession* botSession = new WorldSession(botAccountId, "", nullptr, SEC_PLAYER, EXPANSION_WRATH_OF_THE_LICH_KING, time_t(0), LOCALE_enUS, 0, false, false, 0, true);

    botSession->HandlePlayerLoginFromDB(holder); // will delete lqh

    Player* bot = botSession->GetPlayer();
    if (!bot)
    {
        LogoutPlayerBot(holder.GetGuid());
        // LOG_ERROR("playerbots", "Error logging in bot {}, please try to reset all random bots", holder.GetGuid().ToString().c_str());
        return;
    }

    sRandomPlayerbotMgr->OnPlayerLogin(bot);

    uint32 masterAccount = holder.GetMasterAccountId();
    WorldSession* masterSession = masterAccount ? sWorld->FindSession(masterAccount) : nullptr;
    bool allowed = false;
    if (botAccountId == masterAccount)
        allowed = true;
    else if (masterSession && sPlayerbotAIConfig->allowGuildBots && bot->GetGuildId() != 0 && bot->GetGuildId() == masterSession->GetPlayer()->GetGuildId())
        allowed = true;
    else if (sPlayerbotAIConfig->IsInRandomAccountList(botAccountId))
        allowed = true;

    if (allowed)
    {
        OnBotLogin(bot);
    }
    else
    {
        if (masterSession)
        {
            ChatHandler ch(masterSession);
            ch.PSendSysMessage("You are not allowed to control bot %s", bot->GetName());
        }
        OnBotLogin(bot);
        LogoutPlayerBot(bot->GetGUID());

        // LOG_ERROR("playerbots", "Attempt to add not allowed bot {}, please try to reset all random bots", bot->GetName());
    }
}

void PlayerbotHolder::UpdateSessions()
{
    for (PlayerBotMap::const_iterator itr = GetPlayerBotsBegin(); itr != GetPlayerBotsEnd(); ++itr)
    {
        Player* const bot = itr->second;
        if (bot->IsBeingTeleported())
        {
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
            if (botAI) {
                botAI->HandleTeleportAck();
            }
        }
        else if (bot->IsInWorld())
        {
            HandleBotPackets(bot->GetSession());
        }
    }
}

void PlayerbotHolder::HandleBotPackets(WorldSession* session)
{
    WorldPacket* packet;
    while (session->GetPacketQueue().next(packet))
    {
        OpcodeClient opcode = static_cast<OpcodeClient>(packet->GetOpcode());
        ClientOpcodeHandler const* opHandle = opcodeTable[opcode];
        opHandle->Call(session, *packet);
        delete packet;
    }
}

void PlayerbotHolder::LogoutAllBots()
{
    /*
    while (true)
    {
        PlayerBotMap::const_iterator itr = GetPlayerBotsBegin();
        if (itr == GetPlayerBotsEnd())
            break;

        Player* bot= itr->second;
        if (!GET_PLAYERBOT_AI(bot)->IsRealPlayer())
            LogoutPlayerBot(bot->GetGUID());
    }
    */

    PlayerBotMap bots = playerBots;
    for (auto& itr : bots)
    {
        Player* bot = itr.second;
        if (!bot)
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI || botAI->IsRealPlayer())
            continue;

        LogoutPlayerBot(bot->GetGUID());
    }
}

void PlayerbotMgr::CancelLogout()
{
    Player* master = GetMaster();
    if (!master)
        return;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI || botAI->IsRealPlayer())
            continue;

        if (bot->GetSession()->isLogingOut())
        {
            WorldPackets::Character::LogoutCancel data = WorldPacket(CMSG_LOGOUT_CANCEL);
            bot->GetSession()->HandleLogoutCancelOpcode(data);
            botAI->TellMaster("Logout cancelled!");
        }
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI || botAI->IsRealPlayer())
            continue;

        if (botAI->GetMaster() != master)
            continue;

        if (bot->GetSession()->isLogingOut())
        {
            WorldPackets::Character::LogoutCancel data = WorldPacket(CMSG_LOGOUT_CANCEL);
            bot->GetSession()->HandleLogoutCancelOpcode(data);
        }
    }
}

void PlayerbotHolder::LogoutPlayerBot(ObjectGuid guid)
{
    if (Player* bot = GetPlayerBot(guid))
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI)
            return;

        Group* group = bot->GetGroup();
        if (group && !bot->InBattleground() && !bot->InBattlegroundQueue() && botAI->HasActivePlayerMaster())
        {
            sPlayerbotDbStore->Save(botAI);
        }

        LOG_INFO("playerbots", "Bot {} logging out", bot->GetName().c_str());
        bot->SaveToDB(false, false);

        WorldSession* botWorldSessionPtr = bot->GetSession();
        WorldSession* masterWorldSessionPtr = nullptr;

        if (botWorldSessionPtr->isLogingOut())
            return;

        Player* master = botAI->GetMaster();
        if (master)
            masterWorldSessionPtr = master->GetSession();

        // check for instant logout
        bool logout = botWorldSessionPtr->ShouldLogOut(time(nullptr));

        if (masterWorldSessionPtr && masterWorldSessionPtr->ShouldLogOut(time(nullptr)))
            logout = true;

        if (masterWorldSessionPtr && !masterWorldSessionPtr->GetPlayer())
            logout = true;

        if (bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || bot->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
            botWorldSessionPtr->GetSecurity() >= (AccountTypes)sWorld->getIntConfig(CONFIG_INSTANT_LOGOUT))
        {
            logout = true;
        }

        if (master && (master->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || master->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
            (masterWorldSessionPtr && masterWorldSessionPtr->GetSecurity() >= (AccountTypes)sWorld->getIntConfig(CONFIG_INSTANT_LOGOUT))))
        {
            logout = true;
        }

        TravelTarget* target = nullptr;
        if (botAI->GetAiObjectContext()) //Maybe some day re-write to delate all pointer values.
        {
            target = botAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
        }

        // Peiru: Allow bots to always instant logout to see if this resolves logout crashes
        logout = true;

        // if no instant logout, request normal logout
        if (!logout)
        {
            if (bot->GetSession()->isLogingOut())
                return;
            else if (bot)
            {
                botAI->TellMaster("I'm logging out!");
                WorldPackets::Character::LogoutRequest data = WorldPacket(CMSG_LOGOUT_REQUEST);
                botWorldSessionPtr->HandleLogoutRequestOpcode(data);
                if (!bot)
                {
                    playerBots.erase(guid);
                    delete botWorldSessionPtr;
                    if (target)
                        delete target;
                }
                return;
            }
            else
            {
                playerBots.erase(guid);    // deletes bot player ptr inside this WorldSession PlayerBotMap
                delete botWorldSessionPtr;  // finally delete the bot's WorldSession
                if (target)
                    delete target;
            }
            return;
        } // if instant logout possible, do it
        else if (bot && (logout || !botWorldSessionPtr->isLogingOut()))
        {
            botAI->TellMaster("Goodbye!");
            playerBots.erase(guid);                 // deletes bot player ptr inside this WorldSession PlayerBotMap
            botWorldSessionPtr->LogoutPlayer(true); // this will delete the bot Player object and PlayerbotAI object
            delete botWorldSessionPtr;              // finally delete the bot's WorldSession
        }
    }
}

void PlayerbotHolder::DisablePlayerBot(ObjectGuid guid)
{
    if (Player* bot = GetPlayerBot(guid))
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI) {
            return;
        }
        botAI->TellMaster("Goodbye!");
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();

        Group* group = bot->GetGroup();
        if (group && !bot->InBattleground() && !bot->InBattlegroundQueue() && botAI->HasActivePlayerMaster())
        {
            sPlayerbotDbStore->Save(botAI);
        }

        LOG_DEBUG("playerbots", "Bot {} logged out", bot->GetName().c_str());

        bot->SaveToDB(false, false);

        if (botAI->GetAiObjectContext()) //Maybe some day re-write to delate all pointer values.
        {
            TravelTarget* target = botAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
            if (target)
                delete target;
        }

        playerBots.erase(guid);    // deletes bot player ptr inside this WorldSession PlayerBotMap

        delete botAI;
    }
}

Player* PlayerbotHolder::GetPlayerBot(ObjectGuid playerGuid) const
{
    PlayerBotMap::const_iterator it = playerBots.find(playerGuid);
    return (it == playerBots.end()) ? 0 : it->second;
}

Player* PlayerbotHolder::GetPlayerBot(ObjectGuid::LowType lowGuid) const
{
    ObjectGuid playerGuid = ObjectGuid::Create<HighGuid::Player>(lowGuid);
    PlayerBotMap::const_iterator it = playerBots.find(playerGuid);
    return (it == playerBots.end()) ? 0 : it->second;
}

void PlayerbotHolder::OnBotLogin(Player* const bot)
{
    sPlayerbotsMgr->AddPlayerbotData(bot, true);
    OnBotLoginInternal(bot);

    playerBots[bot->GetGUID()] = bot;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI) {
        return;
    }
    Player* master = botAI->GetMaster();
    if (master)
    {
        ObjectGuid masterGuid = master->GetGUID();
        if (master->GetGroup() && ! master->GetGroup()->IsLeader(masterGuid))
            master->GetGroup()->ChangeLeader(masterGuid);
    }

    Group* group = bot->GetGroup();
    if (group)
    {
        bool groupValid = false;
        Group::MemberSlotList const& slots = group->GetMemberSlots();
        for (Group::MemberSlotList::const_iterator i = slots.begin(); i != slots.end(); ++i)
        {
            ObjectGuid member = i->guid;
            if (master)
            {
                if (master->GetGUID() == member)
                {
                    groupValid = true;
                    break;
                }
            }
            else
            {
                uint32 account = sCharacterCache->GetCharacterAccountIdByGuid(member);
                if (!sPlayerbotAIConfig->IsInRandomAccountList(account))
                {
                    groupValid = true;
                    break;
                }
            }
        }

        if (!groupValid)
        {
            WorldPacket p;
            std::string const member = bot->GetName();
            p << uint32(PARTY_OP_LEAVE) << member << uint32(0);
            bot->GetSession()->HandleGroupDisbandOpcode(p);
        }
    }

    group = bot->GetGroup();
    if (group)
    {
        botAI->ResetStrategies();
    }
    else
    {
        // botAI->ResetStrategies(!sRandomPlayerbotMgr->IsRandomBot(bot));
        botAI->ResetStrategies();
    }

    if (master && !master->HasUnitState(UNIT_STATE_IN_FLIGHT))
    {
        bot->GetMotionMaster()->MovementExpired();
        bot->CleanupAfterTaxiFlight();
    }

    // check activity
    botAI->AllowActivity(ALL_ACTIVITY, true);

    // set delay on login
    botAI->SetNextCheckDelay(urand(2000, 4000));

    botAI->TellMaster("Hello!", PLAYERBOT_SECURITY_TALK);

    if (master && master->GetGroup() && !group) {
        master->GetGroup()->AddMember(bot);
    }

    uint32 accountId = bot->GetSession()->GetAccountId();
    bool isRandomAccount = sPlayerbotAIConfig->IsInRandomAccountList(accountId);

    bot->SaveToDB(false, false);
    if (master && isRandomAccount && master->GetLevel() < bot->GetLevel()) {
        // PlayerbotFactory factory(bot, master->getLevel());
        // factory.Randomize(false);
        uint32 mixedGearScore = PlayerbotAI::GetMixedGearScore(master, true, false, 12) * sPlayerbotAIConfig->autoInitEquipLevelLimitRatio;
        PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_LEGENDARY, mixedGearScore);
        factory.Randomize(false);
    }

    // bots join World chat if not solo oriented
    if (bot->getLevel() >= 10 && sRandomPlayerbotMgr->IsRandomBot(bot) && GET_PLAYERBOT_AI(bot) && GET_PLAYERBOT_AI(bot)->GetGrouperType() != GrouperType::SOLO)
    {
        // TODO make action/config
        // Make the bot join the world channel for chat
        WorldPacket pkt(CMSG_JOIN_CHANNEL);
        pkt << uint32(0) << uint8(0) << uint8(0);
        pkt << std::string("World");
        pkt << ""; // Pass
        bot->GetSession()->HandleJoinChannel(pkt);
    }
    // join standard channels
    AreaTableEntry const* current_zone = sAreaTableStore.LookupEntry(bot->GetAreaId());
    ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId());
    std::string current_zone_name = current_zone ? current_zone->area_name[0] : "";

    if (current_zone && cMgr)
    {
        for (uint32 i = 0; i < sChatChannelsStore.GetNumRows(); ++i)
        {
            ChatChannelsEntry const* channel = sChatChannelsStore.LookupEntry(i);
            if (!channel) continue;

            bool isLfg = (channel->flags & CHANNEL_DBC_FLAG_LFG) != 0;

            // skip non built-in channels or global channel without zone name in pattern
            if (!isLfg && (!channel || (channel->flags & 4) == 4))
                continue;

            //  new channel
            Channel* new_channel = nullptr;
            if (isLfg)
            {
                std::string lfgChannelName = channel->pattern[0];
                new_channel = cMgr->GetJoinChannel("LookingForGroup", channel->ChannelID);
            }
            else
            {
                char new_channel_name_buf[100];
                snprintf(new_channel_name_buf, 100, channel->pattern[0], current_zone_name.c_str());
                new_channel = cMgr->GetJoinChannel(new_channel_name_buf, channel->ChannelID);
            }
            if (new_channel && new_channel->GetName().length() > 0)
                new_channel->JoinChannel(bot, "");
        }
    }
}

std::string const PlayerbotHolder::ProcessBotCommand(std::string const cmd, ObjectGuid guid, ObjectGuid masterguid, bool admin, uint32 masterAccountId, uint32 masterGuildId)
{
    if (!sPlayerbotAIConfig->enabled || guid.IsEmpty())
        return "bot system is disabled";

    uint32 botAccount = sCharacterCache->GetCharacterAccountIdByGuid(guid);
    bool isRandomBot = sRandomPlayerbotMgr->IsRandomBot(guid.GetCounter());
    bool isRandomAccount = sPlayerbotAIConfig->IsInRandomAccountList(botAccount);
    bool isMasterAccount = (masterAccountId == botAccount);

    if (!isRandomAccount && !isMasterAccount && !admin && masterguid)
    {
        Player* master = ObjectAccessor::FindConnectedPlayer(masterguid);
        if (master && (!sPlayerbotAIConfig->allowGuildBots || !masterGuildId || (masterGuildId && sCharacterCache->GetCharacterGuildIdByGuid(guid) != masterGuildId)))
            return "not in your guild or account";
    }

    if (cmd == "add" || cmd == "login")
    {
        if (ObjectAccessor::FindPlayer(guid))
            return "player already logged in";

        AddPlayerBot(guid, masterAccountId);
        return "ok";
    }
    else if (cmd == "remove" || cmd == "logout" || cmd == "rm")
    {
        if (!ObjectAccessor::FindPlayer(guid))
            return "player is offline";

        if (!GetPlayerBot(guid))
            return "not your bot";

        LogoutPlayerBot(guid);
        return "ok";
    }

    // if (admin)
    // {
    Player* bot = GetPlayerBot(guid);
    if (!bot)
        bot = sRandomPlayerbotMgr->GetPlayerBot(guid);

    if (!bot)
        return "bot not found";

    if (!isRandomAccount || isRandomBot) {
        return "ERROR: You can not use this command on non-summoned random bot.";
    }

    if (GET_PLAYERBOT_AI(bot)) {
        if (Player* master = GET_PLAYERBOT_AI(bot)->GetMaster())
        {
            if (master->GetSession()->GetSecurity() <= SEC_PLAYER && sPlayerbotAIConfig->autoInitOnly && cmd != "init=auto") {
                return "The command is not allowed, use init=auto instead.";
            }
            int gs;
            if (cmd == "init=white" || cmd == "init=common")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_NORMAL);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=green" || cmd == "init=uncommon")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_UNCOMMON);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=blue" || cmd == "init=rare")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_RARE);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=epic" || cmd == "init=purple")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_EPIC);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=legendary" || cmd == "init=yellow")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_LEGENDARY);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=auto")
            {
                uint32 mixedGearScore = PlayerbotAI::GetMixedGearScore(master, true, false, 12) * sPlayerbotAIConfig->autoInitEquipLevelLimitRatio;
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_LEGENDARY, mixedGearScore);
                factory.Randomize(false);
                return "ok, gear score limit: " + std::to_string(mixedGearScore / (ITEM_QUALITY_EPIC + 1)) + "(for epic)";
            }
            else if (cmd.starts_with("init=") && sscanf(cmd.c_str(), "init=%d", &gs) != -1)
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_LEGENDARY, gs);
                factory.Randomize(false);
                return "ok, gear score limit: " + std::to_string(gs / (ITEM_QUALITY_EPIC + 1)) + "(for epic)";
            }
        }
    }

    if (cmd == "levelup" || cmd == "level")
    {
        PlayerbotFactory factory(bot, bot->getLevel());
        factory.Randomize(true);
        return "ok";
    }
    else if (cmd == "refresh")
    {
        PlayerbotFactory factory(bot, bot->getLevel());
        factory.Refresh();
        return "ok";
    }
    else if (cmd == "random")
    {
        sRandomPlayerbotMgr->Randomize(bot);
        return "ok";
    }
    // }

    return "unknown command";
}

bool PlayerbotMgr::HandlePlayerbotMgrCommand(ChatHandler* handler, char const* args)
{
    if (!sPlayerbotAIConfig->enabled)
    {
        handler->PSendSysMessage("|cffff0000Playerbot system is currently disabled!");
        return false;
    }

    WorldSession* m_session = handler->GetSession();
    if (!m_session)
    {
        handler->PSendSysMessage("You may only add bots from an active session");
        return false;
    }

    Player* player = m_session->GetPlayer();
    PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(player);
    if (!mgr)
    {
        handler->PSendSysMessage("You cannot control bots yet");
        return false;
    }

    std::vector<std::string> messages = mgr->HandlePlayerbotCommand(args, player);
    if (messages.empty())
        return true;

    for (std::vector<std::string>::iterator i = messages.begin(); i != messages.end(); ++i)
    {
        handler->PSendSysMessage("%s", i->c_str());
    }

    return true;
}

std::vector<std::string> PlayerbotHolder::HandlePlayerbotCommand(char const* args, Player* master)
{
    std::vector<std::string> messages;

    if (!*args)
    {
        messages.push_back("usage: list/reload/tweak/self or add/init/remove PLAYERNAME");
        messages.push_back("       addclass CLASSNAME");
        return messages;
    }

    char* cmd = strtok ((char*)args, " ");
    char* charname = strtok (nullptr, " ");
    if (!cmd)
    {
        messages.push_back("usage: list/reload/tweak/self or add/init/remove PLAYERNAME or addclass CLASSNAME");
        return messages;
    }

    if (!strcmp(cmd, "initself")) {
        if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER) {
            // OnBotLogin(master);
            PlayerbotFactory factory(master, master->getLevel(), ITEM_QUALITY_EPIC);
            factory.Randomize(false);
            messages.push_back("initself ok");
            return messages;
        } else {
            messages.push_back("ERROR: Only GM can use this command.");
            return messages;
        }
    }

    if (!strncmp(cmd, "initself=", 9)) {
        if (!strcmp(cmd, "initself=rare")) {
            if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER) {
                // OnBotLogin(master);
                PlayerbotFactory factory(master, master->getLevel(), ITEM_QUALITY_RARE);
                factory.Randomize(false);
                messages.push_back("initself ok");
                return messages;
            } else {
                messages.push_back("ERROR: Only GM can use this command.");
                return messages;
            }
        }
        if (!strcmp(cmd, "initself=epic")) {
            if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER) {
                // OnBotLogin(master);
                PlayerbotFactory factory(master, master->getLevel(), ITEM_QUALITY_EPIC);
                factory.Randomize(false);
                messages.push_back("initself ok");
                return messages;
            } else {
                messages.push_back("ERROR: Only GM can use this command.");
                return messages;
            }
        }
        int32 gs;
        if (sscanf(cmd, "initself=%d", &gs) != -1) {
            if (master->GetSession()->GetSecurity() >= SEC_GAMEMASTER) {
                // OnBotLogin(master);
                PlayerbotFactory factory(master, master->getLevel(), ITEM_QUALITY_LEGENDARY, gs);
                factory.Randomize(false);
                messages.push_back("initself ok, gs = " + std::to_string(gs));
                return messages;
            } else {
                messages.push_back("ERROR: Only GM can use this command.");
                return messages;
            }
        }
    }

    if (!strcmp(cmd, "list"))
    {
        messages.push_back(ListBots(master));
        return messages;
    }

    if (!strcmp(cmd, "reload"))
    {
        messages.push_back("Reloading config");
        sPlayerbotAIConfig->Initialize();
        return messages;
    }

    if (!strcmp(cmd, "tweak"))
    {
        sPlayerbotAIConfig->tweakValue = sPlayerbotAIConfig->tweakValue++;
        if (sPlayerbotAIConfig->tweakValue > 2)
            sPlayerbotAIConfig->tweakValue = 0;

        messages.push_back("Set tweakvalue to " + std::to_string(sPlayerbotAIConfig->tweakValue));
        return messages;
    }

    if (!strcmp(cmd, "self"))
    {
        if (GET_PLAYERBOT_AI(master))
        {
            messages.push_back("Disable player botAI");
            DisablePlayerBot(master->GetGUID());
        }
        else if (sPlayerbotAIConfig->selfBotLevel == 0)
            messages.push_back("Self-bot is disabled");
        else if (sPlayerbotAIConfig->selfBotLevel == 1 && master->GetSession()->GetSecurity() < SEC_GAMEMASTER)
            messages.push_back("You do not have permission to enable player botAI");
        else
        {
            messages.push_back("Enable player botAI");
            OnBotLogin(master);
        }

        return messages;
    }

    if (!strcmp(cmd, "lookup"))
    {
        messages.push_back(LookupBots(master));
        return messages;
    }

    if (!strcmp(cmd, "addclass"))
    {
        if (sPlayerbotAIConfig->addClassCommand == 0 && master->GetSession()->GetSecurity() < SEC_GAMEMASTER) {
            messages.push_back("You do not have permission to create bot by addclass command");
            return messages;
        }
        if (!charname) {
            messages.push_back("addclass: invalid CLASSNAME(warrior/paladin/hunter/rogue/priest/shaman/mage/warlock/druid/dk)");
            return messages;
        }
        uint8 claz;
        if (!strcmp(charname, "warrior"))
        {
            claz = 1;
        }
        else if (!strcmp(charname, "paladin"))
        {
            claz = 2;
        }
        else if (!strcmp(charname, "hunter"))
        {
            claz = 3;
        }
        else if (!strcmp(charname, "rogue"))
        {
            claz = 4;
        }
        else if (!strcmp(charname, "priest"))
        {
            claz = 5;
        }
        else if (!strcmp(charname, "shaman"))
        {
            claz = 7;
        }
        else if (!strcmp(charname, "mage"))
        {
            claz = 8;
        }
        else if (!strcmp(charname, "warlock"))
        {
            claz = 9;
        }
        else if (!strcmp(charname, "druid"))
        {
            claz = 11;
        }
        else if (!strcmp(charname, "dk"))
        {
            claz = 6;
        }
        else
        {
            messages.push_back("Error: Invalid Class. Try again.");
            return messages;
        }
        uint8 master_race = master->getRace();
        std::string race_limit;
        switch (master_race)
        {
            case 1:
            case 3:
            case 4:
            case 7:
            case 11:
                race_limit = "1, 3, 4, 7, 11";
                break;
            case 2:
            case 5:
            case 6:
            case 8:
            case 10:
                race_limit = "2, 5, 6, 8, 10";
                break;
        }
        // find a bot fit conditions and not in any guild
        QueryResult results = CharacterDatabase.Query("SELECT guid FROM characters "
            "WHERE name IN (SELECT name FROM playerbots_names) AND class = '{}' AND online = 0 AND race IN ({}) AND guid NOT IN ( SELECT guid FROM guild_member ) "
            "ORDER BY account DESC LIMIT 1", claz, race_limit);
        if (results)
        {
            Field* fields = results->Fetch();
            ObjectGuid guid = ObjectGuid(HighGuid::Player, fields[0].Get<uint32>());
            AddPlayerBot(guid, master->GetSession()->GetAccountId());

            messages.push_back("addclass " + std::string(charname) + " ok");
            return messages;
        }
        messages.push_back("addclass failed.");
        return messages;
    }

    std::string charnameStr;

    if (!charname)
    {
        std::string name;
        bool isPlayer = sCharacterCache->GetCharacterNameByGuid(master->GetTarget(), name);
        // Player* tPlayer = ObjectAccessor::FindConnectedPlayer(master->GetTarget());
        if (isPlayer) {
            charnameStr = name;
        } else {
            messages.push_back("usage: list/reload/tweak/self or add/init/remove PLAYERNAME");
            return messages;
        }
    } else {
        charnameStr = charname;
    }

    std::string const cmdStr = cmd;

    std::set<std::string> bots;
    if (charnameStr == "*" && master)
    {
        Group* group = master->GetGroup();
        if (!group)
        {
            messages.push_back("you must be in group");
            return messages;
        }

        Group::MemberSlotList slots = group->GetMemberSlots();
        for (Group::member_citerator i = slots.begin(); i != slots.end(); i++)
        {
            ObjectGuid member = i->guid;

            if (member == master->GetGUID())
                continue;

            std::string bot;
            if (sCharacterCache->GetCharacterNameByGuid(member, bot))
                bots.insert(bot);
        }
    }

    if (charnameStr == "!" && master && master->GetSession()->GetSecurity() > SEC_GAMEMASTER)
    {
        for (PlayerBotMap::const_iterator i = GetPlayerBotsBegin(); i != GetPlayerBotsEnd(); ++i)
        {
            if (Player* bot = i->second)
                if (bot->IsInWorld())
                    bots.insert(bot->GetName());
        }
    }

    std::vector<std::string> chars = split(charnameStr, ',');
    for (std::vector<std::string>::iterator i = chars.begin(); i != chars.end(); i++)
    {
        std::string const s = *i;

        uint32 accountId = GetAccountId(s);
        if (!accountId)
        {
            bots.insert(s);
            continue;
        }

        QueryResult results = CharacterDatabase.Query("SELECT name FROM characters WHERE account = {}", accountId);
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                std::string const charName = fields[0].Get<std::string>();
                bots.insert(charName);
            } while (results->NextRow());
        }
    }

    for (std::set<std::string>::iterator i = bots.begin(); i != bots.end(); ++i)
    {
        std::string const bot = *i;

        std::ostringstream out;
        out << cmdStr << ": " << bot << " - ";

        ObjectGuid member = sCharacterCache->GetCharacterGuidByName(bot);
        if (!member)
        {
            out << "character not found";
        }
        else if (master && member != master->GetGUID())
        {
            out << ProcessBotCommand(cmdStr, member, master->GetGUID(), master->GetSession()->GetSecurity() >= SEC_GAMEMASTER, master->GetSession()->GetAccountId(), master->GetGuildId());
        }
        else if (!master)
        {
            out << ProcessBotCommand(cmdStr, member, ObjectGuid::Empty, true, -1, -1);
        }

        messages.push_back(out.str());
    }

    return messages;
}

uint32 PlayerbotHolder::GetAccountId(std::string const name)
{
    return AccountMgr::GetId(name);
}

uint32 PlayerbotHolder::GetAccountId(ObjectGuid guid)
{
    if (!guid.IsPlayer())
        return 0;

    // prevent DB access for online player
    if (Player* player = ObjectAccessor::FindConnectedPlayer(guid))
        return player->GetSession()->GetAccountId();

    ObjectGuid::LowType lowguid = guid.GetCounter();

    if (QueryResult result = CharacterDatabase.Query("SELECT account FROM characters WHERE guid = {}", lowguid))
    {
        uint32 acc = (*result)[0].Get<uint32>();
        return acc;
    }

    return 0;
}

std::string const PlayerbotHolder::ListBots(Player* master)
{
    std::set<std::string> bots;
    std::map<uint8, std::string> classNames;

    classNames[CLASS_DEATH_KNIGHT] = "Death Knight";
    classNames[CLASS_DRUID] = "Druid";
    classNames[CLASS_HUNTER] = "Hunter";
    classNames[CLASS_MAGE] = "Mage";
    classNames[CLASS_PALADIN] = "Paladin";
    classNames[CLASS_PRIEST] = "Priest";
    classNames[CLASS_ROGUE] = "Rogue";
    classNames[CLASS_SHAMAN] = "Shaman";
    classNames[CLASS_WARLOCK] = "Warlock";
    classNames[CLASS_WARRIOR] = "Warrior";
    classNames[CLASS_DEATH_KNIGHT] = "DeathKnight";

    std::map<std::string, std::string> online;
    std::vector<std::string> names;
    std::map<std::string, std::string> classes;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        std::string const name = bot->GetName();
        bots.insert(name);

        names.push_back(name);
        online[name] = "+";
        classes[name] = classNames[bot->getClass()];
    }

    if (master)
    {
        QueryResult results = CharacterDatabase.Query("SELECT class, name FROM characters WHERE account = {}", master->GetSession()->GetAccountId());
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                uint8 cls = fields[0].Get<uint8>();
                std::string const name = fields[1].Get<std::string>();
                if (bots.find(name) == bots.end() && name != master->GetSession()->GetPlayerName())
                {
                    names.push_back(name);
                    online[name] = "-";
                    classes[name] = classNames[cls];
                }
            } while (results->NextRow());
        }
    }

    std::sort(names.begin(), names.end());

    if (Group* group = master->GetGroup())
    {
        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player *member = ObjectAccessor::FindPlayer(itr->guid);
            if (member && sRandomPlayerbotMgr->IsRandomBot(member))
            {
                std::string const name = member->GetName();

                names.push_back(name);
                online[name] = "+";
                classes[name] = classNames[member->getClass()];
            }
        }
    }

    std::ostringstream out;
    bool first = true;
    out << "Bot roster: ";
    for (std::vector<std::string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        if (first)
            first = false;
        else
            out << ", ";

        std::string const name = *i;
        out << online[name] << name << " " << classes[name];
    }

    return out.str();
}

std::string const PlayerbotHolder::LookupBots(Player* master)
{
    std::list<std::string> messages;
    messages.push_back("Classes Available:");
    messages.push_back("|TInterface\\icons\\INV_Sword_27.png:25:25:0:-1|t Warrior");
    messages.push_back("|TInterface\\icons\\INV_Hammer_01.png:25:25:0:-1|t Paladin");
    messages.push_back("|TInterface\\icons\\INV_Weapon_Bow_07.png:25:25:0:-1|t Hunter");
    messages.push_back("|TInterface\\icons\\INV_ThrowingKnife_04.png:25:25:0:-1|t Rogue");
    messages.push_back("|TInterface\\icons\\INV_Staff_30.png:25:25:0:-1|t Priest");
    messages.push_back("|TInterface\\icons\\inv_jewelry_talisman_04.png:25:25:0:-1|t Shaman");
    messages.push_back("|TInterface\\icons\\INV_staff_30.png:25:25:0:-1|t Mage");
    messages.push_back("|TInterface\\icons\\INV_staff_30.png:25:25:0:-1|t Warlock");
    messages.push_back("|TInterface\\icons\\Ability_Druid_Maul.png:25:25:0:-1|t Druid");
    messages.push_back("DK");
    messages.push_back("(Usage: .bot lookup CLASS)");
    std::string ret_msg;
    for (std::string msg: messages) {
        ret_msg += msg + "\n";
    }
    return ret_msg;
}

PlayerbotMgr::PlayerbotMgr(Player* const master) : PlayerbotHolder(),  master(master), lastErrorTell(0)
{
}

PlayerbotMgr::~PlayerbotMgr()
{
    if (master)
        sPlayerbotsMgr->RemovePlayerBotData(master->GetGUID());
}

void PlayerbotMgr::UpdateAIInternal(uint32 elapsed, bool /*minimal*/)
{
    SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
    CheckTellErrors(elapsed);
}

void PlayerbotMgr::HandleCommand(uint32 type, std::string const text)
{
    Player* master = GetMaster();
    if (!master)
        return;

    if (text.find(sPlayerbotAIConfig->commandSeparator) != std::string::npos)
    {
        std::vector<std::string> commands;
        split(commands, text, sPlayerbotAIConfig->commandSeparator.c_str());
        for (std::vector<std::string>::iterator i = commands.begin(); i != commands.end(); ++i)
        {
            HandleCommand(type, *i);
        }

        return;
    }

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI)
            botAI->HandleCommand(type, text, master);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI && botAI->GetMaster() == master)
            botAI->HandleCommand(type, text, master);
    }
}

void PlayerbotMgr::HandleMasterIncomingPacket(WorldPacket const& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI)
            botAI->HandleMasterIncomingPacket(packet);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI && botAI->GetMaster() == GetMaster())
            botAI->HandleMasterIncomingPacket(packet);
    }

    switch (packet.GetOpcode())
    {
        // if master is logging out, log out all bots
        case CMSG_LOGOUT_REQUEST:
        {
            LogoutAllBots();
            break;
        }
        // if master cancelled logout, cancel too
        case CMSG_LOGOUT_CANCEL:
        {
            CancelLogout();
            break;
        }
    }
}

void PlayerbotMgr::HandleMasterOutgoingPacket(WorldPacket const& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI)
            botAI->HandleMasterOutgoingPacket(packet);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI && botAI->GetMaster() == GetMaster())
            botAI->HandleMasterOutgoingPacket(packet);
    }
}

void PlayerbotMgr::SaveToDB()
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->SaveToDB(false, false);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr->GetPlayerBotsBegin(); it != sRandomPlayerbotMgr->GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (GET_PLAYERBOT_AI(bot) && GET_PLAYERBOT_AI(bot)->GetMaster() == GetMaster())
            bot->SaveToDB(false, false);
    }
}

void PlayerbotMgr::OnBotLoginInternal(Player * const bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI) {
        return;
    }
    botAI->SetMaster(master);
    botAI->ResetStrategies();

    LOG_INFO("playerbots", "Bot {} logged in", bot->GetName().c_str());
}

void PlayerbotMgr::OnPlayerLogin(Player* player)
{
    // set locale priority for bot texts
    sPlayerbotTextMgr->AddLocalePriority(player->GetSession()->GetSessionDbcLocale());

    if (sPlayerbotAIConfig->selfBotLevel > 2)
        HandlePlayerbotCommand("self", player);

    if (!sPlayerbotAIConfig->botAutologin)
        return;

    uint32 accountId = player->GetSession()->GetAccountId();
    QueryResult results = CharacterDatabase.Query("SELECT name FROM characters WHERE account = {}", accountId);
    if (results)
    {
        std::ostringstream out;
        out << "add ";
        bool first = true;
        do
        {
            Field* fields = results->Fetch();

            if (first)
                first = false;
            else
                out << ",";

            out << fields[0].Get<std::string>();
        } while (results->NextRow());

        HandlePlayerbotCommand(out.str().c_str(), player);
    }
}

void PlayerbotMgr::TellError(std::string const botName, std::string const text)
{
    std::set<std::string> names = errors[text];
    if (names.find(botName) == names.end())
    {
        names.insert(botName);
    }

    errors[text] = names;
}

void PlayerbotMgr::CheckTellErrors(uint32 elapsed)
{
    time_t now = time(nullptr);
    if ((now - lastErrorTell) < sPlayerbotAIConfig->errorDelay / 1000)
        return;

    lastErrorTell = now;

    for (PlayerBotErrorMap::iterator i = errors.begin(); i != errors.end(); ++i)
    {
        std::string const text = i->first;
        std::set<std::string> names = i->second;

        std::ostringstream out;
        bool first = true;
        for (std::set<std::string>::iterator j = names.begin(); j != names.end(); ++j)
        {
            if (!first)
                out << ", ";
            else
                first = false;

            out << *j;
        }

        out << "|cfff00000: " << text;

        ChatHandler(master->GetSession()).PSendSysMessage(out.str().c_str());
    }

    errors.clear();
}

void PlayerbotsMgr::AddPlayerbotData(Player* player, bool isBotAI)
{
    if (!player)
    {
        return;
    }
    // If the guid already exists in the map, remove it
    std::unordered_map<ObjectGuid, PlayerbotAIBase*>::iterator itr = _playerbotsMap.find(player->GetGUID());
    if (itr != _playerbotsMap.end())
    {
        _playerbotsMap.erase(itr);
    }
    if (!isBotAI)
    {
        PlayerbotMgr* playerbotMgr = new PlayerbotMgr(player);
        ASSERT(_playerbotsMap.emplace(player->GetGUID(), playerbotMgr).second);

        playerbotMgr->OnPlayerLogin(player);
    }
    else
    {
        PlayerbotAI* botAI = new PlayerbotAI(player);
        ASSERT(_playerbotsMap.emplace(player->GetGUID(), botAI).second);
    }
}

void PlayerbotsMgr::RemovePlayerBotData(ObjectGuid const& guid)
{
    std::unordered_map<ObjectGuid, PlayerbotAIBase*>::iterator itr = _playerbotsMap.find(guid);
    if (itr != _playerbotsMap.end())
    {
        _playerbotsMap.erase(itr);
    }
}

PlayerbotAI* PlayerbotsMgr::GetPlayerbotAI(Player* player)
{
    if (!(sPlayerbotAIConfig->enabled) || !player)
    {
        return nullptr;
    }
    // if (player->GetSession()->isLogingOut() || player->IsDuringRemoveFromWorld()) {
    //     return nullptr;
    // }
    auto itr = _playerbotsMap.find(player->GetGUID());
    if (itr != _playerbotsMap.end())
    {
        if (itr->second->IsBotAI())
            return reinterpret_cast<PlayerbotAI*>(itr->second);
    }

    return nullptr;
}

PlayerbotMgr* PlayerbotsMgr::GetPlayerbotMgr(Player* player)
{
    if (!(sPlayerbotAIConfig->enabled) || !player)
    {
        return nullptr;
    }
    auto itr = _playerbotsMap.find(player->GetGUID());
    if (itr != _playerbotsMap.end())
    {
        if (!itr->second->IsBotAI())
            return reinterpret_cast<PlayerbotMgr*>(itr->second);
    }

    return nullptr;
}
