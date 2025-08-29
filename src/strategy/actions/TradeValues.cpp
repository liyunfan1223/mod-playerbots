/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TradeValues.h"

#include "ItemUsageValue.h"
#include "Playerbots.h"

std::vector<Item*> ItemsUsefulToGiveValue::Calculate()
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    Player* player = guidP.GetPlayer();

    std::vector<Item*> giveItems;

    if (botAI->HasActivePlayerMaster() || !GET_PLAYERBOT_AI(player))
        return giveItems;

    std::vector<ItemUsage> myUsages = {ITEM_USAGE_NONE, ITEM_USAGE_VENDOR, ITEM_USAGE_AH, ITEM_USAGE_DISENCHANT};

    for (auto& myUsage : myUsages)
    {
        std::vector<Item*> myItems =
            AI_VALUE2(std::vector<Item*>, "inventory items", "usage " + std::to_string(myUsage));
        std::reverse(myItems.begin(), myItems.end());

        for (auto& item : myItems)
        {
            if (!item->CanBeTraded())
                continue;

            if (bot->GetTradeData() && bot->GetTradeData()->HasItem(item->GetGUID()))
                continue;

            ItemUsage otherUsage = PAI_VALUE2(ItemUsage, "item usage", item->GetEntry());

            if (std::find(myUsages.begin(), myUsages.end(), otherUsage) == myUsages.end())
                giveItems.push_back(item);
        }
    }

    return giveItems;
}
