/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */ 
#include "FishingAction.h"
#include "Event.h"

#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ItemPackets.h"
#include "LastMovementValue.h"
#include "Map.h"
#include "MovementActions.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "Position.h"

const uint32 FISHING_SPELL = 7620;
const uint32 FISHING_POLE = 6256;
const uint32 FISHING_BOBBER = 35591;
const float HEIGHT_ABOVE_WATER_TOLERANCE = 0.2f;

static bool isFishingPole(const Item* item)
{
    if (!item)
        return false;
    const ItemTemplate* proto = item->GetTemplate();
    return proto && proto->Class == ITEM_CLASS_WEAPON && 
        proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE;
}

bool hasWaterOrLand(float x, float y, float z,  Map* map, uint32 phaseMask, bool checkSolidSurface=false) 
{
    if (!map)
        return false;

    float ground = INVALID_HEIGHT;
    LiquidData const& liq = map->GetLiquidData(phaseMask, x, y, z, DEFAULT_COLLISION_HEIGHT, MAP_ALL_LIQUIDS);
    
    if (liq.Entry != 0 && (abs(liq.Level-z) > HEIGHT_ABOVE_WATER_TOLERANCE))
        ground = map->GetHeight(phaseMask, x, y, z, true); 

    if (!checkSolidSurface)
    {
        return (liq.Entry != 0 && liq.Level > liq.DepthLevel && (ground == INVALID_HEIGHT ||liq.Level + HEIGHT_ABOVE_WATER_TOLERANCE > ground));
    }

    if (ground != INVALID_HEIGHT)
        ground = map->GetHeight(phaseMask, x, y, z, true); // useVmaps = true
    
    return (ground != INVALID_HEIGHT && (liq.Level < ground));
}

WorldPosition findWaterLinear(Player* bot, float startDistance, float endDistance, float increment, float orientation, bool findLand, WorldPosition targetPos) 
{
    Map* map = bot->GetMap();
    uint32 mapId = bot->GetMapId();
    uint32 phaseMask = bot->GetPhaseMask();

    float x = bot->GetPositionX();
    float y = bot->GetPositionY();
    float z = bot->GetPositionZ();
    float dist = startDistance;

    if (targetPos)
    {
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
        bool inLOS = true;
        
        if (findLand)
        {
            inLOS = map->isInLineOfSight(checkX, checkY, z, targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(), phaseMask, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing);
        }
        if (isWater && inLOS)
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
    GameObject* nearestFishingHole = nullptr;
    float minDist = std::numeric_limits<float>::max();
    for (const auto& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;
        if (go->GetGoType() == GAMEOBJECT_TYPE_FISHINGHOLE)
        {
            float dist = player->GetDistance2d(go);
            if (dist < minDist)
            {
                minDist = dist;
                nearestFishingHole = go;
            }
        }
    }
    if (nearestFishingHole)
    {
        return WorldPosition(nearestFishingHole->GetMapId(), nearestFishingHole->GetPositionX(), nearestFishingHole->GetPositionY(), nearestFishingHole->GetPositionZ());
    }
    return WorldPosition();
}

bool MoveNearWaterAction::Execute(Event event)
{
    if(landSpot.GetPositionX() != 0.0f && landSpot.GetPositionY() != 0.0f)
    {   
        return MoveTo(landSpot.GetMapId(), landSpot.GetPositionX(), landSpot.GetPositionY(), landSpot.GetPositionZ());
    }
    return false;
}

bool MoveNearWaterAction::isUseful()
{
    if (fishingPosition.GetPositionX() == 0.0f && fishingPosition.GetPositionY() == 0.0f)
    {
        LOG_ERROR("Playerbots", "No Fishing area defined");
        return false;
    }
    Position pos = fishingPosition;
    float distance = bot->GetExactDist2d(&pos);
    bool hasLOS = bot->IsWithinLOS(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());


    if (distance > 18.0f || distance < 12.0F || !hasLOS)
    {
        LOG_ERROR("Playerbots","Fishing area is outside of range. Is useful to move ");
        return true;
    }
    
    // Distance is correct, no movement is needed.
    return false;
}

bool MoveNearWaterAction::isPossible()
{
    if (fishingPosition.GetPositionX() == 0.0f && fishingPosition.GetPositionY() == 0.0f)
        return false;
    
    orientation = bot->GetAngle(fishingPosition.GetPositionX(), fishingPosition.GetPositionY());
    Player* master = botAI->GetMaster();
          
    for (float orientationOffset {0.0, -+0.26, -0.26})
    {
        float testOrientation = orientation + orientationOffset;
        landSpot = findWaterLinear(bot, 10.0f,20.0f, 1.0f, testOrientation, true, fishingPosition);
        if (landSpot.GetPositionX() != 0.0f && landSpot.GetPositionY() != 0.0f)
        {
            LOG_ERROR("Playerbots", "Found land spot");
            
            if (master)
            {
                float distance = master ->GetExactDist2d(&landSpot);
                LOG_ERROR("Playerbots","Have master and spot is {}",distance);
                return (distance < 5.0f);
            }
            LOG_ERROR("Playerbots","No master and spot found.");
            return true;
        }
    LOG_ERROR("Playerbots","No land spot found.");
    return false;
}

bool EquipFishingPoleAction::Execute(Event event)
{
    if (!pole)
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
    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (isFishingPole(mainHand))
    {
        return false;
    }
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            if (isFishingPole(item))
            {
                pole = item;
                return true;
            }
        }
    }
        
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
                        return true;
                    }
                }
            }
        }
    }
    
    
    if (sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        bot->StoreNewItemInBestSlots(FISHING_POLE, 1);  // Try to get a fishing pole
        return true;
    }

    std::string text = sPlayerbotTextMgr->GetBotTextOrDefault(
    "no_fishing_pole_error", "I don't Have a Fishing Pole",{});
    std::string master = botAI->GetMaster()->GetName();
    botAI->Whisper(text, master);
    return false;
}

