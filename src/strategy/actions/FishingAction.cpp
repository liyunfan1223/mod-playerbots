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

bool HasWater(Player* bot, float x, float y, float z) 
{
    Map* map = bot->GetMap();
    uint32 phaseMask = bot->GetPhaseMask();
    if (!map)
        return false;

    LiquidData const& liq = map->GetLiquidData(phaseMask, x, y, z, DEFAULT_COLLISION_HEIGHT, MAP_ALL_LIQUIDS);
    return (liq.Entry != 0 && liq.Level > liq.DepthLevel);
}
WorldPosition FindWater(Player* bot, float x, float y, float z, uint32 mapId,
                                       float minDistance, float maxDistance, float increment, bool findLand = false) 
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
                
            bool isWater = HasWater(bot, checkX, checkY, z);
            if (findLand ? !isWater : isWater)
            {
                boundaryPoints.push_back(WorldPosition(checkX, checkY, z));
            }
        }
        if (!boundaryPoints.empty())
            break;
        else
            dist += increment;

        if (dist > maxDistance)
            return WorldPosition();
            
    }
    
    size_t midIndex = boundaryPoints.size() / 2;
    
    return boundaryPoints[midIndex];
}


WorldPosition FindFishingSpot(PlayerbotAI* botAI) 
{
    // Check for fishing Nodes first.
    Player* player = botAI->GetBot();
    GuidVector gos = PAI_VALUE(GuidVector, "nearest game objects no los"); 
    for (const auto& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;
        if (go->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE)
            {
                return WorldPosition(go->GetMapId(), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
            }
    }

    // Face the Master direction if facing water. 
    if (Player* master = botAI->GetMaster()) 
    {
        float x = master->GetPositionX();
        float y = master->GetPositionY();
        float z = master->GetPositionZ();
        uint32 mapId = master->GetMapId();
        float orientation = master->GetOrientation();
        for (float dist = 1.0f; dist <= 20.0f; dist += 1.0f) 
        {
            float checkX = x + dist * cos(orientation);
            float checkY = y + dist * sin(orientation);
            bool isWater = HasWater(player, checkX, checkY, z);
            if (isWater)
                return WorldPosition(checkX, checkY, z);
        }
    }
    return FindWater(player, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), 5.0f, 20.0f, 1.0f, false);
}

bool MoveToFishAction::Execute(Event event)
{
    WorldPosition FishSpot = FindWater(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId(), 0.0f, sPlayerbotAIConfig->fishingDistance, 2.5f, false);
    if (FishSpot.GetPositionX() != 0.0f && FishSpot.GetPositionY() != 0.0f)
    {
        WorldPosition LandSpot = FindWater(bot, FishSpot.GetPositionX(), FishSpot.GetPositionY(), FishSpot.GetPositionZ(), FishSpot.GetMapId(), 0.0f,10.0f, 1.0f, true);
        if(LandSpot.GetPositionX() != 0.0f && LandSpot.GetPositionY() != 0.0f)
            return MoveTo(LandSpot.GetMapId(), LandSpot.GetPositionX(), LandSpot.GetPositionY(), LandSpot.GetPositionZ());
    }
    return false;
}

bool MoveToFishAction::isUseful()
{
    if (!AI_VALUE(bool, "can fish"))  // verify spell and skill.
        return false;
    
    WorldPosition nearwater = FindWater(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId(), 5.0f, 20.0f, 2.5f, false);
    if (nearwater.GetPositionX() != 0.0f && nearwater.GetPositionY() != 0.0f)
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

    float orientation = bot->GetOrientation();
    bool facingWater = false;
    float x = bot->GetPositionX();
    float y = bot->GetPositionY();
    float z = bot->GetPositionZ();
    for (float dist = 1.0f; dist <= 20.0f; dist += 1.0f) 
    {
        float checkX = x + dist * cos(orientation);
        float checkY = y + dist * sin(orientation);
        bool isWater = HasWater(bot, checkX, checkY, z);
        if(isWater)
        {
            facingWater = true;
            break;
        }
    }
    if (!facingWater)
    {
        WorldPosition FishSpot = FindFishingSpot(botAI);

        if (FishSpot.GetPositionX() != 0.0f && FishSpot.GetPositionY() != 0.0f)
        {
            float angle = bot->GetAngle(FishSpot.GetPositionX(), FishSpot.GetPositionY());
            bot->SetOrientation(angle);
            bot->SendMovementFlagUpdate();
            return false;
        }
        else
        {
            return MoveToFishAction(botAI).Execute(event);
        }
    }

    EquipFishingPoleAction equipAction(botAI);
    if (equipAction.isUseful())
        return equipAction.Execute(event);

    botAI->CastSpell(FISHING_SPELL, bot);
    botAI->SetNextCheckDelay(500);
    botAI->ChangeStrategy("+usebobber", BOT_STATE_NON_COMBAT);
   
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
    if (!bot || !botAI)
        return false;

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
                botAI->ChangeStrategy("-usebobber", BOT_STATE_NON_COMBAT);
                return true;
            }

        }
    }
    return false;
}

bool EndFishing::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

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