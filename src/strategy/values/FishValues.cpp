/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FishValues.h"
#include "PlayerbotAI.h"
#include "RandomPlayerbotMgr.h"
#include "Map.h"

bool CanFishValue::Calculate()
{
    LOG_ERROR("playerbots","Entering CanFishValue::Calculate");

  if (!bot)
  {
      LOG_ERROR( "playerbots","Failed at bot null check");
    return false;
  }

  if (!botAI)
        {
        LOG_ERROR("playerbots","Failed at botAI null check");
        return false;
    }
  uint32 SkillFishing = bot->GetSkillValue(SKILL_FISHING);
    LOG_ERROR("playerbots","passed GetSKillValue at botAI null check");

  if (SkillFishing == 0)
    {
        LOG_ERROR("playerbots","Inside skill check");
        botAI->TellError("I don't know how to fish");
        return false;
    }

  int32 zone_skill = sObjectMgr->GetFishingBaseSkillLevel(bot->GetAreaId());
    if (!zone_skill)
        zone_skill = sObjectMgr->GetFishingBaseSkillLevel(bot->GetZoneId());
    if (SkillFishing < zone_skill)
        {
        botAI->TellError("I don't have enough skill to fish here");
        return false;
        }

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
        if (sRandomPlayerbotMgr->IsRandomBot(bot))
        {
            bot->StoreNewItemInBestSlots(6256, 1); // Try to get a fishing pole
        }
        else
        {
            botAI->TellError("I don't have a fishing pole");
            return false;
        }
    }
    if (!pole)
        return false;
    
   if (!bot->GetSession())
       return false;
   
    if (pole->GetSlot() != EQUIPMENT_SLOT_MAINHAND)
    {
        WorldPacket eqPacket(CMSG_AUTOEQUIP_ITEM_SLOT, 2);
        eqPacket << pole->GetGUID() << uint8(EQUIPMENT_SLOT_MAINHAND);
        bot->GetSession()->HandleAutoEquipItemSlotOpcode(eqPacket);
    }
	return true;
}

bool CanOpenBobberValue::Calculate()
{

   /* Player* bot = AI_VALUE(Player*, "self target");
    for (auto obj : bot->GetGameObjectList())
    {
        if (obj->GetGoType() == GAMEOBJECT_TYPE_FISHING_BOBBER && obj->GetOwnerGUID() == bot->GetGUID())
            return true;
    }
    */
    return false;
    
}
