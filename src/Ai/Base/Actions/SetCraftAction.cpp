/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SetCraftAction.h"
#include "ChatHelper.h"
#include "CraftValue.h"
#include "Event.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

#include <array>

// Mining is here for its Smelting window.
static constexpr std::array<uint32, 11> CRAFTING_SKILLS = {
    SKILL_ALCHEMY,     SKILL_BLACKSMITHING, SKILL_COOKING,        SKILL_ENCHANTING, SKILL_ENGINEERING, SKILL_FIRST_AID,
    SKILL_INSCRIPTION, SKILL_JEWELCRAFTING, SKILL_LEATHERWORKING, SKILL_MINING,     SKILL_TAILORING};

std::map<uint32, SkillLineAbilityEntry const*> SetCraftAction::skillSpells;

// Tells the bot's master through the usual channel, anyone else by whisper.
static void Tell(PlayerbotAI* botAI, Player* player, std::string const& text)
{
    if (player == botAI->GetMaster())
        botAI->TellMaster(text);
    else
        botAI->GetBot()->Whisper(text, LANG_UNIVERSAL, player);
}

bool SetCraftAction::Execute(Event event)
{
    Player* master = GetMaster();
    Player* requester = event.getOwner() ? event.getOwner() : master;
    if (!requester)
        return false;

    std::string const link = event.getParam();
    if (link.empty())
    {
        TellProfessions(requester);
        return true;
    }

    if (!master)
        return false;

    CraftData& data = AI_VALUE(CraftData&, "craft");
    if (link == "reset")
    {
        data.Reset();
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "craft_reset", "I will not craft anything", {}));
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
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "craft_usage", "Usage: 'craft', 'craft [item link]', 'craft ?' or 'craft reset'", {}));
        return false;
    }

    uint32 itemId = *itemIds.begin();
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return false;

    if (skillSpells.empty())
    {
        for (SkillLineAbilityEntry const* skillLine : sSkillLineAbilityStore)
            skillSpells[skillLine->Spell] = skillLine;
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

        SkillLineAbilityEntry const* skillLine = skillSpells[spellId];
        if (skillLine != nullptr)
        {
            for (uint8 i = 0; i < 3; ++i)
            {
                if (spellInfo->Effects[i].Effect == SPELL_EFFECT_CREATE_ITEM &&
                    itemId == spellInfo->Effects[i].ItemType)
                {
                    for (uint32 x = 0; x < MAX_SPELL_REAGENTS; ++x)
                    {
                        if (spellInfo->Reagent[x] <= 0)
                            continue;

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
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "craft_cannot_craft", "I cannot craft this", {}));
        return false;
    }

    data.itemId = itemId;

    TellCraft();
    return true;
}

void SetCraftAction::TellProfessions(Player* requester)
{
    bool any = false;
    for (uint32 skill : CRAFTING_SKILLS)
    {
        std::string const link = ChatHelper::FormatProfessionLink(bot, skill);
        if (link.empty())
            continue;

        std::ostringstream out;
        out << link << " " << bot->GetSkillValue(skill) << "/" << bot->GetMaxSkillValue(skill);
        Tell(botAI, requester, out.str());
        any = true;
    }

    if (!any)
    {
        Tell(botAI, requester,
             PlayerbotTextMgr::instance().GetBotTextOrDefault("craft_no_professions",
                                                              "I don't know any crafting professions", {}));
    }
}

void SetCraftAction::TellCraft()
{
    CraftData& data = AI_VALUE(CraftData&, "craft");
    if (data.IsEmpty())
    {
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "craft_reset", "I will not craft anything", {}));
        return;
    }

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(data.itemId);
    if (!proto)
        return;

    std::ostringstream reagentsOut;

    bool first = true;
    for (std::map<uint32, uint32>::iterator i = data.required.begin(); i != data.required.end(); ++i)
    {
        uint32 item = i->first;
        uint32 required = i->second;

        if (ItemTemplate const* reagent = sObjectMgr->GetItemTemplate(item))
        {
            if (first)
                first = false;
            else
                reagentsOut << ", ";

            reagentsOut << chat->FormatItem(reagent, required);

            uint32 given = data.obtained[item];
            if (given)
                reagentsOut << "|cffffff00(x" << given << " given)|r ";
        }
    }

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "craft_summary",
        "I will craft %item using reagents: %reagents (craft fee: %money)",
        {{"%item", chat->FormatItem(proto)},
         {"%reagents", reagentsOut.str()},
         {"%money", chat->formatMoney(GetCraftFee(data))}}));
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
