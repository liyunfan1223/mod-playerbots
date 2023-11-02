/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ItemUsageValue.h"
#include "AiFactory.h"
#include "ChatHelper.h"
#include "GuildTaskMgr.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "RandomItemMgr.h"
#include "ServerFacade.h"

ItemUsage ItemUsageValue::Calculate()
{
    uint32 itemId = atoi(qualifier.c_str());
    if (!itemId)
        return ITEM_USAGE_NONE;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return ITEM_USAGE_NONE;

    if (botAI->HasActivePlayerMaster())
    {
        if (IsItemUsefulForSkill(proto) || IsItemNeededForSkill(proto))
            return ITEM_USAGE_SKILL;
    }
    else
    {
        bool needItem = false;

        if (IsItemNeededForSkill(proto))
            needItem = true;
        else
        {
            bool lowBagSpace = AI_VALUE(uint8, "bag space") > 50;

            if (proto->Class == ITEM_CLASS_TRADE_GOODS || proto->Class == ITEM_CLASS_MISC || proto->Class == ITEM_CLASS_REAGENT)
                needItem = IsItemNeededForUsefullSpell(proto, lowBagSpace);
            else if (proto->Class == ITEM_CLASS_RECIPE)
            {
                if (bot->HasSpell(proto->Spells[2].SpellId))
                    needItem = false;
                else
                    needItem = bot->CanUseItem(proto) == EQUIP_ERR_OK;
            }
        }

        if (needItem)
        {
            float stacks = CurrentStacks(proto);
            if (stacks < 1)
                return ITEM_USAGE_SKILL; //Buy more.
            if (stacks < 2)
                return ITEM_USAGE_KEEP; //Keep current amount.
        }
    }

    if (proto->Class == ITEM_CLASS_KEY)
        return ITEM_USAGE_USE;

    if (proto->Class == ITEM_CLASS_CONSUMABLE)
    {
        std::string const foodType = GetConsumableType(proto, bot->GetPower(POWER_MANA));

        if (!foodType.empty() && bot->CanUseItem(proto) == EQUIP_ERR_OK)
        {
            float stacks = BetterStacks(proto, foodType);
            if (stacks < 2)
            {
                stacks += CurrentStacks(proto);

                if (stacks < 2)
                    return ITEM_USAGE_USE; // Buy some to get to 2 stacks
                else if (stacks < 3)       // Keep the item if less than 3 stacks
                    return ITEM_USAGE_KEEP;
            }
        }
    }
    
    if (bot->GetGuildId() && sGuildTaskMgr->IsGuildTaskItem(itemId, bot->GetGuildId()))
        return ITEM_USAGE_GUILD_TASK;

    ItemUsage equip = QueryItemUsageForEquip(proto);
    if (equip != ITEM_USAGE_NONE)
        return equip;

    if ((proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON) && proto->Bonding != BIND_WHEN_PICKED_UP &&
            botAI->HasSkill(SKILL_ENCHANTING) && proto->Quality >= ITEM_QUALITY_UNCOMMON)
        return ITEM_USAGE_DISENCHANT;

    //While sync is on, do not loot quest items that are also Useful for master. Master
    if (!botAI->GetMaster() || !sPlayerbotAIConfig->syncQuestWithPlayer || !IsItemUsefulForQuest(botAI->GetMaster(), proto))
        if (IsItemUsefulForQuest(bot, proto))
            return ITEM_USAGE_QUEST;

    if (proto->Class == ITEM_CLASS_PROJECTILE && bot->CanUseItem(proto) == EQUIP_ERR_OK)
        if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_ROGUE || bot->getClass() == CLASS_WARRIOR)
        {
            if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
            {
                uint32 subClass = 0;
                switch (pItem->GetTemplate()->SubClass)
                {
                    case ITEM_SUBCLASS_WEAPON_GUN:
                        subClass = ITEM_SUBCLASS_BULLET;
                        break;
                    case ITEM_SUBCLASS_WEAPON_BOW:
                    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                        subClass = ITEM_SUBCLASS_ARROW;
                        break;
                }

                if (proto->SubClass == subClass)
                {
                    float ammo = BetterStacks(proto, "ammo");
                    float needAmmo = (bot->getClass() == CLASS_HUNTER) ? 8 : 2;

                    if (ammo < needAmmo) //We already have enough of the current ammo.
                    {
                        ammo += CurrentStacks(proto);

                        if (ammo < needAmmo)         //Buy ammo to get to the proper supply
                            return ITEM_USAGE_AMMO;
                        else if (ammo < needAmmo + 1)
                            return ITEM_USAGE_KEEP; // Keep the ammo until we have too much.
                    }
                }
            }
        }

    //Need to add something like free bagspace or item value.
    if (proto->SellPrice > 0)
    {
        if (proto->Quality > ITEM_QUALITY_NORMAL)
        {
            return ITEM_USAGE_AH;
        }
        else
        {
            return ITEM_USAGE_VENDOR;
        }
    }

    return ITEM_USAGE_NONE;
}

