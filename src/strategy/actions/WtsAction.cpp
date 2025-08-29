/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WtsAction.h"

#include "AiFactory.h"
#include "Event.h"
#include "ItemUsageValue.h"
#include "ItemVisitors.h"
#include "Playerbots.h"

bool WtsAction::Execute(Event event)
{
    Player* owner = event.getOwner();
    if (!owner)
        return false;

    std::ostringstream out;
    std::string const text = event.getParam();

    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    std::string const link = event.getParam();

    ItemIds itemIds = chat->parseItems(link);
    if (itemIds.empty())
        return false;

    for (ItemIds::iterator i = itemIds.begin(); i != itemIds.end(); i++)
    {
        uint32 itemId = *i;
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            continue;

        std::ostringstream out;
        out << itemId;
        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", out.str());
        if (usage == ITEM_USAGE_NONE)
            continue;

        int32 buyPrice = proto->BuyPrice * sRandomPlayerbotMgr->GetBuyMultiplier(bot);
        if (!buyPrice)
            continue;

        if (urand(0, 15) > 2)
            continue;

        std::ostringstream tell;
        tell << "I'll buy " << chat->FormatItem(proto) << " for " << chat->formatMoney(buyPrice);

        // ignore random bot chat filter
        bot->Whisper(tell.str(), LANG_UNIVERSAL, owner);
    }

    return true;
}
