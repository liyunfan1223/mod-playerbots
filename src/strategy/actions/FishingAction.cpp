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
#include "ItemPackets.h"


const uint32 FISHING_SPELL = 7620;
const uint32 FISHING_POLE = 6256;
const uint32 FISHING_BOBBER = 35591;

static bool isFishingPole(const Item* item)
{
    if (!item)
        return false;
    const ItemTemplate* proto = item->GetTemplate();
    return proto && proto->Class == ITEM_CLASS_WEAPON && 
        proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE;
}

WorldPosition FindWater(Player* bot, float x, float y, float z, uint32 mapId, float distance, float increment, bool findLand)
{
    for (float checkX = x - distance; checkX <= x + distance; checkX += increment)
    {
        for (float checkY = y - distance; checkY <= y + distance; checkY += increment)
        {
            bool isWater = bot->GetMap()->IsInWater(mapId, checkX, checkY, z, DEFAULT_COLLISION_HEIGHT);
            if (findLand ? !isWater : isWater)
            {
                return WorldPosition(checkX, checkY, z);
            }
        }
    }
    return nullptr;
}

bool MovetoFishAction::Execute(Event event)
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

bool MovetoFishAction::isUseful()
{
    if (!AI_VALUE(bool, "can fish"))  // verify spell and skill.
    {
        return false;
    }
    return true;
}

bool EquipFishingPoleAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    
    if (Item* mainHandItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
    {
        if (isFishingPole(mainHandItem))
            return true;
    }

    // Find a fishing pole in other equipment slots or inventory.
    Item* pole = nullptr;

    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            if (isFishingPole(item))
            {
                pole = item;
                break;
            }
    }
        
    if (!pole)
    {
        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
        {
            if (Bag* pBag = bot->GetBagByPos(bag))
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                    if (Item* item = pBag->GetItemByPos(j))
                        if (isFishingPole(item))
                        {
                            pole = item;
                            break;
                        }
            }
            if (pole) //Break out if it finds the pole between bags
                break;
        }
    }

    if (!pole)
    {
        if (sRandomPlayerbotMgr->IsRandomBot(bot))
        {
            botAI->SetNextCheckDelay(50);
            bot->StoreNewItemInBestSlots(FISHING_POLE, 1);  // Try to get a fishing pole
            return true;
        }
        else
        {
            botAI->TellError("I don't have a fishing pole");
            return false;
        }
    }
    
    if (!bot->GetSession())
        return false;

    WorldPacket eqPacket(CMSG_AUTOEQUIP_ITEM_SLOT, 2);
    eqPacket << pole->GetGUID() << uint8(EQUIPMENT_SLOT_MAINHAND);
    WorldPackets::Item::AutoEquipItemSlot nicePacket(std::move(eqPacket));
    nicePacket.Read();
    bot->GetSession()->HandleAutoEquipItemSlotOpcode(nicePacket);

    return true;
}

bool EquipFishingPoleAction::isUseful()
{
    if (!bot)
        return false;
    
    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    return !isFishingPole(mainHand);
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
        return MovetoFishAction(botAI).Execute(event);
    }
    
    EquipFishingPoleAction equipAction(botAI);
    if (equipAction.isUseful())
        return equipAction.Execute(event);

    botAI->SetNextCheckDelay(100);
    botAI->CastSpell(FISHING_SPELL, bot);
    botAI->ChangeStrategy("+usebobber", BOT_STATE_NON_COMBAT);
   
    return true;
}

bool FishingAction::isUseful()
{
    if (!AI_VALUE(bool, "can fish"))  // verify spell and skill.
    {
        return false;
    }
    return true;
}

bool UseBobber::isUseful()
{
    if (!AI_VALUE(bool, "can use fishing bobber"))
    {
        return false;
    }
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
            if (go->GetEntry() != FISHING_BOBBER)
                continue;
            if (go->GetOwnerGUID() != bot->GetGUID())
                continue;
            if (go->getLootState() == GO_READY)
            {
                botAI->ChangeStrategy("-usebobber", BOT_STATE_NON_COMBAT);
                go->Use(bot);
                return true;
            }

        }
    }
    return false;
}

bool EndFishing::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }
    botAI->ChangeStrategy("-masterfishing", BOT_STATE_NON_COMBAT);
    return true;
}


bool RemoveBobberStrategyAction::Execute(Event event)
{
  if (!botAI)
    return false;

  botAI->ChangeStrategy("-usebobber", BOT_STATE_NON_COMBAT);
  return true;
}