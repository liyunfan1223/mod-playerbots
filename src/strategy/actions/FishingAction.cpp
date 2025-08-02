/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */ 
#include "FishingAction.h"
#include "Event.h"
#include "Playerbots.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "MoveToTravelTargetAction.h"
#include "PlayerbotAI.h"


extern const uint32 FISHING_SPELL = 7620;

WorldPosition FindWater(Player* bot, float x, float y, float z, uint32 mapId, float distance, float increment, bool findLand)
{
    for (float checkX = x - distance; checkX <= x + distance; checkX += increment)
    {
        for (float checkY = y - distance; checkY <= y + distance; checkY += increment)
        {
            if (bot->GetMap()->IsInWater(mapId, checkX, checkY, z, DEFAULT_COLLISION_HEIGHT))
                {
                return WorldPosition(checkX, checkY, z);   
                
                }
        }
    }
    return nullptr;
}

bool MovetoFish::Execute(Event event)
{
    if (qualifier == "travel")
    {
        TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

        if (target->getStatus() != TravelStatus::TRAVEL_STATUS_TRAVEL)
            return false;
    }
    WorldPosition nearwater = FindWater(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId(), 5.0f, 0.2f, false);
    if (nearwater)
        return false;
        
    WorldPosition FishSpot = FindWater(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId(), sPlayerbotAIConfig->fishingDistance, 2.5f, false);
    if (FishSpot)
    {
        WorldPosition LandSpot = FindWater(bot, FishSpot.GetPositionX(), FishSpot.GetPositionY(), FishSpot.GetPositionZ(), FishSpot.GetMapId(), 3.0f, 0.2f, true);
        if(LandSpot)
            return MoveTo(LandSpot.GetMapId(), LandSpot.GetPositionX(), LandSpot.GetPositionY(), LandSpot.GetPositionZ());
    }
    return false;
}
bool MovetoFish::isUseful()
{
    if (!AI_VALUE(bool, "can fish"))  // verify spell and skill.
        return false;
    return true;
}

bool FishingAction::Execute(Event event)
{
    if (qualifier == "travel")
    {
        TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

        if (target->getStatus() != TravelStatus::TRAVEL_STATUS_TRAVEL)
            return false;
    }
    WorldPosition FishSpot = FindWater(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId(), 5.0f, 0.2f, false);

    if (FishSpot)
    {
        bot->SetFacingTo(FishSpot);
        bot->SendMovementFlagUpdate();
    }
    else
    {
        return MovetoFish(botAI).Execute(event);
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
            bot->StoreNewItemInBestSlots(6256, 1);  // Try to get a fishing pole
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

    botAI->SetNextCheckDelay(100);
    botAI->CastSpell(FISHING_SPELL, bot);
    botAI->ChangeStrategy("+usebobber", BOT_STATE_NON_COMBAT);
   
    return true;
}

bool FishingAction::isUseful()
{
    if (!AI_VALUE(bool, "can fish"))  // verify spell and skill.
        return false;
    return true;
}
bool UseBobber::isUseful()
{
    if (!AI_VALUE(bool, "can use fishing bobber"))
        return false;
    return true;
}

bool UseBobber::Execute(Event event)
{
    if (!bot || !botAI)
        return false;
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects no los");
    for (const auto& guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
        {
            if (!go)
                continue;
            if (go->GetEntry() != 35591)
                continue;
            if (go->GetOwnerGUID() != bot->GetGUID())
                continue;
            if (go->getLootState() != GO_READY)
            {
                time_t bobberActiveTime = go->GetRespawnTime() - FISHING_BOBBER_READY_TIME;
                if (bobberActiveTime > time(0))
                    botAI->SetNextCheckDelay((bobberActiveTime - time(0)) * IN_MILLISECONDS + 500);
                else
                    botAI->SetNextCheckDelay(1000);
                return false;
            }
            botAI->ChangeStrategy("-usebobber", BOT_STATE_NON_COMBAT);
            go->Use(bot);
            return true;

        }
    }
    botAI->ChangeStrategy("-usebobber", BOT_STATE_NON_COMBAT);
    return false;
}
bool EndFishing::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    botAI->ChangeStrategy("-masterfishing", BOT_STATE_NON_COMBAT);
    return true;
}