/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */
#include "FishWithMasterAction.h"

#include "Event.h"
#include "Playerbots.h"

static constexpr uint32 FISHING_SPELL = 7620;

bool FishWithMasterAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint8 castCount = 0, castFlags = 0;
    uint32 spellId = 0;
    p >> castCount >> spellId >> castFlags;

    if (spellId != FISHING_SPELL)
        return false;

    auto isFishingPole = [](Item* item) -> bool
    {
        if (!item)
            return false;
        ItemTemplate const* proto = item->GetTemplate();
        return proto && proto->Class == ITEM_CLASS_WEAPON && proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE;
    };

    Item* pole = nullptr;
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END && !pole; ++slot)
    {
        pole = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!isFishingPole(pole))
            pole = nullptr;
    }
    if (!pole)
    {
        for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END && !pole; ++slot)
        {
            Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (isFishingPole(item))
                pole = item;
        }
        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END && !pole; ++bag)
        {
            Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
            if (!pBag)
                continue;
            for (uint32 j = 0; j < pBag->GetBagSize() && !pole; ++j)
            {
                Item* item = pBag->GetItemByPos(j);
                if (isFishingPole(item))
                    pole = item;
            }
        }
    }

    if (!pole)
    {
        botAI->TellError("I don't have a fishing pole");
        return false;
    }

    if (pole->GetSlot() != EQUIPMENT_SLOT_MAINHAND)
    {
        WorldPacket eqPacket(CMSG_AUTOEQUIP_ITEM_SLOT, 2);
        eqPacket << pole->GetGUID() << uint8(EQUIPMENT_SLOT_MAINHAND);
        bot->GetSession()->HandleAutoEquipItemSlotOpcode(eqPacket);
    }

    return botAI->CastSpell(FISHING_SPELL, bot);
}