ItemUsage ItemUsageValue::QueryItemUsageForEquip(ItemTemplate const* itemProto)
{
    if (bot->CanUseItem(itemProto) != EQUIP_ERR_OK)
        return ITEM_USAGE_NONE;

    if (itemProto->InventoryType == INVTYPE_NON_EQUIP)
        return ITEM_USAGE_NONE;

    Item* pItem = Item::CreateItem(itemProto->ItemId, 1, bot, false, 0, true);
    if (!pItem)
        return ITEM_USAGE_NONE;

    uint16 dest;
    InventoryResult result = bot->CanEquipItem(NULL_SLOT, dest, pItem, true, false);
    pItem->RemoveFromUpdateQueueOf(bot);
    delete pItem;

    if (result != EQUIP_ERR_OK )
        return ITEM_USAGE_NONE;

    if (itemProto->Class == ITEM_CLASS_QUIVER)
        if (bot->getClass() != CLASS_HUNTER)
            return ITEM_USAGE_NONE;

    if (itemProto->Class == ITEM_CLASS_CONTAINER)
    {
        if (itemProto->SubClass != ITEM_SUBCLASS_CONTAINER)
            return ITEM_USAGE_NONE; //Todo add logic for non-bag containers. We want to look at professions/class and only replace if non-bag is larger than bag.

        if (GetSmallestBagSize() >= itemProto->ContainerSlots)
            return ITEM_USAGE_NONE;

        return ITEM_USAGE_EQUIP;
    }

    bool shouldEquip = false;
    // uint32 statWeight = sRandomItemMgr->GetLiveStatWeight(bot, itemProto->ItemId);
    float itemScore = PlayerbotFactory::CalculateItemScore(itemProto->ItemId, bot);
    if (itemScore)
        shouldEquip = true;

    if (itemProto->Class == ITEM_CLASS_WEAPON && !sRandomItemMgr->CanEquipWeapon(bot->getClass(), itemProto))
        shouldEquip = false;
    if (itemProto->Class == ITEM_CLASS_ARMOR && !sRandomItemMgr->CanEquipArmor(bot->getClass(), bot->getLevel(), itemProto))
        shouldEquip = false;

    Item* oldItem = bot->GetItemByPos(dest);

    //No item equiped
    if (!oldItem)
        if (shouldEquip)
            return ITEM_USAGE_EQUIP;
        else
            return ITEM_USAGE_BAD_EQUIP;

    ItemTemplate const* oldItemProto = oldItem->GetTemplate();
    float oldScore = PlayerbotFactory::CalculateItemScore(oldItemProto->ItemId, bot);
    if (oldItem)
    {
        // uint32 oldStatWeight = sRandomItemMgr->GetLiveStatWeight(bot, oldItemProto->ItemId);
        if (itemScore || oldScore)
        {
            shouldEquip = itemScore > oldScore * sPlayerbotAIConfig->equipUpgradeThreshold;
        }
    }

    // Bigger quiver
    if (itemProto->Class == ITEM_CLASS_QUIVER)
        if (!oldItem || oldItemProto->ContainerSlots < itemProto->ContainerSlots)
            return ITEM_USAGE_EQUIP;
        else
            return ITEM_USAGE_NONE;

    bool existingShouldEquip = true;
    if (oldItemProto->Class == ITEM_CLASS_WEAPON && !sRandomItemMgr->CanEquipWeapon(bot->getClass(), oldItemProto))
        existingShouldEquip = false;

    if (oldItemProto->Class == ITEM_CLASS_ARMOR && !sRandomItemMgr->CanEquipArmor(bot->getClass(), bot->getLevel(), oldItemProto))
        existingShouldEquip = false;

    // uint32 oldItemPower = sRandomItemMgr->GetLiveStatWeight(bot, oldItemProto->ItemId);
    // uint32 newItemPower = sRandomItemMgr->GetLiveStatWeight(bot, itemProto->ItemId);

    //Compare items based on item level, quality or itemId.
    bool isBetter = false;
    if (itemScore > oldScore)
        isBetter = true;
    // else if (newItemPower == oldScore && itemProto->Quality > oldItemProto->Quality)
    //     isBetter = true;
    // else if (newItemPower == oldScore && itemProto->Quality == oldItemProto->Quality && itemProto->ItemId > oldItemProto->ItemId)
    //     isBetter = true;

    Item* item = CurrentItem(itemProto);
    bool itemIsBroken = item && item->GetUInt32Value(ITEM_FIELD_DURABILITY) == 0 && item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) > 0;
    bool oldItemIsBroken = oldItem->GetUInt32Value(ITEM_FIELD_DURABILITY) == 0 && oldItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) > 0;

    if (itemProto->ItemId != oldItemProto->ItemId && (shouldEquip || !existingShouldEquip) && isBetter)
    {
        switch (itemProto->Class)
        {
            case ITEM_CLASS_ARMOR:
            if (oldItemProto->SubClass <= itemProto->SubClass)
            {
                if (itemIsBroken && !oldItemIsBroken)
                    return ITEM_USAGE_BROKEN_EQUIP;
                else if (shouldEquip)
                    return ITEM_USAGE_REPLACE;
                else
                    return ITEM_USAGE_BAD_EQUIP;

                break;
            }
            default:
            {
                if (itemIsBroken && !oldItemIsBroken)
                    return ITEM_USAGE_BROKEN_EQUIP;
                else if (shouldEquip)
                    return ITEM_USAGE_EQUIP;
                else
                    return ITEM_USAGE_BAD_EQUIP;
            }
        }
    }

    // Item is not better but current item is broken and new one is not.
    if (oldItemIsBroken && !itemIsBroken)
        return ITEM_USAGE_EQUIP;

    return ITEM_USAGE_NONE;
}

