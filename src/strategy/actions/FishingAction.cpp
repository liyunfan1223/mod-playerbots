/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */ 
#include "FishingAction.h"
#include "Event.h"
#include "Playerbots.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "MovementActions.h"
#include "LastMovementValue.h"
#include "PlayerbotAI.h"
#include "ItemPackets.h"
#include "Position.h"


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

bool hasWaterOrLand(float x, float y, float z, Map* map, uint32 phaseMask, bool checkSolidSurface=false) 
{
    if (!map)
        return false;

    LiquidData const& liq = map->GetLiquidData(phaseMask, x, y, z, DEFAULT_COLLISION_HEIGHT, MAP_ALL_LIQUIDS);
    if (!checkSolidSurface)
    {
        return (liq.Entry != 0 && liq.Level > liq.DepthLevel);
    }
    float ground = map->GetHeight(phaseMask, x, y, z, true); // useVmaps = true
    if (ground != INVALID_HEIGHT && (liq.Level < ground))
        return true;
    return false;
}

WorldPosition findWaterLinear(Player* bot, float startDistance, float endDistance, float increment, bool findLand, WorldPosition targetPos) 
{
    float orientation = bot->GetOrientation();
    Map* map = bot->GetMap();
    uint32 mapId = bot->GetMapId();
    uint32 phaseMask = bot->GetPhaseMask();
    float x = bot->GetPositionX();
    float y = bot->GetPositionY();
    float z = bot->GetPositionZ();
    float dist = startDistance;
    if (targetPos)
    {
        orientation = bot->GetAngle(targetPos.GetPositionX(), targetPos.GetPositionY());
        x = targetPos.GetPositionX();
        y = targetPos.GetPositionY();

        increment = -increment;
        dist = -startDistance;
        endDistance = -endDistance;
    }
    while ((increment > 0 && dist <= endDistance) || (increment < 0 && dist >= endDistance)) 
    {
        float checkX = x + dist * cos(orientation);
        float checkY = y + dist * sin(orientation);
        bool isWater = hasWaterOrLand(checkX, checkY, z, map, phaseMask, findLand);
        
        if (isWater)
        {
            return WorldPosition(mapId, checkX, checkY, z);
        }
        
        dist += increment;
    }

    return WorldPosition();
}

WorldPosition findWaterRadial(Player* bot, float x, float y, float z, Map* map, uint32 phaseMask, float minDistance, float maxDistance, float increment, bool findLand) 
{
    const int numDirections = 16;
    std::vector<WorldPosition> boundaryPoints;
    float dist = minDistance;

    while (dist <= maxDistance)
    {
        for (int i = 0; i < numDirections; ++i) 
        {
            float angle = (2.0f * M_PI * i) / numDirections;
            float checkX = x + cos(angle) * dist;
            float checkY = y + sin(angle) * dist;

            bool isWater = hasWaterOrLand(checkX, checkY, z, map, phaseMask, findLand);
            if (isWater)
            {
                boundaryPoints.push_back(WorldPosition(bot->GetMapId(), checkX, checkY, z));
            }
        }
        if (!boundaryPoints.empty())
            break;
        else
            dist += increment;

        if (dist > maxDistance)
            return WorldPosition();
            
    }
    
    if (boundaryPoints.empty())
        return WorldPosition();

    if (boundaryPoints.size() == 1)
        return boundaryPoints[0];   

    size_t midIndex = boundaryPoints.size() / 2;
    return boundaryPoints[midIndex];
}

WorldPosition findFishingHole(PlayerbotAI* botAI) 
{
    Player* player = botAI->GetBot();
    GuidVector gos = PAI_VALUE(GuidVector, "nearest game objects no los"); 
    for (const auto& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;
        if (go->GetGoType() == GAMEOBJECT_TYPE_FISHINGHOLE)
        {
            return WorldPosition(go->GetMapId(), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
        }
    }
    return WorldPosition();
}

bool MoveToFishAction::Execute(Event event)
{
    WorldPosition fishHole =findFishingHole(botAI);
    if (fishHole.GetPositionX() != 0.0f && fishHole.GetPositionY() != 0.0f)
    {
        WorldPosition LandSpot = findWaterLinear(bot, 15.0f,20.0f, 1.0f, true, fishHole);
        if(LandSpot.GetPositionX() != 0.0f && LandSpot.GetPositionY() != 0.0f)
        {
            return MoveTo(LandSpot.GetMapId(), LandSpot.GetPositionX(), LandSpot.GetPositionY(), LandSpot.GetPositionZ());
        }
        // No else, if cannot find a fishing spot to get to fishing hole it defaults to a radial search.
    }
    WorldPosition FishSpot = findWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMap(), bot->GetPhaseMask(), 10.0f, sPlayerbotAIConfig->fishingDistance, 2.5f, false);
    if (FishSpot.GetPositionX() != 0.0f && FishSpot.GetPositionY() != 0.0f)
    {
        WorldPosition LandSpot = findWaterLinear(bot, 10.0f,20.0f, 2.5f, true, FishSpot);
        if(LandSpot.GetPositionX() != 0.0f && LandSpot.GetPositionY() != 0.0f)
            return MoveTo(LandSpot.GetMapId(), LandSpot.GetPositionX(), LandSpot.GetPositionY(), LandSpot.GetPositionZ());
    }
    return false;
}

