/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_INVENTORYACTION_H
#define _PLAYERBOT_INVENTORYACTION_H

#include "Action.h"
#include "ItemVisitors.h"

class PlayerbotAI;

struct ItemTemplate;

class InventoryAction : public Action
{
public:
    InventoryAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name) {}

protected:
    void IterateItems(IterateItemsVisitor* visitor, IterateItemsMask mask = ITERATE_ITEMS_IN_BAGS);
    void TellItems(std::map<uint32, uint32> items, std::map<uint32, bool> soulbound);
    void TellItem(ItemTemplate const* proto, uint32 count, bool soulbound);
    std::vector<Item*> parseItems(std::string const text, IterateItemsMask mask = ITERATE_ALL_ITEMS);
    uint32 GetItemCount(FindItemVisitor* visitor, IterateItemsMask mask = ITERATE_ITEMS_IN_BAGS);
    std::string const parseOutfitName(std::string const outfit);
    ItemIds parseOutfitItems(std::string const outfit);
    ItemIds FindOutfitItems(std::string const name);

private:
    void IterateItemsInBags(IterateItemsVisitor* visitor);
    void IterateItemsInEquip(IterateItemsVisitor* visitor);
    void IterateItemsInBank(IterateItemsVisitor* visitor);
};

#endif
