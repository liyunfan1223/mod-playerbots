/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ChatFilter.h"

#include "Group.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "AiFactory.h"

#include <algorithm>
#include <cctype>
#include <string>

static std::string ToLower(const std::string& str)
{
    std::string out = str;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
    return out;
}

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
        std::string msgLower = ToLower(message);

        bool tank = msgLower.find("@tank") == 0;
        if (tank && !botAI->IsTank(bot))
            return "";

        bool dps = msgLower.find("@dps") == 0;
        if (dps && (botAI->IsTank(bot) || botAI->IsHeal(bot)))
            return "";

        bool heal = msgLower.find("@heal") == 0;
        if (heal && !botAI->IsHeal(bot))
            return "";

        bool ranged = msgLower.find("@ranged") == 0;
        if (ranged && !botAI->IsRanged(bot))
            return "";

        bool melee = msgLower.find("@melee") == 0;
        if (melee && botAI->IsRanged(bot))
            return "";

        bool rangeddps = msgLower.find("@rangeddps") == 0;
        if (rangeddps && (!botAI->IsRanged(bot) || botAI->IsTank(bot) || botAI->IsHeal(bot)))
            return "";

        bool meleedps = msgLower.find("@meleedps") == 0;
        if (meleedps && (!botAI->IsMelee(bot) || botAI->IsTank(bot) || botAI->IsHeal(bot)))
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
        std::string msgLower = ToLower(message);
        if (msgLower[0] != '@')
            return message;

        if (msgLower.find("-") != std::string::npos)
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
        std::string msgLower = ToLower(message);

        bool melee = msgLower.find("@melee") == 0;
        bool ranged = msgLower.find("@ranged") == 0;

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
        std::string msgLower = ToLower(message);
        Group* group = bot->GetGroup();
        if (!group)
            return message;

        bool found = false;
        for (std::vector<std::string>::iterator i = rtis.begin(); i != rtis.end(); i++)
        {
            std::string const rti = *i;
            std::string rtiLower = ToLower(rti);
            bool isRti = msgLower.find(rtiLower) == 0;
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
        classNames["@dk"] = CLASS_DEATH_KNIGHT;
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
        std::string msgLower = ToLower(message);

        bool found = false;
        for (std::map<std::string, uint8>::iterator i = classNames.begin(); i != classNames.end(); i++)
        {
            bool isClass = msgLower.find(ToLower(i->first)) == 0;
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
        std::string msgLower = ToLower(message);
        if (msgLower.find("@group") == 0)
        {
            size_t spacePos = message.find(" ");
            if (spacePos == std::string::npos)
                return message;

            std::string pnum = message.substr(6, spacePos - 6);
            std::string actualMessage = message.substr(spacePos + 1);

            std::set<uint32> targets;
            std::istringstream ss(pnum);
            std::string token;

            while (std::getline(ss, token, ','))
            {
                size_t dashPos = token.find("-");
                if (dashPos != std::string::npos)
                {
                    uint32 from = atoi(token.substr(0, dashPos).c_str());
                    uint32 to = atoi(token.substr(dashPos + 1).c_str());
                    if (from > to) std::swap(from, to);
                    for (uint32 i = from; i <= to; ++i)
                        targets.insert(i);
                }
                else
                {
                    uint32 index = atoi(token.c_str());
                    targets.insert(index);
                }
            }

            if (!bot->GetGroup())
                return message;

            uint32 sg = bot->GetSubGroup() + 1;
            if (targets.find(sg) != targets.end())
                return ChatFilter::Filter(actualMessage);
        }

        return message;
    }
};