//Return smaltest bag size equipped
uint32 ItemUsageValue::GetSmallestBagSize()
{
    int8 curSlot = 0;
    uint32 curSlots = 0;
    for (uint8 bag = INVENTORY_SLOT_BAG_START + 1; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
        {
            if (curSlot > 0 && curSlots < pBag->GetBagSize())
                continue;

            curSlot = pBag->GetSlot();
            curSlots = pBag->GetBagSize();
        }
        else
            return 0;
    }

    return curSlots;
}

bool ItemUsageValue::IsItemUsefulForQuest(Player* player, ItemTemplate const* proto)
{
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 entry = player->GetQuestSlotQuestId(slot);
        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
        if (!quest)
            continue;

        for (uint8 i = 0; i < 4; i++)
        {
            if (quest->RequiredItemId[i] != proto->ItemId)
                continue;

            if (GET_PLAYERBOT_AI(player) && AI_VALUE2(uint32, "item count", proto->Name1) >= quest->RequiredItemCount[i])
                continue;

            return true;
        }
    }

    return false;
}

bool ItemUsageValue::IsItemNeededForSkill(ItemTemplate const* proto)
{
    switch (proto->ItemId)
    {
        case 2901: //Mining pick
            return botAI->HasSkill(SKILL_MINING);
        case 5956: //Blacksmith Hammer
            return botAI->HasSkill(SKILL_BLACKSMITHING) || botAI->HasSkill(SKILL_ENGINEERING);
        case 6219: //Arclight Spanner
            return botAI->HasSkill(SKILL_ENGINEERING);
        case 6218: // Runed copper rod
            return botAI->HasSkill(SKILL_ENCHANTING);
        case 6339: // Runed silver rod
            return botAI->HasSkill(SKILL_ENCHANTING);
        case 11130: // Runed golden rod
            return botAI->HasSkill(SKILL_ENCHANTING);
        case 11145: // Runed truesilver rod
            return botAI->HasSkill(SKILL_ENCHANTING);
        case 16207: //Runed Arcanite Rod
            return botAI->HasSkill(SKILL_ENCHANTING);
        case 7005: //Skinning Knife
            return botAI->HasSkill(SKILL_SKINNING);
        case 4471: //Flint and Tinder
            return botAI->HasSkill(SKILL_COOKING);
        case 4470: //Simple Wood
            return botAI->HasSkill(SKILL_COOKING);
        case 6256: //Fishing Rod
            return botAI->HasSkill(SKILL_FISHING);
    }

    return false;
}

