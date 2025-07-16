/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */ 
#include "FishingAction.h"
#include "Event.h"
#include "Playerbots.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"


extern const uint32 FISHING_SPELL = 7620;

WorldPosition FishingAction::FindWater(Player* bot, float distance, float increment)
{
    float x = bot->GetPositionX();
    float y = bot->GetPositionY();
    float z = bot->GetPositionZ();
    uint32 mapId = bot->GetMapId();
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
};

bool FishingAction::Execute(Event event)
{
    if (qualifier == "travel")
    {
        TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

        if (target->getStatus() != TravelStatus::TRAVEL_STATUS_TRAVEL)
            return false;
    }
    WorldPosition FishSpot = FindWater(bot, 5.0f, 0.2f);
    if (FishSpot)
    {
        bot->SetFacingTo(FishSpot);
    }

    else
    {
   //     botAI->RpgTaxiAction(FindWater(bot, 100.0f));
        bot->SetFacingTo(FindWater(bot, 100.0f, 5.0f));
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

    botAI->CastSpell(FISHING_SPELL, bot);
   
    return true;
};


bool FishingAction::isUseful()
{
    if (!AI_VALUE(bool, "can fish"))  // verify spell and fishing pole. Adds pole to Rndbot if missing.
        return false;
};

bool UseBobber::Execute(Event event)
{
    if (!bot)
        return false;
    if (!botAI)
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
            {
                continue;
            }

            if (go->getLootState() != GO_READY)
            {
                time_t bobberActiveTime = go->GetRespawnTime() - FISHING_BOBBER_READY_TIME;
                if (bobberActiveTime > time(0))
                    botAI->SetNextCheckDelay((bobberActiveTime - time(0)) * IN_MILLISECONDS + 500);
                else
                    botAI->SetNextCheckDelay(1000);
                return false;
            }
            
            if (!bot->GetSession())
            return false;

            WorldPacket* packet = new WorldPacket(CMSG_GAMEOBJ_USE);
            *packet << guid;
            bot->GetSession()->SendPacket(packet);


        }
    }

    return false;
};