class SpecChatFilter : public ChatFilter
{
public:
    SpecChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI)
    {
        // Map (class, specTab) to spec+class string
        specTabNames[{CLASS_PALADIN, 0}] = "hpal";
        specTabNames[{CLASS_PALADIN, 1}] = "ppal";
        specTabNames[{CLASS_PALADIN, 2}] = "rpal";
        specTabNames[{CLASS_PRIEST, 0}] = "disc";
        specTabNames[{CLASS_PRIEST, 1}] = "hpr";
        specTabNames[{CLASS_PRIEST, 2}] = "spr";
        specTabNames[{CLASS_MAGE, 0}] = "arc";
        specTabNames[{CLASS_MAGE, 1}] = "frost";
        specTabNames[{CLASS_MAGE, 2}] = "fire";
        specTabNames[{CLASS_WARRIOR, 0}] = "arms";
        specTabNames[{CLASS_WARRIOR, 1}] = "fury";
        specTabNames[{CLASS_WARRIOR, 2}] = "pwar";
        specTabNames[{CLASS_WARLOCK, 0}] = "affl";
        specTabNames[{CLASS_WARLOCK, 1}] = "demo";
        specTabNames[{CLASS_WARLOCK, 2}] = "dest";
        specTabNames[{CLASS_SHAMAN, 0}] = "ele";
        specTabNames[{CLASS_SHAMAN, 1}] = "enh";
        specTabNames[{CLASS_SHAMAN, 2}] = "rsha";
        specTabNames[{CLASS_DRUID, 0}] = "bal";
        // See below for feral druid
        specTabNames[{CLASS_DRUID, 2}] = "rdru";
        specTabNames[{CLASS_HUNTER, 0}] = "bmh";
        specTabNames[{CLASS_HUNTER, 1}] = "mmh";
        specTabNames[{CLASS_HUNTER, 2}] = "svh";
        specTabNames[{CLASS_ROGUE, 0}] = "mut";
        specTabNames[{CLASS_ROGUE, 1}] = "comb";
        specTabNames[{CLASS_ROGUE, 2}] = "sub";
        // See below for blood death knight
        specTabNames[{CLASS_DEATH_KNIGHT, 1}] = "fdk";
        specTabNames[{CLASS_DEATH_KNIGHT, 2}] = "udk";
    }

    std::string const Filter(std::string& message) override
    {
        std::string msgLower = ToLower(message);
        std::string specPrefix;
        std::string rest;
        if (!ParseSpecPrefix(message, specPrefix, rest))
        {
            return message;
        }

        Player* bot = botAI->GetBot();
        if (!MatchesSpec(bot, specPrefix))
        {
            return "";
        }

        std::string result = ChatFilter::Filter(rest);
        return result;
    }

private:
    std::map<std::pair<uint8, int>, std::string> specTabNames;

    bool ParseSpecPrefix(const std::string& message, std::string& specPrefix, std::string& rest)
    {
        std::string msgLower = ToLower(message);
        for (const auto& entry : specTabNames)
        {
            std::string prefix = "@" + entry.second;
            if (msgLower.find(ToLower(prefix)) == 0)
            {
                specPrefix = entry.second;
                size_t spacePos = message.find(' ');
                rest = (spacePos != std::string::npos) ? message.substr(spacePos + 1) : "";
                return true;
            }
        }
        return false;
    }

    bool MatchesSpec(Player* bot, const std::string& specPrefix)
    {
        uint8 cls = bot->getClass();
        int specTab = AiFactory::GetPlayerSpecTab(bot);
        std::string botSpecClass;
        // For druids, specTab==1 is always feral; distinguish bear/cat at runtime by role
        if (cls == CLASS_DRUID && specTab == 1)
        {
            botSpecClass = botAI->IsTank(bot) ? "bear" : "cat";
        }
        // For death knights, specTab==0 is always blood; distinguish tank/dps at runtime by role
        else if (cls == CLASS_DEATH_KNIGHT && specTab == 0)
        {
            botSpecClass = botAI->IsTank(bot) ? "bdkt" : "bdkd";
        }
        else
        {
            auto it = specTabNames.find({cls, specTab});
            if (it != specTabNames.end())
                botSpecClass = it->second;
        }
        return ToLower(botSpecClass) == ToLower(specPrefix);
    }
};

