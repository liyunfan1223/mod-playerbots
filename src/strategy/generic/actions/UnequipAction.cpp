/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "UnequipAction.h"

#include "Event.h"
#include "ItemCountValue.h"
#include "Playerbots.h"

std::vector<std::string> split(std::string const s, char delim);

bool UnequipAction::Execute(Event event)
{
    std::string const text = event.getParam();

    ItemIds ids = chat->parseItems(text);
    if (ids.empty())
    {
        std::vector<std::string> names = split(text, ',');
        for (std::vector<std::string>::iterator i = names.begin(); i != names.end(); ++i)
        {
            uint32 slot = chat->parseSlot(*i);
            if (slot != EQUIPMENT_SLOT_END)
            {
                if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                    UnequipItem(pItem);
            }
        }
    }
    else
    {
        for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
        {
            FindItemByIdVisitor visitor(*i);
            UnequipItem(&visitor);
        }
    }

    return true;
}

void UnequipAction::UnequipItem(FindItemVisitor* visitor)
{
    IterateItems(visitor, ITERATE_ALL_ITEMS);
    std::vector<Item*> items = visitor->GetResult();
    if (!items.empty())
        UnequipItem(*items.begin());
}

void UnequipAction::UnequipItem(Item* item)
{
    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 dstBag = NULL_BAG;

    WorldPacket packet(CMSG_AUTOSTORE_BAG_ITEM, 3);
    packet << bagIndex << slot << dstBag;
    bot->GetSession()->HandleAutoStoreBagItemOpcode(packet);

    std::ostringstream out;
    out << chat->FormatItem(item->GetTemplate()) << " unequipped";
    botAI->TellMaster(out);
}
