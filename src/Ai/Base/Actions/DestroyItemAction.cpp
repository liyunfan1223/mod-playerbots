/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DestroyItemAction.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "Playerbots.h"
#include <algorithm>

namespace
{
// Percent of bag slots used at or above which smart destroy keeps going.
constexpr uint8 BAG_SPACE_DESTROY_THRESHOLD = 90;

std::vector<uint32> DistinctItemIds(std::vector<Item*> const& items)
{
    std::vector<uint32> itemIds;
    for (Item* item : items)
    {
        uint32 const itemId = item->GetEntry();
        if (std::find(itemIds.begin(), itemIds.end(), itemId) == itemIds.end())
            itemIds.push_back(itemId);
    }

    return itemIds;
}
}  // namespace

bool DestroyItemAction::Execute(Event event)
{
    std::string const text = event.getParam();
    ItemIds ids = chat->parseItems(text);

    for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        DestroyItem(&visitor);
    }

    return true;
}

void DestroyItemAction::DestroyItem(FindItemVisitor* visitor)
{
    IterateItems(visitor);
    std::vector<Item*> items = visitor->GetResult();
    for (Item* item : items)
    {
        std::ostringstream out;
        out << chat->FormatItem(item->GetTemplate()) << " destroyed";
        botAI->TellMaster(out);

        bot->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
    }
}

bool SmartDestroyItemAction::isUseful() { return !IsRealPlayer(botAI->GetMaster()); }

bool SmartDestroyItemAction::Execute(Event /*event*/)
{
    uint8 bagSpace = AI_VALUE(uint8, "bag space");

    if (bagSpace < BAG_SPACE_DESTROY_THRESHOLD)
        return false;

    // Only destroy grey items when the master is a real player or SelfBot, and the bot is in a real guild.
    if (botAI->HasGameClientMaster() && botAI->IsInRealGuild())
    {
        FindItemsToTradeByQualityVisitor visitor(ITEM_QUALITY_POOR, 5);
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
        DestroyUntilBagSpace(DistinctItemIds(visitor.GetResult()));
        return true;
    }

    std::vector<uint32> bestToDestroy = {ITEM_USAGE_NONE};  // First destroy anything useless.

    if (!AI_VALUE(bool, "can sell") &&
        AI_VALUE(
            bool,
            "should get money"))  // We need money so quest items are less important since they can't directly be sold.
        bestToDestroy.push_back(ITEM_USAGE_QUEST);
    else  // We don't need money so destroy the cheapest stuff.
    {
        bestToDestroy.push_back(ITEM_USAGE_VENDOR);
        bestToDestroy.push_back(ITEM_USAGE_AH);
    }

    // If we still need room
    bestToDestroy.push_back(
        ITEM_USAGE_SKILL);  // Items that might help tradeskill are more important than above but still expenable.
    bestToDestroy.push_back(ITEM_USAGE_USE);  // These are more likely to be usefull 'soon' but still expenable.

    for (auto& usage : bestToDestroy)
    {
        std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", "usage " + std::to_string(usage));
        std::reverse(items.begin(), items.end());

        if (DestroyUntilBagSpace(DistinctItemIds(items)))
            return true;
    }

    return false;
}

bool SmartDestroyItemAction::DestroyUntilBagSpace(std::vector<uint32> const& itemIds)
{
    // Ids, not Item*: one destroy frees every stack of an id, including later entries.
    for (uint32 const itemId : itemIds)
    {
        FindItemByIdVisitor visitor(itemId);
        DestroyItem(&visitor);

        if (AI_VALUE(uint8, "bag space") < BAG_SPACE_DESTROY_THRESHOLD)
            return true;
    }

    return false;
}
