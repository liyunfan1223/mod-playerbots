/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotAI.h"

#include <cmath>
#include <sstream>
#include <string>

#include "AiFactory.h"
#include "BudgetValues.h"
#include "ChannelMgr.h"
#include "CharacterPackets.h"
#include "CreatureAIImpl.h"
#include "EmoteAction.h"
#include "Engine.h"
#include "ExternalEventHelper.h"
#include "GuildMgr.h"
#include "GuildTaskMgr.h"
#include "LFGMgr.h"
#include "LastMovementValue.h"
#include "LastSpellCastValue.h"
#include "LogLevelAction.h"
#include "LootObjectStack.h"
#include "MapMgr.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "ObjectGuid.h"
#include "PerformanceMonitor.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotDbStore.h"
#include "PlayerbotMgr.h"
#include "Playerbots.h"
#include "PointMovementGenerator.h"
#include "PositionValue.h"
#include "SayAction.h"
#include "ScriptMgr.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "SocialMgr.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "Transport.h"
#include "Unit.h"
#include "UpdateTime.h"
#include "Vehicle.h"

std::vector<std::string> PlayerbotAI::dispel_whitelist = {
    "mutating injection",
    "frostbolt",
};

std::vector<std::string>& split(std::string const s, char delim, std::vector<std::string>& elems);
std::vector<std::string> split(std::string const s, char delim);
char* strstri(char const* str1, char const* str2);
std::string& trim(std::string& s);

std::set<std::string> PlayerbotAI::unsecuredCommands;

PlayerbotChatHandler::PlayerbotChatHandler(Player* pMasterPlayer) : ChatHandler(pMasterPlayer->GetSession()) {}

uint32 PlayerbotChatHandler::extractQuestId(std::string const str)
{
    char* source = (char*)str.c_str();
    char* cId = extractKeyFromLink(source, "Hquest");
    return cId ? atol(cId) : 0;
}

void PacketHandlingHelper::AddHandler(uint16 opcode, std::string const handler) { handlers[opcode] = handler; }

void PacketHandlingHelper::Handle(ExternalEventHelper& helper)
{
    while (!queue.empty())
    {
        helper.HandlePacket(handlers, queue.top());
        queue.pop();
    }
}

void PacketHandlingHelper::AddPacket(WorldPacket const& packet)
{
    if (packet.empty())
        return;
    // assert(handlers);
    // assert(packet);
    // assert(packet.GetOpcode());
    if (handlers.find(packet.GetOpcode()) != handlers.end())
        queue.push(WorldPacket(packet));
}

PlayerbotAI::PlayerbotAI()
    : PlayerbotAIBase(true),
      bot(nullptr),
      aiObjectContext(nullptr),
      currentEngine(nullptr),
      chatHelper(this),
      chatFilter(this),
      accountId(0),
      security(nullptr),
      master(nullptr),
      currentState(BOT_STATE_NON_COMBAT)
{
    for (uint8 i = 0; i < BOT_STATE_MAX; i++)
        engines[i] = nullptr;

    for (uint8 i = 0; i < MAX_ACTIVITY_TYPE; i++)
    {
        allowActiveCheckTimer[i] = time(nullptr);
        allowActive[i] = false;
    }
}

PlayerbotAI::PlayerbotAI(Player* bot)
    : PlayerbotAIBase(true),
      bot(bot),
      chatHelper(this),
      chatFilter(this),
      master(nullptr),
      security(bot)  // reorder args - whipowill
{
    if (!bot->isTaxiCheater() && HasCheat((BotCheatMask::taxi)))
        bot->SetTaxiCheater(true);

    for (uint8 i = 0; i < MAX_ACTIVITY_TYPE; i++)
    {
        allowActiveCheckTimer[i] = time(nullptr);
        allowActive[i] = false;
    }

    accountId = bot->GetSession()->GetAccountId();

    aiObjectContext = AiFactory::createAiObjectContext(bot, this);

    engines[BOT_STATE_COMBAT] = AiFactory::createCombatEngine(bot, this, aiObjectContext);
    engines[BOT_STATE_NON_COMBAT] = AiFactory::createNonCombatEngine(bot, this, aiObjectContext);
    engines[BOT_STATE_DEAD] = AiFactory::createDeadEngine(bot, this, aiObjectContext);
    if (sPlayerbotAIConfig->applyInstanceStrategies)
        ApplyInstanceStrategies(bot->GetMapId());
    currentEngine = engines[BOT_STATE_NON_COMBAT];
    currentState = BOT_STATE_NON_COMBAT;

    masterIncomingPacketHandlers.AddHandler(CMSG_GAMEOBJ_USE, "use game object");
    masterIncomingPacketHandlers.AddHandler(CMSG_AREATRIGGER, "area trigger");
    // masterIncomingPacketHandlers.AddHandler(CMSG_GAMEOBJ_USE, "use game object");
    masterIncomingPacketHandlers.AddHandler(CMSG_LOOT_ROLL, "loot roll");
    masterIncomingPacketHandlers.AddHandler(CMSG_GOSSIP_HELLO, "gossip hello");
    masterIncomingPacketHandlers.AddHandler(CMSG_QUESTGIVER_HELLO, "gossip hello");
    masterIncomingPacketHandlers.AddHandler(CMSG_ACTIVATETAXI, "activate taxi");
    masterIncomingPacketHandlers.AddHandler(CMSG_ACTIVATETAXIEXPRESS, "activate taxi");
    masterIncomingPacketHandlers.AddHandler(CMSG_TAXICLEARALLNODES, "taxi done");
    masterIncomingPacketHandlers.AddHandler(CMSG_TAXICLEARNODE, "taxi done");
    masterIncomingPacketHandlers.AddHandler(CMSG_GROUP_UNINVITE, "uninvite");
    masterIncomingPacketHandlers.AddHandler(CMSG_GROUP_UNINVITE_GUID, "uninvite guid");
    masterIncomingPacketHandlers.AddHandler(CMSG_LFG_TELEPORT, "lfg teleport");
    masterIncomingPacketHandlers.AddHandler(CMSG_CAST_SPELL, "see spell");
    masterIncomingPacketHandlers.AddHandler(CMSG_REPOP_REQUEST, "release spirit");
    masterIncomingPacketHandlers.AddHandler(CMSG_RECLAIM_CORPSE, "revive from corpse");

    botOutgoingPacketHandlers.AddHandler(SMSG_PETITION_SHOW_SIGNATURES, "petition offer");
    botOutgoingPacketHandlers.AddHandler(SMSG_GROUP_INVITE, "group invite");
    botOutgoingPacketHandlers.AddHandler(SMSG_GUILD_INVITE, "guild invite");
    botOutgoingPacketHandlers.AddHandler(BUY_ERR_NOT_ENOUGHT_MONEY, "not enough money");
    botOutgoingPacketHandlers.AddHandler(BUY_ERR_REPUTATION_REQUIRE, "not enough reputation");
    botOutgoingPacketHandlers.AddHandler(SMSG_GROUP_SET_LEADER, "group set leader");
    botOutgoingPacketHandlers.AddHandler(SMSG_FORCE_RUN_SPEED_CHANGE, "check mount state");
    botOutgoingPacketHandlers.AddHandler(SMSG_RESURRECT_REQUEST, "resurrect request");
    botOutgoingPacketHandlers.AddHandler(SMSG_INVENTORY_CHANGE_FAILURE, "cannot equip");
    botOutgoingPacketHandlers.AddHandler(SMSG_TRADE_STATUS, "trade status");
    botOutgoingPacketHandlers.AddHandler(SMSG_LOOT_RESPONSE, "loot response");
    botOutgoingPacketHandlers.AddHandler(SMSG_ITEM_PUSH_RESULT, "item push result");
    botOutgoingPacketHandlers.AddHandler(SMSG_PARTY_COMMAND_RESULT, "party command");
    botOutgoingPacketHandlers.AddHandler(SMSG_LEVELUP_INFO, "levelup");
    botOutgoingPacketHandlers.AddHandler(SMSG_LOG_XPGAIN, "xpgain");
    botOutgoingPacketHandlers.AddHandler(SMSG_CAST_FAILED, "cast failed");
    botOutgoingPacketHandlers.AddHandler(SMSG_DUEL_REQUESTED, "duel requested");
    botOutgoingPacketHandlers.AddHandler(SMSG_INVENTORY_CHANGE_FAILURE, "inventory change failure");
    botOutgoingPacketHandlers.AddHandler(SMSG_BATTLEFIELD_STATUS, "bg status");
    botOutgoingPacketHandlers.AddHandler(SMSG_LFG_ROLE_CHECK_UPDATE, "lfg role check");
    botOutgoingPacketHandlers.AddHandler(SMSG_LFG_PROPOSAL_UPDATE, "lfg proposal");
    botOutgoingPacketHandlers.AddHandler(SMSG_TEXT_EMOTE, "receive text emote");
    botOutgoingPacketHandlers.AddHandler(SMSG_EMOTE, "receive emote");
    botOutgoingPacketHandlers.AddHandler(SMSG_LOOT_START_ROLL, "master loot roll");
    botOutgoingPacketHandlers.AddHandler(SMSG_ARENA_TEAM_INVITE, "arena team invite");
    botOutgoingPacketHandlers.AddHandler(SMSG_GROUP_DESTROYED, "group destroyed");
    botOutgoingPacketHandlers.AddHandler(SMSG_GROUP_LIST, "group list");

    masterOutgoingPacketHandlers.AddHandler(SMSG_PARTY_COMMAND_RESULT, "party command");
    masterOutgoingPacketHandlers.AddHandler(MSG_RAID_READY_CHECK, "ready check");
    masterOutgoingPacketHandlers.AddHandler(MSG_RAID_READY_CHECK_FINISHED, "ready check finished");
    masterOutgoingPacketHandlers.AddHandler(SMSG_QUESTGIVER_OFFER_REWARD, "questgiver quest details");

    // quest packet
    masterIncomingPacketHandlers.AddHandler(CMSG_QUESTGIVER_COMPLETE_QUEST, "complete quest");
    masterIncomingPacketHandlers.AddHandler(CMSG_QUESTGIVER_ACCEPT_QUEST, "accept quest");
    masterIncomingPacketHandlers.AddHandler(CMSG_QUEST_CONFIRM_ACCEPT, "confirm quest");
    masterIncomingPacketHandlers.AddHandler(CMSG_PUSHQUESTTOPARTY, "quest share");
    botOutgoingPacketHandlers.AddHandler(SMSG_QUESTUPDATE_COMPLETE, "quest update complete");
    botOutgoingPacketHandlers.AddHandler(SMSG_QUESTUPDATE_ADD_KILL, "quest update add kill");
    botOutgoingPacketHandlers.AddHandler(SMSG_QUESTUPDATE_ADD_ITEM, "quest update add item");
    botOutgoingPacketHandlers.AddHandler(SMSG_QUEST_CONFIRM_ACCEPT, "confirm quest");
}

PlayerbotAI::~PlayerbotAI()
{
    for (uint8 i = 0; i < BOT_STATE_MAX; i++)
    {
        if (engines[i])
            delete engines[i];
    }

    if (aiObjectContext)
        delete aiObjectContext;

    if (bot)
        sPlayerbotsMgr->RemovePlayerBotData(bot->GetGUID(), true);
}

void PlayerbotAI::UpdateAI(uint32 elapsed, bool minimal)
{
    if (nextAICheckDelay > elapsed)
        nextAICheckDelay -= elapsed;
    else
        nextAICheckDelay = 0;

    if (!bot || !bot->IsInWorld())
    {
        return;
    }

    if (!bot->GetSession() || bot->GetSession()->isLogingOut())
    {
        return;
    }
    if (bot->IsDuringRemoveFromWorld())
    {
        return;
    }

    // if (!GetMaster() || !GetMaster()->IsInWorld() || !GetMaster()->GetSession() ||
    // GetMaster()->GetSession()->isLogingOut()) {
    //     return;
    // }
    // if (bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING)) {
    //     bot->Say("Falling!", LANG_UNIVERSAL);
    // }
    // if (!bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING) && bot->GetPositionZ() - bot->GetFloorZ() > 0.1f) {
    //     bot->AddUnitMovementFlag(MOVEMENTFLAG_FALLING);
    //     // bot->GetMotionMaster()->MoveFall();
    // }
    // if (bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING) && bot->GetPositionZ() - bot->GetFloorZ() <= 0.1f) {
    //     bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING);
    // }
    //  else {
    //     bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING);
    // }

    // bot->SendMovementFlagUpdate();

    // bot->GetMotionMaster()->MoveFall();
    // if (bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING)) {
    //     // bot->GetUnitMovementFlags();
    //     bot->Say("falling... flag: " + std::to_string(bot->GetUnitMovementFlags()), LANG_UNIVERSAL);
    // }
    // bot->SendMovementFlagUpdate();
    // float x, y, z;
    // bot->GetPosition(x, y, z);
    // bot->UpdateGroundPositionZ(x, y, z);
    // if (bot->GetPositionZ() - z > 0.1f) {

    // }

    // wake up if in combat
    // if (bot->IsInCombat())
    // {
    //     if (!inCombat)
    //         nextAICheckDelay = 0;
    //     else if (!AllowActivity())
    //     {
    //         if (AllowActivity(ALL_ACTIVITY, true))
    //             nextAICheckDelay = 0;
    //     }

    //     inCombat = true;
    // }
    // else
    // {
    //     if (inCombat)
    //         nextAICheckDelay = 0;

    //     inCombat = false;
    // }

    // force stop if moving but should not
    // shouldn't stop charging
    // if (bot->isMoving() && !CanMove() && !bot->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING))
    // {
    //     bot->StopMoving();
    //     bot->GetMotionMaster()->Clear();
    //     bot->GetMotionMaster()->MoveIdle();
    // }
    
    // cheat options
    if (bot->IsAlive() && ((uint32)GetCheat() > 0 || (uint32)sPlayerbotAIConfig->botCheatMask > 0))
    {
        if (HasCheat(BotCheatMask::health))
            bot->SetFullHealth();
        if (HasCheat(BotCheatMask::mana) && bot->getPowerType() == POWER_MANA)
            bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));
        if (HasCheat(BotCheatMask::power) && bot->getPowerType() != POWER_MANA)
            bot->SetPower(bot->getPowerType(), bot->GetMaxPower(bot->getPowerType()));
    }

    AllowActivity();

    
    if (!CanUpdateAI())
        return;

    Spell* currentSpell = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!currentSpell)
        currentSpell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (currentSpell && currentSpell->GetSpellInfo() && currentSpell->getState() == SPELL_STATE_PREPARING)
    {
        const SpellInfo* spellInfo = currentSpell->GetSpellInfo();
        
        // interrupt if target is dead
        if (currentSpell->m_targets.GetUnitTarget() && !currentSpell->m_targets.GetUnitTarget()->IsAlive() &&
            !spellInfo->IsAllowingDeadTarget())
        {
            InterruptSpell();
            SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
        }

        bool isHeal = false;
        bool isSingleTarget = true;

        for (uint8 i = 0; i < 3; ++i)
        {
            if (!spellInfo->Effects[i].Effect)
                continue;

            if (spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL ||
                spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL_MAX_HEALTH ||
                spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL_MECHANICAL)
                isHeal = true;
            
            if ((spellInfo->Effects[i].TargetA.GetTarget() && spellInfo->Effects[i].TargetA.GetTarget() != TARGET_UNIT_TARGET_ALLY) || 
                (spellInfo->Effects[i].TargetB.GetTarget() && spellInfo->Effects[i].TargetB.GetTarget() != TARGET_UNIT_TARGET_ALLY))
            {
                isSingleTarget = false;
            }
        }
        // interrupt if target ally has full health (heal by other member)
        if (isHeal && isSingleTarget && currentSpell->m_targets.GetUnitTarget() && currentSpell->m_targets.GetUnitTarget()->IsFullHealth())
        {
            InterruptSpell();
            SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
        }

        // wait for spell cast
        YieldThread(GetReactDelay());
        return;
    }

    if (nextTransportCheck > elapsed)
        nextTransportCheck -= elapsed;
    else
        nextTransportCheck = 0;

    if (!nextTransportCheck)
    {
        nextTransportCheck = 1000;
        Transport* newTransport = bot->GetMap()->GetTransportForPos(bot->GetPhaseMask(), bot->GetPositionX(),
                                                                    bot->GetPositionY(), bot->GetPositionZ(), bot);
        if (newTransport != bot->GetTransport())
        {
            LOG_DEBUG("playerbots", "Bot {} is on a transport", bot->GetName());

            if (bot->GetTransport())
                bot->GetTransport()->RemovePassenger(bot, true);

            if (newTransport)
                newTransport->AddPassenger(bot, true);

            bot->StopMovingOnCurrentPos();
        }
    }

    if (!bot->InBattleground() && !bot->inRandomLfgDungeon() && bot->GetGroup())
	{
		Player* leader = bot->GetGroup()->GetLeader();
		if (leader && leader != bot) // Checks if the leader is valid and is not the bot itself
		{
			PlayerbotAI* leaderAI = GET_PLAYERBOT_AI(leader);
			if (leaderAI && !leaderAI->IsRealPlayer())
			{
				bot->RemoveFromGroup();
				ResetStrategies();
			}
		}
	}

    bool min = minimal;
    UpdateAIInternal(elapsed, min);
    YieldThread(GetReactDelay());
}

void PlayerbotAI::UpdateAIInternal([[maybe_unused]] uint32 elapsed, bool minimal)
{
    if (bot->IsBeingTeleported() || !bot->IsInWorld())
        return;

    std::string const mapString = WorldPosition(bot).isOverworld() ? std::to_string(bot->GetMapId()) : "I";
    PerformanceMonitorOperation* pmo =
        sPerformanceMonitor->start(PERF_MON_TOTAL, "PlayerbotAI::UpdateAIInternal " + mapString);
    ExternalEventHelper helper(aiObjectContext);

    // chat replies
    for (auto it = chatReplies.begin(); it != chatReplies.end();)
    {
        time_t checkTime = it->m_time;
        if (checkTime && time(0) < checkTime)
        {
            ++it;
            continue;
        }

        ChatReplyAction::ChatReplyDo(bot, it->m_type, it->m_guid1, it->m_guid2, it->m_msg, it->m_chanName, it->m_name);
        it = chatReplies.erase(it);
    }

    HandleCommands();

    // logout if logout timer is ready or if instant logout is possible
    if (bot->GetSession()->isLogingOut())
    {
        WorldSession* botWorldSessionPtr = bot->GetSession();
        bool logout = botWorldSessionPtr->ShouldLogOut(time(nullptr));
        if (!master || !master->GetSession()->GetPlayer())
            logout = true;

        if (bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || bot->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
            botWorldSessionPtr->GetSecurity() >= (AccountTypes)sWorld->getIntConfig(CONFIG_INSTANT_LOGOUT))
        {
            logout = true;
        }

        if (master &&
            (master->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || master->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
             (master->GetSession() &&
              master->GetSession()->GetSecurity() >= (AccountTypes)sWorld->getIntConfig(CONFIG_INSTANT_LOGOUT))))
        {
            logout = true;
        }

        if (logout)
        {
            PlayerbotMgr* masterBotMgr = nullptr;
            if (master)
                masterBotMgr = GET_PLAYERBOT_MGR(master);
            if (masterBotMgr)
            {
                masterBotMgr->LogoutPlayerBot(bot->GetGUID());
            }
            else
            {
                sRandomPlayerbotMgr->LogoutPlayerBot(bot->GetGUID());
            }
            return;
        }

        SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
        return;
    }

    botOutgoingPacketHandlers.Handle(helper);
    masterIncomingPacketHandlers.Handle(helper);
    masterOutgoingPacketHandlers.Handle(helper);

    DoNextAction(minimal);

    if (pmo)
        pmo->finish();
}

void PlayerbotAI::HandleCommands()
{
    ExternalEventHelper helper(aiObjectContext);
    for (auto it = chatCommands.begin(); it != chatCommands.end();)
    {
        time_t& checkTime = it->GetTime();
        if (checkTime && time(0) < checkTime)
        {
            ++it;
            continue;
        }

        const std::string& command = it->GetCommand();
        Player* owner = it->GetOwner();
        if (!helper.ParseChatCommand(command, owner) && it->GetType() == CHAT_MSG_WHISPER)
        {
            // ostringstream out; out << "Unknown command " << command;
            // TellPlayer(out);
            // helper.ParseChatCommand("help");
        }
        it = chatCommands.erase(it);
    }
}

