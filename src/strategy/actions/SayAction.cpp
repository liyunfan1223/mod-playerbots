/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SayAction.h"
#include "Event.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "ChannelMgr.h"
#include "GuildMgr.h"
#include <regex>

SayAction::SayAction(PlayerbotAI* botAI) : Action(botAI, "say"), Qualified()
{
}

bool SayAction::Execute(Event event)
{
    std::string text = "";
    std::map<std::string, std::string> placeholders;
    Unit* target = AI_VALUE(Unit*, "tank target");
    if (!target)
        target = AI_VALUE(Unit*, "current target");

    // set replace strings
    if (target) placeholders["<target>"] = target->GetName();
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
        if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(bot->GetAreaId()))
            placeholders["<subzone>"] = area->area_name[0];
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
            if (memberAi) members.push_back(member);
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
        for (auto & member : members)
        {
            PlayerbotAI* memberAi = GET_PLAYERBOT_AI(member);
            if (memberAi)
                memberAi->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime + (20 * ++index) + urand(1, 15));
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

    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    return (time(nullptr) - lastSaid) > 30;
}

void ChatReplyAction::ChatReplyDo(Player* bot, uint32 type, uint32 guid1, uint32 guid2, std::string msg, std::string chanName, std::string name)
{
    ChatReplyType replyType = REPLY_NOT_UNDERSTAND; // default not understand
    std::string respondsText = "";

    // Chat Logic
    int32 verb_pos = -1;
    int32 verb_type = -1;
    int32 is_quest = 0;
    bool found = false;
    std::stringstream text(msg);
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

    if (msg.find("?") != std::string::npos)
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
//        // blame gm with chat tag
//        if (Player* plr = sObjectMgr->GetPlayer(ObjectGuid(HIGHGUID_PLAYER, guid1)))
//        {
//            if (plr->isGMChat())
//            {
//                replyType = REPLY_ADMIN_ABUSE;
//                found = true;
//                break;
//            }
//        }
//
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
                verb_type = 2; // present
            }
            else if (word[i] == "will")
            {
                verb_pos = i;
                verb_type = 3; // future
            }
            else if (word[i] == "was" || word[i] == "were")
            {
                verb_pos = i;
                verb_type = 1; // past
            }
            else if (word[i] == "shut" || word[i] == "noob")
            {
                if (msg.find(bot->GetName()) == std::string::npos)
                {
                    continue; // not react
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
                        msg = word[verb_pos - 1] + " will " + word[verb_pos + 1] + " again though %s";
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
                        msg = "yeah i know " + word[verb_pos - 1] + " is a " + word[verb_pos + 1];
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
                        msg = "are you saying " + word[verb_pos - 1] + " will " + word[verb_pos + 1] + " " + word[verb_pos + 2] + " %s?";
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
        if (msg.find(bot->GetName()) != std::string::npos)
        {
            replyType = REPLY_NAME;
            found = true;
        }
        else // Does not understand
        {
            replyType = REPLY_NOT_UNDERSTAND;
            found = true;
        }
    }

    // send responds
    //
    if (found)
    {
        // load text if needed
        if (respondsText.empty())
        {
            respondsText = BOT_TEXT2(replyType, name);
        }
        const char* c = respondsText.c_str();
        if (strlen(c) > 255)
            return;

        if (chanName == "World")
        {
            if (ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId()))
            {
                std::string worldChan = "World";
                if (Channel* chn = cMgr->GetJoinChannel(worldChan.c_str(), 0)) {
                    if (bot->GetTeamId() == TEAM_ALLIANCE)
                        chn->Say(bot->GetGUID(), c, LANG_COMMON);
                    else
                        chn->Say(bot->GetGUID(), c, LANG_ORCISH);
                }
            }
        }
        else
        {
            if (type == CHAT_MSG_WHISPER)
            {
                ObjectGuid receiver = sCharacterCache->GetCharacterGuidByName(name);
                if (!receiver || !receiver.IsPlayer() || !ObjectAccessor::FindPlayer(receiver))
                {
                    return;
                }
                if (bot->GetTeamId() == TEAM_ALLIANCE)
                {
                    bot->Whisper(c, LANG_COMMON, ObjectAccessor::FindPlayer(receiver));
                }
                else
                {
                    bot->Whisper(c, LANG_ORCISH, ObjectAccessor::FindPlayer(receiver));
                }
            }

            if (type == CHAT_MSG_SAY)
            {
                if (bot->GetTeamId() == TEAM_ALLIANCE)
                    bot->Say(respondsText, LANG_COMMON);
                else
                    bot->Say(respondsText, LANG_ORCISH);
            }

            if (type == CHAT_MSG_YELL)
            {
                if (bot->GetTeamId() == TEAM_ALLIANCE)
                    bot->Yell(respondsText, LANG_COMMON);
                else
                    bot->Yell(respondsText, LANG_ORCISH);
            }

            if (type == CHAT_MSG_GUILD)
            {
                if (!bot->GetGuildId())
                    return;

                Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
                if (!guild)
                    return;

                guild->BroadcastToGuild(bot->GetSession(), false, respondsText, LANG_UNIVERSAL);
            }
        }
        GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(nullptr) + urand(5, 25));
    }
}