bool ItemUsageValue::IsItemUsefulForSkill(ItemTemplate const* proto)
{
    switch (proto->Class)
    {
        case ITEM_CLASS_TRADE_GOODS:
        case ITEM_CLASS_MISC:
        case ITEM_CLASS_REAGENT:
        case ITEM_CLASS_GEM:
        {
            if (botAI->HasSkill(SKILL_TAILORING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_TAILORING))
                return true;
            if (botAI->HasSkill(SKILL_LEATHERWORKING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_LEATHERWORKING))
                return true;
            if (botAI->HasSkill(SKILL_ENGINEERING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_ENGINEERING))
                return true;
            if (botAI->HasSkill(SKILL_BLACKSMITHING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_BLACKSMITHING))
                return true;
            if (botAI->HasSkill(SKILL_ALCHEMY) && RandomItemMgr::IsUsedBySkill(proto, SKILL_ALCHEMY))
                return true;
            if (botAI->HasSkill(SKILL_ENCHANTING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_ENCHANTING))
                return true;
            if (botAI->HasSkill(SKILL_FISHING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_FISHING))
                return true;
            if (botAI->HasSkill(SKILL_FIRST_AID) && RandomItemMgr::IsUsedBySkill(proto, SKILL_FIRST_AID))
                return true;
            if (botAI->HasSkill(SKILL_COOKING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_COOKING))
                return true;
            if (botAI->HasSkill(SKILL_JEWELCRAFTING) && RandomItemMgr::IsUsedBySkill(proto, SKILL_JEWELCRAFTING))
                return true;
            if (botAI->HasSkill(SKILL_MINING) && (RandomItemMgr::IsUsedBySkill(proto, SKILL_MINING) || RandomItemMgr::IsUsedBySkill(proto, SKILL_BLACKSMITHING) ||
                RandomItemMgr::IsUsedBySkill(proto, SKILL_JEWELCRAFTING) || RandomItemMgr::IsUsedBySkill(proto, SKILL_ENGINEERING)))
                return true;
            if (botAI->HasSkill(SKILL_SKINNING) && (RandomItemMgr::IsUsedBySkill(proto, SKILL_SKINNING) || RandomItemMgr::IsUsedBySkill(proto, SKILL_LEATHERWORKING)))
                return true;
            if (botAI->HasSkill(SKILL_HERBALISM) && (RandomItemMgr::IsUsedBySkill(proto, SKILL_HERBALISM) || RandomItemMgr::IsUsedBySkill(proto, SKILL_ALCHEMY)))
                return true;

            return false;
        }
        case ITEM_CLASS_RECIPE:
        {
            if (bot->HasSpell(proto->Spells[2].SpellId))
                break;

            switch (proto->SubClass)
            {
                case ITEM_SUBCLASS_LEATHERWORKING_PATTERN:
                    return botAI->HasSkill(SKILL_LEATHERWORKING);
                case ITEM_SUBCLASS_TAILORING_PATTERN:
                    return botAI->HasSkill(SKILL_TAILORING);
                case ITEM_SUBCLASS_ENGINEERING_SCHEMATIC:
                    return botAI->HasSkill(SKILL_ENGINEERING);
                case ITEM_SUBCLASS_BLACKSMITHING:
                    return botAI->HasSkill(SKILL_BLACKSMITHING);
                case ITEM_SUBCLASS_COOKING_RECIPE:
                    return botAI->HasSkill(SKILL_COOKING);
                case ITEM_SUBCLASS_ALCHEMY_RECIPE:
                    return botAI->HasSkill(SKILL_ALCHEMY);
                case ITEM_SUBCLASS_FIRST_AID_MANUAL:
                    return botAI->HasSkill(SKILL_FIRST_AID);
                case ITEM_SUBCLASS_ENCHANTING_FORMULA:
                    return botAI->HasSkill(SKILL_ENCHANTING);
                case ITEM_SUBCLASS_FISHING_MANUAL:
                    return botAI->HasSkill(SKILL_FISHING);
            }
        }
    }

    return false;
}