class AuraChatFilter : public ChatFilter
{
public:
    AuraChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) {}

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();
        std::string msgLower = ToLower(message);
        const std::string auraPrefix = "@aura";
        const std::string noAuraPrefix = "@noaura";
        size_t prefixLen = 0;
        bool isNoAura = false;
        if (msgLower.find(auraPrefix) == 0)
        {
            prefixLen = auraPrefix.length();
            isNoAura = false;
        }
        else if (msgLower.find(noAuraPrefix) == 0)
        {
            prefixLen = noAuraPrefix.length();
            isNoAura = true;
        }
        else
        {
            return message;
        }

        // Trim any leading spaces after @aura or @noaura (can use space between prefix and spell ID if desired, but not required)
        std::string auraIdOrName = message.substr(prefixLen);
        auraIdOrName.erase(0, auraIdOrName.find_first_not_of(' '));
        if (auraIdOrName.empty())
        {
            return message;
        }

        uint32 auraId = 0;
        size_t spacePos = auraIdOrName.find(' ');
        std::string idStr = (spacePos != std::string::npos) ? auraIdOrName.substr(0, spacePos) : auraIdOrName;
        std::string rest = (spacePos != std::string::npos) ? auraIdOrName.substr(spacePos + 1) : "";
        if (!idStr.empty())
        {
            bool isNumeric = std::all_of(idStr.begin(), idStr.end(), ::isdigit);
            if (isNumeric)
            {
                auraId = atoi(idStr.c_str());
            }
        }

        if (auraId == 0)
            return message;

        bool hasAura = bot->HasAura(auraId);
        bool match = isNoAura ? !hasAura : hasAura;
        std::string result = match ? ChatFilter::Filter(rest) : "";
        return result;
    }
};

class AggroByChatFilter : public ChatFilter
{
public:
    AggroByChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) {}

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();
        std::string msgLower = ToLower(message);
        const std::string prefix = "@aggroby";
        size_t prefixLen = prefix.length();
        if (msgLower.find(prefix) != 0)
        {
            return message;
        }

        // Trim any leading spaces after @aggroby (can use space between prefix and entry ID/creature name if desired, but not required)
        std::string enemyStr = message.substr(prefixLen);
        enemyStr.erase(0, enemyStr.find_first_not_of(' '));
        if (enemyStr.empty())
        {
            return message;
        }

        // If creature name is more than one word, it must be enclosed in quotes, e.g. @aggroby "Scarlet Commander Mograine" flee
        std::string rest = "";
        std::string enemyName = "";
        bool isName = false;
        uint32 entryId = 0;

        if (enemyStr[0] == '"')
        {
            size_t endQuote = enemyStr.find('"', 1);
            if (endQuote != std::string::npos)
            {
                enemyName = enemyStr.substr(1, endQuote - 1);
                isName = true;
                size_t spacePos = enemyStr.find(' ', endQuote + 1);
                if (spacePos != std::string::npos)
                {
                    rest = enemyStr.substr(spacePos + 1);
                }
                else
                {
                    rest = "";
                }
            }
            else
            {
                enemyName = enemyStr.substr(1);
                isName = true;
                rest = "";
            }
        }
        else
        {
            size_t splitPos = enemyStr.find_first_of(" ");
            std::string idOrName = (splitPos != std::string::npos) ? enemyStr.substr(0, splitPos) : enemyStr;
            if (splitPos != std::string::npos)
            {
                rest = enemyStr.substr(splitPos + 1);
            }
            else
            {
                rest = "";
            }
            if (!idOrName.empty())
            {
                bool isNumeric = std::all_of(idOrName.begin(), idOrName.end(), ::isdigit);
                if (isNumeric)
                {
                    entryId = atoi(idOrName.c_str());
                }
                else
                {
                    enemyName = idOrName;
                    isName = true;
                }
            }
        }

        const float radius = 100.0f;
        GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        bool match = false;
        for (const auto& guid : npcs)
        {
            Creature* c = botAI->GetCreature(guid);
            if (!c)
            {
                continue;
            }

            bool nameMatch = isName && ToLower(c->GetName()) == ToLower(enemyName);
            bool idMatch = (entryId != 0) && c->GetEntry() == entryId;
            if ((nameMatch || idMatch) && c->GetDistance2d(bot) <= radius)
            {
                Unit* victim = c->GetVictim();
                if (victim && victim->GetGUID() == bot->GetGUID())
                {
                    match = true;
                    break;
                }
            }
        }
        std::string result = match ? ChatFilter::Filter(rest) : "";
        return result;
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
    filters.push_back(new SpecChatFilter(botAI));
    filters.push_back(new AuraChatFilter(botAI));
    filters.push_back(new AggroByChatFilter(botAI));
}

CompositeChatFilter::~CompositeChatFilter()
{
    for (std::vector<ChatFilter*>::iterator i = filters.begin(); i != filters.end(); i++)
        delete (*i);
}

std::string const CompositeChatFilter::Filter(std::string& message)
{
    for (auto* filter : filters)
    {
        message = filter->Filter(message);
        if (message.empty())
            break;
    }

    return message;
}