std::map<std::string, ChatMsg> chatMap;
void PlayerbotAI::HandleCommand(uint32 type, const std::string& text, Player& fromPlayer, const uint32 lang)
{
    std::string filtered = text;

    if (!IsAllowedCommand(filtered) && !GetSecurity()->CheckLevelFor(PlayerbotSecurityLevel::PLAYERBOT_SECURITY_INVITE,
                                                                     type != CHAT_MSG_WHISPER, &fromPlayer))
        return;

    if (type == CHAT_MSG_ADDON)
        return;

    if (filtered.find("BOT\t") == 0)  // Mangosbot has BOT prefix so we remove that.
        filtered = filtered.substr(4);
    else if (lang == LANG_ADDON)  // Other addon messages should not command bots.
        return;

    if (type == CHAT_MSG_SYSTEM)
        return;

    if (filtered.find(sPlayerbotAIConfig->commandSeparator) != std::string::npos)
    {
        std::vector<std::string> commands;
        split(commands, filtered, sPlayerbotAIConfig->commandSeparator.c_str());
        for (std::vector<std::string>::iterator i = commands.begin(); i != commands.end(); ++i)
        {
            HandleCommand(type, *i, fromPlayer);
        }
        return;
    }

    if (!sPlayerbotAIConfig->commandPrefix.empty())
    {
        if (filtered.find(sPlayerbotAIConfig->commandPrefix) != 0)
            return;

        filtered = filtered.substr(sPlayerbotAIConfig->commandPrefix.size());
    }

    if (chatMap.empty())
    {
        chatMap["#w "] = CHAT_MSG_WHISPER;
        chatMap["#p "] = CHAT_MSG_PARTY;
        chatMap["#r "] = CHAT_MSG_RAID;
        chatMap["#a "] = CHAT_MSG_ADDON;
        chatMap["#g "] = CHAT_MSG_GUILD;
    }
    currentChat = std::pair<ChatMsg, time_t>(CHAT_MSG_WHISPER, 0);
    for (std::map<std::string, ChatMsg>::iterator i = chatMap.begin(); i != chatMap.end(); ++i)
    {
        if (filtered.find(i->first) == 0)
        {
            filtered = filtered.substr(3);
            currentChat = std::pair<ChatMsg, time_t>(i->second, time(0) + 2);
            break;
        }
    }

    filtered = chatFilter.Filter(trim((std::string&)filtered));
    if (filtered.empty())
        return;

    if (filtered.substr(0, 6) == "debug ")
    {
        std::string response = HandleRemoteCommand(filtered.substr(6));
        WorldPacket data;
        ChatHandler::BuildChatPacket(data, CHAT_MSG_ADDON, response.c_str(), LANG_ADDON, CHAT_TAG_NONE, bot->GetGUID(),
                                     bot->GetName());
        sServerFacade->SendPacket(&fromPlayer, &data);
        return;
    }

    if (!IsAllowedCommand(filtered) &&
        !GetSecurity()->CheckLevelFor(PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, type != CHAT_MSG_WHISPER,
                                      &fromPlayer))
        return;

    if (type == CHAT_MSG_RAID_WARNING && filtered.find(bot->GetName()) != std::string::npos &&
        filtered.find("award") == std::string::npos)
    {
        chatCommands.push_back(ChatCommandHolder("warning", &fromPlayer, type));
        return;
    }

    if ((filtered.size() > 2 && filtered.substr(0, 2) == "d ") ||
        (filtered.size() > 3 && filtered.substr(0, 3) == "do "))
    {
        Event event("do", "", &fromPlayer);
        std::string action = filtered.substr(filtered.find(" ") + 1);
        DoSpecificAction(action, event);
    }

    if (ChatHelper::parseValue("command", filtered).substr(0, 3) == "do ")
    {
        Event event("do", "", &fromPlayer);
        std::string action = ChatHelper::parseValue("command", filtered);
        action = action.substr(3);
        DoSpecificAction(action, event);
    }
    else if (type != CHAT_MSG_WHISPER && filtered.size() > 6 && filtered.substr(0, 6) == "queue ")
    {
        std::string remaining = filtered.substr(filtered.find(" ") + 1);
        int index = 1;
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                if (ref->GetSource() == master)
                    continue;

                if (ref->GetSource() == bot)
                    break;

                index++;
            }
        }

        chatCommands.push_back(ChatCommandHolder(remaining, &fromPlayer, type, time(0) + index));
    }
    else if (filtered == "reset")
    {
        Reset(true);
    }

    // TODO: missing implementation to port
    /*else if (filtered == "logout")
    {
        if (!(bot->IsStunnedByLogout() || bot->GetSession()->isLogingOut()))
        {
            if (type == CHAT_MSG_WHISPER)
                TellPlayer(&fromPlayer, BOT_TEXT("logout_start"));

            if (master && master->GetPlayerbotMgr())
                SetShouldLogOut(true);
        }
    }
    else if (filtered == "logout cancel")
    {
        if (bot->IsStunnedByLogout() || bot->GetSession()->isLogingOut())
        {
            if (type == CHAT_MSG_WHISPER)
                TellPlayer(&fromPlayer, BOT_TEXT("logout_cancel"));

            WorldPacket p;
            bot->GetSession()->HandleLogoutCancelOpcode(p);
            SetShouldLogOut(false);
        }
    }
    else if ((filtered.size() > 5) && (filtered.substr(0, 5) == "wait ") && (filtered.find("wait for attack") ==
    std::string::npos))
    {
        std::string remaining = filtered.substr(filtered.find(" ") + 1);
        uint32 delay = atof(remaining.c_str()) * IN_MILLISECONDS;
        if (delay > 20000)
        {
            bot->TellMaster(&fromPlayer, "Max wait time is 20 seconds!");
            return;
        }

        IncreaseAIInternalUpdateDelay(delay);
        isWaiting = true;
        TellPlayer(&fromPlayer, "Waiting for " + remaining + " seconds!");
        return;
    }*/

    else
    {
        chatCommands.push_back(ChatCommandHolder(filtered, &fromPlayer, type));
    }
}

void PlayerbotAI::HandleTeleportAck()
{
    if (IsRealPlayer())
        return;

    bot->GetMotionMaster()->Clear(true);
    bot->StopMoving();
    if (bot->IsBeingTeleportedNear())
    {
        // Temporary fix for instance can not enter
        if (!bot->IsInWorld())
        {
            bot->GetMap()->AddPlayerToMap(bot);
        }
        while (bot->IsInWorld() && bot->IsBeingTeleportedNear())
        {
            Player* plMover = bot->m_mover->ToPlayer();
            if (!plMover)
                return;
            WorldPacket p = WorldPacket(MSG_MOVE_TELEPORT_ACK, 20);
            p << plMover->GetPackGUID();
            p << (uint32)0;  // supposed to be flags? not used currently
            p << (uint32)0;  // time - not currently used
            bot->GetSession()->HandleMoveTeleportAck(p);
        };
    }
    if (bot->IsBeingTeleportedFar())
    {
        while (bot->IsBeingTeleportedFar())
        {
            bot->GetSession()->HandleMoveWorldportAck();
        }
        // SetNextCheckDelay(urand(2000, 5000));
        if (sPlayerbotAIConfig->applyInstanceStrategies)
            ApplyInstanceStrategies(bot->GetMapId(), true);
        Reset();
    }

    SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
}

void PlayerbotAI::Reset(bool full)
{
    if (bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return;

    WorldSession* botWorldSessionPtr = bot->GetSession();
    bool logout = botWorldSessionPtr->ShouldLogOut(time(nullptr));

    // cancel logout
    if (!logout && bot->GetSession()->isLogingOut())
    {
        WorldPackets::Character::LogoutCancel data = WorldPacket(CMSG_LOGOUT_CANCEL);
        bot->GetSession()->HandleLogoutCancelOpcode(data);
        TellMaster("Logout cancelled!");
    }

    currentEngine = engines[BOT_STATE_NON_COMBAT];
    currentState = BOT_STATE_NON_COMBAT;
    nextAICheckDelay = 0;
    whispers.clear();

    aiObjectContext->GetValue<Unit*>("old target")->Set(nullptr);
    aiObjectContext->GetValue<Unit*>("current target")->Set(nullptr);
    aiObjectContext->GetValue<GuidVector>("prioritized targets")->Reset();
    aiObjectContext->GetValue<ObjectGuid>("pull target")->Set(ObjectGuid::Empty);
    aiObjectContext->GetValue<GuidPosition>("rpg target")->Set(GuidPosition());
    aiObjectContext->GetValue<LootObject>("loot target")->Set(LootObject());
    aiObjectContext->GetValue<uint32>("lfg proposal")->Set(0);
    bot->SetTarget();

    LastSpellCast& lastSpell = aiObjectContext->GetValue<LastSpellCast&>("last spell cast")->Get();
    lastSpell.Reset();

    if (full)
    {
        aiObjectContext->GetValue<LastMovement&>("last movement")->Get().Set(nullptr);
        aiObjectContext->GetValue<LastMovement&>("last area trigger")->Get().Set(nullptr);
        aiObjectContext->GetValue<LastMovement&>("last taxi")->Get().Set(nullptr);
        aiObjectContext->GetValue<TravelTarget*>("travel target")
            ->Get()
            ->setTarget(sTravelMgr->nullTravelDestination, sTravelMgr->nullWorldPosition, true);
        aiObjectContext->GetValue<TravelTarget*>("travel target")->Get()->setStatus(TRAVEL_STATUS_EXPIRED);
        aiObjectContext->GetValue<TravelTarget*>("travel target")->Get()->setExpireIn(1000);
    }

    aiObjectContext->GetValue<GuidSet&>("ignore rpg target")->Get().clear();

    bot->GetMotionMaster()->Clear();
    // bot->CleanupAfterTaxiFlight();
    InterruptSpell();

    if (full)
    {
        for (uint8 i = 0; i < BOT_STATE_MAX; i++)
        {
            engines[i]->Init();
        }
    }
}

bool PlayerbotAI::IsAllowedCommand(std::string const text)
{
    if (unsecuredCommands.empty())
    {
        unsecuredCommands.insert("who");
        unsecuredCommands.insert("wts");
        unsecuredCommands.insert("sendmail");
        unsecuredCommands.insert("invite");
        unsecuredCommands.insert("leave");
    }

    for (std::set<std::string>::iterator i = unsecuredCommands.begin(); i != unsecuredCommands.end(); ++i)
    {
        if (text.find(*i) == 0)
        {
            return true;
        }
    }

    return false;
}

void PlayerbotAI::HandleCommand(uint32 type, std::string const text, Player* fromPlayer)
{
    if (!GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, type != CHAT_MSG_WHISPER, fromPlayer))
        return;

    if (type == CHAT_MSG_ADDON)
        return;

    if (type == CHAT_MSG_SYSTEM)
        return;

    if (text.find(sPlayerbotAIConfig->commandSeparator) != std::string::npos)
    {
        std::vector<std::string> commands;
        split(commands, text, sPlayerbotAIConfig->commandSeparator.c_str());
        for (std::vector<std::string>::iterator i = commands.begin(); i != commands.end(); ++i)
        {
            HandleCommand(type, *i, fromPlayer);
        }

        return;
    }

    std::string filtered = text;
    if (!sPlayerbotAIConfig->commandPrefix.empty())
    {
        if (filtered.find(sPlayerbotAIConfig->commandPrefix) != 0)
            return;

        filtered = filtered.substr(sPlayerbotAIConfig->commandPrefix.size());
    }

    if (chatMap.empty())
    {
        chatMap["#w "] = CHAT_MSG_WHISPER;
        chatMap["#p "] = CHAT_MSG_PARTY;
        chatMap["#r "] = CHAT_MSG_RAID;
        chatMap["#a "] = CHAT_MSG_ADDON;
        chatMap["#g "] = CHAT_MSG_GUILD;
    }

    currentChat = std::pair<ChatMsg, time_t>(CHAT_MSG_WHISPER, 0);
    for (std::map<std::string, ChatMsg>::iterator i = chatMap.begin(); i != chatMap.end(); ++i)
    {
        if (filtered.find(i->first) == 0)
        {
            filtered = filtered.substr(3);
            currentChat = std::pair<ChatMsg, time_t>(i->second, time(nullptr) + 2);
            break;
        }
    }

    filtered = chatFilter.Filter(trim(filtered));
    if (filtered.empty())
        return;

    if (filtered.substr(0, 6) == "debug ")
    {
        std::string const response = HandleRemoteCommand(filtered.substr(6));
        WorldPacket data;
        ChatHandler::BuildChatPacket(data, (ChatMsg)type, type == CHAT_MSG_ADDON ? LANG_ADDON : LANG_UNIVERSAL, bot,
                                     nullptr, response.c_str());
        fromPlayer->SendDirectMessage(&data);
        return;
    }

    if (!IsAllowedCommand(filtered) &&
        (!GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_ALLOW_ALL, type != CHAT_MSG_WHISPER, fromPlayer)))
        return;

    if (type == CHAT_MSG_RAID_WARNING && filtered.find(bot->GetName()) != std::string::npos &&
        filtered.find("award") == std::string::npos)
    {
        chatCommands.push_back(ChatCommandHolder("warning", fromPlayer, type));
        return;
    }

    if ((filtered.size() > 2 && filtered.substr(0, 2) == "d ") ||
        (filtered.size() > 3 && filtered.substr(0, 3) == "do "))
    {
        std::string const action = filtered.substr(filtered.find(" ") + 1);
        DoSpecificAction(action);
    }
    else if (type != CHAT_MSG_WHISPER && filtered.size() > 6 && filtered.substr(0, 6) == "queue ")
    {
        std::string const remaining = filtered.substr(filtered.find(" ") + 1);
        uint32 index = 1;

        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                if (ref->GetSource() == master)
                    continue;

                if (ref->GetSource() == bot)
                    break;

                ++index;
            }
        }

        chatCommands.push_back(ChatCommandHolder(remaining, fromPlayer, type, time(nullptr) + index));
    }
    else if (filtered == "reset")
    {
        Reset(true);
    }
    else if (filtered == "logout")
    {
        if (!bot->GetSession()->isLogingOut())
        {
            if (type == CHAT_MSG_WHISPER)
                TellMaster("I'm logging out!");

            PlayerbotMgr* masterBotMgr = nullptr;
            if (master)
                masterBotMgr = GET_PLAYERBOT_MGR(master);
            if (masterBotMgr)
                masterBotMgr->LogoutPlayerBot(bot->GetGUID());
        }
    }
    else if (filtered == "logout cancel")
    {
        if (bot->GetSession()->isLogingOut())
        {
            if (type == CHAT_MSG_WHISPER)
                TellMaster("Logout cancelled!");

            WorldPackets::Character::LogoutCancel data = WorldPacket(CMSG_LOGOUT_CANCEL);
            bot->GetSession()->HandleLogoutCancelOpcode(data);
        }
    }
    else
    {
        chatCommands.push_back(ChatCommandHolder(filtered, fromPlayer, type));
    }
}

void PlayerbotAI::HandleBotOutgoingPacket(WorldPacket const& packet)
{
    if (packet.empty())
        return;
    if (!bot || !bot->IsInWorld() || bot->IsDuringRemoveFromWorld())
    {
        return;
    }
    switch (packet.GetOpcode())
    {
        case SMSG_SPELL_FAILURE:
        {
            WorldPacket p(packet);
            p.rpos(0);
            ObjectGuid casterGuid;
            p >> casterGuid.ReadAsPacked();
            if (casterGuid != bot->GetGUID())
                return;
            uint8 count, result;
            uint32 spellId;
            p >> count >> spellId >> result;
            SpellInterrupted(spellId);
            return;
        }
        case SMSG_SPELL_DELAYED:
        {
            WorldPacket p(packet);
            p.rpos(0);
            ObjectGuid casterGuid;
            p >> casterGuid.ReadAsPacked();
            if (casterGuid != bot->GetGUID())
                return;

            uint32 delaytime;
            p >> delaytime;
            if (delaytime <= 1000)
                IncreaseNextCheckDelay(delaytime);
            return;
        }
        case SMSG_EMOTE:  // do not react to NPC emotes
        {
            WorldPacket p(packet);
            ObjectGuid source;
            uint32 emoteId;
            p.rpos(0);
            p >> emoteId >> source;
            if (source.IsPlayer())
                botOutgoingPacketHandlers.AddPacket(packet);

            return;
        }
        case SMSG_MESSAGECHAT:  // do not react to self or if not ready to reply
        {
            if (!sPlayerbotAIConfig->randomBotTalk)
                return;

            if (!AllowActivity())
                return;

            WorldPacket p(packet);
            if (!p.empty() && (p.GetOpcode() == SMSG_MESSAGECHAT || p.GetOpcode() == SMSG_GM_MESSAGECHAT))
            {
                p.rpos(0);
                uint8 msgtype, chatTag;
                uint32 lang, textLen, nameLen, unused;
                ObjectGuid guid1, guid2;
                std::string name = "";
                std::string chanName = "";
                std::string message = "";

                p >> msgtype >> lang;
                p >> guid1 >> unused;
                if (guid1.IsEmpty() || p.size() > p.DEFAULT_SIZE)
                    return;

                if (p.GetOpcode() == SMSG_GM_MESSAGECHAT)
                {
                    p >> textLen;
                    p >> name;
                }

                switch (msgtype)
                {
                    case CHAT_MSG_CHANNEL:
                        p >> chanName;
                        [[fallthrough]];
                    case CHAT_MSG_SAY:
                    case CHAT_MSG_PARTY:
                    case CHAT_MSG_YELL:
                    case CHAT_MSG_WHISPER:
                    case CHAT_MSG_GUILD:
                        p >> guid2;
                        p >> textLen >> message >> chatTag;
                        break;
                    default:
                        return;
                }

                // do not reply to self but always try to reply to real player
                if (guid1 != bot->GetGUID())
                {
                    time_t lastChat = GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Get();
                    bool isPaused = time(0) < lastChat;
                    bool shouldReply = false;
                    bool isFromFreeBot = false;
                    sCharacterCache->GetCharacterNameByGuid(guid1, name);
                    uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(guid1);
                    isFromFreeBot = sPlayerbotAIConfig->IsInRandomAccountList(accountId);
                    bool isMentioned = message.find(bot->GetName()) != std::string::npos;

                    ChatChannelSource chatChannelSource = GetChatChannelSource(bot, msgtype, chanName);

                    // random bot speaks, chat CD
                    if (isFromFreeBot && isPaused)
                        return;

                    // BG: react only if mentioned or if not channel and real player spoke
                    if (bot->InBattleground() && !(isMentioned || (msgtype != CHAT_MSG_CHANNEL && !isFromFreeBot)))
                        return;

                    if (HasRealPlayerMaster() && guid1 != GetMaster()->GetGUID())
                        return;
                    if (lang == LANG_ADDON)
                        return;

                    if (message.starts_with(sPlayerbotAIConfig->toxicLinksPrefix) &&
                        (GetChatHelper()->ExtractAllItemIds(message).size() > 0 ||
                        GetChatHelper()->ExtractAllQuestIds(message).size() > 0) &&
                        sPlayerbotAIConfig->toxicLinksRepliesChance)
                    {
                        if (urand(0, 50) > 0 || urand(1, 100) > sPlayerbotAIConfig->toxicLinksRepliesChance)
                        {
                            return;
                        }
                    }
                    else if ((GetChatHelper()->ExtractAllItemIds(message).count(19019) &&
                            sPlayerbotAIConfig->thunderfuryRepliesChance))
                    {
                        if (urand(0, 60) > 0 || urand(1, 100) > sPlayerbotAIConfig->thunderfuryRepliesChance)
                        {
                            return;
                        }
                    }
                    else
                    {
                        if (isFromFreeBot && urand(0, 20))
                            return;

                        // if (msgtype == CHAT_MSG_GUILD && (!sPlayerbotAIConfig->guildRepliesRate || urand(1, 100) >=
                        // sPlayerbotAIConfig->guildRepliesRate)) return;

                        if (!isFromFreeBot)
                        {
                            if (!isMentioned && urand(0, 4))
                                return;
                        }
                        else
                        {
                            if (urand(0, 20 + 10 * isMentioned))
                                return;
                        }
                    }

                    QueueChatResponse(
                        std::move(ChatQueuedReply{msgtype, guid1.GetCounter(), guid2.GetCounter(), message, chanName,
                                                  name, time(nullptr) + urand(inCombat ? 10 : 5, inCombat ? 25 : 15)}));
                    GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 25));
                    return;
                }
            }

            return;
        }
        case SMSG_MOVE_KNOCK_BACK:  // handle knockbacks
        {
            WorldPacket p(packet);
            p.rpos(0);

            ObjectGuid guid;
            uint32 counter;
            float vcos, vsin, horizontalSpeed, verticalSpeed = 0.f;

            p >> guid.ReadAsPacked() >> counter >> vcos >> vsin >> horizontalSpeed >> verticalSpeed;
            if (horizontalSpeed <= 0.1f)
            {
                horizontalSpeed = 0.11f;
            }
            verticalSpeed = -verticalSpeed;
            // high vertical may result in stuck as bot can not handle gravity
            if (verticalSpeed > 35.0f)
                break;
            // stop casting
            InterruptSpell();

            // stop movement
            bot->StopMoving();
            bot->GetMotionMaster()->Clear();

            Unit* currentTarget = GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
            bot->GetMotionMaster()->MoveKnockbackFromForPlayer(bot->GetPositionX() - vcos, bot->GetPositionY() - vsin,
                                                               horizontalSpeed, verticalSpeed);

            // bot->AddUnitMovementFlag(MOVEMENTFLAG_FALLING);
            // bot->AddUnitMovementFlag(MOVEMENTFLAG_FORWARD);
            // bot->m_movementInfo.AddMovementFlag(MOVEMENTFLAG_PENDING_STOP);
            // if (bot->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_SPLINE_ELEVATION))
            //     bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_SPLINE_ELEVATION);
            // bot->GetMotionMaster()->MoveIdle();
            // Position dest = bot->GetPosition();
            // float moveTimeHalf = verticalSpeed / Movement::gravity;
            // float dist = 2 * moveTimeHalf * horizontalSpeed;
            // float max_height = -Movement::computeFallElevation(moveTimeHalf, false, -verticalSpeed);

            // Use a mmap raycast to get a valid destination.
            // bot->GetMotionMaster()->MoveKnockbackFrom(fx, fy, horizontalSpeed, verticalSpeed);

            // // set delay based on actual distance
            // float newdis = sqrt(sServerFacade->GetDistance2d(bot, fx, fy));
            // SetNextCheckDelay((uint32)((newdis / dis) * moveTimeHalf * 4 * IN_MILLISECONDS));

            // // add moveflags

            // // copy MovementInfo
            // MovementInfo movementInfo = bot->m_movementInfo;

            // // send ack
            // WorldPacket ack(CMSG_MOVE_KNOCK_BACK_ACK);
            // // movementInfo.jump.cosAngle = vcos;
            // // movementInfo.jump.sinAngle = vsin;
            // // movementInfo.jump.zspeed = -verticalSpeed;
            // // movementInfo.jump.xyspeed = horizontalSpeed;
            // ack << bot->GetGUID().WriteAsPacked();
            // // bot->m_mover->BuildMovementPacket(&ack);
            // ack << (uint32)0;
            // bot->BuildMovementPacket(&ack);
            // // ack << movementInfo.jump.sinAngle;
            // // ack << movementInfo.jump.cosAngle;
            // // ack << movementInfo.jump.xyspeed;
            // // ack << movementInfo.jump.zspeed;
            // bot->GetSession()->HandleMoveKnockBackAck(ack);

            // // // set jump destination for MSG_LAND packet
            // SetJumpDestination(Position(x, y, z, bot->GetOrientation()));

            // bot->Heart();

            // */
            return;
        }
        default:
            botOutgoingPacketHandlers.AddPacket(packet);
    }
}

void PlayerbotAI::SpellInterrupted(uint32 spellid)
{
    for (uint8 type = CURRENT_MELEE_SPELL; type <= CURRENT_CHANNELED_SPELL; type++)
    {
        Spell* spell = bot->GetCurrentSpell((CurrentSpellTypes)type);
        if (!spell)
            continue;
        if (spell->GetSpellInfo()->Id == spellid)
            bot->InterruptSpell((CurrentSpellTypes)type);
    }
    // LastSpellCast& lastSpell = aiObjectContext->GetValue<LastSpellCast&>("last spell cast")->Get();
    // lastSpell.id = 0;
}

int32 PlayerbotAI::CalculateGlobalCooldown(uint32 spellid)
{
    if (!spellid)
        return 0;

    if (bot->HasSpellCooldown(spellid))
        return sPlayerbotAIConfig->globalCoolDown;

    return sPlayerbotAIConfig->reactDelay;
}

void PlayerbotAI::HandleMasterIncomingPacket(WorldPacket const& packet)
{
    masterIncomingPacketHandlers.AddPacket(packet);
}

void PlayerbotAI::HandleMasterOutgoingPacket(WorldPacket const& packet)
{
    masterOutgoingPacketHandlers.AddPacket(packet);
}

void PlayerbotAI::ChangeEngine(BotState type)
{
    Engine* engine = engines[type];

    if (currentEngine != engine)
    {
        currentEngine = engine;
        currentState = type;
        ReInitCurrentEngine();

        switch (type)
        {
            case BOT_STATE_COMBAT:
                // LOG_DEBUG("playerbots",  "=== {} COMBAT ===", bot->GetName().c_str());
                break;
            case BOT_STATE_NON_COMBAT:
                // LOG_DEBUG("playerbots",  "=== {} NON-COMBAT ===", bot->GetName().c_str());
                break;
            case BOT_STATE_DEAD:
                // LOG_DEBUG("playerbots",  "=== {} DEAD ===", bot->GetName().c_str());
                break;
            default:
                break;
        }
    }
}

