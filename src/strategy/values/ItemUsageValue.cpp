/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ItemUsageValue.h"

#include "AiFactory.h"
#include "ChatHelper.h"
#include "GuildTaskMgr.h"
#include "Item.h"
#include "LootObjectStack.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "RandomItemMgr.h"
#include "ServerFacade.h"
#include "StatsWeightCalculator.h"

ItemUsage ItemUsageValue::Calculate()
{
    uint32 itemId = 0;
    uint32 randomPropertyId = 0;
    size_t pos = qualifier.find(",");
    if (pos != std::string::npos) {
        itemId = atoi(qualifier.substr(0, pos).c_str());
        randomPropertyId = atoi(qualifier.substr(pos + 1).c_str());
    } else {
        itemId = atoi(qualifier.c_str());
    }

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

            if (proto->Class == ITEM_CLASS_TRADE_GOODS || proto->Class == ITEM_CLASS_MISC ||
                proto->Class == ITEM_CLASS_REAGENT)
                needItem = IsItemNeededForUsefullSpell(proto, lowBagSpace);
            else if (proto->Class == ITEM_CLASS_RECIPE)
            {
                if (bot->HasSpell(proto->Spells[2].SpellId))
                    needItem = false;
                else
                    needItem = bot->BotCanUseItem(proto) == EQUIP_ERR_OK;
            }
        }

        if (needItem)
        {
            float stacks = CurrentStacks(proto);
            if (stacks < 1)
                return ITEM_USAGE_SKILL;  // Buy more.
            if (stacks < 2)
                return ITEM_USAGE_KEEP;  // Keep current amount.
        }
    }

    if (proto->Class == ITEM_CLASS_KEY)
        return ITEM_USAGE_USE;

    if (proto->Class == ITEM_CLASS_CONSUMABLE &&
        (proto->MaxCount == 0 || AI_VALUE2(uint32, "item count", proto->Name1) < proto->MaxCount))
    {
        std::string const foodType = GetConsumableType(proto, bot->GetPower(POWER_MANA));

        if (!foodType.empty() && bot->CanUseItem(proto) == EQUIP_ERR_OK)
        {
            float stacks = BetterStacks(proto, foodType);
            if (stacks < 2)
            {
                stacks += CurrentStacks(proto);

                if (stacks < 2)
                    return ITEM_USAGE_USE;  // Buy some to get to 2 stacks
                else if (stacks < 3)        // Keep the item if less than 3 stacks
                    return ITEM_USAGE_KEEP;
            }
        }
    }

    if (bot->GetGuildId() && sGuildTaskMgr->IsGuildTaskItem(itemId, bot->GetGuildId()))
        return ITEM_USAGE_GUILD_TASK;

    ItemUsage equip = QueryItemUsageForEquip(proto, randomPropertyId);
    if (equip != ITEM_USAGE_NONE)
        return equip;

    // Get item instance to check if it's soulbound
    Item* item = bot->GetItemByEntry(proto->ItemId);
    bool isSoulbound = item && item->IsSoulBound();

    if ((proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON) &&
        botAI->HasSkill(SKILL_ENCHANTING) &&
        proto->Quality >= ITEM_QUALITY_UNCOMMON)
    {
        // Retrieve the bot's Enchanting skill level
        uint32 enchantingSkill = bot->GetSkillValue(SKILL_ENCHANTING);

        // Check if the bot has a high enough skill to disenchant this item
        if (proto->RequiredDisenchantSkill > 0 && enchantingSkill < proto->RequiredDisenchantSkill)
            return ITEM_USAGE_NONE; // Not skilled enough to disenchant

        // BoE (Bind on Equip) items should NOT be disenchanted unless they are already bound
        if (proto->Bonding == BIND_WHEN_PICKED_UP || (proto->Bonding == BIND_WHEN_EQUIPPED && isSoulbound))
            return ITEM_USAGE_DISENCHANT;
    }

    Player* master = botAI->GetMaster();
    bool isSelfBot = (master == bot);
    bool botNeedsItemForQuest = IsItemUsefulForQuest(bot, proto);
    bool masterNeedsItemForQuest = master && sPlayerbotAIConfig->syncQuestWithPlayer && IsItemUsefulForQuest(master, proto);

    // Identify the source of loot
    LootObject lootObject = AI_VALUE(LootObject, "loot target");

    // Get GUID of loot source
    ObjectGuid lootGuid = lootObject.guid;

    // Check if loot source is an item
    bool isLootFromItem = lootGuid.IsItem();

    // If the loot is from an item in the bot’s bags, ignore syncQuestWithPlayer
    if (isLootFromItem && botNeedsItemForQuest)
    {
        return ITEM_USAGE_QUEST;
    }

    // If the bot is NOT acting alone and the master needs this quest item, defer to the master
    if (!isSelfBot && masterNeedsItemForQuest)
    {
        return ITEM_USAGE_NONE;
    }

    // If the bot itself needs the item for a quest, allow looting
    if (botNeedsItemForQuest)
    {
        return ITEM_USAGE_QUEST;
    }

    if (proto->Class == ITEM_CLASS_PROJECTILE && bot->CanUseItem(proto) == EQUIP_ERR_OK)
    {
        if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_ROGUE || bot->getClass() == CLASS_WARRIOR)
        {
            Item* rangedWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
            uint32 requiredSubClass = 0;

            if (rangedWeapon)
            {
                switch (rangedWeapon->GetTemplate()->SubClass)
                {
                    case ITEM_SUBCLASS_WEAPON_GUN:
                        requiredSubClass = ITEM_SUBCLASS_BULLET;
                        break;
                    case ITEM_SUBCLASS_WEAPON_BOW:
                    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                        requiredSubClass = ITEM_SUBCLASS_ARROW;
                        break;
                }
            }

            // Ensure the item is the correct ammo type for the equipped ranged weapon
            if (proto->SubClass == requiredSubClass)
            {
                float ammoCount = BetterStacks(proto, "ammo");
                float requiredAmmo = (bot->getClass() == CLASS_HUNTER) ? 8 : 2; // Hunters get 8 stacks, others 2
                uint32 currentAmmoId = bot->GetUInt32Value(PLAYER_AMMO_ID);

                // Check if the bot has an ammo type assigned
                if (currentAmmoId == 0)
                {
                    return ITEM_USAGE_EQUIP;  // Equip the ammo if no ammo
                }
                // Compare new ammo vs current equipped ammo
                ItemTemplate const* currentAmmoProto = sObjectMgr->GetItemTemplate(currentAmmoId);
                if (currentAmmoProto)
                {
                    uint32 currentAmmoDPS = (currentAmmoProto->Damage[0].DamageMin + currentAmmoProto->Damage[0].DamageMax) * 1000 / 2;
                    uint32 newAmmoDPS = (proto->Damage[0].DamageMin + proto->Damage[0].DamageMax) * 1000 / 2;

                    if (newAmmoDPS > currentAmmoDPS) // New ammo meets upgrade condition
                    {
                        return ITEM_USAGE_EQUIP;
                    }
                    if (newAmmoDPS < currentAmmoDPS) // New ammo is worse
                    {
                        return ITEM_USAGE_NONE;
                    }
                }
                // Ensure we have enough ammo in the inventory
                if (ammoCount < requiredAmmo)
                {
                    ammoCount += CurrentStacks(proto);

                    if (ammoCount < requiredAmmo)  // Buy ammo to reach the proper supply
                        return ITEM_USAGE_AMMO;
                    else if (ammoCount < requiredAmmo + 1)
                        return ITEM_USAGE_KEEP;  // Keep the ammo if we don't have too much.
                }
            }
        }
    }

    // Need to add something like free bagspace or item value.
    if (proto->SellPrice > 0)
    {
        if (proto->Quality >= ITEM_QUALITY_NORMAL && !isSoulbound)
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

ItemUsage ItemUsageValue::QueryItemUsageForEquip(ItemTemplate const* itemProto, int32 randomPropertyId)
{
    if (bot->BotCanUseItem(itemProto) != EQUIP_ERR_OK)
        return ITEM_USAGE_NONE;

    if (itemProto->InventoryType == INVTYPE_NON_EQUIP)
        return ITEM_USAGE_NONE;

    Item* pItem = Item::CreateItem(itemProto->ItemId, 1, bot, false, 0, true);
    if (!pItem)
        return ITEM_USAGE_NONE;

    uint16 dest;
    InventoryResult result = botAI->CanEquipItem(NULL_SLOT, dest, pItem, true, true);
    pItem->RemoveFromUpdateQueueOf(bot);
    delete pItem;

    if (result != EQUIP_ERR_OK && result != EQUIP_ERR_CANT_CARRY_MORE_OF_THIS)
    {
        return ITEM_USAGE_NONE;
    }
    // Check is unique items are equipped or not
    bool needToCheckUnique = false;
    if (result == EQUIP_ERR_CANT_CARRY_MORE_OF_THIS)
    {
        needToCheckUnique = true;
    }
    else if (itemProto->Flags & ITEM_FLAG_UNIQUE_EQUIPPABLE)
    {
        needToCheckUnique = true;
    }

    if (needToCheckUnique)
    {
        // Count the total number of the item (equipped + in bags)
        uint32 totalItemCount = bot->GetItemCount(itemProto->ItemId, true);

        // Count the number of the item in bags only
        uint32 bagItemCount = bot->GetItemCount(itemProto->ItemId, false);

        // Determine if the unique item is already equipped
        bool isEquipped = (totalItemCount > bagItemCount);

        if (isEquipped)
        {
            return ITEM_USAGE_NONE;  // Item is already equipped
        }
        // If not equipped, continue processing
    }

    if (itemProto->Class == ITEM_CLASS_QUIVER)
        if (bot->getClass() != CLASS_HUNTER)
            return ITEM_USAGE_NONE;

    if (itemProto->Class == ITEM_CLASS_CONTAINER)
    {
        if (itemProto->SubClass != ITEM_SUBCLASS_CONTAINER)
            return ITEM_USAGE_NONE;  // Todo add logic for non-bag containers. We want to look at professions/class and
                                     // only replace if non-bag is larger than bag.

        if (GetSmallestBagSize() >= itemProto->ContainerSlots)
            return ITEM_USAGE_NONE;

        return ITEM_USAGE_EQUIP;
    }

    bool shouldEquip = false;
    // uint32 statWeight = sRandomItemMgr->GetLiveStatWeight(bot, itemProto->ItemId);
    StatsWeightCalculator calculator(bot);
    calculator.SetItemSetBonus(false);
    calculator.SetOverflowPenalty(false);

    float itemScore = calculator.CalculateItem(itemProto->ItemId, randomPropertyId);

    if (itemScore)
        shouldEquip = true;

    if (itemProto->Class == ITEM_CLASS_WEAPON && !sRandomItemMgr->CanEquipWeapon(bot->getClass(), itemProto))
        shouldEquip = false;
    if (itemProto->Class == ITEM_CLASS_ARMOR &&
        !sRandomItemMgr->CanEquipArmor(bot->getClass(), bot->GetLevel(), itemProto))
        shouldEquip = false;

    uint8 possibleSlots = 1;
    uint8 dstSlot = botAI->FindEquipSlot(itemProto, NULL_SLOT, true);
    // Check if dest wasn't set correctly by CanEquipItem and use FindEquipSlot instead
    // This occurs with unique items that are already in the bots bags when CanEquipItem is called
    if (dest == 0)
    {
        if (dstSlot != NULL_SLOT)
        {
            // Construct dest from dstSlot
            dest = (INVENTORY_SLOT_BAG_0 << 8) | dstSlot;
        }
    }

    if (dstSlot == EQUIPMENT_SLOT_FINGER1 || dstSlot == EQUIPMENT_SLOT_TRINKET1)
    {
        possibleSlots = 2;
    }

    // Check weapon case separately to keep things a bit cleaner
    bool have2HWeapon = false;
    bool isValidTGWeapon = false;

    if (dstSlot == EQUIPMENT_SLOT_MAINHAND)
    {
        Item* currentWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        have2HWeapon = currentWeapon && currentWeapon->GetTemplate()->InventoryType == INVTYPE_2HWEAPON;

        // Determine if the new weapon is a valid Titan Grip weapon
        isValidTGWeapon = (itemProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2 ||
                           itemProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE2 ||
                           itemProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD2);

        // If the bot can Titan Grip, ignore any 2H weapon that isn't a 2H sword, mace, or axe.
        if (bot->CanTitanGrip())
        {
            // If this weapon is 2H but not one of the valid TG weapon types, do not equip it at all.
            if (itemProto->InventoryType == INVTYPE_2HWEAPON && !isValidTGWeapon)
            {
                return ITEM_USAGE_NONE;
            }
        }

        // Now handle the logic for equipping and possible offhand slots
        // If the bot can Dual Wield and:
        // - The weapon is not 2H and we currently don't have a 2H weapon equipped
        // OR
        // - The bot can Titan Grip and it is a valid TG weapon
        // Then we can consider the offhand slot as well.
        if (bot->CanDualWield() &&
            ((itemProto->InventoryType != INVTYPE_2HWEAPON && !have2HWeapon) ||
             (bot->CanTitanGrip() && isValidTGWeapon)))
        {
            possibleSlots = 2;
        }
    }


    for (uint8 i = 0; i < possibleSlots; i++)
    {
        bool shouldEquipInSlot = shouldEquip;
        Item* oldItem = bot->GetItemByPos(dest + i);

        // No item equipped
        if (!oldItem)
        {
            if (shouldEquipInSlot)
                return ITEM_USAGE_EQUIP;
            else
            {
                return ITEM_USAGE_BAD_EQUIP;
            }
        }

        ItemTemplate const* oldItemProto = oldItem->GetTemplate();
        float oldScore = calculator.CalculateItem(oldItemProto->ItemId, oldItem->GetInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID));
        if (oldItem)
        {
            // uint32 oldStatWeight = sRandomItemMgr->GetLiveStatWeight(bot, oldItemProto->ItemId);
            if (itemScore || oldScore)
            {
                shouldEquipInSlot = itemScore > oldScore * sPlayerbotAIConfig->equipUpgradeThreshold;
            }
        }

        // Bigger quiver
        if (itemProto->Class == ITEM_CLASS_QUIVER)
        {
            if (!oldItem || oldItemProto->ContainerSlots < itemProto->ContainerSlots)
            {
                return ITEM_USAGE_EQUIP;
            }
            else
            {
                return ITEM_USAGE_NONE;
            }
        }

        bool existingShouldEquip = true;
        if (oldItemProto->Class == ITEM_CLASS_WEAPON && !sRandomItemMgr->CanEquipWeapon(bot->getClass(), oldItemProto))
            existingShouldEquip = false;

        if (oldItemProto->Class == ITEM_CLASS_ARMOR &&
            !sRandomItemMgr->CanEquipArmor(bot->getClass(), bot->GetLevel(), oldItemProto))
            existingShouldEquip = false;

        // uint32 oldItemPower = sRandomItemMgr->GetLiveStatWeight(bot, oldItemProto->ItemId);
        // uint32 newItemPower = sRandomItemMgr->GetLiveStatWeight(bot, itemProto->ItemId);

        // Compare items based on item level, quality or itemId.
        bool isBetter = false;
        if (itemScore > oldScore)
            isBetter = true;
        // else if (newItemPower == oldScore && itemProto->Quality > oldItemProto->Quality)
        //     isBetter = true;
        // else if (newItemPower == oldScore && itemProto->Quality == oldItemProto->Quality && itemProto->ItemId >
        // oldItemProto->ItemId)
        //     isBetter = true;

        Item* item = CurrentItem(itemProto);
        bool itemIsBroken =
            item && item->GetUInt32Value(ITEM_FIELD_DURABILITY) == 0 && item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) > 0;
        bool oldItemIsBroken =
            oldItem->GetUInt32Value(ITEM_FIELD_DURABILITY) == 0 && oldItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) > 0;

        if (itemProto->ItemId != oldItemProto->ItemId && (shouldEquipInSlot || !existingShouldEquip) && isBetter)
        {
            switch (itemProto->Class)
            {
                case ITEM_CLASS_ARMOR:
                    if (oldItemProto->SubClass <= itemProto->SubClass)
                    {
                        // Need to add some logic to check second slot before returning, but as it happens, all three of these
                        // return vals will result in an attempted equip action so it wouldn't have much effect currently
                        if (itemIsBroken && !oldItemIsBroken)
                            return ITEM_USAGE_BROKEN_EQUIP;
                        else if (shouldEquipInSlot)
                            return ITEM_USAGE_REPLACE;
                        else
                            return ITEM_USAGE_BAD_EQUIP;

                        break;
                    }
                default:
                {
                    if (itemIsBroken && !oldItemIsBroken)
                        return ITEM_USAGE_BROKEN_EQUIP;
                    else if (shouldEquipInSlot)
                        return ITEM_USAGE_EQUIP;
                    else
                        return ITEM_USAGE_BAD_EQUIP;
                }
            }
        }

        // Item is not better but current item is broken and new one is not.
        if (oldItemIsBroken && !itemIsBroken)
            return ITEM_USAGE_EQUIP;
    }
    return ITEM_USAGE_NONE;
}

