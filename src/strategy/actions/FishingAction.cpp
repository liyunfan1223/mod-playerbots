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
     LOG_ERROR("playerbots","Entering FishingAction");

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


    return botAI->CastSpell(FISHING_SPELL, bot);
};


bool FishingAction::isUseful()
{
    if (!AI_VALUE(bool, "can fish"))  // verify spell and fishing pole. Adds pole to Rndbot if missing.
    return false;
};
