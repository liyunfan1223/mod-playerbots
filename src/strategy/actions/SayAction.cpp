/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AiFactory.h"
#include "SayAction.h"

#include <regex>
#include <string>

#include "ChannelMgr.h"
#include "Event.h"
#include "GuildMgr.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

static const std::unordered_set<std::string> noReplyMsgs = {
    "join",
    "leave",
    "follow",
    "attack",
    "pull",
    "flee",
    "reset",
    "reset ai",
    "all ?",
    "talents",
    "talents list",
    "talents auto",
    "talk",
    "stay",
    "stats",
    "who",
    "items",
    "leave",
    "join",
    "repair",
    "summon",
    "nc ?",
    "co ?",
    "de ?",
    "dead ?",
    "follow",
    "los",
    "guard",
    "do accept invitation",
    "stats",
    "react ?",
    "reset strats",
    "home",
};
static const std::unordered_set<std::string> noReplyMsgParts = {
    "+", "-", "@", "follow target", "focus heal", "cast ", "accept [", "e [", "destroy [", "go zone"};
static const std::unordered_set<std::string> noReplyMsgStarts = {"e ", "accept ", "cast ", "destroy "};

SayAction::SayAction(PlayerbotAI* botAI) : Action(botAI, "say"), Qualified() {}

bool SayAction::Execute(Event event)
{
    std::string text = "";
    std::map<std::string, std::string> placeholders;
    Unit* target = AI_VALUE(Unit*, "tank target");
    if (!target)
        target = AI_VALUE(Unit*, "current target");

    // set replace strings
    if (target)
        placeholders["<target>"] = target->GetName();
    placeholders["<randomfaction>"] = IsAlliance(bot->getRace()) ? "Alliance" : "Horde";
    if (qualifier == "low ammo" || qualifier == "no ammo")
    {
        if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
        {
            switch (pItem->GetTemplate()->SubClass)
            {
                case ITEM_SUBCLASS_WEAPON_GUN:
                    placeholders["<ammo>"] = "bullets";
                    break;
                case ITEM_SUBCLASS_WEAPON_BOW:
                case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    placeholders["<ammo>"] = "arrows";
                    break;
            }
        }
    }

    if (bot->GetMap())
    {
        if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(bot->GetMap()->GetZoneId(bot->GetPhaseMask(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
            placeholders["<subzone>"] = zone->area_name[sWorld->GetDefaultDbcLocale()];
    }

    // set delay before next say
    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    uint32 nextTime = time(nullptr) + urand(1, 30);
    botAI->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime);

    Group* group = bot->GetGroup();
    if (group)
    {
        std::vector<Player*> members;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            PlayerbotAI* memberAi = GET_PLAYERBOT_AI(member);
            if (memberAi)
                members.push_back(member);
        }

        uint32 count = members.size();
        if (count > 1)
        {
            for (uint32 i = 0; i < count * 5; i++)
            {
                int i1 = urand(0, count - 1);
                int i2 = urand(0, count - 1);

                Player* item = members[i1];
                members[i1] = members[i2];
                members[i2] = item;
            }
        }

        int index = 0;
        for (auto& member : members)
        {
            PlayerbotAI* memberAi = GET_PLAYERBOT_AI(member);
            if (memberAi)
                memberAi->GetAiObjectContext()
                    ->GetValue<time_t>("last said", qualifier)
                    ->Set(nextTime + (20 * ++index) + urand(1, 15));
        }
    }

    // load text based on chance
    if (!sPlayerbotTextMgr->GetBotText(qualifier, text, placeholders))
        return false;

    if (text.find("/y ") == 0)
        bot->Yell(text.substr(3), (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
    else
        bot->Say(text, (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));

    return true;
}

bool SayAction::isUseful()
{
    if (!botAI->AllowActivity())
        return false;

    if (botAI->HasStrategy("silent", BotState::BOT_STATE_NON_COMBAT))
        return false;

    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    return (time(nullptr) - lastSaid) > 30;
}

void ChatReplyAction::ChatReplyDo(Player* bot, uint32& type, uint32& guid1, uint32& guid2, std::string& msg, std::string& chanName, std::string& name)
{
    ChatReplyType replyType = REPLY_NOT_UNDERSTAND;  // default not understand
    std::string respondsText = "";

    // if we're just commanding bots around, don't respond...
    // first one is for exact word matches
    if (noReplyMsgs.find(msg) != noReplyMsgs.end())
    {
        /*std::ostringstream out;
        out << "DEBUG ChatReplyDo decided to ignore exact blocklist match" << msg;
        bot->Say(out.str(), LANG_UNIVERSAL);*/
        return;
    }

    // second one is for partial matches like + or - where we change strats
    if (std::any_of(noReplyMsgParts.begin(), noReplyMsgParts.end(),
                    [&msg](const std::string& part) { return msg.find(part) != std::string::npos; }))
    {
        /*std::ostringstream out;
        out << "DEBUG ChatReplyDo decided to ignore partial blocklist match" << msg;
        bot->Say(out.str(), LANG_UNIVERSAL);*/
        return;
    }

    if (std::any_of(noReplyMsgStarts.begin(), noReplyMsgStarts.end(),
                    [&msg](const std::string& start)
                    {
                        return msg.find(start) == 0;  // Check if the start matches the beginning of msg
                    }))
    {
        /*std::ostringstream out;
        out << "DEBUG ChatReplyDo decided to ignore start blocklist match" << msg;
        bot->Say(out.str(), LANG_UNIVERSAL);*/
        return;
    }

    ChatChannelSource chatChannelSource = GET_PLAYERBOT_AI(bot)->GetChatChannelSource(bot, type, chanName);
    if ( (msg.starts_with("LFG") || msg.starts_with("LFM")) && HandleLFGQuestsReply(bot, chatChannelSource, msg, name))
    {
        return;
    }

    if (msg.starts_with("WTB") && HandleWTBItemsReply(bot, chatChannelSource, msg, name))
    {
        return;
    }

    //toxic links
    if (msg.starts_with(sPlayerbotAIConfig->toxicLinksPrefix)
        && (GET_PLAYERBOT_AI(bot)->GetChatHelper()->ExtractAllItemIds(msg).size() > 0 || GET_PLAYERBOT_AI(bot)->GetChatHelper()->ExtractAllQuestIds(msg).size() > 0))
    {
        HandleToxicLinksReply(bot, chatChannelSource, msg, name);
        return;
    }

    //thunderfury
    if (GET_PLAYERBOT_AI(bot)->GetChatHelper()->ExtractAllItemIds(msg).count(19019))
    {
        HandleThunderfuryReply(bot, chatChannelSource, msg, name);
        return;
    }

    auto messageRepy = GenerateReplyMessage(bot, msg, guid1, name);
    SendGeneralResponse(bot, chatChannelSource, messageRepy, name);
}

bool ChatReplyAction::HandleThunderfuryReply(Player* bot, ChatChannelSource chatChannelSource, std::string& msg, std::string& name)
{
    std::map<std::string, std::string> placeholders;
    const auto thunderfury = sObjectMgr->GetItemTemplate(19019);
    placeholders["%thunderfury_link"] = GET_PLAYERBOT_AI(bot)->GetChatHelper()->FormatItem(thunderfury);

    std::string responseMessage = BOT_TEXT2("thunderfury_spam", placeholders);

    switch (chatChannelSource)
    {
        case ChatChannelSource::SRC_WORLD:
        {
            GET_PLAYERBOT_AI(bot)->SayToWorld(responseMessage);
            break;
        }
        case ChatChannelSource::SRC_GENERAL:
        {
            GET_PLAYERBOT_AI(bot)->SayToChannel(responseMessage, ChatChannelId::GENERAL);
            break;
        }
        default:
            break;
    }

    GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 25));
    return true;
}

bool ChatReplyAction::HandleToxicLinksReply(Player* bot, ChatChannelSource chatChannelSource, std::string& msg, std::string& name)
{
    //quests
    std::vector<uint32> incompleteQuests;
    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        QuestStatus status = bot->GetQuestStatus(questId);
        if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_NONE)
            incompleteQuests.push_back(questId);
    }

    //items
    std::vector<Item*> botItems = GET_PLAYERBOT_AI(bot)->GetInventoryAndEquippedItems();

    std::map<std::string, std::string> placeholders;
    placeholders["%random_inventory_item_link"] = botItems.size() > 0 ? GET_PLAYERBOT_AI(bot)->GetChatHelper()->FormatItem(botItems[rand() % botItems.size()]->GetTemplate()) : BOT_TEXT1("string_empty_link");
    placeholders["%prefix"] = sPlayerbotAIConfig->toxicLinksPrefix;

    if (incompleteQuests.size() > 0)
    {
        Quest const* quest = sObjectMgr->GetQuestTemplate(incompleteQuests[rand() % incompleteQuests.size()]);
        placeholders["%random_taken_quest_or_item_link"] = GET_PLAYERBOT_AI(bot)->GetChatHelper()->FormatQuest(quest);
    }
    else
    {
        placeholders["%random_taken_quest_or_item_link"] = placeholders["%random_inventory_item_link"];
    }

    placeholders["%my_role"] = ChatHelper::FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));
    AreaTableEntry const* current_area = GET_PLAYERBOT_AI(bot)->GetCurrentArea();
    AreaTableEntry const* current_zone = GET_PLAYERBOT_AI(bot)->GetCurrentZone();
    placeholders["%area_name"] = current_area ? GET_PLAYERBOT_AI(bot)->GetLocalizedAreaName(current_area) : BOT_TEXT1("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? GET_PLAYERBOT_AI(bot)->GetLocalizedAreaName(current_zone) : BOT_TEXT1("string_unknown_area");
    placeholders["%my_class"] = GET_PLAYERBOT_AI(bot)->GetChatHelper()->FormatClass(bot->getClass());
    placeholders["%my_race"] = GET_PLAYERBOT_AI(bot)->GetChatHelper()->FormatRace(bot->getRace());
    placeholders["%my_level"] = std::to_string(bot->GetLevel());

    switch (chatChannelSource)
    {
        case ChatChannelSource::SRC_WORLD:
        {
            GET_PLAYERBOT_AI(bot)->SayToWorld(BOT_TEXT2("suggest_toxic_links", placeholders));
            break;
        }
        case ChatChannelSource::SRC_GENERAL:
        {
            GET_PLAYERBOT_AI(bot)->SayToChannel(BOT_TEXT2("suggest_toxic_links", placeholders), ChatChannelId::GENERAL);
            break;
        }
        case ChatChannelSource::SRC_GUILD:
        {
            GET_PLAYERBOT_AI(bot)->SayToGuild(BOT_TEXT2("suggest_toxic_links", placeholders));
            break;
        }
        default:
            break;
    }

    GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 60));

    return true;
}
bool ChatReplyAction::HandleWTBItemsReply(Player* bot, ChatChannelSource chatChannelSource, std::string& msg, std::string& name)
{
    auto messageItemIds = GET_PLAYERBOT_AI(bot)->GetChatHelper()->ExtractAllItemIds(msg);

    if (messageItemIds.empty())
    {
        return false;
    }

    std::set<uint32> matchingItemIds;

    for (auto messageItemId : messageItemIds)
    {
        if (GET_PLAYERBOT_AI(bot)->HasItemInInventory(messageItemId))
        {
            matchingItemIds.insert(messageItemId);
        }
    }

    if (!matchingItemIds.empty())
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%other_name"] = name;
        AreaTableEntry const* current_area = GET_PLAYERBOT_AI(bot)->GetCurrentArea();
        AreaTableEntry const* current_zone = GET_PLAYERBOT_AI(bot)->GetCurrentZone();
        placeholders["%area_name"] = current_area ? GET_PLAYERBOT_AI(bot)->GetLocalizedAreaName(current_area) : BOT_TEXT1("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? GET_PLAYERBOT_AI(bot)->GetLocalizedAreaName(current_zone) : BOT_TEXT1("string_unknown_area");
        placeholders["%my_class"] = GET_PLAYERBOT_AI(bot)->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = GET_PLAYERBOT_AI(bot)->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());
        placeholders["%my_role"] = ChatHelper::FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        placeholders["%formatted_item_links"] = "";

        for (auto matchingItemId : matchingItemIds)
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(matchingItemId);
            placeholders["%formatted_item_links"] += GET_PLAYERBOT_AI(bot)->GetChatHelper()->FormatItem(proto, GET_PLAYERBOT_AI(bot)->GetInventoryItemsCountWithId(matchingItemId));
            placeholders["%formatted_item_links"] += " ";
        }

        switch (chatChannelSource)
        {
            case ChatChannelSource::SRC_WORLD:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = BOT_TEXT2("response_wtb_items_channel", placeholders);
                    GET_PLAYERBOT_AI(bot)->SayToWorld(responseMessage);
                }
                else
                {
                    std::string responseMessage = BOT_TEXT2("response_wtb_items_whisper", placeholders);
                    GET_PLAYERBOT_AI(bot)->Whisper(responseMessage, name);
                }
                break;
            }
            case ChatChannelSource::SRC_GENERAL:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = BOT_TEXT2("response_wtb_items_channel", placeholders);
                    GET_PLAYERBOT_AI(bot)->SayToChannel(responseMessage, ChatChannelId::GENERAL);
                }
                else
                {
                    std::string responseMessage = BOT_TEXT2("response_wtb_items_whisper", placeholders);
                    GET_PLAYERBOT_AI(bot)->Whisper(responseMessage, name);
                }
                break;
            }
            case ChatChannelSource::SRC_TRADE:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = BOT_TEXT2("response_wtb_items_channel", placeholders);
                    GET_PLAYERBOT_AI(bot)->SayToChannel(responseMessage, ChatChannelId::TRADE);
                }
                else
                {
                    std::string responseMessage = BOT_TEXT2("response_wtb_items_whisper", placeholders);
                    GET_PLAYERBOT_AI(bot)->Whisper(responseMessage, name);
                }
                break;
            }
            default:
            break;
        }
        GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 60));
    }

    return true;
}
bool ChatReplyAction::HandleLFGQuestsReply(Player* bot, ChatChannelSource chatChannelSource, std::string& msg, std::string& name)
{
    auto messageQuestIds = GET_PLAYERBOT_AI(bot)->GetChatHelper()->ExtractAllQuestIds(msg);

    if (messageQuestIds.empty())
    {
        return false;
    }

    auto botQuestIds = GET_PLAYERBOT_AI(bot)->GetAllCurrentQuestIds();
    std::set<uint32> matchingQuestIds;
    for (auto botQuestId : botQuestIds)
    {
        if (messageQuestIds.count(botQuestId) != 0)
        {
            matchingQuestIds.insert(botQuestId);
        }
    }

    if (!matchingQuestIds.empty())
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%other_name"] = name;
        AreaTableEntry const* current_area = GET_PLAYERBOT_AI(bot)->GetCurrentArea();
        AreaTableEntry const* current_zone = GET_PLAYERBOT_AI(bot)->GetCurrentZone();
        placeholders["%area_name"] = current_area ? GET_PLAYERBOT_AI(bot)->GetLocalizedAreaName(current_area) : BOT_TEXT1("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? GET_PLAYERBOT_AI(bot)->GetLocalizedAreaName(current_zone) : BOT_TEXT1("string_unknown_area");
        placeholders["%my_class"] = GET_PLAYERBOT_AI(bot)->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = GET_PLAYERBOT_AI(bot)->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());
        placeholders["%my_role"] = ChatHelper::FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        placeholders["%quest_links"] = "";
        for (auto matchingQuestId : matchingQuestIds)
        {
            Quest const* quest = sObjectMgr->GetQuestTemplate(matchingQuestId);
            placeholders["%quest_links"] += GET_PLAYERBOT_AI(bot)->GetChatHelper()->FormatQuest(quest);
        }

        switch (chatChannelSource)
        {
            case ChatChannelSource::SRC_WORLD:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = BOT_TEXT2("response_lfg_quests_channel", placeholders);
                    GET_PLAYERBOT_AI(bot)->SayToWorld(responseMessage);
                }
                else
                {
                    std::string responseMessage = BOT_TEXT2("response_lfg_quests_whisper", placeholders);
                    GET_PLAYERBOT_AI(bot)->Whisper(responseMessage, name);
                }
                break;
            }
            case ChatChannelSource::SRC_GENERAL:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = BOT_TEXT2("response_lfg_quests_channel", placeholders);
                    GET_PLAYERBOT_AI(bot)->SayToChannel(responseMessage, ChatChannelId::GENERAL);
                }
                else
                {
                    std::string responseMessage = BOT_TEXT2("response_lfg_quests_whisper", placeholders);
                    GET_PLAYERBOT_AI(bot)->Whisper(responseMessage, name);
                }
                break;
            }
            case ChatChannelSource::SRC_LOOKING_FOR_GROUP:
            {
                //do not reply to the chat
                //may whisper
                std::string responseMessage = BOT_TEXT2("response_lfg_quests_whisper", placeholders);
                GET_PLAYERBOT_AI(bot)->Whisper(responseMessage, name);
                break;
            }
            default:
            break;
        }
        GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 25));
    }

    return true;
}