void PlayerbotAI::DoNextAction(bool min)
{
    if (!bot->IsInWorld() || bot->IsBeingTeleported() || (GetMaster() && GetMaster()->IsBeingTeleported()))
    {
        SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
        return;
    }

    if (bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
    {
        SetNextCheckDelay(sPlayerbotAIConfig->passiveDelay);
        return;
    }

    // change engine if just died
    if (currentEngine != engines[BOT_STATE_DEAD] && !bot->IsAlive())
    {
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();
        bot->GetMotionMaster()->MoveIdle();

        // Death Count to prevent skeleton piles
        Player* master = GetMaster();  // warning here - whipowill
        if (!HasActivePlayerMaster() && !bot->InBattleground())
        {
            uint32 dCount = aiObjectContext->GetValue<uint32>("death count")->Get();
            aiObjectContext->GetValue<uint32>("death count")->Set(++dCount);
        }

        aiObjectContext->GetValue<Unit*>("current target")->Set(nullptr);
        aiObjectContext->GetValue<Unit*>("enemy player target")->Set(nullptr);
        aiObjectContext->GetValue<ObjectGuid>("pull target")->Set(ObjectGuid::Empty);
        aiObjectContext->GetValue<LootObject>("loot target")->Set(LootObject());

        ChangeEngine(BOT_STATE_DEAD);
        return;
    }

    // change engine if just ressed
    if (currentEngine == engines[BOT_STATE_DEAD] && bot->IsAlive())
    {
        ChangeEngine(BOT_STATE_NON_COMBAT);
        return;
    }

    // if in combat but stick with old data - clear targets
    if (currentEngine == engines[BOT_STATE_NON_COMBAT] && bot->IsInCombat())
    {
        if (aiObjectContext->GetValue<Unit*>("current target")->Get() != nullptr ||
            aiObjectContext->GetValue<ObjectGuid>("pull target")->Get() != ObjectGuid::Empty ||
            aiObjectContext->GetValue<Unit*>("dps target")->Get() != nullptr)
        {
            Reset();
        }
    }

    bool minimal = !AllowActivity();

    currentEngine->DoNextAction(nullptr, 0, (minimal || min));

    if (minimal)
    {
        if (!bot->isAFK() && !bot->InBattleground() && !HasRealPlayerMaster())
            bot->ToggleAFK();

        SetNextCheckDelay(sPlayerbotAIConfig->passiveDelay);
        return;
    }
    else if (bot->isAFK())
        bot->ToggleAFK();

    Group* group = bot->GetGroup();
    PlayerbotAI* masterBotAI = nullptr;
    if (master)
        masterBotAI = GET_PLAYERBOT_AI(master);

    // test BG master set
    if ((!master || (masterBotAI && !masterBotAI->IsRealPlayer())) && group)
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI)
        {
            return;
        }
        // Ideally we want to have the leader as master.
        Player* newMaster = botAI->GetGroupMaster();
        Player* playerMaster = nullptr;

        // Are there any non-bot players in the group?
        if (!newMaster || GET_PLAYERBOT_AI(newMaster))
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* member = gref->GetSource();

                if (!member)
                    continue;

                if (member == bot)
                    continue;

                if (member == newMaster)
                    continue;

                if (!member->IsInWorld())
                    continue;

                if (!member->IsInSameRaidWith(bot))
                    continue;

                PlayerbotAI* memberBotAI = GET_PLAYERBOT_AI(member);
                if (memberBotAI)
                {
                    if (memberBotAI->IsRealPlayer() && !bot->InBattleground())
                        playerMaster = member;

                    continue;
                }

                // same BG
                if (bot->InBattleground() && bot->GetBattleground() &&
                    bot->GetBattleground()->GetBgTypeID() == BATTLEGROUND_AV && !GET_PLAYERBOT_AI(member) &&
                    member->InBattleground() && bot->GetMapId() == member->GetMapId())
                {
                    // TODO disable move to objective if have master in bg
                    continue;

                    if (!group->SameSubGroup(bot, member))
                        continue;

                    if (member->GetLevel() < bot->GetLevel())
                        continue;

                    // follow real player only if he has more honor/arena points
                    if (bot->GetBattleground()->isArena())
                    {
                        if (group->IsLeader(member->GetGUID()))
                        {
                            playerMaster = member;
                            break;
                        }
                        else
                            continue;
                    }
                    else
                    {
                        uint32 honorpts = member->GetHonorPoints();
                        if (bot->GetHonorPoints() && honorpts < bot->GetHonorPoints())
                            continue;
                    }

                    playerMaster = member;
                    continue;
                }

                if (bot->InBattleground())
                    continue;

                newMaster = member;

                break;
            }
        }

        if (!newMaster && playerMaster)
            newMaster = playerMaster;

        if (newMaster && (!master || master != newMaster) && bot != newMaster)
        {
            master = newMaster;
            botAI->SetMaster(newMaster);
            botAI->ResetStrategies();
            botAI->ChangeStrategy("+follow", BOT_STATE_NON_COMBAT);

            if (botAI->GetMaster() == botAI->GetGroupMaster())
                botAI->TellMaster("Hello, I follow you!");
            else
                botAI->TellMaster(!urand(0, 2) ? "Hello!" : "Hi!");
        }
    }

    if (master && master->IsInWorld())
    {
        if (master->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING) &&
            sServerFacade->GetDistance2d(bot, master) < 20.0f)
            bot->m_movementInfo.AddMovementFlag(MOVEMENTFLAG_WALKING);
        else
            bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_WALKING);

        if (master->IsSitState() && nextAICheckDelay < 1000)
        {
            if (!bot->isMoving() && sServerFacade->GetDistance2d(bot, master) < 10.0f)
                bot->SetStandState(UNIT_STAND_STATE_SIT);
        }
        else if (nextAICheckDelay < 1000)
            bot->SetStandState(UNIT_STAND_STATE_STAND);
    }
    else if (bot->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING))
        bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_WALKING);
    else if ((nextAICheckDelay < 1000) && bot->IsSitState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    bool hasMountAura = bot->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED) ||
                        bot->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED);
    if (hasMountAura && !bot->IsMounted())
    {
        bot->RemoveAurasByType(SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED);
        bot->RemoveAurasByType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED);
    }

    // if (bot->IsFlying() && !bot->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) &&
    // !bot->HasAuraType(SPELL_AURA_FLY))
    // {
    //     if (bot->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FLYING))
    //         bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_FLYING);

    //     if (bot->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_CAN_FLY))
    //         bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_CAN_FLY);

    //     if (bot->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY))
    //         bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
    // }

    /*
        // land after kncokback/jump
        if (bot->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING))
        {
            // stop movement
            bot->StopMoving();
            bot->GetMotionMaster()->Clear();
            bot->GetMotionMaster()->MoveIdle();

            // remove moveFlags
            bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_FALLING);
            bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_PENDING_STOP);

            // set jump destination
            bot->m_movementInfo.pos = !GetJumpDestination().m_positionZ == 0 ? GetJumpDestination() :
       bot->GetPosition(); bot->m_movementInfo.jump = MovementInfo::JumpInfo();

            WorldPacket land(MSG_MOVE_FALL_LAND);
            land << bot->GetGUID().WriteAsPacked();
            bot->m_mover->BuildMovementPacket(&land);
            bot->GetSession()->HandleMovementOpcodes(land);

            // move stop
            WorldPacket stop(MSG_MOVE_STOP);
            stop << bot->GetGUID().WriteAsPacked();
            bot->m_mover->BuildMovementPacket(&stop);
            bot->GetSession()->HandleMovementOpcodes(stop);

            ResetJumpDestination();
        }
    */
}

void PlayerbotAI::ReInitCurrentEngine()
{
    // InterruptSpell();
    currentEngine->Init();
}

void PlayerbotAI::ChangeStrategy(std::string const names, BotState type)
{
    Engine* e = engines[type];
    if (!e)
        return;

    e->ChangeStrategy(names);
}

void PlayerbotAI::ClearStrategies(BotState type)
{
    Engine* e = engines[type];
    if (!e)
        return;

    e->removeAllStrategies();
}

std::vector<std::string> PlayerbotAI::GetStrategies(BotState type)
{
    Engine* e = engines[type];
    if (!e)
        return std::vector<std::string>();

    return e->GetStrategies();
}

void PlayerbotAI::ApplyInstanceStrategies(uint32 mapId, bool tellMaster)
{
    std::string strategyName;
    switch (mapId)
    {
        case 409:
            strategyName = "mc";
            break;
        case 469:
            strategyName = "bwl";
            break;
        case 509:
            strategyName = "aq20";
            break;
        case 533:
            strategyName = "naxx";
            break;
        case 574:
            strategyName = "wotlk-uk";      // Utgarde Keep
            break;
        case 575:
            strategyName = "wotlk-up";      // Utgarde Pinnacle
            break;
        case 576:
            strategyName = "wotlk-nex";     // The Nexus
            break;
        case 578:
            strategyName = "wotlk-occ";     // The Oculus
            break;
        case 595:
            strategyName = "wotlk-cos";     // The Culling of Stratholme
            break;
        case 599:
            strategyName = "wotlk-hos";     // Halls of Stone
            break;
        case 600:
            strategyName = "wotlk-dtk";     // Drak'Tharon Keep
            break;
        case 601:
            strategyName = "wotlk-an";      // Azjol-Nerub
            break;
        case 602:
            strategyName = "wotlk-hol";     // Halls of Lightning
            break;
        case 603:
            strategyName = "uld";
            break;
        case 604:
            strategyName = "wotlk-gd";      // Gundrak
            break;
        case 608:
            strategyName = "wotlk-vh";      // Violet Hold
            break;
        case 619:
            strategyName = "wotlk-ok";      // Ahn'kahet: The Old Kingdom
            break;
        case 631:
            strategyName = "icc";
            break;
        case 632:
            strategyName = "wotlk-fos";     // The Forge of Souls
            break;
        case 650:
            strategyName = "wotlk-toc";     // Trial of the Champion
            break;
        case 658:
            strategyName = "wotlk-pos";     // Pit of Saron
            break;
        case 668:
            strategyName = "wotlk-hor";     // Halls of Reflection
            break;
        default:
            break;
    }
    if (strategyName.empty())
        return;
    engines[BOT_STATE_COMBAT]->addStrategy(strategyName);
    engines[BOT_STATE_NON_COMBAT]->addStrategy(strategyName);
    if (tellMaster && !strategyName.empty())
    {
        std::ostringstream out;
        out << "Add " << strategyName << " instance strategy";
        TellMasterNoFacing(out.str());
    }
}

bool PlayerbotAI::DoSpecificAction(std::string const name, Event event, bool silent, std::string const qualifier)
{
    std::ostringstream out;

    for (uint8 i = 0; i < BOT_STATE_MAX; i++)
    {
        ActionResult res = engines[i]->ExecuteAction(name, event, qualifier);
        switch (res)
        {
            case ACTION_RESULT_UNKNOWN:
                continue;
            case ACTION_RESULT_OK:
                if (!silent)
                {
                    PlaySound(TEXT_EMOTE_NOD);
                }
                return true;
            case ACTION_RESULT_IMPOSSIBLE:
                out << name << ": impossible";
                if (!silent)
                {
                    TellError(out.str());
                    PlaySound(TEXT_EMOTE_NO);
                }
                return false;
            case ACTION_RESULT_USELESS:
                out << name << ": useless";
                if (!silent)
                {
                    TellError(out.str());
                    PlaySound(TEXT_EMOTE_NO);
                }
                return false;
            case ACTION_RESULT_FAILED:
                if (!silent)
                {
                    out << name << ": failed";
                    TellError(out.str());
                }
                return false;
        }
    }

    if (!silent)
    {
        out << name << ": unknown action";
        TellError(out.str());
    }

    return false;
}

bool PlayerbotAI::PlaySound(uint32 emote)
{
    if (EmotesTextSoundEntry const* soundEntry = FindTextSoundEmoteFor(emote, bot->getRace(), bot->getGender()))
    {
        bot->PlayDistanceSound(soundEntry->SoundId);
        return true;
    }

    return false;
}

bool PlayerbotAI::PlayEmote(uint32 emote)
{
    WorldPacket data(SMSG_TEXT_EMOTE);
    data << (TextEmotes)emote;
    data << EmoteAction::GetNumberOfEmoteVariants((TextEmotes)emote, bot->getRace(), bot->getGender());
    data << ((master && (sServerFacade->GetDistance2d(bot, master) < 30.0f) && urand(0, 1)) ? master->GetGUID()
             : (bot->GetTarget() && urand(0, 1))                                            ? bot->GetTarget()
                                                                                            : ObjectGuid::Empty);
    bot->GetSession()->HandleTextEmoteOpcode(data);

    return false;
}

bool PlayerbotAI::ContainsStrategy(StrategyType type)
{
    for (uint8 i = 0; i < BOT_STATE_MAX; i++)
    {
        if (engines[i]->HasStrategyType(type))
            return true;
    }

    return false;
}

bool PlayerbotAI::HasStrategy(std::string const name, BotState type) { return engines[type]->HasStrategy(name); }

void PlayerbotAI::ResetStrategies(bool load)
{
    for (uint8 i = 0; i < BOT_STATE_MAX; i++)
        engines[i]->removeAllStrategies();

    AiFactory::AddDefaultCombatStrategies(bot, this, engines[BOT_STATE_COMBAT]);
    AiFactory::AddDefaultNonCombatStrategies(bot, this, engines[BOT_STATE_NON_COMBAT]);
    AiFactory::AddDefaultDeadStrategies(bot, this, engines[BOT_STATE_DEAD]);
    if (sPlayerbotAIConfig->applyInstanceStrategies)
        ApplyInstanceStrategies(bot->GetMapId());

    for (uint8 i = 0; i < BOT_STATE_MAX; i++)
        engines[i]->Init();

    // if (load)
    //     sPlayerbotDbStore->Load(this);
}

bool PlayerbotAI::IsRanged(Player* player, bool bySpec)
{
    PlayerbotAI* botAi = GET_PLAYERBOT_AI(player);
    if (!bySpec && botAi)
        return botAi->ContainsStrategy(STRATEGY_TYPE_RANGED);

    int tab = AiFactory::GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_DEATH_KNIGHT:
        case CLASS_WARRIOR:
        case CLASS_ROGUE:
            return false;
            break;
        case CLASS_DRUID:
            if (tab == 1)
            {
                return false;
            }
            break;
        case CLASS_PALADIN:
            if (tab != 0)
            {
                return false;
            }
            break;
        case CLASS_SHAMAN:
            if (tab == 1)
            {
                return false;
            }
            break;
    }
    return true;
}

bool PlayerbotAI::IsMelee(Player* player, bool bySpec) { return !IsRanged(player, bySpec); }

bool PlayerbotAI::IsCaster(Player* player, bool bySpec) { return IsRanged(player, bySpec) && player->getClass() != CLASS_HUNTER; }

bool PlayerbotAI::IsCombo(Player* player, bool bySpec)
{
    // int tab = AiFactory::GetPlayerSpecTab(player);
    return player->getClass() == CLASS_ROGUE ||
           (player->getClass() == CLASS_DRUID && player->HasAura(768));  // cat druid
}

bool PlayerbotAI::IsRangedDps(Player* player, bool bySpec) { return IsRanged(player, bySpec) && IsDps(player, bySpec); }

bool PlayerbotAI::IsHealAssistantOfIndex(Player* player, int index)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }
    Group::MemberSlotList const& slots = group->GetMemberSlots();
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (group->IsAssistant(member->GetGUID()) && IsHeal(member))
        {
            if (index == counter)
            {
                return player == member;
            }
            counter++;
        }
    }
    // not enough
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!group->IsAssistant(member->GetGUID()) && IsHeal(member))
        {
            if (index == counter)
            {
                return player == member;
            }
            counter++;
        }
    }
    return false;
}

bool PlayerbotAI::IsRangedDpsAssistantOfIndex(Player* player, int index)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }
    Group::MemberSlotList const& slots = group->GetMemberSlots();
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (group->IsAssistant(member->GetGUID()) && IsRangedDps(member))
        {
            if (index == counter)
            {
                return player == member;
            }
            counter++;
        }
    }
    // not enough
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!group->IsAssistant(member->GetGUID()) && IsRangedDps(member))
        {
            if (index == counter)
            {
                return player == member;
            }
            counter++;
        }
    }
    return false;
}

bool PlayerbotAI::HasAggro(Unit* unit)
{
    if (!unit)
    {
        return false;
    }
    bool isMT = IsMainTank(bot);
    Unit* victim = unit->GetVictim();
    if (victim && (victim->GetGUID() == bot->GetGUID() || (!isMT && victim->ToPlayer() && IsTank(victim->ToPlayer()))))
    {
        return true;
    }
    return false;
}

int32 PlayerbotAI::GetGroupSlotIndex(Player* player)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return -1;
    }
    Group::MemberSlotList const& slots = group->GetMemberSlots();
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (player == member)
        {
            return counter;
        }
        counter++;
    }
    return 0;
}

int32 PlayerbotAI::GetRangedIndex(Player* player)
{
    if (!IsRanged(player))
    {
        return -1;
    }
    Group* group = bot->GetGroup();
    if (!group)
    {
        return -1;
    }
    Group::MemberSlotList const& slots = group->GetMemberSlots();
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (player == member)
        {
            return counter;
        }
        if (IsRanged(member))
        {
            counter++;
        }
    }
    return 0;
}

int32 PlayerbotAI::GetClassIndex(Player* player, uint8_t cls)
{
    if (player->getClass() != cls)
    {
        return -1;
    }
    Group* group = bot->GetGroup();
    if (!group)
    {
        return -1;
    }
    Group::MemberSlotList const& slots = group->GetMemberSlots();
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (player == member)
        {
            return counter;
        }
        if (member->getClass() == cls)
        {
            counter++;
        }
    }
    return 0;
}
int32 PlayerbotAI::GetRangedDpsIndex(Player* player)
{
    if (!IsRangedDps(player))
    {
        return -1;
    }
    Group* group = bot->GetGroup();
    if (!group)
    {
        return -1;
    }
    Group::MemberSlotList const& slots = group->GetMemberSlots();
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (player == member)
        {
            return counter;
        }
        if (IsRangedDps(member))
        {
            counter++;
        }
    }
    return 0;
}

int32 PlayerbotAI::GetMeleeIndex(Player* player)
{
    if (IsRanged(player))
    {
        return -1;
    }
    Group* group = bot->GetGroup();
    if (!group)
    {
        return -1;
    }
    Group::MemberSlotList const& slots = group->GetMemberSlots();
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (player == member)
        {
            return counter;
        }
        if (!IsRanged(member))
        {
            counter++;
        }
    }
    return 0;
}

bool PlayerbotAI::IsTank(Player* player, bool bySpec)
{
    PlayerbotAI* botAi = GET_PLAYERBOT_AI(player);
    if (!bySpec && botAi)
        return botAi->ContainsStrategy(STRATEGY_TYPE_TANK);

    int tab = AiFactory::GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            if (tab == DEATHKNIGHT_TAB_BLOOD)
            {
                return true;
            }
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_PROTECTION)
            {
                return true;
            }
            break;
        case CLASS_WARRIOR:
            if (tab == WARRIOR_TAB_PROTECTION)
            {
                return true;
            }
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_FERAL && (player->GetShapeshiftForm() == FORM_BEAR ||
                                           player->GetShapeshiftForm() == FORM_DIREBEAR || player->HasAura(16931)))
            {
                return true;
            }
            break;
    }
    return false;
}

bool PlayerbotAI::IsHeal(Player* player, bool bySpec)
{
    PlayerbotAI* botAi = GET_PLAYERBOT_AI(player);
    if (!bySpec && botAi)
        return botAi->ContainsStrategy(STRATEGY_TYPE_HEAL);

    int tab = AiFactory::GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_PRIEST:
            if (tab == PRIEST_TAB_DISIPLINE || tab == PRIEST_TAB_HOLY)
            {
                return true;
            }
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_RESTORATION)
            {
                return true;
            }
            break;
        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_RESTORATION)
            {
                return true;
            }
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_HOLY)
            {
                return true;
            }
            break;
    }
    return false;
}

bool PlayerbotAI::IsDps(Player* player, bool bySpec)
{
    PlayerbotAI* botAi = GET_PLAYERBOT_AI(player);
    if (!bySpec && botAi)
        return botAi->ContainsStrategy(STRATEGY_TYPE_DPS);

    int tab = AiFactory::GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_MAGE:
        case CLASS_WARLOCK:
        case CLASS_HUNTER:
        case CLASS_ROGUE:
            return true;
        case CLASS_PRIEST:
            if (tab == PRIEST_TAB_SHADOW)
            {
                return true;
            }
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_BALANCE)
            {
                return true;
            }
            if (tab == DRUID_TAB_FERAL && !IsTank(player, bySpec))
            {
                return true;
            }
            break;
        case CLASS_SHAMAN:
            if (tab != SHAMAN_TAB_RESTORATION)
            {
                return true;
            }
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_RETRIBUTION)
            {
                return true;
            }
            break;
        case CLASS_DEATH_KNIGHT:
            if (tab != DEATHKNIGHT_TAB_BLOOD)
            {
                return true;
            }
            break;
        case CLASS_WARRIOR:
            if (tab != WARRIOR_TAB_PROTECTION)
            {
                return true;
            }
            break;
    }
    return false;
}

bool PlayerbotAI::IsMainTank(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
    {
        return false;
    }
    ObjectGuid mainTank = ObjectGuid();
    Group::MemberSlotList const& slots = group->GetMemberSlots();
    for (Group::member_citerator itr = slots.begin(); itr != slots.end(); ++itr)
    {
        if (itr->flags & MEMBER_FLAG_MAINTANK)
        {
            mainTank = itr->guid;
            break;
        }
    }
    if (mainTank != ObjectGuid::Empty)
    {
        return player->GetGUID() == mainTank;
    }
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (IsTank(member) && member->IsAlive())
        {
            return player->GetGUID() == member->GetGUID();
        }
    }
    return false;
}

uint32 PlayerbotAI::GetGroupTankNum(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
    {
        return 0;
    }
    uint32 result = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (IsTank(member) && member->IsAlive())
        {
            result++;
        }
    }
    return result;
}

bool PlayerbotAI::IsAssistTank(Player* player) { return IsTank(player) && !IsMainTank(player); }

bool PlayerbotAI::IsAssistTankOfIndex(Player* player, int index)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }
    Group::MemberSlotList const& slots = group->GetMemberSlots();
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (group->IsAssistant(member->GetGUID()) && IsAssistTank(member))
        {
            if (index == counter)
            {
                return player == member;
            }
            counter++;
        }
    }
    // not enough
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!group->IsAssistant(member->GetGUID()) && IsAssistTank(member))
        {
            if (index == counter)
            {
                return player == member;
            }
            counter++;
        }
    }
    return false;
}

namespace acore
{
class UnitByGuidInRangeCheck
{
public:
    UnitByGuidInRangeCheck(WorldObject const* obj, ObjectGuid guid, float range)
        : i_obj(obj), i_range(range), i_guid(guid)
    {
    }
    WorldObject const& GetFocusObject() const { return *i_obj; }
    bool operator()(Unit* u) { return u->GetGUID() == i_guid && i_obj->IsWithinDistInMap(u, i_range); }

private:
    WorldObject const* i_obj;
    float i_range;
    ObjectGuid i_guid;
};

class GameObjectByGuidInRangeCheck
{
public:
    GameObjectByGuidInRangeCheck(WorldObject const* obj, ObjectGuid guid, float range)
        : i_obj(obj), i_range(range), i_guid(guid)
    {
    }
    WorldObject const& GetFocusObject() const { return *i_obj; }
    bool operator()(GameObject* u)
    {
        if (u && i_obj->IsWithinDistInMap(u, i_range) && u->isSpawned() && u->GetGOInfo() && u->GetGUID() == i_guid)
            return true;

        return false;
    }

private:
    WorldObject const* i_obj;
    float i_range;
    ObjectGuid i_guid;
};
};  // namespace acore