// Return smaltest bag size equipped
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
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    if (!botAI)
        return false;

    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 entry = player->GetQuestSlotQuestId(slot);
        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
        if (!quest)
            continue;

        // Check if the item itself is needed for the quest
        for (uint8 i = 0; i < 4; i++)
        {
            if (quest->RequiredItemId[i] == proto->ItemId)
            {
                if (AI_VALUE2(uint32, "item count", proto->Name1) >= quest->RequiredItemCount[i])
                    continue;

                return true; // Item is directly required for a quest
            }
        }

        // Check if the item has spells that create a required quest item
        for (uint8 i = 0; i < MAX_ITEM_SPELLS; i++)
        {
            uint32 spellId = proto->Spells[i].SpellId;
            if (!spellId)
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            for (uint8 effectIndex = 0; effectIndex < MAX_SPELL_EFFECTS; effectIndex++)
            {
                if (spellInfo->Effects[effectIndex].Effect == SPELL_EFFECT_CREATE_ITEM)
                {
                    uint32 createdItemId = spellInfo->Effects[effectIndex].ItemType;

                    // Check if the created item is required for a quest
                    for (uint8 j = 0; j < 4; j++)
                    {
                        if (quest->RequiredItemId[j] == createdItemId)
                        {
                            if (AI_VALUE2(uint32, "item count", createdItemId) >= quest->RequiredItemCount[j])
                                continue;

                            return true; // Item is useful because it creates a required quest item
                        }
                    }
                }
            }
        }
    }

    return false; // Item is not useful for any active quests
}

