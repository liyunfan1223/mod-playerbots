/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WhoAction.h"

#include "AiFactory.h"
#include "Event.h"
#include "ItemVisitors.h"
#include "Playerbots.h"

#ifndef WIN32
inline int strcmpi(char const* s1, char const* s2)
{
    for (; *s1 && *s2 && (toupper(*s1) == toupper(*s2)); ++s1, ++s2)
    {
    }
    return *s1 - *s2;
}
#endif

bool WhoAction::Execute(Event event)
{
    Player* owner = event.getOwner();
    if (!owner)
        return false;

    std::ostringstream out;

    std::string const text = event.getParam();
    if (!text.empty())
    {
        out << QuerySkill(text);

        if (sRandomPlayerbotMgr->IsRandomBot(bot))
            out << QueryTrade(text);
    }
    else
    {
        out << QuerySpec(text);
    }

    if (!out.str().empty())
    {
        if (AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(bot->GetAreaId()))
        {
            out << ", (|cffb04040" << areaEntry->area_name[0] << "|r)";
        }
    }

    if (botAI->GetMaster())
    {
        if (!out.str().empty())
            out << ", ";

        out << "playing with " << botAI->GetMaster()->GetName();
    }

    std::string const tell = out.str();
    if (tell.empty())
        return false;

    // ignore random bot chat filter
    bot->Whisper(tell, LANG_UNIVERSAL, owner);
    return true;
}

std::string const WhoAction::QueryTrade(std::string const text)
{
    std::ostringstream out;

    std::vector<Item*> items = InventoryAction::parseItems(text);
    for (Item* sell : items)
    {
        int32 sellPrice =
            sell->GetTemplate()->SellPrice * sRandomPlayerbotMgr->GetSellMultiplier(bot) * sell->GetCount();
        if (!sellPrice)
            continue;

        out << "Selling " << chat->FormatItem(sell->GetTemplate(), sell->GetCount()) << " for "
            << chat->formatMoney(sellPrice);
        return out.str();
    }

    return "";
}

std::string const WhoAction::QuerySkill(std::string const text)
{
    std::ostringstream out;
    uint32 skill = chat->parseSkill(text);
    if (!skill || !botAI->HasSkill((SkillType)skill))
        return "";

    std::string const skillName = chat->FormatSkill(skill);
    uint32 spellId = AI_VALUE2(uint32, "spell id", skillName);
    uint16 value = bot->GetSkillValue(skill);
    uint16 maxSkill = bot->GetMaxSkillValue(skill);
    ObjectGuid guid = bot->GetGUID();

    std::string const data = "0";
    out << "|cFFFFFF00|Htrade:" << spellId << ":" << value << ":" << maxSkill << ":" << std::hex << std::uppercase
        << guid.GetRawValue() << std::nouppercase << std::dec << ":" << data << "|h[" << skillName << "]|h|r"
        << " |h|cff00ff00" << value << "|h|cffffffff/"
        << "|h|cff00ff00" << maxSkill << "|h|cffffffff ";

    return out.str();
}

std::string const WhoAction::QuerySpec(std::string const text)
{
    std::ostringstream out;

    uint8 spec = AiFactory::GetPlayerSpecTab(bot);

    out << "|h|cffffffff" << chat->FormatRace(bot->getRace()) << " [" << (bot->getGender() == GENDER_MALE ? "M" : "F")
        << "] " << chat->FormatClass(bot, spec);
    out << " (|h|cff00ff00" << (uint32)bot->GetLevel() << "|h|cffffffff lvl), ";
    out << "|h|cff00ff00" << botAI->GetEquipGearScore(bot/*, false, false*/) << "|h|cffffffff GS (";

    ItemCountByQuality visitor;
    IterateItems(&visitor, ITERATE_ITEMS_IN_EQUIP);

    bool needSlash = false;
    if (visitor.count[ITEM_QUALITY_LEGENDARY])
    {
        out << "|h|cffff00ff" << visitor.count[ITEM_QUALITY_LEGENDARY] << "|h|cffffffff";
        needSlash = true;
    }

    if (visitor.count[ITEM_QUALITY_EPIC])
    {
        out << "|h|cffff00ff" << visitor.count[ITEM_QUALITY_EPIC] << "|h|cffffffff";
        needSlash = true;
    }

    if (visitor.count[ITEM_QUALITY_RARE])
    {
        if (needSlash)
            out << "/";

        out << "|h|cff8080ff" << visitor.count[ITEM_QUALITY_RARE] << "|h|cffffffff";
        needSlash = true;
    }

    if (visitor.count[ITEM_QUALITY_UNCOMMON])
    {
        if (needSlash)
            out << "/";

        out << "|h|cff00ff00" << visitor.count[ITEM_QUALITY_UNCOMMON] << "|h|cffffffff";
        needSlash = true;
    }

    out << ")";

    return out.str();
}