Unit* PlayerbotAI::GetUnit(ObjectGuid guid)
{
    if (!guid)
        return nullptr;

    return ObjectAccessor::GetUnit(*bot, guid);
}

Player* PlayerbotAI::GetPlayer(ObjectGuid guid)
{
    Unit* unit = GetUnit(guid);
    return unit ? unit->ToPlayer() : nullptr;
}

uint32 GetCreatureIdForCreatureTemplateId(uint32 creatureTemplateId)
{
    QueryResult results =
        WorldDatabase.Query("SELECT guid FROM `creature` WHERE id1 = {} LIMIT 1;", creatureTemplateId);
    if (results)
    {
        Field* fields = results->Fetch();
        return fields[0].Get<uint32>();
    }
    return 0;
}

Unit* PlayerbotAI::GetUnit(CreatureData const* creatureData)
{
    if (!creatureData)
        return nullptr;

    Map* map = sMapMgr->FindMap(creatureData->mapid, 0);
    if (!map)
        return nullptr;

    uint32 spawnId = creatureData->spawnId;
    if (!spawnId)  // workaround for CreatureData with missing spawnId (this just uses first matching creatureId in DB,
                   // but thats ok this method is only used for battlemasters and theres only 1 of each type)
        spawnId = GetCreatureIdForCreatureTemplateId(creatureData->id1);
    auto creatureBounds = map->GetCreatureBySpawnIdStore().equal_range(spawnId);
    if (creatureBounds.first == creatureBounds.second)
        return nullptr;

    return creatureBounds.first->second;
}

Creature* PlayerbotAI::GetCreature(ObjectGuid guid)
{
    if (!guid)
        return nullptr;

    return ObjectAccessor::GetCreature(*bot, guid);
}

GameObject* PlayerbotAI::GetGameObject(ObjectGuid guid)
{
    if (!guid)
        return nullptr;

    return ObjectAccessor::GetGameObject(*bot, guid);
}

// GameObject* PlayerbotAI::GetGameObject(GameObjectData const* gameObjectData)
// {
//     if (!gameObjectData)
//         return nullptr;

//     Map* map = sMapMgr->FindMap(gameObjectData->mapid, 0);
//     if (!map)
//         return nullptr;

//     auto gameobjectBounds = map->GetGameObjectBySpawnIdStore().equal_range(gameObjectData->spawnId);
//     if (gameobjectBounds.first == gameobjectBounds.second)
//         return nullptr;

//     return gameobjectBounds.first->second;
// }

WorldObject* PlayerbotAI::GetWorldObject(ObjectGuid guid)
{
    if (!guid)
        return nullptr;

    return ObjectAccessor::GetWorldObject(*bot, guid);
}

const AreaTableEntry* PlayerbotAI::GetCurrentArea()
{
    return sAreaTableStore.LookupEntry(
        bot->GetMap()->GetAreaId(bot->GetPhaseMask(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()));
}

const AreaTableEntry* PlayerbotAI::GetCurrentZone()
{
    return sAreaTableStore.LookupEntry(
        bot->GetMap()->GetZoneId(bot->GetPhaseMask(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()));
}

std::string PlayerbotAI::GetLocalizedAreaName(const AreaTableEntry* entry)
{
    if (entry)
        return entry->area_name[sWorld->GetDefaultDbcLocale()];

    return "";
}

std::vector<Player*> PlayerbotAI::GetPlayersInGroup()
{
    std::vector<Player*> members;

    Group* group = bot->GetGroup();

    if (!group)
        return members;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (GET_PLAYERBOT_AI(member) && !GET_PLAYERBOT_AI(member)->IsRealPlayer())
            continue;

        members.push_back(ref->GetSource());
    }

    return members;
}

bool PlayerbotAI::SayToGuild(const std::string& msg)
{
    if (msg.empty())
    {
        return false;
    }

    if (bot->GetGuildId())
    {
        if (Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId()))
        {
            if (!guild->HasRankRight(bot, GR_RIGHT_GCHATSPEAK))
            {
                return false;
            }
            guild->BroadcastToGuild(bot->GetSession(), false, msg.c_str(), LANG_UNIVERSAL);
            return true;
        }
    }

    return false;
}

bool PlayerbotAI::SayToWorld(const std::string& msg)
{
    if (msg.empty())
    {
        return false;
    }

    ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId());
    if (!cMgr)
        return false;

    // no zone
    if (Channel* worldChannel = cMgr->GetChannel("World", bot))
    {
        worldChannel->Say(bot->GetGUID(), msg.c_str(), LANG_UNIVERSAL);
        return true;
    }

    return false;
}

bool PlayerbotAI::SayToChannel(const std::string& msg, const ChatChannelId& chanId)
{
    ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId());
    if (!cMgr || msg.empty())
        return false;

    AreaTableEntry const* current_zone = GetCurrentZone();
    if (!current_zone)
        return false;

    const auto current_str_zone = GetLocalizedAreaName(current_zone);
    for (auto const& [key, channel] : cMgr->GetChannels())
    {
        // check for current zone
        if (channel && channel->GetChannelId() == chanId)
        {
            const auto does_contains = channel->GetName().find(current_str_zone) != std::string::npos;
            if (chanId != ChatChannelId::LOOKING_FOR_GROUP && chanId != ChatChannelId::WORLD_DEFENSE && !does_contains)
            {
                continue;
            }
            else if (chanId == ChatChannelId::LOOKING_FOR_GROUP || chanId == ChatChannelId::WORLD_DEFENSE)
            {
                // check if capitals then return false if not
            }

            channel->Say(bot->GetGUID(), msg.c_str(), LANG_UNIVERSAL);
            return true;
        }
    }

    return false;
}

bool PlayerbotAI::SayToParty(const std::string& msg)
{
    if (!bot->GetGroup())
        return false;

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_PARTY, msg.c_str(), LANG_UNIVERSAL, CHAT_TAG_NONE, bot->GetGUID(),
                                 bot->GetName());

    for (auto reciever : GetPlayersInGroup())
    {
        sServerFacade->SendPacket(reciever, &data);
    }

    return true;
}

bool PlayerbotAI::SayToRaid(const std::string& msg)
{
    if (!bot->GetGroup() || bot->GetGroup()->isRaidGroup())
        return false;

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_RAID, msg.c_str(), LANG_UNIVERSAL, CHAT_TAG_NONE, bot->GetGUID(),
                                 bot->GetName());

    for (auto reciever : GetPlayersInGroup())
    {
        sServerFacade->SendPacket(reciever, &data);
    }

    return true;
}

bool PlayerbotAI::Yell(const std::string& msg)
{
    if (bot->GetTeamId() == TeamId::TEAM_ALLIANCE)
    {
        bot->Yell(msg, LANG_COMMON);
    }
    else
    {
        bot->Yell(msg, LANG_ORCISH);
    }

    return true;
}

bool PlayerbotAI::Say(const std::string& msg)
{
    if (bot->GetTeamId() == TeamId::TEAM_ALLIANCE)
    {
        bot->Say(msg, LANG_COMMON);
    }
    else
    {
        bot->Say(msg, LANG_ORCISH);
    }

    return true;
}

bool PlayerbotAI::Whisper(const std::string& msg, const std::string& receiverName)
{
    const auto receiver = ObjectAccessor::FindPlayerByName(receiverName);
    if (!receiver)
    {
        return false;
    }

    if (bot->GetTeamId() == TeamId::TEAM_ALLIANCE)
    {
        bot->Whisper(msg, LANG_COMMON, receiver);
    }
    else
    {
        bot->Whisper(msg, LANG_ORCISH, receiver);
    }

    return true;
}

bool PlayerbotAI::TellMasterNoFacing(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel)
{
    return TellMasterNoFacing(stream.str(), securityLevel);
}

bool PlayerbotAI::TellMasterNoFacing(std::string const text, PlayerbotSecurityLevel securityLevel)
{
    Player* master = GetMaster();
    PlayerbotAI* masterBotAI = nullptr;
    if (master)
        masterBotAI = GET_PLAYERBOT_AI(master);

    if ((!master || (masterBotAI && !masterBotAI->IsRealPlayer())) &&
        (sPlayerbotAIConfig->randomBotSayWithoutMaster || HasStrategy("debug", BOT_STATE_NON_COMBAT)))
    {
        bot->Say(text, (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
        return true;
    }

    if (!IsTellAllowed(securityLevel))
        return false;

    time_t lastSaid = whispers[text];

    if (!lastSaid || (time(nullptr) - lastSaid) >= sPlayerbotAIConfig->repeatDelay / 1000)
    {
        whispers[text] = time(nullptr);

        ChatMsg type = CHAT_MSG_WHISPER;
        if (currentChat.second - time(nullptr) >= 1)
            type = currentChat.first;

        WorldPacket data;
        ChatHandler::BuildChatPacket(data, type == CHAT_MSG_ADDON ? CHAT_MSG_PARTY : type,
                                     type == CHAT_MSG_ADDON ? LANG_ADDON : LANG_UNIVERSAL, bot, nullptr, text.c_str());
        master->SendDirectMessage(&data);
    }

    return true;
}

bool PlayerbotAI::TellError(std::string const text, PlayerbotSecurityLevel securityLevel)
{
    Player* master = GetMaster();
    if (!IsTellAllowed(securityLevel) || !master || GET_PLAYERBOT_AI(master))
        return false;

    if (PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(master))
        mgr->TellError(bot->GetName(), text);

    return false;
}

bool PlayerbotAI::IsTellAllowed(PlayerbotSecurityLevel securityLevel)
{
    Player* master = GetMaster();
    if (!master || master->IsBeingTeleported())
        return false;

    if (!GetSecurity()->CheckLevelFor(securityLevel, true, master))
        return false;

    if (sPlayerbotAIConfig->whisperDistance && !bot->GetGroup() && sRandomPlayerbotMgr->IsRandomBot(bot) &&
        master->GetSession()->GetSecurity() < SEC_GAMEMASTER &&
        (bot->GetMapId() != master->GetMapId() ||
         sServerFacade->GetDistance2d(bot, master) > sPlayerbotAIConfig->whisperDistance))
        return false;

    return true;
}

bool PlayerbotAI::TellMaster(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel)
{
    return TellMaster(stream.str(), securityLevel);
}

bool PlayerbotAI::TellMaster(std::string const text, PlayerbotSecurityLevel securityLevel)
{
    if (!master || !TellMasterNoFacing(text, securityLevel))
        return false;

    if (!bot->isMoving() && !bot->IsInCombat() && bot->GetMapId() == master->GetMapId() &&
        !bot->HasUnitState(UNIT_STATE_IN_FLIGHT) && !bot->IsFlying())
    {
        if (!bot->HasInArc(EMOTE_ANGLE_IN_FRONT, master, sPlayerbotAIConfig->sightDistance))
            bot->SetFacingToObject(master);

        bot->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
    }

    return true;
}

bool IsRealAura(Player* bot, AuraEffect const* aurEff, Unit const* unit)
{
    if (!aurEff)
        return false;

    if (!unit->IsHostileTo(bot))
        return true;

    SpellInfo const* spellInfo = aurEff->GetSpellInfo();

    uint32 stacks = aurEff->GetBase()->GetStackAmount();
    if (stacks >= spellInfo->StackAmount)
        return true;

    if (aurEff->GetCaster() == bot || spellInfo->IsPositive() ||
        spellInfo->Effects[aurEff->GetEffIndex()].IsAreaAuraEffect())
        return true;

    return false;
}

bool PlayerbotAI::HasAura(std::string const name, Unit* unit, bool maxStack, bool checkIsOwner, int maxAuraAmount,
                          bool checkDuration)
{
    if (!unit)
        return false;

    std::wstring wnamepart;
    if (!Utf8toWStr(name, wnamepart))
        return 0;

    wstrToLower(wnamepart);

    int auraAmount = 0;

    for (uint32 auraType = SPELL_AURA_BIND_SIGHT; auraType < TOTAL_AURAS; auraType++)
    {
        Unit::AuraEffectList const& auras = unit->GetAuraEffectsByType((AuraType)auraType);
        if (auras.empty())
            continue;

        for (AuraEffect const* aurEff : auras)
        {
            SpellInfo const* spellInfo = aurEff->GetSpellInfo();

            std::string_view const auraName = spellInfo->SpellName[0];
            if (auraName.empty() || auraName.length() != wnamepart.length() || !Utf8FitTo(auraName, wnamepart))
                continue;

            if (IsRealAura(bot, aurEff, unit))
            {
                if (checkIsOwner && aurEff)
                {
                    if (aurEff->GetCasterGUID() != bot->GetGUID())
                        continue;
                }

                if (checkDuration && aurEff)
                {
                    if (aurEff->GetBase()->GetDuration() == -1)
                    {
                        continue;
                    }
                }
                uint32 maxStackAmount = spellInfo->StackAmount;
                uint32 maxProcCharges = spellInfo->ProcCharges;

                if (maxStack)
                {
                    if (maxStackAmount && aurEff->GetBase()->GetStackAmount() >= maxStackAmount)
                        auraAmount++;

                    if (maxProcCharges && aurEff->GetBase()->GetCharges() >= maxProcCharges)
                        auraAmount++;
                }
                else
                {
                    auraAmount++;
                }
                if (maxAuraAmount < 0)
                    return auraAmount > 0;
            }
        }
    }

    if (maxAuraAmount >= 0)
    {
        return auraAmount == maxAuraAmount || (auraAmount > 0 && auraAmount <= maxAuraAmount);
    }

    return false;
}

bool PlayerbotAI::HasAura(uint32 spellId, Unit const* unit)
{
    if (!spellId || !unit)
        return false;

    return unit->HasAura(spellId);
    // for (uint8 effect = EFFECT_0; effect <= EFFECT_2; effect++)
    // {
    // 	AuraEffect const* aurEff = unit->GetAuraEffect(spellId, effect);
    // 	if (IsRealAura(bot, aurEff, unit))
    // 		return true;
    // }

    // return false;
}

Aura* PlayerbotAI::GetAura(std::string const name, Unit* unit, bool checkIsOwner, bool checkDuration, int checkStack)
{
    if (!unit)
        return nullptr;

    std::wstring wnamepart;
    if (!Utf8toWStr(name, wnamepart))
        return nullptr;

    wstrToLower(wnamepart);

    for (uint32 auraType = SPELL_AURA_BIND_SIGHT; auraType < TOTAL_AURAS; auraType++)
    {
        Unit::AuraEffectList const& auras = unit->GetAuraEffectsByType((AuraType)auraType);
        if (auras.empty())
            continue;

        for (AuraEffect const* aurEff : auras)
        {
            SpellInfo const* spellInfo = aurEff->GetSpellInfo();

            std::string const auraName = spellInfo->SpellName[0];
            if (auraName.empty() || auraName.length() != wnamepart.length() || !Utf8FitTo(auraName, wnamepart))
                continue;

            if (IsRealAura(bot, aurEff, unit))
            {
                if (checkIsOwner && aurEff)
                {
                    if (aurEff->GetCasterGUID() != bot->GetGUID())
                        continue;
                }

                if (checkDuration && aurEff)
                {
                    if (aurEff->GetBase()->GetDuration() == -1)
                    {
                        continue;
                    }
                }

                if (checkStack != -1 && aurEff)
                {
                    if (aurEff->GetBase()->GetStackAmount() < checkStack)
                    {
                        continue;
                    }
                }
                return aurEff->GetBase();
            }
        }
    }

    return nullptr;
}

bool PlayerbotAI::HasAnyAuraOf(Unit* player, ...)
{
    if (!player)
        return false;

    va_list vl;
    va_start(vl, player);

    const char* cur;
    do
    {
        cur = va_arg(vl, const char*);
        if (cur && HasAura(cur, player))
        {
            va_end(vl);
            return true;
        }
    } while (cur);

    va_end(vl);
    return false;
}

bool PlayerbotAI::CanCastSpell(std::string const name, Unit* target, Item* itemTarget)
{
    return CanCastSpell(aiObjectContext->GetValue<uint32>("spell id", name)->Get(), target, true, itemTarget);
}

bool PlayerbotAI::CanCastSpell(uint32 spellid, Unit* target, bool checkHasSpell, Item* itemTarget, Item* castItem)
{
    if (!spellid)
    {
        if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster()))
        {
            LOG_DEBUG("playerbots", "Can cast spell failed. No spellid. - spellid: {}, bot name: {}", spellid,
                      bot->GetName());
        }
        return false;
    }

    if (bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
    {
        if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster()))
        {
            LOG_DEBUG("playerbots", "Can cast spell failed. Unit state lost control. - spellid: {}, bot name: {}",
                      spellid, bot->GetName());
        }
        return false;
    }

    if (!target)
        target = bot;

    // if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster()))
    //     LOG_DEBUG("playerbots", "Can cast spell? - target name: {}, spellid: {}, bot name: {}",
    //             target->GetName(), spellid, bot->GetName());

    if (Pet* pet = bot->GetPet())
        if (pet->HasSpell(spellid))
            return true;

    if (checkHasSpell && !bot->HasSpell(spellid))
    {
        if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster()))
        {
            LOG_DEBUG("playerbots",
                      "Can cast spell failed. Bot not has spell. - target name: {}, spellid: {}, bot name: {}",
                      target->GetName(), spellid, bot->GetName());
        }
        return false;
    }

    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
    {
        if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster()))
        {
            LOG_DEBUG(
                "playerbots",
                "CanCastSpell() target name: {}, spellid: {}, bot name: {}, failed because has current channeled spell",
                target->GetName(), spellid, bot->GetName());
        }
        return false;
    }

    if (bot->HasSpellCooldown(spellid))
    {
        if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster()))
        {
            LOG_DEBUG("playerbots",
                      "Can cast spell failed. Spell not has cooldown. - target name: {}, spellid: {}, bot name: {}",
                      target->GetName(), spellid, bot->GetName());
        }
        return false;
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if (!spellInfo)
    {
        if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster()))
        {
            LOG_DEBUG("playerbots", "Can cast spell failed. No spellInfo. - target name: {}, spellid: {}, bot name: {}",
                      target->GetName(), spellid, bot->GetName());
        }
        return false;
    }

    uint32 CastingTime = !spellInfo->IsChanneled() ? spellInfo->CalcCastTime(bot) : spellInfo->GetDuration();
    // bool interruptOnMove = spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_MOVEMENT;
    if ((CastingTime || spellInfo->IsAutoRepeatRangedSpell()) && bot->isMoving())
    {
        if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster()))
        {
            LOG_DEBUG("playerbots", "Casting time and bot is moving - target name: {}, spellid: {}, bot name: {}",
                      target->GetName(), spellid, bot->GetName());
        }
        return false;
    }

    if (!itemTarget)
    {
        bool positiveSpell = spellInfo->IsPositive();
        // if (positiveSpell && bot->IsHostileTo(target))
        //     return false;

        // if (!positiveSpell && bot->IsFriendlyTo(target))
        //     return false;

        // bool damage = false;
        // for (uint8 i = EFFECT_0; i <= EFFECT_2; i++)
        // {
        //     if (spellInfo->Effects[i].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
        //     {
        //         damage = true;
        //         break;
        //     }
        // }

        if (target->IsImmunedToSpell(spellInfo))
        {
            if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster()))
            {
                LOG_DEBUG("playerbots", "target is immuned to spell - target name: {}, spellid: {}, bot name: {}",
                          target->GetName(), spellid, bot->GetName());
            }
            return false;
        }

        // if (!damage)
        // {
        //     for (uint8 i = EFFECT_0; i <= EFFECT_2; i++)
        //     {
        //         if (target->IsImmunedToSpellEffect(spellInfo, i)) {
        //             if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster())) {
        //                 LOG_DEBUG("playerbots", "target is immuned to spell effect - target name: {}, spellid: {},
        //                 bot name: {}",
        //                     target->GetName(), spellid, bot->GetName());
        //             }
        //             return false;
        //         }
        //     }
        // }

        if (bot != target && sServerFacade->GetDistance2d(bot, target) > sPlayerbotAIConfig->sightDistance)
        {
            if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster()))
            {
                LOG_DEBUG("playerbots", "target is out of sight distance - target name: {}, spellid: {}, bot name: {}",
                          target->GetName(), spellid, bot->GetName());
            }
            return false;
        }
    }

    Unit* oldSel = bot->GetSelectedUnit();
    Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);

    spell->m_targets.SetUnitTarget(target);
    spell->m_CastItem = castItem;
    if (itemTarget == nullptr)
    {
        itemTarget = aiObjectContext->GetValue<Item*>("item for spell", spellid)->Get();
        ;
    }
    spell->m_targets.SetItemTarget(itemTarget);

    SpellCastResult result = spell->CheckCast(true);
    delete spell;

    // if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster())) {
    //     if (result != SPELL_FAILED_NOT_READY && result != SPELL_CAST_OK) {
    //         LOG_DEBUG("playerbots", "CanCastSpell - target name: {}, spellid: {}, bot name: {}, result: {}",
    //             target->GetName(), spellid, bot->GetName(), result);
    //     }
    // }

    if (oldSel)
        bot->SetSelection(oldSel->GetGUID());

    switch (result)
    {
        case SPELL_FAILED_NOT_INFRONT:
        case SPELL_FAILED_NOT_STANDING:
        case SPELL_FAILED_UNIT_NOT_INFRONT:
        case SPELL_FAILED_MOVING:
        case SPELL_FAILED_TRY_AGAIN:
        case SPELL_CAST_OK:
        case SPELL_FAILED_NOT_SHAPESHIFT:
        case SPELL_FAILED_OUT_OF_RANGE:
            return true;
        default:
            if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster()))
            {
                // if (result != SPELL_FAILED_NOT_READY && result != SPELL_CAST_OK) {
                LOG_DEBUG("playerbots",
                          "CanCastSpell Check Failed. - target name: {}, spellid: {}, bot name: {}, result: {}",
                          target->GetName(), spellid, bot->GetName(), result);
            }
            return false;
    }
}