bool ItemUsageValue::IsItemNeededForSkill(ItemTemplate const* proto)
{
    switch (proto->ItemId)
    {
        case 756:  // Tunnel Pick
            return botAI->HasSkill(SKILL_MINING);
        case 778:  // Kobold Excavation Pick
            return botAI->HasSkill(SKILL_MINING);
        case 1819:  // Gouging Pick
            return botAI->HasSkill(SKILL_MINING);
        case 1893:  // Miner's Revenge
            return botAI->HasSkill(SKILL_MINING);
        case 1959:  // Cold Iron Pick
            return botAI->HasSkill(SKILL_MINING);
        case 2901:  // Mining Pick
            return botAI->HasSkill(SKILL_MINING);
        case 9465:  // Digmaster 5000
            return botAI->HasSkill(SKILL_MINING);
        case 20723:  // Brann's Trusty Pick
            return botAI->HasSkill(SKILL_MINING);
        case 40772:  // Gnomish Army Knife
            return botAI->HasSkill(SKILL_MINING) || botAI->HasSkill(SKILL_ENGINEERING) || botAI->HasSkill(SKILL_BLACKSMITHING) || botAI->HasSkill(SKILL_COOKING) || botAI->HasSkill(SKILL_SKINNING);
        case 40892:  // Hammer Pick
            return botAI->HasSkill(SKILL_MINING) || botAI->HasSkill(SKILL_BLACKSMITHING);
        case 40893:  // Bladed Pickaxe
            return botAI->HasSkill(SKILL_MINING) || botAI->HasSkill(SKILL_SKINNING);
        case 5956:  // Blacksmith Hammer
            return botAI->HasSkill(SKILL_BLACKSMITHING) || botAI->HasSkill(SKILL_ENGINEERING);
        case 6219:  // Arclight Spanner
            return botAI->HasSkill(SKILL_ENGINEERING);
        case 6218:  // Runed copper rod
            return botAI->HasSkill(SKILL_ENCHANTING);
        case 6339:  // Runed silver rod
            return botAI->HasSkill(SKILL_ENCHANTING);
        case 11130:  // Runed golden rod
            return botAI->HasSkill(SKILL_ENCHANTING);
        case 11145:  // Runed truesilver rod
            return botAI->HasSkill(SKILL_ENCHANTING);
        case 16207:  // Runed Arcanite Rod
            return botAI->HasSkill(SKILL_ENCHANTING);
        case 7005:  // Skinning Knife
            return botAI->HasSkill(SKILL_SKINNING);
        case 12709:
            return botAI->HasSkill(SKILL_SKINNING);
        case 19901:
            return botAI->HasSkill(SKILL_SKINNING);
        case 4471:  // Flint and Tinder
            return botAI->HasSkill(SKILL_COOKING);
        case 4470:  // Simple Wood
            return botAI->HasSkill(SKILL_COOKING);
        case 6256:  // Fishing Rod
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
            if (botAI->HasSkill(SKILL_MINING) && (RandomItemMgr::IsUsedBySkill(proto, SKILL_MINING) ||
                                                  RandomItemMgr::IsUsedBySkill(proto, SKILL_BLACKSMITHING) ||
                                                  RandomItemMgr::IsUsedBySkill(proto, SKILL_JEWELCRAFTING) ||
                                                  RandomItemMgr::IsUsedBySkill(proto, SKILL_ENGINEERING)))
                return true;
            if (botAI->HasSkill(SKILL_SKINNING) && (RandomItemMgr::IsUsedBySkill(proto, SKILL_SKINNING) ||
                                                    RandomItemMgr::IsUsedBySkill(proto, SKILL_LEATHERWORKING)))
                return true;
            if (botAI->HasSkill(SKILL_HERBALISM) && (RandomItemMgr::IsUsedBySkill(proto, SKILL_HERBALISM) ||
                                                     RandomItemMgr::IsUsedBySkill(proto, SKILL_ALCHEMY)))
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

            if (usage != ITEM_USAGE_REPLACE && usage != ITEM_USAGE_EQUIP && usage != ITEM_USAGE_AMMO &&
                usage != ITEM_USAGE_QUEST && usage != ITEM_USAGE_SKILL && usage != ITEM_USAGE_USE)
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
            if (proto && proto->ItemId == spellInfo->Reagent[i] &&
                spellInfo->ReagentCount[i] == 1)  // If we only need 1 item then current item does not need to be
                                                  // checked since we are looting/buying or already have it.
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

            if (SkillGainChance(SkillValue, skill->TrivialSkillLineRankHigh,
                                (skill->TrivialSkillLineRankHigh + skill->TrivialSkillLineRankLow) / 2,
                                skill->TrivialSkillLineRankLow) > 0)
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
