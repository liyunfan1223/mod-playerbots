/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ListSpellsAction.h"

#include "Event.h"
#include "Playerbots.h"

std::map<uint32, SkillLineAbilityEntry const*> ListSpellsAction::skillSpells;
std::set<uint32> ListSpellsAction::vendorItems;

bool CompareSpells(const std::pair<uint32, std::string>& s1, const std::pair<uint32, std::string>& s2)
{
    SpellInfo const* si1 = sSpellMgr->GetSpellInfo(s1.first);
    SpellInfo const* si2 = sSpellMgr->GetSpellInfo(s2.first);
    if (!si1 || !si2)
    {
        LOG_ERROR("playerbots", "SpellInfo missing. {} {}", s1.first, s2.first);
        return false;
    }
    uint32 p1 = si1->SchoolMask * 20000;
    uint32 p2 = si2->SchoolMask * 20000;

    uint32 skill1 = 0, skill2 = 0;
    uint32 skillValue1 = 0, skillValue2 = 0;
    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        if (SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j))
        {
            if (skillLine->Spell == s1.first)
            {
                skill1 = skillLine->SkillLine;
                skillValue1 = skillLine->TrivialSkillLineRankLow;
            }

            if (skillLine->Spell == s2.first)
            {
                skill2 = skillLine->SkillLine;
                skillValue2 = skillLine->TrivialSkillLineRankLow;
            }
        }

        if (skill1 && skill2)
            break;
    }

    p1 += skill1 * 500;
    p2 += skill2 * 500;

    p1 += skillValue1;
    p2 += skillValue2;

    if (p1 == p2)
    {
        return strcmp(si1->SpellName[0], si2->SpellName[0]) > 0;
    }

    return p1 > p2;
}