bool PlayerbotAI::CanCastSpell(uint32 spellid, GameObject* goTarget, uint8 effectMask, bool checkHasSpell)
{
    if (!spellid)
        return false;

    if (bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
        return false;

    Pet* pet = bot->GetPet();
    if (pet && pet->HasSpell(spellid))
        return true;

    if (checkHasSpell && !bot->HasSpell(spellid))
        return false;

    if (bot->HasSpellCooldown(spellid))
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if (!spellInfo)
        return false;

    int32 CastingTime = !spellInfo->IsChanneled() ? spellInfo->CalcCastTime(bot) : spellInfo->GetDuration();
    if (CastingTime > 0 && bot->isMoving())
        return false;

    bool damage = false;
    for (int32 i = EFFECT_0; i <= EFFECT_2; i++)
    {
        if (spellInfo->Effects[i].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
        {
            damage = true;
            break;
        }
    }

    if (sServerFacade->GetDistance2d(bot, goTarget) > sPlayerbotAIConfig->sightDistance)
        return false;

    // ObjectGuid oldSel = bot->GetTarget();
    // bot->SetTarget(goTarget->GetGUID());
    Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);

    spell->m_targets.SetGOTarget(goTarget);
    spell->m_CastItem = aiObjectContext->GetValue<Item*>("item for spell", spellid)->Get();
    spell->m_targets.SetItemTarget(spell->m_CastItem);

    SpellCastResult result = spell->CheckCast(true);
    delete spell;
    // if (oldSel)
    //     bot->SetTarget(oldSel);

    switch (result)
    {
        case SPELL_FAILED_NOT_INFRONT:
        case SPELL_FAILED_NOT_STANDING:
        case SPELL_FAILED_UNIT_NOT_INFRONT:
        case SPELL_FAILED_MOVING:
        case SPELL_FAILED_TRY_AGAIN:
        case SPELL_CAST_OK:
            return true;
        default:
            break;
    }

    return false;
}

bool PlayerbotAI::CanCastSpell(uint32 spellid, float x, float y, float z, uint8 effectMask, bool checkHasSpell,
                               Item* itemTarget)
{
    if (!spellid)
        return false;

    Pet* pet = bot->GetPet();
    if (pet && pet->HasSpell(spellid))
        return true;

    if (checkHasSpell && !bot->HasSpell(spellid))
        return false;

    if (bot->HasSpellCooldown(spellid))
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if (!spellInfo)
        return false;

    if (!itemTarget)
    {
        if (bot->GetDistance(x, y, z) > sPlayerbotAIConfig->sightDistance)
            return false;
    }

    Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);

    spell->m_targets.SetDst(x, y, z, 0.f);
    spell->m_CastItem = itemTarget ? itemTarget : aiObjectContext->GetValue<Item*>("item for spell", spellid)->Get();
    spell->m_targets.SetItemTarget(spell->m_CastItem);

    SpellCastResult result = spell->CheckCast(true);
    delete spell;

    switch (result)
    {
        case SPELL_FAILED_NOT_INFRONT:
        case SPELL_FAILED_NOT_STANDING:
        case SPELL_FAILED_UNIT_NOT_INFRONT:
        case SPELL_FAILED_MOVING:
        case SPELL_FAILED_TRY_AGAIN:
        case SPELL_CAST_OK:
            return true;
        default:
            return false;
    }
}

bool PlayerbotAI::CastSpell(std::string const name, Unit* target, Item* itemTarget)
{
    bool result = CastSpell(aiObjectContext->GetValue<uint32>("spell id", name)->Get(), target, itemTarget);
    if (result)
    {
        aiObjectContext->GetValue<time_t>("last spell cast time", name)->Set(time(nullptr));
    }

    return result;
}

bool PlayerbotAI::CastSpell(uint32 spellId, Unit* target, Item* itemTarget)
{
    if (!spellId)
    {
        return false;
    }

    if (!target)
        target = bot;

    Pet* pet = bot->GetPet();
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (pet && pet->HasSpell(spellId))
    {
        bool autocast = false;
        for (unsigned int& m_autospell : pet->m_autospells)
        {
            if (m_autospell == spellId)
            {
                autocast = true;
                break;
            }
        }

        pet->ToggleAutocast(spellInfo, !autocast);
        std::ostringstream out;
        out << (autocast ? "|cffff0000|Disabling" : "|cFF00ff00|Enabling") << " pet auto-cast for ";
        out << chatHelper.FormatSpell(spellInfo);
        TellMaster(out);
        return true;
    }

    // aiObjectContext->GetValue<LastMovement&>("last movement")->Get().Set(nullptr);
    // aiObjectContext->GetValue<time_t>("stay time")->Set(0);

    if (bot->IsFlying() || bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
    {
        // if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster())) {
        //     LOG_DEBUG("playerbots", "Spell cast is flying - target name: {}, spellid: {}, bot name: {}}",
        //         target->GetName(), spellId, bot->GetName());
        // }
        return false;
    }

    // bot->ClearUnitState(UNIT_STATE_CHASE);
    // bot->ClearUnitState(UNIT_STATE_FOLLOW);

    bool failWithDelay = false;
    if (!bot->IsStandState())
    {
        bot->SetStandState(UNIT_STAND_STATE_STAND);
        failWithDelay = true;
    }

    ObjectGuid oldSel = bot->GetSelectedUnit() ? bot->GetSelectedUnit()->GetGUID() : ObjectGuid();
    bot->SetSelection(target->GetGUID());
    WorldObject* faceTo = target;
    if (!bot->HasInArc(CAST_ANGLE_IN_FRONT, faceTo) && (spellInfo->FacingCasterFlags & SPELL_FACING_FLAG_INFRONT))
    {
        sServerFacade->SetFacingTo(bot, faceTo);
        // failWithDelay = true;
    }

    if (failWithDelay)
    {
        SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
        return false;
    }

    Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);

    SpellCastTargets targets;
    if (spellInfo->Targets & TARGET_FLAG_ITEM)
    {
        spell->m_CastItem =
            itemTarget ? itemTarget : aiObjectContext->GetValue<Item*>("item for spell", spellId)->Get();
        targets.SetItemTarget(spell->m_CastItem);

        if (bot->GetTradeData())
        {
            bot->GetTradeData()->SetSpell(spellId);
            delete spell;
            // if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster())) {
            //     LOG_DEBUG("playerbots", "Spell cast no item - target name: {}, spellid: {}, bot name: {}",
            //         target->GetName(), spellId, bot->GetName());
            // }
            return true;
        }
    }
    else if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
    {
        // WorldLocation aoe = aiObjectContext->GetValue<WorldLocation>("aoe position")->Get();
        // targets.SetDst(aoe);
        targets.SetDst(*target);
    }
    else if (spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION)
    {
        targets.SetDst(*bot);
    }
    else
    {
        targets.SetUnitTarget(target);
    }

    if (spellInfo->Effects[0].Effect == SPELL_EFFECT_OPEN_LOCK || spellInfo->Effects[0].Effect == SPELL_EFFECT_SKINNING)
    {
        LootObject loot = *aiObjectContext->GetValue<LootObject>("loot target");
        GameObject* go = GetGameObject(loot.guid);
        if (go && go->isSpawned())
        {
            WorldPacket packetgouse(CMSG_GAMEOBJ_USE, 8);
            packetgouse << loot.guid;
            bot->GetSession()->HandleGameObjectUseOpcode(packetgouse);
            targets.SetGOTarget(go);
            faceTo = go;
        }
        else
        {
            if (Unit* creature = GetUnit(loot.guid))
            {
                targets.SetUnitTarget(creature);
                faceTo = creature;
            }
        }
    }

    if (bot->isMoving() && spell->GetCastTime())
    {
        // bot->StopMoving();
        SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
        spell->cancel();
        delete spell;
        return false;
    }

    // spell->m_targets.SetUnitTarget(target);
    // SpellCastResult spellSuccess = spell->CheckCast(true);
    // if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster())) {
    //     LOG_DEBUG("playerbots", "Spell cast result - target name: {}, spellid: {}, bot name: {}, result: {}",
    //         target->GetName(), spellId, bot->GetName(), spellSuccess);
    // }
    // if (spellSuccess != SPELL_CAST_OK)
    //     return false;

    SpellCastResult result = spell->prepare(&targets);

    if (result != SPELL_CAST_OK)
    {
        // if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster())) {
        //     LOG_DEBUG("playerbots", "Spell cast failed. - target name: {}, spellid: {}, bot name: {}, result: {}",
        //         target->GetName(), spellId, bot->GetName(), result);
        // }
        return false;
    }
    // if (spellInfo->Effects[0].Effect == SPELL_EFFECT_OPEN_LOCK || spellInfo->Effects[0].Effect ==
    // SPELL_EFFECT_SKINNING)
    // {
    //     LootObject loot = *aiObjectContext->GetValue<LootObject>("loot target");
    //     if (!loot.IsLootPossible(bot))
    //     {
    //         spell->cancel();
    //         delete spell;
    //         if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster())) {
    //             LOG_DEBUG("playerbots", "Spell cast loot - target name: {}, spellid: {}, bot name: {}",
    //                 target->GetName(), spellId, bot->GetName());
    //         }
    //         return false;
    //     }
    // }

    // WaitForSpellCast(spell);
    
    aiObjectContext->GetValue<LastSpellCast&>("last spell cast")
        ->Get()
        .Set(spellId, target->GetGUID(), time(nullptr));

    aiObjectContext->GetValue<PositionMap&>("position")->Get()["random"].Reset();

    if (oldSel)
        bot->SetSelection(oldSel);

    if (HasStrategy("debug spell", BOT_STATE_NON_COMBAT))
    {
        std::ostringstream out;
        out << "Casting " << ChatHelper::FormatSpell(spellInfo);
        TellMasterNoFacing(out);
    }

    return true;
}

bool PlayerbotAI::CastSpell(uint32 spellId, float x, float y, float z, Item* itemTarget)
{
    if (!spellId)
        return false;

    Pet* pet = bot->GetPet();
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (pet && pet->HasSpell(spellId))
    {
        bool autocast = false;
        for (unsigned int& m_autospell : pet->m_autospells)
        {
            if (m_autospell == spellId)
            {
                autocast = true;
                break;
            }
        }

        pet->ToggleAutocast(spellInfo, !autocast);
        std::ostringstream out;
        out << (autocast ? "|cffff0000|Disabling" : "|cFF00ff00|Enabling") << " pet auto-cast for ";
        out << chatHelper.FormatSpell(spellInfo);
        TellMaster(out);
        return true;
    }

    // aiObjectContext->GetValue<LastMovement&>("last movement")->Get().Set(nullptr);
    // aiObjectContext->GetValue<time_t>("stay time")->Set(0);

    MotionMaster& mm = *bot->GetMotionMaster();

    if (bot->IsFlying() || bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return false;

    // bot->ClearUnitState(UNIT_STATE_CHASE);
    // bot->ClearUnitState(UNIT_STATE_FOLLOW);

    bool failWithDelay = false;
    if (!bot->IsStandState())
    {
        bot->SetStandState(UNIT_STAND_STATE_STAND);
        failWithDelay = true;
    }

    ObjectGuid oldSel = bot->GetSelectedUnit() ? bot->GetSelectedUnit()->GetGUID() : ObjectGuid();

    if (!bot->isMoving())
        bot->SetFacingTo(bot->GetAngle(x, y));

    if (failWithDelay)
    {
        SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
        return false;
    }

    Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);

    SpellCastTargets targets;
    if (spellInfo->Targets & TARGET_FLAG_ITEM)
    {
        spell->m_CastItem =
            itemTarget ? itemTarget : aiObjectContext->GetValue<Item*>("item for spell", spellId)->Get();
        targets.SetItemTarget(spell->m_CastItem);

        if (bot->GetTradeData())
        {
            bot->GetTradeData()->SetSpell(spellId);
            delete spell;
            return true;
        }
    }
    else if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
    {
        WorldLocation aoe = aiObjectContext->GetValue<WorldLocation>("aoe position")->Get();
        targets.SetDst(x, y, z, 0.f);
    }
    else if (spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION)
    {
        targets.SetDst(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), 0.f);
    }
    else
    {
        return false;
    }

    if (spellInfo->Effects[0].Effect == SPELL_EFFECT_OPEN_LOCK || spellInfo->Effects[0].Effect == SPELL_EFFECT_SKINNING)
    {
        return false;
    }

    spell->prepare(&targets);

    if (bot->isMoving() && spell->GetCastTime())
    {
        // bot->StopMoving();
        SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
        spell->cancel();
        delete spell;
        return false;
    }

    if (spellInfo->Effects[0].Effect == SPELL_EFFECT_OPEN_LOCK || spellInfo->Effects[0].Effect == SPELL_EFFECT_SKINNING)
    {
        LootObject loot = *aiObjectContext->GetValue<LootObject>("loot target");
        if (!loot.IsLootPossible(bot))
        {
            spell->cancel();
            delete spell;
            return false;
        }
    }

    // WaitForSpellCast(spell);
    aiObjectContext->GetValue<LastSpellCast&>("last spell cast")->Get().Set(spellId, bot->GetGUID(), time(nullptr));
    aiObjectContext->GetValue<PositionMap&>("position")->Get()["random"].Reset();

    if (oldSel)
        bot->SetSelection(oldSel);

    if (HasStrategy("debug spell", BOT_STATE_NON_COMBAT))
    {
        std::ostringstream out;
        out << "Casting " << ChatHelper::FormatSpell(spellInfo);
        TellMasterNoFacing(out);
    }

    return true;
}

bool PlayerbotAI::CanCastVehicleSpell(uint32 spellId, Unit* target)
{
    if (!spellId)
        return false;

    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicle)
        return false;

    // do not allow if no spells
    VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
    if (!seat || !(seat->m_flags & VEHICLE_SEAT_FLAG_CAN_CAST))
        return false;

    Unit* vehicleBase = vehicle->GetBase();

    Unit* spellTarget = target;
    if (!spellTarget)
        spellTarget = vehicleBase;

    if (!spellTarget)
        return false;

    if (vehicleBase->HasSpellCooldown(spellId))
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    // check BG siege position set in BG Tactics
    PositionInfo siegePos = GetAiObjectContext()->GetValue<PositionMap&>("position")->Get()["bg siege"];

    // do not cast spell on self if spell is location based
    if (!(siegePos.isSet() || spellTarget != vehicleBase) && spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
        return false;

    uint32 CastingTime = !spellInfo->IsChanneled() ? spellInfo->CalcCastTime(vehicleBase) : spellInfo->GetDuration();
    if (CastingTime && vehicleBase->isMoving())
        return false;

    if (vehicleBase != spellTarget && sServerFacade->GetDistance2d(vehicleBase, spellTarget) > 120.0f)
        return false;

    if (!target && siegePos.isSet())
    {
        if (sServerFacade->GetDistance2d(vehicleBase, siegePos.x, siegePos.y) > 120.0f)
            return false;
    }

    Spell* spell = new Spell(vehicleBase, spellInfo, TRIGGERED_NONE);

    WorldLocation dest;
    if (siegePos.isSet())
        dest = WorldLocation(bot->GetMapId(), siegePos.x, siegePos.y, siegePos.z, 0);
    else if (spellTarget != vehicleBase)
        dest = WorldLocation(spellTarget->GetMapId(), spellTarget->GetPosition());

    if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
        spell->m_targets.SetDst(dest);
    else if (spellTarget != vehicleBase)
        spell->m_targets.SetUnitTarget(spellTarget);

    SpellCastResult result = spell->CheckCast(true);
    delete spell;

    switch (result)
    {
        case SPELL_FAILED_NOT_INFRONT:
        case SPELL_FAILED_NOT_STANDING:
        case SPELL_FAILED_UNIT_NOT_INFRONT:
        case SPELL_FAILED_MOVING:
        case SPELL_FAILED_TRY_AGAIN:
        case SPELL_CAST_OK:
            return true;
        default:
            return false;
    }

    return false;
}

bool PlayerbotAI::CastVehicleSpell(uint32 spellId, Unit* target)
{
    if (!spellId)
        return false;

    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicle)
        return false;

    // do not allow if no spells
    VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
    if (!seat || !(seat->m_flags & VEHICLE_SEAT_FLAG_CAN_CAST))
        return false;

    Unit* vehicleBase = vehicle->GetBase();

    Unit* spellTarget = target;
    if (!spellTarget)
        spellTarget = vehicleBase;

    if (!spellTarget)
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    // check BG siege position set in BG Tactics
    PositionInfo siegePos = GetAiObjectContext()->GetValue<PositionMap&>("position")->Get()["bg siege"];
    if (!target && siegePos.isSet())
    {
        if (sServerFacade->GetDistance2d(vehicleBase, siegePos.x, siegePos.y) > 120.0f)
            return false;
    }

    // do not cast spell on self if spell is location based
    if (!(siegePos.isSet() || spellTarget != vehicleBase) && (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION) != 0)
        return false;

    if (seat->CanControl())
    {
        // aiObjectContext->GetValue<LastMovement&>("last movement")->Get().Set(nullptr);
        // aiObjectContext->GetValue<time_t>("stay time")->Set(0);
    }

    // bot->clearUnitState(UNIT_STAT_CHASE);
    // bot->clearUnitState(UNIT_STAT_FOLLOW);

    // ObjectGuid oldSel = bot->GetSelectionGuid();
    // bot->SetSelectionGuid(target->GetGUID());

    // turn vehicle if target is not in front
    bool failWithDelay = false;
    if (spellTarget != vehicleBase && (seat->CanControl() || (seat->m_flags & VEHICLE_SEAT_FLAG_ALLOW_TURNING)))
    {
        if (!vehicleBase->HasInArc(CAST_ANGLE_IN_FRONT, spellTarget, 100.0f))
        {
            vehicleBase->SetFacingToObject(spellTarget);
            failWithDelay = true;
        }
    }

    if (siegePos.isSet() && (seat->CanControl() || (seat->m_flags & VEHICLE_SEAT_FLAG_ALLOW_TURNING)))
    {
        vehicleBase->SetFacingTo(vehicleBase->GetAngle(siegePos.x, siegePos.y));
    }

    if (failWithDelay)
    {
        SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
        return false;
    }

    Spell* spell = new Spell(vehicleBase, spellInfo, TRIGGERED_NONE);

    SpellCastTargets targets;
    if ((spellTarget != vehicleBase || siegePos.isSet()) && (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION))
    {
        WorldLocation dest;
        if (spellTarget != vehicleBase)
            dest = WorldLocation(spellTarget->GetMapId(), spellTarget->GetPosition());
        else if (siegePos.isSet())
            dest = WorldLocation(bot->GetMapId(), siegePos.x + frand(-5.0f, 5.0f), siegePos.y + frand(-5.0f, 5.0f),
                                 siegePos.z, 0.0f);
        else
            return false;

        targets.SetDst(dest);
        targets.SetSpeed(30.0f);
        float dist = vehicleBase->GetPosition().GetExactDist(dest);
        // very much an approximation of the real projectile arc
        float elev = dist >= 110.0f ? 1.0f : pow(((dist + 10.0f) / 120.0f), 2.0f);
        targets.SetElevation(elev);
    }

    if (spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION)
    {
        targets.SetSrc(vehicleBase->GetPositionX(), vehicleBase->GetPositionY(), vehicleBase->GetPositionZ());
    }

    if (target && !(spellInfo->Targets & TARGET_FLAG_DEST_LOCATION))
    {
        targets.SetUnitTarget(spellTarget);
    }

    spell->prepare(&targets);

    if (seat->CanControl() && vehicleBase->isMoving() && spell->GetCastTime())
    {
        vehicleBase->StopMoving();
        SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
        spell->cancel();
        // delete spell;
        return false;
    }

    // WaitForSpellCast(spell);

    // aiObjectContext->GetValue<LastSpellCast&>("last spell cast")->Get().Set(spellId, target->GetGUID(), time(0));
    // aiObjectContext->GetValue<botAI::PositionMap&>("position")->Get()["random"].Reset();

    if (HasStrategy("debug spell", BOT_STATE_NON_COMBAT))
    {
        std::ostringstream out;
        out << "Casting Vehicle Spell" << ChatHelper::FormatSpell(spellInfo);
        TellMasterNoFacing(out);
    }

    return true;
}

bool PlayerbotAI::IsInVehicle(bool canControl, bool canCast, bool canAttack, bool canTurn, bool fixed)
{
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicle)
        return false;

    // get vehicle
    Unit* vehicleBase = vehicle->GetBase();
    if (!vehicleBase || !vehicleBase->IsAlive())
        return false;

    if (!vehicle->GetVehicleInfo())
        return false;

    // get seat
    VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
    if (!seat)
        return false;

    if (!(canControl || canCast || canAttack || canTurn || fixed))
        return true;

    if (canControl)
        return seat->CanControl() && !(vehicle->GetVehicleInfo()->m_flags & VEHICLE_FLAG_FIXED_POSITION);

    if (canCast)
        return (seat->m_flags & VEHICLE_SEAT_FLAG_CAN_CAST) != 0;

    if (canAttack)
        return (seat->m_flags & VEHICLE_SEAT_FLAG_CAN_ATTACK) != 0;

    if (canTurn)
        return (seat->m_flags & VEHICLE_SEAT_FLAG_ALLOW_TURNING) != 0;

    if (fixed)
        return (vehicle->GetVehicleInfo()->m_flags & VEHICLE_FLAG_FIXED_POSITION) != 0;

    return false;
}

void PlayerbotAI::WaitForSpellCast(Spell* spell)
{
    SpellInfo const* spellInfo = spell->GetSpellInfo();
    uint32 castTime = spell->GetCastTime();
    if (spellInfo->IsChanneled())
    {
        int32 duration = spellInfo->GetDuration();
        bot->ApplySpellMod(spellInfo->Id, SPELLMOD_DURATION, duration);
        if (duration > 0)
            castTime += duration;
    }

    SetNextCheckDelay(castTime + sPlayerbotAIConfig->reactDelay);
}

void PlayerbotAI::InterruptSpell()
{
    for (uint8 type = CURRENT_MELEE_SPELL; type <= CURRENT_CHANNELED_SPELL; type++)
    {
        Spell* spell = bot->GetCurrentSpell((CurrentSpellTypes)type);
        if (!spell)
            continue;

        bot->InterruptSpell((CurrentSpellTypes)type);

        WorldPacket data(SMSG_SPELL_FAILURE, 8 + 1 + 4 + 1);
        data << bot->GetPackGUID();
        data << uint8(1);
        data << uint32(spell->m_spellInfo->Id);
        data << uint8(0);
        bot->SendMessageToSet(&data, true);

        data.Initialize(SMSG_SPELL_FAILED_OTHER, 8 + 1 + 4 + 1);
        data << bot->GetPackGUID();
        data << uint8(1);
        data << uint32(spell->m_spellInfo->Id);
        data << uint8(0);
        bot->SendMessageToSet(&data, true);

        SpellInterrupted(spell->m_spellInfo->Id);
    }
}

void PlayerbotAI::RemoveAura(std::string const name)
{
    uint32 spellid = aiObjectContext->GetValue<uint32>("spell id", name)->Get();
    if (spellid && HasAura(spellid, bot))
        bot->RemoveAurasDueToSpell(spellid);
}

