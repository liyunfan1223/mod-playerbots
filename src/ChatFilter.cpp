/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChatFilter.h"

#include "Group.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

std::string const ChatFilter::Filter(std::string& message)
{
    if (message.find("@") == std::string::npos)
        return message;

    return message.substr(message.find(" ") + 1);
}

class StrategyChatFilter : public ChatFilter
{
public:
    StrategyChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) {}

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();

        bool tank = message.find("@tank") == 0;
        if (tank && !botAI->IsTank(bot))
            return "";

        bool dps = message.find("@dps") == 0;
        if (dps && (botAI->IsTank(bot) || botAI->IsHeal(bot)))
            return "";

        bool heal = message.find("@heal") == 0;
        if (heal && !botAI->IsHeal(bot))
            return "";

        bool ranged = message.find("@ranged") == 0;
        if (ranged && !botAI->IsRanged(bot))
            return "";

        bool melee = message.find("@melee") == 0;
        if (melee && botAI->IsRanged(bot))
            return "";

        if (tank || dps || heal || ranged || melee)
            return ChatFilter::Filter(message);

        return message;
    }
};

class LevelChatFilter : public ChatFilter
{
public:
    LevelChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) {}

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();

        if (message[0] != '@')
            return message;

        if (message.find("-") != std::string::npos)
        {
            uint32 fromLevel = atoi(message.substr(message.find("@") + 1, message.find("-")).c_str());
            uint32 toLevel = atoi(message.substr(message.find("-") + 1, message.find(" ")).c_str());

            if (bot->GetLevel() >= fromLevel && bot->GetLevel() <= toLevel)
                return ChatFilter::Filter(message);

            return message;
        }

        uint32 level = atoi(message.substr(message.find("@") + 1, message.find(" ")).c_str());
        if (bot->GetLevel() == level)
            return ChatFilter::Filter(message);

        return message;
    }
};

class CombatTypeChatFilter : public ChatFilter
{
public:
    CombatTypeChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) {}

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();

        bool melee = message.find("@melee") == 0;
        bool ranged = message.find("@ranged") == 0;

        if (!melee && !ranged)
            return message;

        switch (bot->getClass())
        {
            case CLASS_WARRIOR:
            case CLASS_PALADIN:
            case CLASS_ROGUE:
            case CLASS_DEATH_KNIGHT:
                if (ranged)
                    return "";
                break;
            case CLASS_HUNTER:
            case CLASS_PRIEST:
            case CLASS_MAGE:
            case CLASS_WARLOCK:
                if (melee)
                    return "";
                break;
            case CLASS_DRUID:
                if (ranged && botAI->IsTank(bot))
                    return "";
                if (melee && !botAI->IsTank(bot))
                    return "";
                break;
            case CLASS_SHAMAN:
                if (melee && botAI->IsHeal(bot))
                    return "";
                if (ranged && !botAI->IsHeal(bot))
                    return "";
                break;
        }

        return ChatFilter::Filter(message);
    }
};

class RtiChatFilter : public ChatFilter
{
public:
    RtiChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI)
    {
        rtis.push_back("@star");
        rtis.push_back("@circle");
        rtis.push_back("@diamond");
        rtis.push_back("@triangle");
        rtis.push_back("@moon");
        rtis.push_back("@square");
        rtis.push_back("@cross");
        rtis.push_back("@skull");
    }

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();
        Group* group = bot->GetGroup();
        if (!group)
            return message;

        bool found = false;
        bool isRti = false;
        for (std::vector<std::string>::iterator i = rtis.begin(); i != rtis.end(); i++)
        {
            std::string const rti = *i;

            bool isRti = message.find(rti) == 0;
            if (!isRti)
                continue;

            ObjectGuid rtiTarget = group->GetTargetIcon(RtiTargetValue::GetRtiIndex(rti.substr(1)));
            if (bot->GetGUID() == rtiTarget)
                return ChatFilter::Filter(message);

            Unit* target = *botAI->GetAiObjectContext()->GetValue<Unit*>("current target");
            if (!target)
                return "";

            if (target->GetGUID() != rtiTarget)
                return "";

            found |= isRti;
            if (found)
                break;
        }

        if (found)
            return ChatFilter::Filter(message);

        return message;
    }

private:
    std::vector<std::string> rtis;
};

class ClassChatFilter : public ChatFilter
{
public:
    ClassChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI)
    {
        classNames["@death_knight"] = CLASS_DEATH_KNIGHT;
        classNames["@druid"] = CLASS_DRUID;
        classNames["@hunter"] = CLASS_HUNTER;
        classNames["@mage"] = CLASS_MAGE;
        classNames["@paladin"] = CLASS_PALADIN;
        classNames["@priest"] = CLASS_PRIEST;
        classNames["@rogue"] = CLASS_ROGUE;
        classNames["@shaman"] = CLASS_SHAMAN;
        classNames["@warlock"] = CLASS_WARLOCK;
        classNames["@warrior"] = CLASS_WARRIOR;
    }

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();

        bool found = false;
        bool isClass = false;
        for (std::map<std::string, uint8>::iterator i = classNames.begin(); i != classNames.end(); i++)
        {
            bool isClass = message.find(i->first) == 0;
            if (isClass && bot->getClass() != i->second)
                return "";

            found |= isClass;
            if (found)
                break;
        }

        if (found)
            return ChatFilter::Filter(message);

        return message;
    }

private:
    std::map<std::string, uint8> classNames;
};

class SubGroupChatFilter : public ChatFilter
{
public:
    SubGroupChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) {}

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();

        if (message.find("@group") == 0)
        {
            std::string const pnum = message.substr(6, message.find(" "));
            uint32 from = atoi(pnum.c_str());
            uint32 to = from;
            if (pnum.find("-") != std::string::npos)
            {
                from = atoi(pnum.substr(pnum.find("@") + 1, pnum.find("-")).c_str());
                to = atoi(pnum.substr(pnum.find("-") + 1, pnum.find(" ")).c_str());
            }

            if (!bot->GetGroup())
                return message;

            uint32 sg = bot->GetSubGroup() + 1;
            if (sg >= from && sg <= to)
                return ChatFilter::Filter(message);
        }

        return message;
    }
};

CompositeChatFilter::CompositeChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI)
{
    filters.push_back(new StrategyChatFilter(botAI));
    filters.push_back(new ClassChatFilter(botAI));
    filters.push_back(new RtiChatFilter(botAI));
    filters.push_back(new CombatTypeChatFilter(botAI));
    filters.push_back(new LevelChatFilter(botAI));
    filters.push_back(new SubGroupChatFilter(botAI));
}

CompositeChatFilter::~CompositeChatFilter()
{
    for (std::vector<ChatFilter*>::iterator i = filters.begin(); i != filters.end(); i++)
        delete (*i);
}

std::string const CompositeChatFilter::Filter(std::string& message)
{
    for (uint32 j = 0; j < filters.size(); ++j)
    {
        for (std::vector<ChatFilter*>::iterator i = filters.begin(); i != filters.end(); i++)
        {
            message = (*i)->Filter(message);
            if (message.empty())
                break;
        }
    }

    return message;
}
