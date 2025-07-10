/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */ 
#include "FishingAction.h"
#include "Event.h"
#include "Playerbots.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"


static constexpr uint32 FISHING_SPELL = 7620;

WorldPosition FishingAction::FindWater(Player* bot, float distance)
{
    float x = bot->GetPositionX();
    float y = bot->GetPositionY();
    float z = bot->GetPositionZ();
    uint32 mapId = bot->GetMapId();
    for (float checkX = x - distance; checkX <= x + distance; checkX += 5.0f)
    {
        for (float checkY = y - distance; checkY <= y + distance; checkY += 5.0f)
        {
            if (mapId, checkX, checkY, z, DEFAULT_COLLISION_HEIGHT)
                {
                WorldPosition FishSpot = WorldPosition(checkX, checkY, z);   
                return FishSpot;
                }
        }
    }
    return nullptr;
};

bool FishingAction::Execute(Event event)
{
    if (!AI_VALUE(bool, "can fish"))  // verify spell and fishing pole. Adds pole to Rndbot if missing.
        return false;

    if (qualifier == "travel")
    {
        TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

        if (target->getStatus() != TravelStatus::TRAVEL_STATUS_TRAVEL)
            return false;
    }
    WorldPosition FishSpot = FindWater(bot, 5.0f);
    if (FishSpot)
    {
        bot->SetFacingTo(FishSpot);
    }

    else
    {
   //     botAI->RpgTaxiAction(FindWater(bot, 100.0f));
        bot->SetFacingTo(FindWater(bot, 100.0f));
    }

    Item* pole = nullptr;
    if (pole->GetSlot() != EQUIPMENT_SLOT_MAINHAND)
    {
        WorldPacket eqPacket(CMSG_AUTOEQUIP_ITEM_SLOT, 2);
        eqPacket << pole->GetGUID() << uint8(EQUIPMENT_SLOT_MAINHAND);
        bot->GetSession()->HandleAutoEquipItemSlotOpcode(eqPacket);
    }

    return botAI->CastSpell(FISHING_SPELL, bot);
};

bool UseFishingBobberAction::Execute(Event event)
{
    // Find the fishing bobber (fishing node) owned by the bot
    GameObject* bobber = nullptr;
    std::list<GameObject*> objects;
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    for (ObjectGuid const guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
            objects.push_back(go);
    }

    for (auto& obj : objects)
    {
        if (obj->GetEntry() != 35591)
            continue;

        if (obj->GetOwnerGUID() != bot->GetGUID())
            continue;

        if (obj->getLootState() != GO_READY)
        {
            time_t bobberActiveTime = obj->GetRespawnTime() - FISHING_BOBBER_READY_TIME;
            if (bobberActiveTime > time(0))
                botAI->SetNextCheckDelay((bobberActiveTime - time(0)) * IN_MILLISECONDS + 500);
            else
                botAI->SetNextCheckDelay(1000);
            return true;
        }

        std::unique_ptr<WorldPacket> packet(new WorldPacket(CMSG_GAMEOBJ_USE));
        *packet << obj->GetGUID();
        bot->GetSession()->QueuePacket(packet.get());

        return true;
    }

    return false;
};