bool PlayerbotAI::IsInterruptableSpellCasting(Unit* target, std::string const spell)
{
    uint32 spellid = aiObjectContext->GetValue<uint32>("spell id", spell)->Get();
    if (!spellid || !target->IsNonMeleeSpellCast(true))
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if (!spellInfo)
        return false;

    for (uint8 i = EFFECT_0; i <= EFFECT_2; i++)
    {
        if ((spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT) &&
            spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
            return true;

        if (spellInfo->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST &&
            !target->IsImmunedToSpellEffect(spellInfo, i))
            return true;

        if ((spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA) &&
            spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_MOD_SILENCE)
            return true;
    }

    return false;
}

bool PlayerbotAI::HasAuraToDispel(Unit* target, uint32 dispelType)
{
    if (!target->IsInWorld())
    {
        return false;
    }
    bool isFriend = bot->IsFriendlyTo(target);
    Unit::VisibleAuraMap const* visibleAuras = target->GetVisibleAuras();
    for (Unit::VisibleAuraMap::const_iterator itr = visibleAuras->begin(); itr != visibleAuras->end(); ++itr)
    {
        Aura* aura = itr->second->GetBase();

        if (aura->IsPassive())
            continue;

        if (sPlayerbotAIConfig->dispelAuraDuration && aura->GetDuration() &&
            aura->GetDuration() < (int32)sPlayerbotAIConfig->dispelAuraDuration)
            continue;

        SpellInfo const* spellInfo = aura->GetSpellInfo();

        bool isPositiveSpell = spellInfo->IsPositive();
        if (isPositiveSpell && isFriend)
            continue;

        if (!isPositiveSpell && !isFriend)
            continue;

        if (canDispel(spellInfo, dispelType))
            return true;
    }
    return false;
}

#ifndef WIN32
inline int strcmpi(char const* s1, char const* s2)
{
    for (; *s1 && *s2 && (toupper(*s1) == toupper(*s2)); ++s1, ++s2)
    {
    }
    return *s1 - *s2;
}
#endif

bool PlayerbotAI::canDispel(SpellInfo const* spellInfo, uint32 dispelType)
{
    if (spellInfo->Dispel != dispelType)
        return false;

    if (!spellInfo->SpellName[0])
    {
        return true;
    }

    for (std::string& wl : dispel_whitelist)
    {
        if (strcmpi((const char*)spellInfo->SpellName[0], wl.c_str()) == 0)
        {
            return false;
        }
    }

    return !spellInfo->SpellName[0] || (strcmpi((const char*)spellInfo->SpellName[0], "demon skin") &&
                                        strcmpi((const char*)spellInfo->SpellName[0], "mage armor") &&
                                        strcmpi((const char*)spellInfo->SpellName[0], "frost armor") &&
                                        strcmpi((const char*)spellInfo->SpellName[0], "wavering will") &&
                                        strcmpi((const char*)spellInfo->SpellName[0], "chilled") &&
                                        strcmpi((const char*)spellInfo->SpellName[0], "mana tap") &&
                                        strcmpi((const char*)spellInfo->SpellName[0], "ice armor"));
}

bool IsAlliance(uint8 race)
{
    return race == RACE_HUMAN || race == RACE_DWARF || race == RACE_NIGHTELF || race == RACE_GNOME ||
           race == RACE_DRAENEI;
}

bool PlayerbotAI::HasRealPlayerMaster()
{
    if (master)
    {
        PlayerbotAI* masterBotAI = GET_PLAYERBOT_AI(master);
        return !masterBotAI || masterBotAI->IsRealPlayer();
    }

    return false;
}

bool PlayerbotAI::HasActivePlayerMaster() { return master && !GET_PLAYERBOT_AI(master); }

bool PlayerbotAI::IsAlt() { return HasRealPlayerMaster() && !sRandomPlayerbotMgr->IsRandomBot(bot); }

Player* PlayerbotAI::GetGroupMaster()
{
    if (!bot->InBattleground())
        if (Group* group = bot->GetGroup())
            if (Player* player = ObjectAccessor::FindPlayer(group->GetLeaderGUID()))
                return player;

    return master;
}

uint32 PlayerbotAI::GetFixedBotNumer(BotTypeNumber typeNumber, uint32 maxNum, float cyclePerMin)
{
    //deterministic seed 
    uint8 seedNumber = uint8(typeNumber);
    std::mt19937 rng(seedNumber);
    uint32 randseed = rng();                                  // Seed random number
    uint32 randnum = bot->GetGUID().GetCounter() + randseed;  // Semi-random but fixed number for each bot.

    if (cyclePerMin > 0)
    {
        uint32 cycle = floor(getMSTime() / (1000));  // Semi-random number adds 1 each second.
        cycle = cycle * cyclePerMin / 60;            // Cycles cyclePerMin per minute.
        randnum += cycle;                            // Make the random number cylce.
    }

    randnum = (randnum % (maxNum + 1));  // Loops the randomnumber at maxNum. Bassically removes all the numbers above 99.
    return randnum;  // Now we have a number unique for each bot between 0 and maxNum that increases by cyclePerMin.
}

/*
enum GrouperType
{
    SOLO = 0,
    MEMBER = 1,
    LEADER_2 = 2,
    LEADER_3 = 3,
    LEADER_4 = 4,
    LEADER_5 = 5
};
*/

GrouperType PlayerbotAI::GetGrouperType()
{
    uint32 grouperNumber = GetFixedBotNumer(BotTypeNumber::GROUPER_TYPE_NUMBER, 100, 0);

    if (grouperNumber < 20 && !HasRealPlayerMaster())
        return GrouperType::SOLO;

    if (grouperNumber < 80)
        return GrouperType::MEMBER;

    if (grouperNumber < 85)
        return GrouperType::LEADER_2;

    if (grouperNumber < 90)
        return GrouperType::LEADER_3;

    if (grouperNumber < 95)
        return GrouperType::LEADER_4;

    return GrouperType::LEADER_5;
}

GuilderType PlayerbotAI::GetGuilderType()
{
    uint32 grouperNumber = GetFixedBotNumer(BotTypeNumber::GUILDER_TYPE_NUMBER, 100, 0);

    if (grouperNumber < 20 && !HasRealPlayerMaster())
        return GuilderType::SOLO;

    if (grouperNumber < 30)
        return GuilderType::TINY;

    if (grouperNumber < 40)
        return GuilderType::SMALL;

    if (grouperNumber < 60)
        return GuilderType::MEDIUM;

    if (grouperNumber < 80)
        return GuilderType::LARGE;

    return GuilderType::VERY_LARGE;
}

bool PlayerbotAI::HasPlayerNearby(WorldPosition* pos, float range)
{
    float sqRange = range * range;
    bool nearPlayer = false;
    for (auto& player : sRandomPlayerbotMgr->GetPlayers())
    {
        if (!player->IsGameMaster() || player->isGMVisible())
        {
            if (player->GetMapId() != bot->GetMapId())
                continue;

            if (pos->sqDistance(WorldPosition(player)) < sqRange)
                nearPlayer = true;

            // if player is far check farsight/cinematic camera
            WorldObject* viewObj = player->GetViewpoint();
            if (viewObj && viewObj != player)
            {
                if (pos->sqDistance(WorldPosition(viewObj)) < sqRange)
                    nearPlayer = true;
            }
        }
    }

    return nearPlayer;
}

bool PlayerbotAI::HasPlayerNearby(float range)
{
    WorldPosition botPos(bot);
    return HasPlayerNearby(&botPos, range);
};

bool PlayerbotAI::HasManyPlayersNearby(uint32 trigerrValue, float range)
{
    float sqRange = range * range;
    uint32 found = 0;

    for (auto& player : sRandomPlayerbotMgr->GetPlayers())
    {
        if ((!player->IsGameMaster() || player->isGMVisible()) && sServerFacade->GetDistance2d(player, bot) < sqRange)
        {
            found++;

            if (found >= trigerrValue)
                return true;
        }
    }

    return false;
}

inline bool HasRealPlayers(Map* map)
{
    Map::PlayerList const& players = map->GetPlayers();
    if (players.IsEmpty())
    {
        return false;
    }

    for (auto const& itr : players)
    {
        Player* player = itr.GetSource();
        if (!player || !player->IsVisible())
        {
            continue;
        }

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (!botAI || botAI->IsRealPlayer() || botAI->HasRealPlayerMaster())
        {
            return true;
        }
    }

    return false;
}

ActivePiorityType PlayerbotAI::GetPriorityType(ActivityType activityType)
{
    // First priority - priorities disabled or has player master. Always active.
    if (HasRealPlayerMaster())
        return ActivePiorityType::HAS_REAL_PLAYER_MASTER;

    // Self bot in a group with a bot master.
    if (IsRealPlayer())
        return ActivePiorityType::IS_REAL_PLAYER;

    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (!member || !member->IsInWorld() && member->GetMapId() != bot->GetMapId())
                continue;

            if (member == bot)
                continue;

            //IN_GROUP_WITH_REAL_PLAYER
            PlayerbotAI* memberBotAI = GET_PLAYERBOT_AI(member);
            if (!memberBotAI || memberBotAI->HasRealPlayerMaster())
                return ActivePiorityType::IN_GROUP_WITH_REAL_PLAYER;

            //IN_GROUP_WITH_REAL_PLAYER
            if (group->IsLeader(member->GetGUID()))
            {
                if (!memberBotAI->AllowActivity(PARTY_ACTIVITY))
                    return ActivePiorityType::IN_GROUP_WITH_REAL_PLAYER;
            }
        }
    }

    //IN_INSTANCE
    if (bot->IsBeingTeleported())  // Allow activity while teleportation.
        return ActivePiorityType::IN_INSTANCE;

    //IN_INSTANCE
    if (!WorldPosition(bot).isOverworld())
        return ActivePiorityType::IN_INSTANCE;

    //VISIBLE_FOR_PLAYER
    if (HasPlayerNearby(sPlayerbotAIConfig->reactDistance))
        return ActivePiorityType::VISIBLE_FOR_PLAYER;

    //IN_COMBAT
    if (activityType != OUT_OF_PARTY_ACTIVITY && activityType != PACKET_ACTIVITY)
    {
        // Is in combat, defend yourself.
        if (bot->IsInCombat())
            return ActivePiorityType::IN_COMBAT;
    }

    //NEARBY_PLAYER
    if (HasPlayerNearby(300.f))
        return ActivePiorityType::NEARBY_PLAYER;

    //if (sPlayerbotAIConfig->IsFreeAltBot(bot) || HasStrategy("travel once", BotState::BOT_STATE_NON_COMBAT))
    //    return ActivePiorityType::IS_ALWAYS_ACTIVE;

    if (bot->InBattlegroundQueue())
        return ActivePiorityType::IN_BG_QUEUE;

    bool isLFG = false;
    if (group)
    {
        if (sLFGMgr->GetState(group->GetGUID()) != lfg::LFG_STATE_NONE)
        {
            isLFG = true;
        }
    }
    if (sLFGMgr->GetState(bot->GetGUID()) != lfg::LFG_STATE_NONE)
    {
        isLFG = true;
    }
    if (isLFG)
        return ActivePiorityType::IN_LFG;

    //IN_EMPTY_SERVER
    if (sRandomPlayerbotMgr->GetPlayers().empty())
        return ActivePiorityType::IN_EMPTY_SERVER;

    //PLAYER_FRIEND (on friendlist of real player) (needs to be tested for stability)
    /*for (auto& player : sRandomPlayerbotMgr->GetPlayers())
    {
          if (!player || !player->IsInWorld())
              continue;

         if (player->GetSocial() &&
             bot->GetGUID() &&
             player->GetSocial()->HasFriend(bot->GetGUID()))
             return ActivePiorityType::PLAYER_FRIEND;
    }*/

    //PLAYER_GUILD (contains real player)
    if (IsInRealGuild())
        return ActivePiorityType::PLAYER_GUILD;

    //IN_INACTIVE_MAP 
    if (bot->IsBeingTeleported() || !bot->IsInWorld() || !HasRealPlayers(bot->GetMap()))
        return ActivePiorityType::IN_INACTIVE_MAP;

    //IN_ACTIVE_MAP
    if (!bot->IsBeingTeleported() && bot->IsInWorld() && HasRealPlayers(bot->GetMap()))
    return ActivePiorityType::IN_ACTIVE_MAP;

    // IN_ACTIVE_AREA
    if (activityType == OUT_OF_PARTY_ACTIVITY || activityType == GRIND_ACTIVITY)
    {
        if (HasManyPlayersNearby(20, sPlayerbotAIConfig->sightDistance))
            return ActivePiorityType::IN_ACTIVE_AREA;
    }

    return ActivePiorityType::IN_ACTIVE_AREA;
}

// Returns the lower and upper bracket for bots to be active.
// Ie. { 10, 20 } means all bots in this bracket will be inactive below 10% activityMod,
// and will be active above 20% activityMod and scale between those values.
std::pair<uint32, uint32> PlayerbotAI::GetPriorityBracket(ActivePiorityType type)
{
    switch (type)
    {
        case ActivePiorityType::HAS_REAL_PLAYER_MASTER:
        case ActivePiorityType::IS_REAL_PLAYER:
        case ActivePiorityType::IN_GROUP_WITH_REAL_PLAYER:
        case ActivePiorityType::IN_INSTANCE:
        case ActivePiorityType::VISIBLE_FOR_PLAYER:
            return {0, 0};
        case ActivePiorityType::IS_ALWAYS_ACTIVE:
        case ActivePiorityType::IN_COMBAT:
            return {0, 10};
        case ActivePiorityType::IN_BG_QUEUE:
            return {0, 20};
        case ActivePiorityType::IN_LFG:
            return {0, 30};
        case ActivePiorityType::NEARBY_PLAYER:
            return {0, 40};
        case ActivePiorityType::PLAYER_FRIEND:
        case ActivePiorityType::PLAYER_GUILD:
            return {0, 50};
        case ActivePiorityType::IN_ACTIVE_AREA:
        case ActivePiorityType::IN_EMPTY_SERVER:
            return {50, 100};
        case ActivePiorityType::IN_ACTIVE_MAP:
            return {70, 100};
        case ActivePiorityType::IN_INACTIVE_MAP:
            return {80, 100};
        default:
            return {90, 100};
    }

    return {90, 100};
} 

bool PlayerbotAI::AllowActive(ActivityType activityType)
{
    // General exceptions
    if (activityType == PACKET_ACTIVITY)
        return true;

    ActivePiorityType type = GetPriorityType(activityType);
    if (activityType == DETAILED_MOVE_ACTIVITY)
    {
        switch (type)
        {
            case ActivePiorityType::HAS_REAL_PLAYER_MASTER:
            case ActivePiorityType::IS_REAL_PLAYER:
            case ActivePiorityType::IN_GROUP_WITH_REAL_PLAYER:
            case ActivePiorityType::IN_INSTANCE:
            case ActivePiorityType::VISIBLE_FOR_PLAYER:
            case ActivePiorityType::IN_COMBAT:
            case ActivePiorityType::NEARBY_PLAYER:
                return true;
                break;
            case ActivePiorityType::IS_ALWAYS_ACTIVE:
            case ActivePiorityType::IN_BG_QUEUE:
            case ActivePiorityType::IN_LFG:
            case ActivePiorityType::PLAYER_FRIEND:
            case ActivePiorityType::PLAYER_GUILD:
            case ActivePiorityType::IN_ACTIVE_AREA:
            case ActivePiorityType::IN_EMPTY_SERVER:
            case ActivePiorityType::IN_ACTIVE_MAP:
            case ActivePiorityType::IN_INACTIVE_MAP:
            default:
                break;
        }
    }
    else if (activityType == REACT_ACTIVITY)
    {
        switch (type)
        {
            case ActivePiorityType::HAS_REAL_PLAYER_MASTER:
            case ActivePiorityType::IS_REAL_PLAYER:
            case ActivePiorityType::IN_GROUP_WITH_REAL_PLAYER:
            case ActivePiorityType::IN_INSTANCE:
            case ActivePiorityType::VISIBLE_FOR_PLAYER:
            case ActivePiorityType::IS_ALWAYS_ACTIVE:
            case ActivePiorityType::IN_COMBAT:
                return true;
                break;
            case ActivePiorityType::NEARBY_PLAYER:
            case ActivePiorityType::IN_BG_QUEUE:
            case ActivePiorityType::IN_LFG:
            case ActivePiorityType::PLAYER_FRIEND:
            case ActivePiorityType::PLAYER_GUILD:
            case ActivePiorityType::IN_ACTIVE_AREA:
            case ActivePiorityType::IN_EMPTY_SERVER:
            case ActivePiorityType::IN_ACTIVE_MAP:
            case ActivePiorityType::IN_INACTIVE_MAP:
            default:
                return false;
                break;
        }
    }

    // GetPriorityBracket acitivity
    float normalizedBotActiveAlone = sPlayerbotAIConfig->botActiveAlone > 100 ? 100 : sPlayerbotAIConfig->botActiveAlone;
    float activePerc = normalizedBotActiveAlone;
    if (sPlayerbotAIConfig->botActiveAloneSmartScale &&
        bot->GetLevel() >= sPlayerbotAIConfig->botActiveAloneSmartScaleWhenMinLevel &&
        bot->GetLevel() <= sPlayerbotAIConfig->botActiveAloneSmartScaleWhenMaxLevel)
    {
        std::pair<uint8, uint8> priorityBracket = GetPriorityBracket(type);
        if (!priorityBracket.second) return true; 
        float activityPercentage = sRandomPlayerbotMgr->getActivityPercentage();
        if (priorityBracket.first >= activityPercentage) return false;
        if (priorityBracket.second <= activityPercentage && priorityBracket.second < 100) return true;
        activePerc = (activityPercentage - priorityBracket.first) / (priorityBracket.second - priorityBracket.first);
        activePerc *= (priorityBracket.second == 100) ? normalizedBotActiveAlone : 100;
    }

    // The last number if the amount it cycles per min. Currently set to 1% of the active bots.
    uint32 ActivityNumber = GetFixedBotNumer(BotTypeNumber::ACTIVITY_TYPE_NUMBER, 100, activePerc * 0.01f);

    // The given percentage of bots should be active and rotate 1% of those active bots each minute.  
    return ActivityNumber <= (activePerc);
}

bool PlayerbotAI::AllowActivity(ActivityType activityType, bool checkNow)
{
    if (!allowActiveCheckTimer[activityType])
        allowActiveCheckTimer[activityType] = time(nullptr);

    if (!checkNow && time(nullptr) < (allowActiveCheckTimer[activityType] + 5))
        return allowActive[activityType];

    bool allowed = AllowActive(activityType);
    allowActive[activityType] = allowed;
    allowActiveCheckTimer[activityType] = time(nullptr);
    return allowed;
}

bool PlayerbotAI::IsOpposing(Player* player) { return IsOpposing(player->getRace(), bot->getRace()); }

bool PlayerbotAI::IsOpposing(uint8 race1, uint8 race2)
{
    return (IsAlliance(race1) && !IsAlliance(race2)) || (!IsAlliance(race1) && IsAlliance(race2));
}

void PlayerbotAI::RemoveShapeshift()
{
    RemoveAura("bear form");
    RemoveAura("dire bear form");
    RemoveAura("moonkin form");
    RemoveAura("travel form");
    RemoveAura("cat form");
    RemoveAura("flight form");
    RemoveAura("swift flight form");
    RemoveAura("aquatic form");
    RemoveAura("ghost wolf");
    // RemoveAura("tree of life");
}

uint32 PlayerbotAI::GetEquipGearScore(Player* player, bool withBags, bool withBank)
{
    std::vector<uint32> gearScore(EQUIPMENT_SLOT_END);
    uint32 twoHandScore = 0;

    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            _fillGearScoreData(player, item, &gearScore, twoHandScore);
    }

    if (withBags)
    {
        // check inventory
        for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                _fillGearScoreData(player, item, &gearScore, twoHandScore);
        }

        // check bags
        for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        {
            if (Bag* pBag = (Bag*)player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                {
                    if (Item* item2 = pBag->GetItemByPos(j))
                        _fillGearScoreData(player, item2, &gearScore, twoHandScore);
                }
            }
        }
    }

    if (withBank)
    {
        for (uint8 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                _fillGearScoreData(player, item, &gearScore, twoHandScore);
        }

        for (uint8 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                if (item->IsBag())
                {
                    Bag* bag = (Bag*)item;
                    for (uint8 j = 0; j < bag->GetBagSize(); ++j)
                    {
                        if (Item* item2 = bag->GetItemByPos(j))
                            _fillGearScoreData(player, item2, &gearScore, twoHandScore);
                    }
                }
            }
        }
    }

    uint8 count = EQUIPMENT_SLOT_END - 2;  // ignore body and tabard slots
    uint32 sum = 0;

    // check if 2h hand is higher level than main hand + off hand
    if (gearScore[EQUIPMENT_SLOT_MAINHAND] + gearScore[EQUIPMENT_SLOT_OFFHAND] < twoHandScore * 2)
    {
        gearScore[EQUIPMENT_SLOT_OFFHAND] = 0;  // off hand is ignored in calculations if 2h weapon has higher score
        --count;
        gearScore[EQUIPMENT_SLOT_MAINHAND] = twoHandScore;
    }

    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        sum += gearScore[i];
    }

    if (count)
    {
        uint32 res = uint32(sum / count);
        return res;
    }

    return 0;
}

uint32 PlayerbotAI::GetMixedGearScore(Player* player, bool withBags, bool withBank, uint32 topN)
{
    std::vector<uint32> gearScore(EQUIPMENT_SLOT_END);
    uint32 twoHandScore = 0;

    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            _fillGearScoreData(player, item, &gearScore, twoHandScore, true);
    }

    if (withBags)
    {
        // check inventory
        for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                _fillGearScoreData(player, item, &gearScore, twoHandScore, true);
        }

        // check bags
        for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        {
            if (Bag* pBag = (Bag*)player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                {
                    if (Item* item2 = pBag->GetItemByPos(j))
                        _fillGearScoreData(player, item2, &gearScore, twoHandScore, true);
                }
            }
        }
    }

    if (withBank)
    {
        for (uint8 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                _fillGearScoreData(player, item, &gearScore, twoHandScore, true);
        }

        for (uint8 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                if (item->IsBag())
                {
                    Bag* bag = (Bag*)item;
                    for (uint8 j = 0; j < bag->GetBagSize(); ++j)
                    {
                        if (Item* item2 = bag->GetItemByPos(j))
                            _fillGearScoreData(player, item2, &gearScore, twoHandScore, true);
                    }
                }
            }
        }
    }
    if (!topN)
    {
        uint8 count = EQUIPMENT_SLOT_END - 2;  // ignore body and tabard slots
        uint32 sum = 0;

        // check if 2h hand is higher level than main hand + off hand
        if (gearScore[EQUIPMENT_SLOT_MAINHAND] + gearScore[EQUIPMENT_SLOT_OFFHAND] < twoHandScore * 2)
        {
            gearScore[EQUIPMENT_SLOT_OFFHAND] = 0;  // off hand is ignored in calculations if 2h weapon has higher score
            --count;
            gearScore[EQUIPMENT_SLOT_MAINHAND] = twoHandScore;
        }

        for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
        {
            sum += gearScore[i];
        }

        if (count)
        {
            uint32 res = uint32(sum / count);
            return res;
        }

        return 0;
    }
    // topN != 0
    if (gearScore[EQUIPMENT_SLOT_MAINHAND] + gearScore[EQUIPMENT_SLOT_OFFHAND] < twoHandScore * 2)
    {
        gearScore[EQUIPMENT_SLOT_OFFHAND] = twoHandScore;
        gearScore[EQUIPMENT_SLOT_MAINHAND] = twoHandScore;
    }
    std::vector<uint32> topGearScore;
    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        topGearScore.push_back(gearScore[i]);
    }
    std::sort(topGearScore.begin(), topGearScore.end(), [&](const uint32 lhs, const uint32 rhs) { return lhs > rhs; });
    uint32 sum = 0;
    for (int i = 0; i < std::min((uint32)topGearScore.size(), topN); i++)
    {
        sum += topGearScore[i];
    }
    return sum / topN;
}