bool ItemUsageValue::IsItemNeededForUsefullSpell(ItemTemplate const* proto, bool checkAllReagents)
{
    for (auto spellId : SpellsUsingItem(proto->ItemId, bot))
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (checkAllReagents && !HasItemsNeededForSpell(spellId, proto))
            continue;

        if (SpellGivesSkillUp(spellId, bot))
            return true;

        uint32 newItemId = spellInfo->Effects[EFFECT_0].ItemType;
        if (newItemId && newItemId != proto->ItemId)
        {
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", newItemId);

            if (usage != ITEM_USAGE_REPLACE && usage != ITEM_USAGE_EQUIP && usage != ITEM_USAGE_AMMO && usage != ITEM_USAGE_QUEST && usage != ITEM_USAGE_SKILL && usage != ITEM_USAGE_USE)
                continue;

            return true;
        }
    }

    return false;
}

bool ItemUsageValue::HasItemsNeededForSpell(uint32 spellId, ItemTemplate const* proto)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    for (uint8 i = 0; i < MAX_SPELL_REAGENTS; i++)
        if (spellInfo->ReagentCount[i] > 0 && spellInfo->Reagent[i])
        {
            if (proto && proto->ItemId == spellInfo->Reagent[i] && spellInfo->ReagentCount[i] == 1) //If we only need 1 item then current item does not need to be checked since we are looting/buying or already have it.
                continue;

            ItemTemplate const* reqProto = sObjectMgr->GetItemTemplate(spellInfo->Reagent[i]);

            uint32 count = AI_VALUE2(uint32, "item count", reqProto->Name1);

            if (count < spellInfo->ReagentCount[i])
                return false;
        }

    return true;
}

Item* ItemUsageValue::CurrentItem(ItemTemplate const* proto)
{
    Item* bestItem = nullptr;
    std::vector<Item*> found = AI_VALUE2(std::vector<Item*>, "inventory items", chat->FormatItem(proto));
    for (auto item : found)
    {
        if (bestItem && item->GetUInt32Value(ITEM_FIELD_DURABILITY) < bestItem->GetUInt32Value(ITEM_FIELD_DURABILITY))
            continue;

        if (bestItem && item->GetCount() < bestItem->GetCount())
            continue;

        bestItem = item;
    }

    return bestItem;
}

float ItemUsageValue::CurrentStacks(ItemTemplate const* proto)
{
    uint32 maxStack = proto->GetMaxStackSize();

    std::vector<Item*> found = AI_VALUE2(std::vector<Item*>, "inventory items", chat->FormatItem(proto));

    float itemCount = 0;

    for (auto stack : found)
    {
        itemCount += stack->GetCount();
    }

    return itemCount / maxStack;
}

