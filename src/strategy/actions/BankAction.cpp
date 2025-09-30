/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BankAction.h"

#include "Event.h"
#include "ItemCountValue.h"
#include "Playerbots.h"

bool BankAction::Execute(Event event)
{
    std::string const text = event.getParam();

    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* npc = botAI->GetUnit(*i);
        if (!npc || !npc->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER))
            continue;

        return ExecuteBank(text, npc);
    }

    botAI->TellError("Cannot find banker nearby");
    return false;
}

bool BankAction::ExecuteBank(std::string const text, Unit* bank)
{
    if (text.empty() || text == "?")
    {
        ListItems();
        return true;
    }

    bool result = false;
    if (text[0] == '-')
    {
        std::vector<Item*> found = parseItems(text.substr(1), ITERATE_ITEMS_IN_BANK);
        for (std::vector<Item*>::iterator i = found.begin(); i != found.end(); i++)
        {
            Item* item = *i;
            result &= Withdraw(item->GetTemplate()->ItemId);
        }
    }
    else
    {
        std::vector<Item*> found = parseItems(text, ITERATE_ITEMS_IN_BAGS);
        if (found.empty())
            return false;

        for (std::vector<Item*>::iterator i = found.begin(); i != found.end(); i++)
        {
            Item* item = *i;
            if (!item)
                continue;

            result &= Deposit(item);
        }
    }

    return result;
}

bool BankAction::Withdraw(uint32 itemid)
{
    Item* pItem = FindItemInBank(itemid);
    if (!pItem)
        return false;

    ItemPosCountVec dest;
    InventoryResult msg = bot->CanStoreItem(NULL_BAG, NULL_SLOT, dest, pItem, false);
    if (msg != EQUIP_ERR_OK)
    {
        bot->SendEquipError(msg, pItem, nullptr);
        return false;
    }

    bot->RemoveItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
    bot->StoreItem(dest, pItem, true);

    std::ostringstream out;
    out << "got " << chat->FormatItem(pItem->GetTemplate(), pItem->GetCount()) << " from bank";
    botAI->TellMaster(out.str());
    return true;
}

bool BankAction::Deposit(Item* pItem)
{
    std::ostringstream out;

    ItemPosCountVec dest;
    InventoryResult msg = bot->CanBankItem(NULL_BAG, NULL_SLOT, dest, pItem, false);
    if (msg != EQUIP_ERR_OK)
    {
        bot->SendEquipError(msg, pItem, nullptr);
        return false;
    }

    bot->RemoveItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
    bot->BankItem(dest, pItem, true);

    out << "put " << chat->FormatItem(pItem->GetTemplate(), pItem->GetCount()) << " to bank";
    botAI->TellMaster(out.str());
    return true;
}

void BankAction::ListItems()
{
    botAI->TellMaster("=== Bank ===");

    std::map<uint32, uint32> items;
    std::map<uint32, bool> soulbound;
    for (uint32 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem)
            {
                items[pItem->GetTemplate()->ItemId] += pItem->GetCount();
                soulbound[pItem->GetTemplate()->ItemId] = pItem->IsSoulBound();
            }

    for (uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pBag)
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                    if (Item* pItem = pBag->GetItemByPos(j))
                        if (pItem)
                        {
                            items[pItem->GetTemplate()->ItemId] += pItem->GetCount();
                            soulbound[pItem->GetTemplate()->ItemId] = pItem->IsSoulBound();
                        }

    TellItems(items, soulbound);
}

Item* BankAction::FindItemInBank(uint32 ItemId)
{
    for (uint8 slot = BANK_SLOT_ITEM_START; slot < BANK_SLOT_ITEM_END; slot++)
    {
        if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            ItemTemplate const* const pItemProto = pItem->GetTemplate();
            if (!pItemProto)
                continue;

            if (pItemProto->ItemId == ItemId)  // have required item
                return pItem;
        }
    }

    for (uint8 bag = BANK_SLOT_BAG_START; bag < BANK_SLOT_BAG_END; ++bag)
    {
        Bag const* const pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (pBag)
            for (uint8 slot = 0; slot < pBag->GetBagSize(); ++slot)
            {
                Item* const pItem = bot->GetItemByPos(bag, slot);
                if (pItem)
                {
                    ItemTemplate const* const pItemProto = pItem->GetTemplate();
                    if (!pItemProto)
                        continue;

                    if (pItemProto->ItemId == ItemId)
                        return pItem;
                }
            }
    }

    return nullptr;
}