void PlayerbotAI::_fillGearScoreData(Player* player, Item* item, std::vector<uint32>* gearScore, uint32& twoHandScore,
                                     bool mixed)
{
    if (!item)
        return;

    ItemTemplate const* proto = item->GetTemplate();
    if (player->CanUseItem(proto) != EQUIP_ERR_OK)
        return;

    uint8 type = proto->InventoryType;
    uint32 level = mixed ? proto->ItemLevel * PlayerbotAI::GetItemScoreMultiplier(ItemQualities(proto->Quality)) : proto->ItemLevel;

    switch (type)
    {
        case INVTYPE_2HWEAPON:
            twoHandScore = std::max(twoHandScore, level);
            break;
        case INVTYPE_WEAPON:
        case INVTYPE_WEAPONMAINHAND:
            (*gearScore)[SLOT_MAIN_HAND] = std::max((*gearScore)[SLOT_MAIN_HAND], level);
            break;
        case INVTYPE_SHIELD:
        case INVTYPE_WEAPONOFFHAND:
            (*gearScore)[EQUIPMENT_SLOT_OFFHAND] = std::max((*gearScore)[EQUIPMENT_SLOT_OFFHAND], level);
            break;
        case INVTYPE_THROWN:
        case INVTYPE_RANGEDRIGHT:
        case INVTYPE_RANGED:
        case INVTYPE_QUIVER:
        case INVTYPE_RELIC:
            (*gearScore)[EQUIPMENT_SLOT_RANGED] = std::max((*gearScore)[EQUIPMENT_SLOT_RANGED], level);
            break;
        case INVTYPE_HEAD:
            (*gearScore)[EQUIPMENT_SLOT_HEAD] = std::max((*gearScore)[EQUIPMENT_SLOT_HEAD], level);
            break;
        case INVTYPE_NECK:
            (*gearScore)[EQUIPMENT_SLOT_NECK] = std::max((*gearScore)[EQUIPMENT_SLOT_NECK], level);
            break;
        case INVTYPE_SHOULDERS:
            (*gearScore)[EQUIPMENT_SLOT_SHOULDERS] = std::max((*gearScore)[EQUIPMENT_SLOT_SHOULDERS], level);
            break;
        case INVTYPE_BODY:
            (*gearScore)[EQUIPMENT_SLOT_BODY] = std::max((*gearScore)[EQUIPMENT_SLOT_BODY], level);
            break;
        case INVTYPE_CHEST:
            (*gearScore)[EQUIPMENT_SLOT_CHEST] = std::max((*gearScore)[EQUIPMENT_SLOT_CHEST], level);
            break;
        case INVTYPE_WAIST:
            (*gearScore)[EQUIPMENT_SLOT_WAIST] = std::max((*gearScore)[EQUIPMENT_SLOT_WAIST], level);
            break;
        case INVTYPE_LEGS:
            (*gearScore)[EQUIPMENT_SLOT_LEGS] = std::max((*gearScore)[EQUIPMENT_SLOT_LEGS], level);
            break;
        case INVTYPE_FEET:
            (*gearScore)[EQUIPMENT_SLOT_FEET] = std::max((*gearScore)[EQUIPMENT_SLOT_FEET], level);
            break;
        case INVTYPE_WRISTS:
            (*gearScore)[EQUIPMENT_SLOT_WRISTS] = std::max((*gearScore)[EQUIPMENT_SLOT_WRISTS], level);
            break;
        case INVTYPE_HANDS:
            (*gearScore)[EQUIPMENT_SLOT_HEAD] = std::max((*gearScore)[EQUIPMENT_SLOT_HEAD], level);
            break;
        // equipped gear score check uses both rings and trinkets for calculation, assume that for bags/banks it is the
        // same with keeping second highest score at second slot
        case INVTYPE_FINGER:
        {
            if ((*gearScore)[EQUIPMENT_SLOT_FINGER1] < level)
            {
                (*gearScore)[EQUIPMENT_SLOT_FINGER2] = (*gearScore)[EQUIPMENT_SLOT_FINGER1];
                (*gearScore)[EQUIPMENT_SLOT_FINGER1] = level;
            }
            else if ((*gearScore)[EQUIPMENT_SLOT_FINGER2] < level)
                (*gearScore)[EQUIPMENT_SLOT_FINGER2] = level;
            break;
        }
        case INVTYPE_TRINKET:
        {
            if ((*gearScore)[EQUIPMENT_SLOT_TRINKET1] < level)
            {
                (*gearScore)[EQUIPMENT_SLOT_TRINKET2] = (*gearScore)[EQUIPMENT_SLOT_TRINKET1];
                (*gearScore)[EQUIPMENT_SLOT_TRINKET1] = level;
            }
            else if ((*gearScore)[EQUIPMENT_SLOT_TRINKET2] < level)
                (*gearScore)[EQUIPMENT_SLOT_TRINKET2] = level;
            break;
        }
        case INVTYPE_CLOAK:
            (*gearScore)[EQUIPMENT_SLOT_BACK] = std::max((*gearScore)[EQUIPMENT_SLOT_BACK], level);
            break;
        default:
            break;
    }
}

std::string const PlayerbotAI::HandleRemoteCommand(std::string const command)
{
    if (command == "state")
    {
        switch (currentState)
        {
            case BOT_STATE_COMBAT:
                return "combat";
            case BOT_STATE_DEAD:
                return "dead";
            case BOT_STATE_NON_COMBAT:
                return "non-combat";
            default:
                return "unknown";
        }
    }
    else if (command == "position")
    {
        std::ostringstream out;
        out << bot->GetPositionX() << " " << bot->GetPositionY() << " " << bot->GetPositionZ() << " " << bot->GetMapId()
            << " " << bot->GetOrientation();

        if (AreaTableEntry const* zoneEntry = sAreaTableStore.LookupEntry(bot->GetZoneId()))
            out << " |" << zoneEntry->area_name[0] << "|";

        return out.str();
    }
    else if (command == "tpos")
    {
        Unit* target = *GetAiObjectContext()->GetValue<Unit*>("current target");
        if (!target)
        {
            return "";
        }

        std::ostringstream out;
        out << target->GetPositionX() << " " << target->GetPositionY() << " " << target->GetPositionZ() << " "
            << target->GetMapId() << " " << target->GetOrientation();
        return out.str();
    }
    else if (command == "movement")
    {
        LastMovement& data = *GetAiObjectContext()->GetValue<LastMovement&>("last movement");
        std::ostringstream out;
        out << data.lastMoveShort.getX() << " " << data.lastMoveShort.getY() << " " << data.lastMoveShort.getZ() << " "
            << data.lastMoveShort.getMapId() << " " << data.lastMoveShort.getO();
        return out.str();
    }
    else if (command == "target")
    {
        Unit* target = *GetAiObjectContext()->GetValue<Unit*>("current target");
        if (!target)
        {
            return "";
        }

        return target->GetName();
    }
    else if (command == "hp")
    {
        uint32 pct = static_cast<uint32>(bot->GetHealthPct());
        std::ostringstream out;
        out << pct << "%";

        Unit* target = *GetAiObjectContext()->GetValue<Unit*>("current target");
        if (!target)
        {
            return out.str();
        }

        pct = static_cast<uint32>(target->GetHealthPct());
        out << " / " << pct << "%";
        return out.str();
    }
    else if (command == "strategy")
    {
        return currentEngine->ListStrategies();
    }
    else if (command == "action")
    {
        return currentEngine->GetLastAction();
    }
    else if (command == "values")
    {
        return GetAiObjectContext()->FormatValues();
    }
    else if (command == "travel")
    {
        std::ostringstream out;

        TravelTarget* target = GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
        if (target->getDestination())
        {
            out << "Destination = " << target->getDestination()->getName();

            out << ": " << target->getDestination()->getTitle();

            out << " v: " << target->getDestination()->getVisitors();

            if (!(*target->getPosition() == WorldPosition()))
            {
                out << "(" << target->getPosition()->getAreaName() << ")";
                out << " distance: " << target->getPosition()->distance(bot) << "y";
                out << " v: " << target->getPosition()->getVisitors();
            }
        }

        out << " Status = ";

        if (target->getStatus() == TRAVEL_STATUS_NONE)
            out << " none";
        else if (target->getStatus() == TRAVEL_STATUS_PREPARE)
            out << " prepare";
        else if (target->getStatus() == TRAVEL_STATUS_TRAVEL)
            out << " travel";
        else if (target->getStatus() == TRAVEL_STATUS_WORK)
            out << " work";
        else if (target->getStatus() == TRAVEL_STATUS_COOLDOWN)
            out << " cooldown";
        else if (target->getStatus() == TRAVEL_STATUS_EXPIRED)
            out << " expired";

        if (target->getStatus() != TRAVEL_STATUS_EXPIRED)
            out << " Expire in " << (target->getTimeLeft() / 1000) << "s";

        out << " Retry " << target->getRetryCount(true) << "/" << target->getRetryCount(false);

        return out.str();
    }
    else if (command == "budget")
    {
        std::ostringstream out;

        AiObjectContext* context = GetAiObjectContext();

        out << "Current money: " << ChatHelper::formatMoney(bot->GetMoney()) << " free to use:"
            << ChatHelper::formatMoney(AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::anything)) << "\n";
        out << "Purpose | Available / Needed \n";

        for (uint32 i = 1; i < (uint32)NeedMoneyFor::anything; i++)
        {
            NeedMoneyFor needMoneyFor = NeedMoneyFor(i);

            switch (needMoneyFor)
            {
                case NeedMoneyFor::none:
                    out << "nothing";
                    break;
                case NeedMoneyFor::repair:
                    out << "repair";
                    break;
                case NeedMoneyFor::ammo:
                    out << "ammo";
                    break;
                case NeedMoneyFor::spells:
                    out << "spells";
                    break;
                case NeedMoneyFor::travel:
                    out << "travel";
                    break;
                case NeedMoneyFor::consumables:
                    out << "consumables";
                    break;
                case NeedMoneyFor::gear:
                    out << "gear";
                    break;
                case NeedMoneyFor::guild:
                    out << "guild";
                    break;
                default:
                    break;
            }

            out << " | " << ChatHelper::formatMoney(AI_VALUE2(uint32, "free money for", i)) << " / "
                << ChatHelper::formatMoney(AI_VALUE2(uint32, "money needed for", i)) << "\n";
        }

        return out.str();
    }

    std::ostringstream out;
    out << "invalid command: " << command;
    return out.str();
}

bool PlayerbotAI::HasSkill(SkillType skill) { return bot->HasSkill(skill) && bot->GetSkillValue(skill) > 0; }

float PlayerbotAI::GetRange(std::string const type)
{
    float val = 0;
    if (aiObjectContext)
        val = aiObjectContext->GetValue<float>("range", type)->Get();

    if (abs(val) >= 0.1f)
        return val;

    if (type == "spell")
        return sPlayerbotAIConfig->spellDistance;

    if (type == "shoot")
        return sPlayerbotAIConfig->shootDistance;

    if (type == "flee")
        return sPlayerbotAIConfig->fleeDistance;

    if (type == "heal")
        return sPlayerbotAIConfig->healDistance;

    if (type == "melee")
        return sPlayerbotAIConfig->meleeDistance;

    return 0;
}

void PlayerbotAI::Ping(float x, float y)
{
    WorldPacket data(MSG_MINIMAP_PING, (8 + 4 + 4));
    data << bot->GetGUID();
    data << x;
    data << y;

    if (bot->GetGroup())
    {
        bot->GetGroup()->BroadcastPacket(&data, true, -1, bot->GetGUID());
    }
    else
    {
        bot->GetSession()->SendPacket(&data);
    }
}

// Find Poison ...Natsukawa
Item* PlayerbotAI::FindPoison() const
{
    // list out items in main backpack
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; slot++)
    {
        if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            ItemTemplate const* pItemProto = pItem->GetTemplate();
            if (!pItemProto || bot->CanUseItem(pItemProto) != EQUIP_ERR_OK)
                continue;

            if (pItemProto->Class == ITEM_CLASS_CONSUMABLE && pItemProto->SubClass == 6)
                return pItem;
        }
    }
    // list out items in other removable backpacks
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
        {
            for (uint8 slot = 0; slot < pBag->GetBagSize(); ++slot)
            {
                if (Item* const pItem = bot->GetItemByPos(bag, slot))
                {
                    ItemTemplate const* pItemProto = pItem->GetTemplate();
                    if (!pItemProto || bot->CanUseItem(pItemProto) != EQUIP_ERR_OK)
                        continue;

                    if (pItemProto->Class == ITEM_CLASS_CONSUMABLE &&
                        pItemProto->SubClass == ITEM_SUBCLASS_ITEM_ENHANCEMENT)
                        return pItem;
                }
            }
        }
    }

    return nullptr;
}

Item* PlayerbotAI::FindConsumable(uint32 displayId) const
{
    // list out items in main backpack
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; slot++)
    {
        if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            ItemTemplate const* pItemProto = pItem->GetTemplate();
            if (!pItemProto || bot->CanUseItem(pItemProto) != EQUIP_ERR_OK)
                continue;

            if ((pItemProto->Class == ITEM_CLASS_CONSUMABLE || pItemProto->Class == ITEM_CLASS_TRADE_GOODS) &&
                pItemProto->DisplayInfoID == displayId)
                return pItem;
        }
    }

    // list out items in other removable backpacks
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* const pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
        {
            for (uint8 slot = 0; slot < pBag->GetBagSize(); ++slot)
            {
                if (Item* const pItem = bot->GetItemByPos(bag, slot))
                {
                    ItemTemplate const* pItemProto = pItem->GetTemplate();
                    if (!pItemProto || bot->CanUseItem(pItemProto) != EQUIP_ERR_OK)
                        continue;

                    if ((pItemProto->Class == ITEM_CLASS_CONSUMABLE || pItemProto->Class == ITEM_CLASS_TRADE_GOODS) &&
                        pItemProto->DisplayInfoID == displayId)
                        return pItem;
                }
            }
        }
    }

    return nullptr;
}

Item* PlayerbotAI::FindBandage() const
{
    // list out items in main backpack
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; slot++)
    {
        if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            ItemTemplate const* pItemProto = pItem->GetTemplate();
            if (!pItemProto || bot->CanUseItem(pItemProto) != EQUIP_ERR_OK)
                continue;

            if (pItemProto->Class == ITEM_CLASS_CONSUMABLE && pItemProto->SubClass == ITEM_SUBCLASS_BANDAGE)
                return pItem;
        }
    }

    // list out items in other removable backpacks
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* const pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
        {
            for (uint8 slot = 0; slot < pBag->GetBagSize(); ++slot)
            {
                if (Item* const pItem = bot->GetItemByPos(bag, slot))
                {
                    ItemTemplate const* pItemProto = pItem->GetTemplate();
                    if (!pItemProto || bot->CanUseItem(pItemProto) != EQUIP_ERR_OK)
                        continue;

                    if (pItemProto->Class == ITEM_CLASS_CONSUMABLE && pItemProto->SubClass == ITEM_SUBCLASS_BANDAGE)
                        return pItem;
                }
            }
        }
    }

    return nullptr;
}

static const uint32 uPriorizedSharpStoneIds[8] = {ADAMANTITE_SHARPENING_DISPLAYID, FEL_SHARPENING_DISPLAYID,
                                                  ELEMENTAL_SHARPENING_DISPLAYID,  DENSE_SHARPENING_DISPLAYID,
                                                  SOLID_SHARPENING_DISPLAYID,      HEAVY_SHARPENING_DISPLAYID,
                                                  COARSE_SHARPENING_DISPLAYID,     ROUGH_SHARPENING_DISPLAYID};

static const uint32 uPriorizedWeightStoneIds[7] = {ADAMANTITE_WEIGHTSTONE_DISPLAYID, FEL_WEIGHTSTONE_DISPLAYID,
                                                   DENSE_WEIGHTSTONE_DISPLAYID,      SOLID_WEIGHTSTONE_DISPLAYID,
                                                   HEAVY_WEIGHTSTONE_DISPLAYID,      COARSE_WEIGHTSTONE_DISPLAYID,
                                                   ROUGH_WEIGHTSTONE_DISPLAYID};

/**
 * FindStoneFor()
 * return Item* Returns sharpening/weight stone item eligible to enchant a bot weapon
 *
 * params:weapon Item* the weap�n the function should search and return a enchanting item for
 * return nullptr if no relevant item is found in bot inventory, else return a sharpening or weight
 * stone based on the weapon subclass
 *
 */
Item* PlayerbotAI::FindStoneFor(Item* weapon) const
{
    Item* stone = nullptr;
    ItemTemplate const* pProto = weapon->GetTemplate();
    if (pProto && (pProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD || pProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD2 ||
                   pProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE || pProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2 ||
                   pProto->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER))
    {
        for (uint8 i = 0; i < std::size(uPriorizedSharpStoneIds); ++i)
        {
            stone = FindConsumable(uPriorizedSharpStoneIds[i]);
            if (stone)
            {
                return stone;
            }
        }
    }
    else if (pProto &&
             (pProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE || pProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE2))
    {
        for (uint8 i = 0; i < std::size(uPriorizedWeightStoneIds); ++i)
        {
            stone = FindConsumable(uPriorizedWeightStoneIds[i]);
            if (stone)
            {
                return stone;
            }
        }
    }

    return stone;
}

Item* PlayerbotAI::FindOilFor(Item* weapon) const
{
    if (!weapon)
        return nullptr;

    const ItemTemplate* item_template = weapon->GetTemplate();
    if (!item_template)
        return nullptr;

    // static const will only get created once whatever the call amout
    static const std::vector<uint32_t> uPriorizedWizardOilIds = {
        MINOR_WIZARD_OIL,   MINOR_MANA_OIL, LESSER_WIZARD_OIL, LESSER_MANA_OIL,    BRILLIANT_WIZARD_OIL,
        BRILLIANT_MANA_OIL, WIZARD_OIL,     SUPERIOR_MANA_OIL, SUPERIOR_WIZARD_OIL};

    // static const will only get created once whatever the call amout
    static const std::vector<uint32_t> uPriorizedManaOilIds = {
        MINOR_MANA_OIL,       MINOR_WIZARD_OIL,  LESSER_MANA_OIL, LESSER_WIZARD_OIL,  BRILLIANT_MANA_OIL,
        BRILLIANT_WIZARD_OIL, SUPERIOR_MANA_OIL, WIZARD_OIL,      SUPERIOR_WIZARD_OIL};

    Item* oil = nullptr;
    if (item_template->SubClass == ITEM_SUBCLASS_WEAPON_SWORD ||
        item_template->SubClass == ITEM_SUBCLASS_WEAPON_STAFF || item_template->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER)
    {
        for (const auto& id : uPriorizedWizardOilIds)
        {
            oil = FindConsumable(id);
            if (oil)
                return oil;
        }
    }
    else if (item_template->SubClass == ITEM_SUBCLASS_WEAPON_MACE ||
             item_template->SubClass == ITEM_SUBCLASS_WEAPON_MACE2)
    {
        for (const auto& id : uPriorizedManaOilIds)
        {
            oil = FindConsumable(id);
            if (oil)
                return oil;
        }
    }

    return oil;
}

std::vector<Item*> PlayerbotAI::GetInventoryAndEquippedItems()
{
    std::vector<Item*> items;

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                if (Item* pItem = pBag->GetItemByPos(j))
                {
                    items.push_back(pItem);
                }
            }
        }
    }

    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            items.push_back(pItem);
        }
    }

    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            items.push_back(pItem);
        }
    }

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            items.push_back(pItem);
        }
    }

    return items;
}

std::vector<Item*> PlayerbotAI::GetInventoryItems()
{
    std::vector<Item*> items;

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                if (Item* pItem = pBag->GetItemByPos(j))
                {
                    items.push_back(pItem);
                }
            }
        }
    }

    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            items.push_back(pItem);
        }
    }

    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            items.push_back(pItem);
        }
    }

    return items;
}

uint32 PlayerbotAI::GetInventoryItemsCountWithId(uint32 itemId)
{
    uint32 count = 0;

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                if (Item* pItem = pBag->GetItemByPos(j))
                {
                    if (pItem->GetTemplate()->ItemId == itemId)
                    {
                        count += pItem->GetCount();
                    }
                }
            }
        }
    }

    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem->GetTemplate()->ItemId == itemId)
            {
                count += pItem->GetCount();
            }
        }
    }

    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem->GetTemplate()->ItemId == itemId)
            {
                count += pItem->GetCount();
            }
        }
    }

    return count;
}

bool PlayerbotAI::HasItemInInventory(uint32 itemId)
{
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                if (Item* pItem = pBag->GetItemByPos(j))
                {
                    if (pItem->GetTemplate()->ItemId == itemId)
                    {
                        return true;
                    }
                }
            }
        }
    }

    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem->GetTemplate()->ItemId == itemId)
            {
                return true;
            }
        }
    }

    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem->GetTemplate()->ItemId == itemId)
            {
                return true;
            }
        }
    }

    return false;
}

std::vector<std::pair<const Quest*, uint32>> PlayerbotAI::GetCurrentQuestsRequiringItemId(uint32 itemId)
{
    std::vector<std::pair<const Quest*, uint32>> result;

    if (!itemId)
    {
        return result;
    }

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        QuestStatus status = bot->GetQuestStatus(questId);
        const Quest* quest = sObjectMgr->GetQuestTemplate(questId);
        for (uint8 i = 0; i < std::size(quest->RequiredItemId); ++i)
        {
            if (quest->RequiredItemId[i] == itemId)
            {
                result.push_back(std::pair(quest, quest->RequiredItemId[i]));
                break;
            }
        }
    }

    return result;
}

//  on self
void PlayerbotAI::ImbueItem(Item* item) { ImbueItem(item, TARGET_FLAG_NONE, ObjectGuid::Empty); }

//  item on unit
void PlayerbotAI::ImbueItem(Item* item, Unit* target)
{
    if (!target)
        return;

    ImbueItem(item, TARGET_FLAG_UNIT, target->GetGUID());
}

//  item on equipped item
void PlayerbotAI::ImbueItem(Item* item, uint8 targetInventorySlot)
{
    if (targetInventorySlot >= EQUIPMENT_SLOT_END)
        return;

    Item* const targetItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, targetInventorySlot);
    if (!targetItem)
        return;

    ImbueItem(item, TARGET_FLAG_ITEM, targetItem->GetGUID());
}

// generic item use method
void PlayerbotAI::ImbueItem(Item* item, uint32 targetFlag, ObjectGuid targetGUID)
{
    if (!item)
        return;

    uint32 glyphIndex = 0;
    uint8 castFlags = 0;
    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 cast_count = 0;
    ObjectGuid item_guid = item->GetGUID();

    uint32 spellId = 0;
    uint8 spell_index = 0;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (item->GetTemplate()->Spells[i].SpellId > 0)
        {
            spellId = item->GetTemplate()->Spells[i].SpellId;
            spell_index = i;
            break;
        }
    }

    WorldPacket* packet = new WorldPacket(CMSG_USE_ITEM);
    *packet << bagIndex;
    *packet << slot;
    *packet << cast_count;
    *packet << spellId;
    *packet << item_guid;
    *packet << glyphIndex;
    *packet << castFlags;
    *packet << targetFlag;

    if (targetFlag & (TARGET_FLAG_UNIT | TARGET_FLAG_ITEM | TARGET_FLAG_GAMEOBJECT))
        *packet << targetGUID.WriteAsPacked();

    bot->GetSession()->QueuePacket(packet);
}