float ItemUsageValue::BetterStacks(ItemTemplate const* proto, std::string const itemType)
{
    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", itemType);

    float stacks = 0;

    for (auto& otherItem : items)
    {
        ItemTemplate const* otherProto = otherItem->GetTemplate();

        if (otherProto->Class != proto->Class || otherProto->SubClass != proto->SubClass)
            continue;

        if (otherProto->ItemLevel < proto->ItemLevel)
            continue;

        if (otherProto->ItemId == proto->ItemId)
            continue;

        stacks += CurrentStacks(otherProto);
    }

    return stacks;
}

std::vector<uint32> ItemUsageValue::SpellsUsingItem(uint32 itemId, Player* bot)
{
    std::vector<uint32> retSpells;

    PlayerSpellMap const& spellMap = bot->GetSpellMap();

    for (auto& spell : spellMap)
    {
        uint32 spellId = spell.first;

        if (spell.second->State == PLAYERSPELL_REMOVED || !spell.second->Active)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (spellInfo->IsPassive())
            continue;

        if (spellInfo->Effects[EFFECT_0].Effect != SPELL_EFFECT_CREATE_ITEM)
            continue;

        for (uint8 i = 0; i < MAX_SPELL_REAGENTS; i++)
            if (spellInfo->ReagentCount[i] > 0 && spellInfo->Reagent[i] == itemId)
                retSpells.push_back(spellId);
    }

    return retSpells;
}

inline int32 SkillGainChance(uint32 SkillValue, uint32 GrayLevel, uint32 GreenLevel, uint32 YellowLevel)
{
    if (SkillValue >= GrayLevel)
        return sWorld->getIntConfig(CONFIG_SKILL_CHANCE_GREY) * 10;

    if (SkillValue >= GreenLevel)
        return sWorld->getIntConfig(CONFIG_SKILL_CHANCE_GREEN) * 10;

    if (SkillValue >= YellowLevel)
        return sWorld->getIntConfig(CONFIG_SKILL_CHANCE_YELLOW) * 10;

    return sWorld->getIntConfig(CONFIG_SKILL_CHANCE_ORANGE) * 10;
}

bool ItemUsageValue::SpellGivesSkillUp(uint32 spellId, Player* bot)
{
    SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellId);

    for (SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        SkillLineAbilityEntry const* skill = _spell_idx->second;
        if (skill->SkillLine)
        {
            uint32 SkillValue = bot->GetPureSkillValue(skill->SkillLine);

            uint32 craft_skill_gain = sWorld->getIntConfig(CONFIG_SKILL_GAIN_CRAFTING);

            if (SkillGainChance(SkillValue, skill->TrivialSkillLineRankHigh, (skill->TrivialSkillLineRankHigh + skill->TrivialSkillLineRankLow) / 2, skill->TrivialSkillLineRankLow) > 0)
                return true;
        }
    }

    return false;
}

std::string const ItemUsageValue::GetConsumableType(ItemTemplate const* proto, bool hasMana)
{
    std::string const foodType = "";

    if ((proto->SubClass == ITEM_SUBCLASS_CONSUMABLE || proto->SubClass == ITEM_SUBCLASS_FOOD))
    {
        if (proto->Spells[0].SpellCategory == 11)
            return "food";
        else if (proto->Spells[0].SpellCategory == 59 && hasMana)
            return "drink";
    }

    if (proto->SubClass == ITEM_SUBCLASS_POTION || proto->SubClass == ITEM_SUBCLASS_FLASK)
    {
        for (int j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
            if (spellInfo)
                for (int i = 0; i < 3; i++)
                {
                    if (spellInfo->Effects[i].Effect == SPELL_EFFECT_ENERGIZE && hasMana)
                        return "mana potion";

                    if (spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL)
                        return "healing potion";
                }
        }
    }

    if (proto->SubClass == ITEM_SUBCLASS_BANDAGE)
    {
        return "bandage";
    }

    return "";
}
