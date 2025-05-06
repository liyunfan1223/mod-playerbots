/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ItemCountValue.h"

#include "Playerbots.h"

std::vector<Item*> InventoryItemValueBase::Find(std::string const qualifier)
{
    std::vector<Item*> result;

    Player* bot = InventoryAction::botAI->GetBot();

    std::vector<Item*> items = InventoryAction::parseItems(qualifier);
    for (Item* item : items)
        result.push_back(item);

    return result;
}

uint32 ItemCountValue::Calculate()
{
    uint32 count = 0;
    std::vector<Item*> items = Find(qualifier);
    for (Item* item : items)
    {
        count += item->GetCount();
    }

    return count;
}

std::vector<Item*> InventoryItemValue::Calculate() { return Find(qualifier); }