std::vector<std::pair<uint32, std::string>> ListSpellsAction::GetSpellList(std::string filter)
{
    if (skillSpells.empty())
    {
        for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
        {
            if (SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j))
                skillSpells[skillLine->Spell] = skillLine;
        }
    }

    if (vendorItems.empty())
    {
        QueryResult results = WorldDatabase.Query("SELECT item FROM npc_vendor WHERE maxcount = 0");
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                int32 entry = fields[0].Get<int32>();
                if (entry <= 0)
                    continue;

                vendorItems.insert(entry);
            } while (results->NextRow());
        }
    }

    std::ostringstream posOut;
    std::ostringstream negOut;

    uint32 skill = 0;

    std::vector<std::string> ss = split(filter, ' ');
    if (!ss.empty())
    {
        skill = chat->parseSkill(ss[0]);
        if (skill != SKILL_NONE)
        {
            filter = ss.size() > 1 ? filter = ss[1] : "";
        }

        if (ss[0] == "first" && ss[1] == "aid")
        {
            skill = SKILL_FIRST_AID;
            filter = ss.size() > 2 ? filter = ss[2] : "";
        }
    }

    std::string const ignoreList =
        ",Opening,Closing,Stuck,Remove Insignia,Opening - No Text,Grovel,Duel,Honorless Target,";
    std::string alreadySeenList = ",";

    uint32 minLevel = 0;
    uint32 maxLevel = 0;
    if (filter.find("-") != std::string::npos)
    {
        std::vector<std::string> ff = split(filter, '-');
        minLevel = atoi(ff[0].c_str());
        maxLevel = atoi(ff[1].c_str());
        filter = "";
    }

    bool craftableOnly = false;
    if (filter.find("+") != std::string::npos)
    {
        craftableOnly = true;
        filter.erase(remove(filter.begin(), filter.end(), '+'), filter.end());
    }

    uint32 slot = chat->parseSlot(filter);
    if (slot != EQUIPMENT_SLOT_END)
        filter = "";

    std::vector<std::pair<uint32, std::string>> spells;
    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        if (itr->second->State == PLAYERSPELL_REMOVED || !itr->second->Active)
            continue;

        if (!(itr->second->specMask & bot->GetActiveSpecMask()))
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);
        if (!spellInfo)
            continue;

        if (spellInfo->IsPassive())
            continue;

        SkillLineAbilityEntry const* skillLine = skillSpells[itr->first];
        if (skill != SKILL_NONE && (!skillLine || skillLine->SkillLine != skill))
            continue;

        std::string const comp = spellInfo->SpellName[0];
        if (!(ignoreList.find(comp) == std::string::npos && alreadySeenList.find(comp) == std::string::npos))
            continue;

        if (!filter.empty() && !strstri(spellInfo->SpellName[0], filter.c_str()))
            continue;

        bool first = true;
        int32 craftCount = -1;
        std::ostringstream materials;
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
                if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemid))
                {
                    if (first)
                    {
                        materials << ": ";
                        first = false;
                    }
                    else
                        materials << ", ";

                    materials << chat->FormatItem(proto, reagentsRequired);

                    FindItemByIdVisitor visitor(itemid);
                    uint32 reagentsInInventory = InventoryAction::GetItemCount(&visitor);
                    bool buyable = (vendorItems.find(itemid) != vendorItems.end());
                    if (!buyable)
                    {
                        uint32 craftable = reagentsInInventory / reagentsRequired;
                        if (craftCount < 0 || craftCount > craftable)
                            craftCount = craftable;
                    }

                    if (reagentsInInventory)
                        materials << "|cffffff00(x" << reagentsInInventory << ")|r ";
                    else if (buyable)
                        materials << "|cffffff00(buy)|r ";
                }
            }
        }

        if (craftCount < 0)
            craftCount = 0;

        std::ostringstream out;
        bool filtered = false;
        if (skillLine)
        {
            for (uint8 i = 0; i < 3; ++i)
            {
                if (spellInfo->Effects[i].Effect == SPELL_EFFECT_CREATE_ITEM)
                {
                    if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(spellInfo->Effects[i].ItemType))
                    {
                        if (craftCount)
                            out << "|cffffff00(x" << craftCount << ")|r ";

                        out << chat->FormatItem(proto);

                        if ((minLevel || maxLevel) && (!proto->RequiredLevel || proto->RequiredLevel < minLevel ||
                                                       proto->RequiredLevel > maxLevel))
                        {
                            filtered = true;
                            break;
                        }

                        if (slot != EQUIPMENT_SLOT_END && bot->FindEquipSlot(proto, slot, true) != slot)
                        {
                            filtered = true;
                            break;
                        }
                    }
                }
            }
        }

        if (out.str().empty())
            out << chat->FormatSpell(spellInfo);

        if (filtered)
            continue;

        if (craftableOnly && !craftCount)
            continue;

        out << materials.str();

        if (skillLine && skillLine->SkillLine)
        {
            uint32 GrayLevel = skillLine->TrivialSkillLineRankHigh;
            uint32 GreenLevel = (skillLine->TrivialSkillLineRankHigh + skillLine->MinSkillLineRank) / 2;
            uint32 YellowLevel = skillLine->MinSkillLineRank;
            uint32 SkillValue = bot->GetSkillValue(skillLine->SkillLine);

            out << " - ";
            if (SkillValue >= GrayLevel)
                out << " |cff808080gray";
            else if (SkillValue >= GreenLevel)
                out << " |cff80be80green";
            else if (SkillValue >= YellowLevel)
                out << " |cffffff00yellow";
            else
                out << " |cffff8040orange";

            out << "|r";
        }

        if (out.str().empty())
            continue;

        if (itr->first == 0)
        {
            LOG_ERROR("playerbots", "?! {}", itr->first);
        }
        spells.push_back(std::pair<uint32, std::string>(itr->first, out.str()));
        alreadySeenList += spellInfo->SpellName[0];
        alreadySeenList += ",";
    }

    return spells;
}

bool ListSpellsAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    std::string const filter = event.getParam();

    std::vector<std::pair<uint32, std::string>> spells = GetSpellList(filter);

    botAI->TellMaster("=== Spells ===");

    std::sort(spells.begin(), spells.end(), CompareSpells);

    uint32 count = 0;
    for (std::vector<std::pair<uint32, std::string>>::iterator i = spells.begin(); i != spells.end(); ++i)
    {
        botAI->TellMasterNoFacing(i->second);

        // if (++count >= 50)
        // {
        //     std::ostringstream msg;
        //     msg << (spells.size() - 50) << " more...";
        //     botAI->TellMasterNoFacing(msg.str());
        //     break;
        // }
    }

    return true;
}