bool ChatReplyAction::SendGeneralResponse(Player* bot, ChatChannelSource chatChannelSource, std::string& responseMessage, std::string& name)
{
    // send responds
    switch (chatChannelSource)
    {
        case ChatChannelSource::SRC_WORLD:
        {
            //may reply to the same channel or whisper
            GET_PLAYERBOT_AI(bot)->SayToWorld(responseMessage);
            break;
        }
        case ChatChannelSource::SRC_GENERAL:
        {
            //may reply to the same channel 80% or whisper
            if (urand(0, 100) < 80)
                GET_PLAYERBOT_AI(bot)->SayToChannel(responseMessage, ChatChannelId::GENERAL);
            else
                GET_PLAYERBOT_AI(bot)->Whisper(responseMessage, name);
            break;
        }
        case ChatChannelSource::SRC_TRADE:
        {
            //do not reply to the chat
            //may whisper
            break;
        }
        case ChatChannelSource::SRC_LOCAL_DEFENSE:
        {
            //may reply to the same channel or whisper
            GET_PLAYERBOT_AI(bot)->SayToChannel(responseMessage, ChatChannelId::LOCAL_DEFENSE);
            break;
        }
        case ChatChannelSource::SRC_WORLD_DEFENSE:
        {
            //may whisper
            break;
        }
        case ChatChannelSource::SRC_LOOKING_FOR_GROUP:
        {
            //do not reply to the chat
            break;
        }
        case ChatChannelSource::SRC_GUILD_RECRUITMENT:
        {
            //do not reply to the chat
            break;
        }
        case ChatChannelSource::SRC_WHISPER:
        {
            GET_PLAYERBOT_AI(bot)->Whisper(responseMessage, name);
            break;
        }
        case ChatChannelSource::SRC_SAY:
        {
            GET_PLAYERBOT_AI(bot)->Say(responseMessage);
            break;
        }
        case ChatChannelSource::SRC_YELL:
        {
            GET_PLAYERBOT_AI(bot)->Yell(responseMessage);
            break;
        }
        case ChatChannelSource::SRC_GUILD:
        {
            GET_PLAYERBOT_AI(bot)->SayToGuild(responseMessage);
            break;
        }
        default:
            break;
    }
    GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 25));

    return true;
}