bool FishingAction::Execute(Event event)
{
    bool facingWater = false;
    WorldPosition fishingHole = findFishingHole(botAI);
    if (fishingHole.GetPositionX() != 0.0f && fishingHole.GetPositionY() != 0.0f)
    {
        Position pos = fishingHole; 
        float distance = bot->GetExactDist2d(&pos);
        LOG_ERROR("Playerbots","Have found fishing hole at {}", distance);
        bool hasLOS = bot->IsWithinLOS(fishingHole.GetPositionX(), fishingHole.GetPositionY(), fishingHole.GetPositionZ());
        
        if (distance > 18.0f || distance < 12.0f || !hasLOS)
        {
            LOG_ERROR("Playerbots","Need to move to fishing hole");
            MoveNearWaterAction moveToFishingHole(botAI, fishingHole);
            if (moveToFishingHole.isUseful())
            {
                LOG_ERROR("Playerbots","Moving to fishing hole isuseful");
                if(moveToFishingHole.isPossible())
                {
                    LOG_ERROR("Playerbots","Moving to fishing hole ispossible"); //Seperate if statement since this call is more expensive.
                    return moveToFishingHole.Execute(event);
                }
            }
        } 
        else
        {
            LOG_ERROR("Playerbots","Facing fishing hole");
            if (bot->HasInArc(1.0, &pos, 1.0) && hasLOS)
                facingWater = true;
            else
            {
                float angle = bot->GetAngle(fishingHole.GetPositionX(), fishingHole.GetPositionY());
                bot->SetOrientation(angle);
                bot->SendMovementFlagUpdate();
                return false;
            }
        }
    }
    if (!facingWater)
    {
        LOG_ERROR("Playerbots","Need to find water");
        WorldPosition FishSpot = findWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMap(), bot->GetPhaseMask(), 10.0f, 20.0f, 5.0f, false);
        if (FishSpot.GetPositionX() != 0.0f && FishSpot.GetPositionY() != 0.0f)
        {
            Position pos = FishSpot; 
            bool hasLOS = bot->IsWithinLOS(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
            if (!hasLOS)
            {
                LOG_ERROR("Playerbots","Bot does not have LOS to water pos x {}, y {}, and z {}", pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
                
                WorldPosition moveToFishSpot = findWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMap(), bot->GetPhaseMask(), 15.0f, 30.0f, 4.0f, false);
                MoveNearWaterAction moveToFishingSpot(botAI, moveToFishSpot);
                if (moveToFishingSpot.isUseful())
                {
                    if(moveToFishingSpot.isPossible()) //Seperate if statement since this call is more expensive.
                        return moveToFishingSpot.Execute(event);
                    
                    return false;
                }
            }
            else if (bot->HasInArc(1.0, &pos, 5.0))
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
        else
            return false;
    }

    EquipFishingPoleAction equipAction(botAI);
    if (equipAction.isUseful())
        return equipAction.Execute(event);

    botAI->CastSpell(FISHING_SPELL, bot);
    botAI->ChangeStrategy("+use bobber", BOT_STATE_NON_COMBAT);
   
    return true;
}

bool FishingAction::isUseful()
{
    if (!AI_VALUE(bool, "can fish"))
        return false;

    float angle = bot->GetOrientation();
    float dist = 15.0f;
    float x = bot->GetPositionX() + cos(angle) * dist;
    float y = bot->GetPositionY() + sin(angle) * dist;
    float z = bot->GetPositionZ();

    Map* map = bot->GetMap();

    if (!hasWaterOrLand(x, y, z, map, bot->GetPhaseMask()))
        return false;

    if (!bot->IsWithinLOS(x, y, liq.Level))
        return false;
    
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