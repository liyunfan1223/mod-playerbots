/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SetCraftAction.h"

#include "ChatHelper.h"
#include "CraftValue.h"
#include "Event.h"
#include "Playerbots.h"

std::map<uint32, SkillLineAbilityEntry const*> SetCraftAction::skillSpells;

bool SetCraftAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    std::string const link = event.getParam();

    CraftData& data = AI_VALUE(CraftData&, "craft");
    if (link == "reset")
    {
        data.Reset();
        botAI->TellMaster("I will not craft anything");
        return true;
    }

    if (link == "?")
    {
        TellCraft();
        return true;
    }

    ItemIds itemIds = chat->parseItems(link);
    if (itemIds.empty())
    {
        botAI->TellMaster("Usage: 'craft [itemId]' or 'craft reset'");
        return false;
    }

    uint32 itemId = *itemIds.begin();
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return false;

    if (skillSpells.empty())
    {
        for (SkillLineAbilityEntry const* skillLine : sSkillLineAbilityStore)
        {
            skillSpells[skillLine->Spell] = skillLine;
        }
    }

    data.required.clear();
    data.obtained.clear();

    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;

        if (itr->second->State == PLAYERSPELL_REMOVED || !itr->second->Active)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (SkillLineAbilityEntry const* skillLine = skillSpells[spellId])
        {
            for (uint8 i = 0; i < 3; ++i)
            {
                if (spellInfo->Effects[i].Effect == SPELL_EFFECT_CREATE_ITEM &&
                    itemId == spellInfo->Effects[i].ItemType)
                {
                    for (uint32 x = 0; x < MAX_SPELL_REAGENTS; ++x)
                    {
                        if (spellInfo->Reagent[x] <= 0)
                        {
                            continue;
                        }

                        uint32 itemid = spellInfo->Reagent[x];
                        uint32 reagentsRequired = spellInfo->ReagentCount[x];
                        if (itemid)
                        {
                            data.required[itemid] = reagentsRequired;
                            data.obtained[itemid] = 0;
                        }
                    }
                }
            }
        }
    }

    if (data.required.empty())
    {
        botAI->TellMaster("I cannot craft this");
        return false;
    }

    data.itemId = itemId;

    TellCraft();
    return true;
}

void SetCraftAction::TellCraft()
{
    CraftData& data = AI_VALUE(CraftData&, "craft");
    if (data.IsEmpty())
    {
        botAI->TellMaster("I will not craft anything");
        return;
    }

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(data.itemId);
    if (!proto)
        return;

    std::ostringstream out;
    out << "I will craft " << chat->FormatItem(proto) << " using reagents: ";

    bool first = true;
    for (std::map<uint32, uint32>::iterator i = data.required.begin(); i != data.required.end(); ++i)
    {
        uint32 item = i->first;
        uint32 required = i->second;

        if (ItemTemplate const* reagent = sObjectMgr->GetItemTemplate(item))
        {
            if (first)
            {
                first = false;
            }
            else
                out << ", ";

            out << chat->FormatItem(reagent, required);

            uint32 given = data.obtained[item];
            if (given)
            {
                out << "|cffffff00(x" << given << " given)|r ";
            }
        }
    }

    out << " (craft fee: " << chat->formatMoney(GetCraftFee(data)) << ")";
    botAI->TellMaster(out.str());
}

uint32 SetCraftAction::GetCraftFee(CraftData& data)
{
    if (data.IsEmpty())
        return 0;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(data.itemId);
    if (!proto)
        return 0;

    uint32 level = std::max(proto->ItemLevel, proto->RequiredLevel);
    return level * level / 40;
}