std::string ChatReplyAction::GenerateReplyMessage(Player* bot, std::string& incomingMessage, uint32& guid1, std::string& name)
{
    ChatReplyType replyType = REPLY_NOT_UNDERSTAND; // default not understand

    std::string respondsText = "";

    // Chat Logic
    int32 verb_pos = -1;
    int32 verb_type = -1;
    int32 is_quest = 0;
    bool found = false;
    std::stringstream text(incomingMessage);
    std::string segment;
    std::vector<std::string> word;
    while (std::getline(text, segment, ' '))
    {
        word.push_back(segment);
    }

    for (uint32 i = 0; i < 15; i++)
    {
        if (word.size() < i)
            word.push_back("");
    }

    if (incomingMessage.find("?") != std::string::npos)
        is_quest = 1;
    if (word[0].find("what") != std::string::npos)
        is_quest = 2;
    else if (word[0].find("who") != std::string::npos)
        is_quest = 3;
    else if (word[0] == "when")
        is_quest = 4;
    else if (word[0] == "where")
        is_quest = 5;
    else if (word[0] == "why")
        is_quest = 6;

    // Responds
    for (uint32 i = 0; i < 8; i++)
    {
        // blame gm with chat tag
        if (Player* plr = ObjectAccessor::FindPlayer(ObjectGuid(HighGuid::Player, guid1)))
        {
            if (plr->isGMChat())
            {
                replyType = REPLY_ADMIN_ABUSE;
                found = true;
                break;
            }
        }

        if (word[i] == "hi" || word[i] == "hey" || word[i] == "hello" || word[i] == "wazzup")
        {
            replyType = REPLY_HELLO;
            found = true;
            break;
        }

        if (verb_type < 4)
        {
            if (word[i] == "am" || word[i] == "are" || word[i] == "is")
            {
                verb_pos = i;
                verb_type = 2;  // present
                if (verb_pos == 0)
                    is_quest = 1;
            }
            else if (word[i] == "will")
            {
                verb_pos = i;
                verb_type = 3;  // future
            }
            else if (word[i] == "was" || word[i] == "were")
            {
                verb_pos = i;
                verb_type = 1;  // past
            }
            else if (word[i] == "shut" || word[i] == "noob")
            {
                if (incomingMessage.find(bot->GetName()) == std::string::npos)
                {
                    continue;  // not react
                    uint32 rnd = urand(0, 2);
                    std::string msg = "";
                    if (rnd == 0)
                        msg = "sorry %s, ill shut up now";
                    if (rnd == 1)
                        msg = "ok ok %s";
                    if (rnd == 2)
                        msg = "fine, i wont talk to you anymore %s";

                    msg = std::regex_replace(msg, std::regex("%s"), name);
                    respondsText = msg;
                    found = true;
                    break;
                }
                else
                {
                    replyType = REPLY_GRUDGE;
                    found = true;
                    break;
                }
            }
        }
    }
    if (verb_type < 4 && is_quest && !found)
    {
        switch (is_quest)
        {
        case 2:
        {
            uint32 rnd = urand(0, 3);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "i dont know what";
                break;
            case 1:
                msg = "i dont know %s";
                break;
            case 2:
                msg = "who cares";
                break;
            case 3:
                msg = "afraid that was before i was around or paying attention";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 3:
        {
            uint32 rnd = urand(0, 4);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "nobody";
                break;
            case 1:
                msg = "we all do";
                break;
            case 2:
                msg = "perhaps its you, %s";
                break;
            case 3:
                msg = "dunno %s";
                break;
            case 4:
                msg = "is it me?";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 4:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "soon perhaps %s";
                break;
            case 1:
                msg = "probably later";
                break;
            case 2:
                msg = "never";
                break;
            case 3:
                msg = "what do i look like, a psychic?";
                break;
            case 4:
                msg = "a few minutes, maybe an hour ... years?";
                break;
            case 5:
                msg = "when? good question %s";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 5:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "really want me to answer that?";
                break;
            case 1:
                msg = "on the map?";
                break;
            case 2:
                msg = "who cares";
                break;
            case 3:
                msg = "afk?";
                break;
            case 4:
                msg = "none of your buisiness where";
                break;
            case 5:
                msg = "yeah, where?";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 6:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "dunno %s";
                break;
            case 1:
                msg = "why? just because %s";
                break;
            case 2:
                msg = "why is the sky blue?";
                break;
            case 3:
                msg = "dont ask me %s, im just a bot";
                break;
            case 4:
                msg = "your asking the wrong person";
                break;
            case 5:
                msg = "who knows?";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        default:
        {
            switch (verb_type)
            {
            case 1:
            {
                uint32 rnd = urand(0, 3);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "its true, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 4];
                    break;
                case 1:
                    msg = "ya %s but thats in the past";
                    break;
                case 2:
                    msg = "nah, but " + word[verb_pos + 1] + " will " + word[verb_pos + 3] + " again though %s";
                    break;
                case 3:
                    msg = "afraid that was before i was around or paying attention";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 2:
            {
                uint32 rnd = urand(0, 6);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "its true, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                    break;
                case 1:
                    msg = "ya %s thats true";
                    break;
                case 2:
                    msg = "maybe " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                    break;
                case 3:
                    msg = "dunno %s";
                    break;
                case 4:
                    msg = "i dont think so %s";
                    break;
                case 5:
                    msg = "yes";
                    break;
                case 6:
                    msg = "no";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 3:
            {
                uint32 rnd = urand(0, 8);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "dunno %s";
                    break;
                case 1:
                    msg = "beats me %s";
                    break;
                case 2:
                    msg = "how should i know %s";
                    break;
                case 3:
                    msg = "dont ask me %s, im just a bot";
                    break;
                case 4:
                    msg = "your asking the wrong person";
                    break;
                case 5:
                    msg = "what do i look like, a psychic?";
                    break;
                case 6:
                    msg = "sure %s";
                    break;
                case 7:
                    msg = "i dont think so %s";
                    break;
                case 8:
                    msg = "maybe";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            }
        }
        }
    }
    else if (!found)
    {
        switch (verb_type)
        {
        case 1:
        {
            uint32 rnd = urand(0, 2);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "yeah %s, the key word being " + word[verb_pos] + " " + word[verb_pos + 1];
                break;
            case 1:
                msg = "ya %s but thats in the past";
                break;
            case 2:
                msg = word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " will " + word[verb_pos + 1] + " again though %s";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 2:
        {
            uint32 rnd = urand(0, 2);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "%s, what do you mean " + word[verb_pos + 1] + "?";
                break;
            case 1:
                msg = "%s, what is a " + word[verb_pos + 1] + "?";
                break;
            case 2:
                msg = "yeah i know " + word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " is a " + word[verb_pos + 1];
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 3:
        {
            uint32 rnd = urand(0, 1);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "are you sure thats going to happen %s?";
                break;
            case 1:
                msg = "%s, what will happen %s?";
                break;
            case 2:
                msg = "are you saying " + word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " will " + word[verb_pos + 1] + " " + word[verb_pos + 2] + " %s?";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        }
    }

    if (!found)
    {
        // Name Responds
        if (incomingMessage.find(bot->GetName()) != std::string::npos)
        {
            replyType = REPLY_NAME;
            found = true;
        }
        else  // Does not understand
        {
            replyType = REPLY_NOT_UNDERSTAND;
            found = true;
        }
    }

    // load text if needed
    if (respondsText.empty())
    {
        respondsText = BOT_TEXT2(replyType, name);
    }

    if (respondsText.size() > 255)
    {
        respondsText.resize(255);
    }

    return respondsText;
}
