/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "EquipAction.h"

#include "Event.h"
#include "ItemCountValue.h"
#include "ItemUsageValue.h"
#include "Playerbots.h"
#include "StatsWeightCalculator.h"

bool EquipAction::Execute(Event event)
{
    std::string const text = event.getParam();
    ItemIds ids = chat->parseItems(text);
    EquipItems(ids);
    return true;
}

void EquipAction::EquipItems(ItemIds ids)
{
    for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        EquipItem(&visitor);
    }
}

// Return bagslot with smalest bag.
uint8 EquipAction::GetSmallestBagSlot()
{
    int8 curBag = 0;
    uint32 curSlots = 0;
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        const Bag* const pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (pBag)
        {
            if (curBag > 0 && curSlots < pBag->GetBagSize())
                continue;

            curBag = bag;
            curSlots = pBag->GetBagSize();
        }
        else
            return bag;
    }

    return curBag;
}

void EquipAction::EquipItem(FindItemVisitor* visitor)
{
    IterateItems(visitor);
    std::vector<Item*> items = visitor->GetResult();
    if (!items.empty())
        EquipItem(*items.begin());
}

void EquipAction::EquipItem(Item* item)
{
    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    const ItemTemplate* itemProto = item->GetTemplate();
    uint32 itemId = itemProto->ItemId;

    if (itemProto->InventoryType == INVTYPE_AMMO)
    {
        bot->SetAmmo(itemId);
    }
    else
    {
        bool equippedBag = false;
        if (itemProto->Class == ITEM_CLASS_CONTAINER)
        {
            Bag* pBag = (Bag*)&item;
            uint8 newBagSlot = GetSmallestBagSlot();
            if (newBagSlot > 0)
            {
                uint16 src = ((bagIndex << 8) | slot);
                uint16 dst = ((INVENTORY_SLOT_BAG_0 << 8) | newBagSlot);
                bot->SwapItem(src, dst);
                equippedBag = true;
            }
        }

        if (!equippedBag)
        {
            uint8 dstSlot = botAI->FindEquipSlot(itemProto, NULL_SLOT, true);
            bool have2HWeapon = false;
            bool isValidTGWeapon = false;
            if (dstSlot == EQUIPMENT_SLOT_MAINHAND)
            {
                Item* currentWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                have2HWeapon = currentWeapon && currentWeapon->GetTemplate()->InventoryType == INVTYPE_2HWEAPON;
                isValidTGWeapon = itemProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2 ||
                                  itemProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE2 ||
                                  itemProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD2;
            }
            
            if (dstSlot == EQUIPMENT_SLOT_FINGER1 ||
                dstSlot == EQUIPMENT_SLOT_TRINKET1 ||
                (dstSlot == EQUIPMENT_SLOT_MAINHAND && bot->CanDualWield() &&
                    ((itemProto->InventoryType != INVTYPE_2HWEAPON && !have2HWeapon) || (bot->CanTitanGrip() && isValidTGWeapon))))
            {
                Item* const equippedItems[2] = {
                    bot->GetItemByPos(INVENTORY_SLOT_BAG_0, dstSlot),
                    bot->GetItemByPos(INVENTORY_SLOT_BAG_0, dstSlot + 1)
                };

                if (equippedItems[0])
                {
                    if (equippedItems[1])
                    {
                        // Both slots are full - determine worst item to replace
                        StatsWeightCalculator calculator(bot);
                        calculator.SetItemSetBonus(false);
                        calculator.SetOverflowPenalty(false);
                        
                        // float newItemScore = calculator.CalculateItem(itemId);
                        float equippedItemScore[2] = {
                            equippedItemScore[0] = calculator.CalculateItem(equippedItems[0]->GetTemplate()->ItemId),
                            equippedItemScore[1] = calculator.CalculateItem(equippedItems[1]->GetTemplate()->ItemId)
                        };

                        // Second item is worse than first, equip candidate item in second slot
                        if (equippedItemScore[0] > equippedItemScore[1])
                        {
                            dstSlot++;
                        }
                    }
                    else    // No item equipped in slot 2, equip in that slot instead of replacing first item
                    {
                        dstSlot++;
                    }
                }
            }

            WorldPacket packet(CMSG_AUTOEQUIP_ITEM_SLOT, 2);
            ObjectGuid itemguid = item->GetGUID();
            
            packet << itemguid << dstSlot;
            bot->GetSession()->HandleAutoEquipItemSlotOpcode(packet);

            // WorldPacket packet(CMSG_AUTOEQUIP_ITEM, 2);
            // packet << bagIndex << slot;
            // bot->GetSession()->HandleAutoEquipItemOpcode(packet);
        }
    }

    std::ostringstream out;
    out << "equipping " << chat->FormatItem(itemProto);
    botAI->TellMaster(out);
}

bool EquipUpgradesAction::Execute(Event event)
{
    if (!sPlayerbotAIConfig->autoEquipUpgradeLoot && !sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    if (event.GetSource() == "trade status")
    {
        WorldPacket p(event.getPacket());
        p.rpos(0);
        uint32 status;
        p >> status;

        if (status != TRADE_STATUS_TRADE_ACCEPT)
            return false;
    }

    ListItemsVisitor visitor;
    IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);

    ItemIds items;
    for (std::map<uint32, uint32>::iterator i = visitor.items.begin(); i != visitor.items.end(); ++i)
    {
        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", i->first);
        if (usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE || usage == ITEM_USAGE_BAD_EQUIP)
        {
            // LOG_INFO("playerbots", "Bot {} <{}> auto equips item {} ({})", bot->GetGUID().ToString().c_str(),
            // bot->GetName().c_str(), i->first, usage == 1 ? "no item in slot" : usage == 2 ? "replace" : usage == 3 ?
            // "wrong item but empty slot" : "");
            items.insert(i->first);
        }
    }

    EquipItems(items);
    return true;
}

bool EquipUpgradeAction::Execute(Event event)
{
    ListItemsVisitor visitor;
    IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);

    ItemIds items;
    for (std::map<uint32, uint32>::iterator i = visitor.items.begin(); i != visitor.items.end(); ++i)
    {
        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", i->first);
        if (usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE || usage == ITEM_USAGE_BAD_EQUIP)
        {
            items.insert(i->first);
        }
    }
    EquipItems(items);
    return true;
}
