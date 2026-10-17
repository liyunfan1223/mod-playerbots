/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "OpenItemAction.h"
#include "AiObjectContext.h"
#include "ItemTemplate.h"
#include "LootObjectStack.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "WorldPacket.h"

bool OpenItemAction::Execute(Event /*event*/)
{
    bool foundOpenable = false;

    Item* item = botAI->FindOpenableItem();
    if (item)
    {
        uint8 bag = item->GetBagSlot();  // Retrieves the bag slot (0 for main inventory)
        uint8 slot = item->GetSlot();    // Retrieves the actual slot inside the bag

        OpenItem(item, bag, slot);
        foundOpenable = true;
    }

    return foundOpenable;
}

void OpenItemAction::OpenItem(Item* item, uint8 bag, uint8 slot)
{
    ObjectGuid const itemGuid = item->GetGUID();
    std::ostringstream out;
    out << "Opened item: " << item->GetTemplate()->Name1;

    WorldPacket packet(CMSG_OPEN_ITEM);
    packet << bag << slot;
    bot->GetSession()->HandleOpenItemOpcode(packet);

    // Store the item GUID as the loot target
    LootObject lootObject;
    lootObject.guid = itemGuid;
    botAI->GetAiObjectContext()->GetValue<LootObject>("loot target")->Set(lootObject);

    botAI->TellMaster(out.str());
}