void PlayerbotAI::EnchantItemT(uint32 spellid, uint8 slot)
{
    Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    if (!pItem || !pItem->IsInWorld() || !pItem->GetOwner() || !pItem->GetOwner()->IsInWorld() ||
        !pItem->GetOwner()->GetSession())
        return;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if (!spellInfo)
        return;

    uint32 enchantid = spellInfo->Effects[0].MiscValue;
    if (!enchantid)
    {
        // LOG_ERROR("playerbots", "{}: Invalid enchantid ", enchantid, " report to devs", bot->GetName().c_str());
        return;
    }

    if (!((1 << pItem->GetTemplate()->SubClass) & spellInfo->EquippedItemSubClassMask) &&
        !((1 << pItem->GetTemplate()->InventoryType) & spellInfo->EquippedItemInventoryTypeMask))
    {
        // LOG_ERROR("playerbots", "{}: items could not be enchanted, wrong item type equipped",
        // bot->GetName().c_str());
        return;
    }

    bot->ApplyEnchantment(pItem, PERM_ENCHANTMENT_SLOT, false);
    pItem->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchantid, 0, 0);
    bot->ApplyEnchantment(pItem, PERM_ENCHANTMENT_SLOT, true);

    LOG_INFO("playerbots", "{}: items was enchanted successfully!", bot->GetName().c_str());
}

uint32 PlayerbotAI::GetBuffedCount(Player* player, std::string const spellname)
{
    uint32 bcount = 0;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (!member || !member->IsInWorld())
                continue;

            if (!member->IsInSameRaidWith(player))
                continue;

            if (HasAura(spellname, member, true))
                bcount++;
        }
    }

    return bcount;
}

int32 PlayerbotAI::GetNearGroupMemberCount(float dis)
{
    int count = 1; // yourself
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (member == bot) // calculated
                continue;

            if (!member || !member->IsInWorld())
                continue;

            if (member->GetMapId() != bot->GetMapId())
                continue;
            
            if (member->GetExactDist(bot) > dis)
                continue;

            count++;
        }
    }
    return count;
}

bool PlayerbotAI::CanMove()
{
    // do not allow if not vehicle driver
    if (IsInVehicle() && !IsInVehicle(true))
        return false;

    if (bot->isFrozen() ||
        bot->IsPolymorphed() ||
        (bot->isDead() && !bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST)) ||
        bot->IsBeingTeleported() ||
        bot->isInRoots() ||
        bot->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION) ||
        bot->HasAuraType(SPELL_AURA_MOD_CONFUSE) ||
        bot->IsCharmed() ||
        bot->HasAuraType(SPELL_AURA_MOD_STUN) ||
        bot->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
        bot->HasUnitState(UNIT_STATE_LOST_CONTROL))

        return false;

    return bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FLIGHT_MOTION_TYPE;
}

bool PlayerbotAI::IsTaxiFlying()
{
    return bot->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) &&
        bot->HasUnitState(UNIT_STATE_IGNORE_PATHFINDING);
}

bool PlayerbotAI::IsInRealGuild()
{
    if (!bot->GetGuildId())
        return false;

    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
    if (!guild)
    {
        return false;
    }
    uint32 leaderAccount = sCharacterCache->GetCharacterAccountIdByGuid(guild->GetLeaderGUID());
    if (!leaderAccount)
        return false;

    return !(sPlayerbotAIConfig->IsInRandomAccountList(leaderAccount));
}

void PlayerbotAI::QueueChatResponse(const ChatQueuedReply chatReply) { chatReplies.push_back(std::move(chatReply)); }

bool PlayerbotAI::EqualLowercaseName(std::string s1, std::string s2)
{
    if (s1.length() != s2.length())
    {
        return false;
    }
    for (int i = 0; i < s1.length(); i++)
    {
        if (tolower(s1[i]) != tolower(s2[i]))
        {
            return false;
        }
    }
    return true;
}

// A custom CanEquipItem (remove AutoUnequipOffhand in FindEquipSlot to prevent unequip on `item usage` calculation)
InventoryResult PlayerbotAI::CanEquipItem(uint8 slot, uint16& dest, Item* pItem, bool swap, bool not_loading) const
{
    dest = 0;
    if (pItem)
    {
        LOG_DEBUG("entities.player.items", "STORAGE: CanEquipItem slot = {}, item = {}, count = {}", slot,
                  pItem->GetEntry(), pItem->GetCount());
        ItemTemplate const* pProto = pItem->GetTemplate();
        if (pProto)
        {
            if (!sScriptMgr->CanEquipItem(bot, slot, dest, pItem, swap, not_loading))
                return EQUIP_ERR_CANT_DO_RIGHT_NOW;

            // item used
            if (pItem->m_lootGenerated)
                return EQUIP_ERR_ALREADY_LOOTED;

            if (pItem->IsBindedNotWith(bot))
                return EQUIP_ERR_DONT_OWN_THAT_ITEM;

            InventoryResult res = bot->CanTakeMoreSimilarItems(pItem);
            if (res != EQUIP_ERR_OK)
                return res;

            ScalingStatDistributionEntry const* ssd =
                pProto->ScalingStatDistribution
                    ? sScalingStatDistributionStore.LookupEntry(pProto->ScalingStatDistribution)
                    : 0;
            // check allowed level (extend range to upper values if MaxLevel more or equal max player level, this let GM
            // set high level with 1...max range items)
            if (ssd && ssd->MaxLevel < DEFAULT_MAX_LEVEL && ssd->MaxLevel < bot->GetLevel())
                return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;

            uint8 eslot = FindEquipSlot(pProto, slot, swap);
            if (eslot == NULL_SLOT)
                return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;

            // Xinef: dont allow to equip items on disarmed slot
            if (!bot->CanUseAttackType(bot->GetAttackBySlot(eslot)))
                return EQUIP_ERR_NOT_WHILE_DISARMED;

            res = bot->CanUseItem(pItem, not_loading);
            if (res != EQUIP_ERR_OK)
                return res;

            if (!swap && bot->GetItemByPos(INVENTORY_SLOT_BAG_0, eslot))
                return EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE;

            // if we are swapping 2 equiped items, CanEquipUniqueItem check
            // should ignore the item we are trying to swap, and not the
            // destination item. CanEquipUniqueItem should ignore destination
            // item only when we are swapping weapon from bag
            uint8 ignore = uint8(NULL_SLOT);
            switch (eslot)
            {
                case EQUIPMENT_SLOT_MAINHAND:
                    ignore = EQUIPMENT_SLOT_OFFHAND;
                    break;
                case EQUIPMENT_SLOT_OFFHAND:
                    ignore = EQUIPMENT_SLOT_MAINHAND;
                    break;
                case EQUIPMENT_SLOT_FINGER1:
                    ignore = EQUIPMENT_SLOT_FINGER2;
                    break;
                case EQUIPMENT_SLOT_FINGER2:
                    ignore = EQUIPMENT_SLOT_FINGER1;
                    break;
                case EQUIPMENT_SLOT_TRINKET1:
                    ignore = EQUIPMENT_SLOT_TRINKET2;
                    break;
                case EQUIPMENT_SLOT_TRINKET2:
                    ignore = EQUIPMENT_SLOT_TRINKET1;
                    break;
            }

            if (ignore == uint8(NULL_SLOT) || pItem != bot->GetItemByPos(INVENTORY_SLOT_BAG_0, ignore))
                ignore = eslot;

            InventoryResult res2 = bot->CanEquipUniqueItem(pItem, swap ? ignore : uint8(NULL_SLOT));
            if (res2 != EQUIP_ERR_OK)
                return res2;

            // check unique-equipped special item classes
            if (pProto->Class == ITEM_CLASS_QUIVER)
                for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
                    if (Item* pBag = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                        if (pBag != pItem)
                            if (ItemTemplate const* pBagProto = pBag->GetTemplate())
                                if (pBagProto->Class == pProto->Class && (!swap || pBag->GetSlot() != eslot))
                                    return (pBagProto->SubClass == ITEM_SUBCLASS_AMMO_POUCH)
                                               ? EQUIP_ERR_CAN_EQUIP_ONLY1_AMMOPOUCH
                                               : EQUIP_ERR_CAN_EQUIP_ONLY1_QUIVER;

            uint32 type = pProto->InventoryType;

            if (eslot == EQUIPMENT_SLOT_OFFHAND)
            {
                // Do not allow polearm to be equipped in the offhand (rare case for the only 1h polearm 41750)
                // xinef: same for fishing poles
                if (type == INVTYPE_WEAPON && (pProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM ||
                                               pProto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE))
                    return EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT;

                else if (type == INVTYPE_WEAPON || type == INVTYPE_WEAPONOFFHAND)
                {
                    if (!bot->CanDualWield())
                        return EQUIP_ERR_CANT_DUAL_WIELD;
                }
                else if (type == INVTYPE_2HWEAPON)
                {
                    if (!bot->CanDualWield() || !bot->CanTitanGrip())
                        return EQUIP_ERR_CANT_DUAL_WIELD;
                }

                if (bot->IsTwoHandUsed())
                    return EQUIP_ERR_CANT_EQUIP_WITH_TWOHANDED;
            }

            // equip two-hand weapon case (with possible unequip 2 items)
            if (type == INVTYPE_2HWEAPON)
            {
                if (eslot == EQUIPMENT_SLOT_OFFHAND)
                {
                    if (!bot->CanTitanGrip())
                        return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;
                }
                else if (eslot != EQUIPMENT_SLOT_MAINHAND)
                    return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;

                if (!bot->CanTitanGrip())
                {
                    // offhand item must can be stored in inventory for offhand item and it also must be unequipped
                    Item* offItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    ItemPosCountVec off_dest;
                    if (offItem && (!not_loading ||
                                    bot->CanUnequipItem(uint16(INVENTORY_SLOT_BAG_0) << 8 | EQUIPMENT_SLOT_OFFHAND,
                                                        false) != EQUIP_ERR_OK ||
                                    bot->CanStoreItem(NULL_BAG, NULL_SLOT, off_dest, offItem, false) != EQUIP_ERR_OK))
                        return swap ? EQUIP_ERR_ITEMS_CANT_BE_SWAPPED : EQUIP_ERR_INVENTORY_FULL;
                }
            }
            dest = ((INVENTORY_SLOT_BAG_0 << 8) | eslot);
            return EQUIP_ERR_OK;
        }
    }

    return !swap ? EQUIP_ERR_ITEM_NOT_FOUND : EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;
}

uint8 PlayerbotAI::FindEquipSlot(ItemTemplate const* proto, uint32 slot, bool swap) const
{
    uint8 slots[4];
    slots[0] = NULL_SLOT;
    slots[1] = NULL_SLOT;
    slots[2] = NULL_SLOT;
    slots[3] = NULL_SLOT;
    switch (proto->InventoryType)
    {
        case INVTYPE_HEAD:
            slots[0] = EQUIPMENT_SLOT_HEAD;
            break;
        case INVTYPE_NECK:
            slots[0] = EQUIPMENT_SLOT_NECK;
            break;
        case INVTYPE_SHOULDERS:
            slots[0] = EQUIPMENT_SLOT_SHOULDERS;
            break;
        case INVTYPE_BODY:
            slots[0] = EQUIPMENT_SLOT_BODY;
            break;
        case INVTYPE_CHEST:
        case INVTYPE_ROBE:
            slots[0] = EQUIPMENT_SLOT_CHEST;
            break;
        case INVTYPE_WAIST:
            slots[0] = EQUIPMENT_SLOT_WAIST;
            break;
        case INVTYPE_LEGS:
            slots[0] = EQUIPMENT_SLOT_LEGS;
            break;
        case INVTYPE_FEET:
            slots[0] = EQUIPMENT_SLOT_FEET;
            break;
        case INVTYPE_WRISTS:
            slots[0] = EQUIPMENT_SLOT_WRISTS;
            break;
        case INVTYPE_HANDS:
            slots[0] = EQUIPMENT_SLOT_HANDS;
            break;
        case INVTYPE_FINGER:
            slots[0] = EQUIPMENT_SLOT_FINGER1;
            slots[1] = EQUIPMENT_SLOT_FINGER2;
            break;
        case INVTYPE_TRINKET:
            slots[0] = EQUIPMENT_SLOT_TRINKET1;
            slots[1] = EQUIPMENT_SLOT_TRINKET2;
            break;
        case INVTYPE_CLOAK:
            slots[0] = EQUIPMENT_SLOT_BACK;
            break;
        case INVTYPE_WEAPON:
        {
            slots[0] = EQUIPMENT_SLOT_MAINHAND;

            // suggest offhand slot only if know dual wielding
            // (this will be replace mainhand weapon at auto equip instead unwonted "you don't known dual wielding" ...
            if (bot->CanDualWield())
                slots[1] = EQUIPMENT_SLOT_OFFHAND;
            break;
        }
        case INVTYPE_SHIELD:
        case INVTYPE_WEAPONOFFHAND:
        case INVTYPE_HOLDABLE:
            slots[0] = EQUIPMENT_SLOT_OFFHAND;
            break;
        case INVTYPE_RANGED:
        case INVTYPE_RANGEDRIGHT:
        case INVTYPE_THROWN:
            slots[0] = EQUIPMENT_SLOT_RANGED;
            break;
        case INVTYPE_2HWEAPON:
            slots[0] = EQUIPMENT_SLOT_MAINHAND;
            if (Item* mhWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
            {
                if (ItemTemplate const* mhWeaponProto = mhWeapon->GetTemplate())
                {
                    if (mhWeaponProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM ||
                        mhWeaponProto->SubClass == ITEM_SUBCLASS_WEAPON_STAFF)
                    {
                        bot->AutoUnequipOffhandIfNeed(true);
                        break;
                    }
                }
            }

            if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
            {
                if (proto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM || proto->SubClass == ITEM_SUBCLASS_WEAPON_STAFF)
                {
                    break;
                }
            }
            if (bot->CanDualWield() && bot->CanTitanGrip() && proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && proto->SubClass != ITEM_SUBCLASS_WEAPON_FISHING_POLE)
                slots[1] = EQUIPMENT_SLOT_OFFHAND;
            break;
        case INVTYPE_TABARD:
            slots[0] = EQUIPMENT_SLOT_TABARD;
            break;
        case INVTYPE_WEAPONMAINHAND:
            slots[0] = EQUIPMENT_SLOT_MAINHAND;
            break;
        case INVTYPE_BAG:
            slots[0] = INVENTORY_SLOT_BAG_START + 0;
            slots[1] = INVENTORY_SLOT_BAG_START + 1;
            slots[2] = INVENTORY_SLOT_BAG_START + 2;
            slots[3] = INVENTORY_SLOT_BAG_START + 3;
            break;
        case INVTYPE_RELIC:
        {
            switch (proto->SubClass)
            {
                case ITEM_SUBCLASS_ARMOR_LIBRAM:
                    if (bot->IsClass(CLASS_PALADIN, CLASS_CONTEXT_EQUIP_RELIC))
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
                case ITEM_SUBCLASS_ARMOR_IDOL:
                    if (bot->IsClass(CLASS_DRUID, CLASS_CONTEXT_EQUIP_RELIC))
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
                case ITEM_SUBCLASS_ARMOR_TOTEM:
                    if (bot->IsClass(CLASS_SHAMAN, CLASS_CONTEXT_EQUIP_RELIC))
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
                case ITEM_SUBCLASS_ARMOR_MISC:
                    if (bot->IsClass(CLASS_WARLOCK, CLASS_CONTEXT_EQUIP_RELIC))
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
                case ITEM_SUBCLASS_ARMOR_SIGIL:
                    if (bot->IsClass(CLASS_DEATH_KNIGHT, CLASS_CONTEXT_EQUIP_RELIC))
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
            }
            break;
        }
        default:
            return NULL_SLOT;
    }

    if (slot != NULL_SLOT)
    {
        if (swap || !bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            for (uint8 i = 0; i < 4; ++i)
                if (slots[i] == slot)
                    return slot;
    }
    else
    {
        // search free slot at first
        for (uint8 i = 0; i < 4; ++i)
            if (slots[i] != NULL_SLOT && !bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slots[i]))
                // in case 2hand equipped weapon (without titan grip) offhand slot empty but not free
                if (slots[i] != EQUIPMENT_SLOT_OFFHAND || !bot->IsTwoHandUsed())
                    return slots[i];

        // if not found free and can swap return first appropriate from used
        for (uint8 i = 0; i < 4; ++i)
            if (slots[i] != NULL_SLOT && swap)
                return slots[i];
    }

    // no free position
    return NULL_SLOT;
}

bool PlayerbotAI::IsSafe(Player* player)
{
    return player && player->GetMapId() == bot->GetMapId() && player->GetInstanceId() == bot->GetInstanceId() &&
           !player->IsBeingTeleported();
}
bool PlayerbotAI::IsSafe(WorldObject* obj)
{
    return obj && obj->GetMapId() == bot->GetMapId() && obj->GetInstanceId() == bot->GetInstanceId() &&
           (!obj->IsPlayer() || !((Player*)obj)->IsBeingTeleported());
}
ChatChannelSource PlayerbotAI::GetChatChannelSource(Player* bot, uint32 type, std::string channelName)
{
    if (type == CHAT_MSG_CHANNEL)
    {
        if (channelName == "World")
            return ChatChannelSource::SRC_WORLD;
        else
        {
            ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId());
            if (!cMgr)
            {
                return ChatChannelSource::SRC_UNDEFINED;
            }

            const Channel* channel = cMgr->GetChannel(channelName, bot);
            if (channel)
            {
                switch (channel->GetChannelId())
                {
                    case ChatChannelId::GENERAL:
                    {
                        return ChatChannelSource::SRC_GENERAL;
                    }
                    case ChatChannelId::TRADE:
                    {
                        return ChatChannelSource::SRC_TRADE;
                    }
                    case ChatChannelId::LOCAL_DEFENSE:
                    {
                        return ChatChannelSource::SRC_LOCAL_DEFENSE;
                    }
                    case ChatChannelId::WORLD_DEFENSE:
                    {
                        return ChatChannelSource::SRC_WORLD_DEFENSE;
                    }
                    case ChatChannelId::LOOKING_FOR_GROUP:
                    {
                        return ChatChannelSource::SRC_LOOKING_FOR_GROUP;
                    }
                    case ChatChannelId::GUILD_RECRUITMENT:
                    {
                        return ChatChannelSource::SRC_GUILD_RECRUITMENT;
                    }
                    default:
                    {
                        return ChatChannelSource::SRC_UNDEFINED;
                    }
                }
            }
        }
    }
    else
    {
        switch (type)
        {
            case CHAT_MSG_WHISPER:
            {
                return ChatChannelSource::SRC_WHISPER;
            }
            case CHAT_MSG_SAY:
            {
                return ChatChannelSource::SRC_SAY;
            }
            case CHAT_MSG_YELL:
            {
                return ChatChannelSource::SRC_YELL;
            }
            case CHAT_MSG_GUILD:
            {
                return ChatChannelSource::SRC_GUILD;
            }
            case CHAT_MSG_PARTY:
            {
                return ChatChannelSource::SRC_PARTY;
            }
            case CHAT_MSG_RAID:
            {
                return ChatChannelSource::SRC_RAID;
            }
            case CHAT_MSG_EMOTE:
            {
                return ChatChannelSource::SRC_EMOTE;
            }
            case CHAT_MSG_TEXT_EMOTE:
            {
                return ChatChannelSource::SRC_TEXT_EMOTE;
            }
            default:
            {
                return ChatChannelSource::SRC_UNDEFINED;
            }
        }
    }
    return ChatChannelSource::SRC_UNDEFINED;
}

std::vector<const Quest*> PlayerbotAI::GetAllCurrentQuests()
{
    std::vector<const Quest*> result;

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
        {
            continue;
        }

        result.push_back(sObjectMgr->GetQuestTemplate(questId));
    }

    return result;
}

std::vector<const Quest*> PlayerbotAI::GetCurrentIncompleteQuests()
{
    std::vector<const Quest*> result;

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
        {
            continue;
        }

        QuestStatus status = bot->GetQuestStatus(questId);
        if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_NONE)
        {
            result.push_back(sObjectMgr->GetQuestTemplate(questId));
        }
    }

    return result;
}

std::set<uint32> PlayerbotAI::GetAllCurrentQuestIds()
{
    std::set<uint32> result;

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
        {
            continue;
        }

        result.insert(questId);
    }

    return result;
}

std::set<uint32> PlayerbotAI::GetCurrentIncompleteQuestIds()
{
    std::set<uint32> result;

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
        {
            continue;
        }

        QuestStatus status = bot->GetQuestStatus(questId);
        if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_NONE)
        {
            result.insert(questId);
        }
    }

    return result;
}

uint32 PlayerbotAI::GetReactDelay()
{
    uint32 base = sPlayerbotAIConfig->reactDelay;
    // old calculate method
    if (!sPlayerbotAIConfig->dynamicReactDelay)
    {
        inCombat = bot->IsInCombat();
        bool min = false;
        // test fix lags because of BG
        bool inBG = bot->InBattleground() || bot->InArena();
        if (bot && !inCombat)
            min = true;

        if (HasRealPlayerMaster() || (sPlayerbotAIConfig->fastReactInBG && inBG))
            min = false;
        if (min)
            return base * 10;

        return base;
    }

    float multiplier = 1.0f;

    if (HasRealPlayerMaster())
    {
        multiplier = 1.0f;
        return base * multiplier;
    }

    bool inBg = bot->InBattleground() || bot->InArena();
    if (inBg)
    {
        multiplier = sPlayerbotAIConfig->fastReactInBG ? 1.0f : 10.0f;
        return base * multiplier;
    }

    if (bot->IsInCombat() || currentState == BOT_STATE_COMBAT)
    {
        multiplier = 5.0f;
        return base * multiplier;
    }

    bool isResting = bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
    if (!isResting)
    {
        multiplier = urand(10, 30);
        return base * multiplier;
    }

    multiplier = urand(20, 200);
    return base * multiplier;
}

void PlayerbotAI::PetFollow()
{
    Pet* pet = bot->GetPet();
    if (!pet)
        return;
    pet->AttackStop();
    pet->InterruptNonMeleeSpells(false);
    pet->ClearInPetCombat();
    pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());
    if (pet->ToPet())
        pet->ToPet()->ClearCastWhenWillAvailable();
    CharmInfo* charmInfo = pet->GetCharmInfo();
    if (!charmInfo)
        return;
    charmInfo->SetCommandState(COMMAND_FOLLOW);

    charmInfo->SetIsCommandAttack(false);
    charmInfo->SetIsAtStay(false);
    charmInfo->SetIsReturning(true);
    charmInfo->SetIsCommandFollow(true);
    charmInfo->SetIsFollowing(false);
    charmInfo->RemoveStayPosition();
    charmInfo->SetForcedSpell(0);
    charmInfo->SetForcedTargetGUID();
}

float PlayerbotAI::GetItemScoreMultiplier(ItemQualities quality)
{
    switch (quality)
    {
        // each quality increase 1.1x
        case ITEM_QUALITY_POOR:
            return 1.0f;
            break;
        case ITEM_QUALITY_NORMAL:
            return 1.1f;
            break;
        case ITEM_QUALITY_UNCOMMON:
            return 1.21f;
            break;
        case ITEM_QUALITY_RARE:
            return 1.331f;
            break;
        case ITEM_QUALITY_EPIC:
            return 1.4641f;
            break;
        case ITEM_QUALITY_LEGENDARY:
            return 1.61051f;
            break;
        default:
            break;
    }
    return 1.0f;
}