bool MoveToFishAction::isUseful()
{
    if (!AI_VALUE(bool, "can fish"))  // verify spell and skill.
        return false;
    
    if (AI_VALUE(bool, "is near water")) // verify near water
        return false;
        
    return true;
}

bool EquipFishingPoleAction::Execute(Event event)
{
    if (isFishingPole(bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND)))
            return true;

    // Find a fishing pole in other equipment slots or inventory.
    Item* pole = nullptr;

    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            if (isFishingPole(item))
            {
                pole = item;
                break;
            }
        }
    }
        
    if (!pole)
    {
        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
        {
            if (Bag* pBag = bot->GetBagByPos(bag))
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                {
                    if (Item* item = pBag->GetItemByPos(j))
                    {
                        if (isFishingPole(item))
                        {
                            pole = item;
                            break;
                        }
                    }
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
    
    WorldPacket eqPacket(CMSG_AUTOEQUIP_ITEM_SLOT, 2);
    eqPacket << pole->GetGUID() << uint8(EQUIPMENT_SLOT_MAINHAND);
    WorldPackets::Item::AutoEquipItemSlot nicePacket(std::move(eqPacket));
    nicePacket.Read();
    bot->GetSession()->HandleAutoEquipItemSlotOpcode(nicePacket);

    return true;
}

bool EquipFishingPoleAction::isUseful()
{
    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    return !isFishingPole(mainHand);
}

bool FishingAction::Execute(Event event)
{
    bool facingWater = false;
    WorldPosition fishingHole = findFishingHole(botAI);
    if (fishingHole.GetPositionX() != 0.0f && fishingHole.GetPositionY() != 0.0f)
    {
        Position pos = fishingHole.getPosition(); 
        float distance = bot->GetExactDist2d(&pos);

        if (distance > 20.0f || distance < 10.0f)
        {
            return MoveToFishAction(botAI).Execute(event);
        } 
        if (bot->HasInArc(1.0, &pos, 1.0))
        {
            facingWater = true;
        }
        else
        {
            float angle = bot->GetAngle(fishingHole.GetPositionX(), fishingHole.GetPositionY());
            bot->SetOrientation(angle);
            bot->SendMovementFlagUpdate();
            return false;
        }
    }
    else
    {
        WorldPosition FishSpot = findWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMap(), bot->GetPhaseMask(), 10.0f, 20.0f, 5.0f, false);
        if (FishSpot.GetPositionX() != 0.0f && FishSpot.GetPositionY() != 0.0f)
        {
            Position pos = FishSpot.getPosition(); 
            if (bot->HasInArc(1.0, &pos, 1.0))
            {
                facingWater = true;
            }
            else
            {
                float angle = bot->GetAngle(FishSpot.GetPositionX(), FishSpot.GetPositionY());
                bot->SetOrientation(angle);
                bot->SendMovementFlagUpdate();
                return false;
            }
        }
    }
    EquipFishingPoleAction equipAction(botAI);
    if (equipAction.isUseful())
        return equipAction.Execute(event);

    botAI->CastSpell(FISHING_SPELL, bot);
    botAI->SetNextCheckDelay(500);
    botAI->ChangeStrategy("+use bobber", BOT_STATE_NON_COMBAT);
   
    return true;
}

bool FishingAction::isUseful()
{
    return AI_VALUE(bool, "can fish");
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
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects no los");
    for (const auto& guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
        {
            if (go->GetEntry() != FISHING_BOBBER)
                continue;
            if (go->GetOwnerGUID() != bot->GetGUID())
                continue;
            if (go->getLootState() == GO_READY)
            {
                go->Use(bot);
                botAI->ChangeStrategy("-use bobber", BOT_STATE_NON_COMBAT);
                return true;
            }

        }
    }
    return false;
}

bool EndFishing::Execute(Event event)
{
    botAI->ChangeStrategy("-master fishing", BOT_STATE_NON_COMBAT);
    return true;
}


bool RemoveBobberStrategyAction::Execute(Event event)
{
    botAI->ChangeStrategy("-use bobber", BOT_STATE_NON_COMBAT);
    return true;
}