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
const float MIN_DISTANCE_TO_WATER = 10.0f;
const float MAX_DISTANCE_TO_WATER = 20.0f;
const float MIN_WATER_SEARCH_DISTANCE = 10.0f;
const float MAX_WATER_SEARCH_DISTANCE = 40.0f;
const float HEIGHT_ABOVE_WATER_TOLERANCE = 0.2f;
const float SEARCH_INCREMENT = 2.5f;
const float MAX_MASTER_DISTANCE = 5.0f;
const float HEIGHT_SEARCH_BUFFER = 10.0f;


bool IsValid(const WorldPosition& pos)
{
    return (pos.GetPositionX() != 0.0f || pos.GetPositionY() != 0.0f);
}

static bool isFishingPole(const Item* item)
{
    if (!item)
        return false;
    const ItemTemplate* proto = item->GetTemplate();
    return proto && proto->Class == ITEM_CLASS_WEAPON && 
        proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE;
}

float hasFishableWaterOrLand(float x, float y, float z,  Map* map, uint32 phaseMask, bool checkForLand=false)
{
    if (!map)
        return INVALID_HEIGHT;

    LiquidData const& liq = map->GetLiquidData(phaseMask, x, y, z+HEIGHT_ABOVE_WATER_TOLERANCE, DEFAULT_COLLISION_HEIGHT, MAP_ALL_LIQUIDS);
    float ground = map->GetHeight(phaseMask, x, y, z + HEIGHT_SEARCH_BUFFER, true);

    if (liq.Entry == 0)
    {
        if (checkForLand)
            return ground;
        return INVALID_HEIGHT;
    }
    if (checkForLand)
    {
        if (ground > liq.Level - HEIGHT_ABOVE_WATER_TOLERANCE)
            return ground;
        return INVALID_HEIGHT;
    }
    if (liq.Level + HEIGHT_ABOVE_WATER_TOLERANCE > ground)
    {
        return liq.Level;
    }
    return INVALID_HEIGHT;
}

bool hasLosToWater(Player* bot, float wx, float wy, float waterZ)
{
    return bot->GetMap()->isInLineOfSight(
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        wx, wy, waterZ,
        bot->GetPhaseMask(),
        LINEOFSIGHT_ALL_CHECKS,
        VMAP::ModelIgnoreFlags::Nothing);
}

WorldPosition findLandFromPosition(Player* bot, float startDistance, float endDistance, float increment, float orientation, WorldPosition targetPos, bool checkLOS = true)
{
    Map* map = bot->GetMap();
    uint32 phaseMask = bot->GetPhaseMask();

    float targetX = targetPos.GetPositionX();
    float targetY = targetPos.GetPositionY();
    float targetZ = targetPos.GetPositionZ();

    float dist = startDistance;
    while (dist <= endDistance) 
    {
        //step backwards from pos to bot to find edge of shore.
        float checkX = targetX - dist * cos(orientation);
        float checkY = targetY - dist * sin(orientation);
        
        float groundZ = map->GetHeight(phaseMask, checkX, checkY, targetZ + HEIGHT_SEARCH_BUFFER, true);
        if (groundZ == INVALID_HEIGHT)
        {
            dist += increment;
            continue;
        }

        LiquidData const& liq = map->GetLiquidData(phaseMask, checkX, checkY, targetZ, DEFAULT_COLLISION_HEIGHT, MAP_ALL_LIQUIDS);
        if (liq.Entry == 0 || groundZ > liq.DepthLevel + HEIGHT_ABOVE_WATER_TOLERANCE)
        {
            if (checkLOS)
            {
                if (!map->isInLineOfSight(checkX, checkY, groundZ, targetX, targetY, targetZ, phaseMask, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing))
                    continue;
            }
            return WorldPosition(bot->GetMapId(), checkX, checkY, groundZ);
        }
        dist += increment;
    }

    return WorldPosition();
}

WorldPosition findLandRadialFromPosition (Player* bot, WorldPosition targetPos, float startDistance, float endDistance, float increment, int angles = 16)
{
    const int numDirections = angles;
    std::vector<WorldPosition> boundaryPoints;

    Map* map = bot->GetMap();
    uint32 phaseMask = bot->GetPhaseMask();

    float targetX = targetPos.GetPositionX();
    float targetY = targetPos.GetPositionY();
    float targetZ = targetPos.GetPositionZ();
    float dist = startDistance;
    while (dist <= endDistance)
    {
        for (int i = 0; i < numDirections; ++i) 
        {
            float angle = (2.0f * M_PI * i) / numDirections;
            float checkX = targetX - cos(angle) * dist;
            float checkY = targetY - sin(angle) * dist;

            float groundZ = hasFishableWaterOrLand(checkX, checkY, targetZ, map, phaseMask, true);
            if (groundZ == INVALID_HEIGHT)
                continue;

            if (map->isInLineOfSight(checkX, checkY, groundZ, targetX, targetY, targetZ, phaseMask, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing))
            {
                boundaryPoints.emplace_back(WorldPosition(bot->GetMapId(), checkX, checkY, groundZ));
            }
        }
        
        if (!boundaryPoints.empty())
            break;
        
        dist += increment;
    }

    if (boundaryPoints.empty())
        return WorldPosition();

    if (boundaryPoints.size() == 1)
        return boundaryPoints[0];

    float minDistance = 100000;
    WorldLocation closestPoint = WorldPosition();
    for (const auto& pos : boundaryPoints)
    {
        float distance = bot->GetExactDist2d(&pos);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestPoint = pos;
        }
    }
    return closestPoint;
}

WorldPosition findWaterRadial(Player* bot, float x, float y, float z, Map* map, uint32 phaseMask, float minDistance, float maxDistance, float increment, bool checkLOS) 
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

            float waterZ = hasFishableWaterOrLand(checkX, checkY, z, map, phaseMask);
            if (waterZ == INVALID_HEIGHT)
                continue;

            if (checkLOS && !hasLosToWater(bot, checkX, checkY, waterZ))
            {
                continue;
            }
            boundaryPoints.emplace_back(WorldPosition(bot->GetMapId(), checkX, checkY, waterZ));
        }

        if (!boundaryPoints.empty())
            break;

        dist += increment;
    }

    if (boundaryPoints.empty())
        return WorldPosition();

    if (boundaryPoints.size() == 1)
        return boundaryPoints[0];

    return boundaryPoints[boundaryPoints.size() / 2];
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
    if(IsValid(landSpot))
    {
        return MoveTo(landSpot.GetMapId(), landSpot.GetPositionX(), landSpot.GetPositionY(), landSpot.GetPositionZ());
    }
    return false;
}

bool MoveNearWaterAction::isUseful()
{
    return (AI_VALUE(bool, "can fish"));
}

bool MoveNearWaterAction::isPossible()
{
    Player* master = botAI->GetMaster();
    WorldPosition fishingHole = findFishingHole(botAI);

    if (IsValid(fishingHole))
    {
        float distance = bot->GetExactDist2d(&fishingHole);
        bool hasLOS = bot->IsWithinLOS(fishingHole.GetPositionX(), fishingHole.GetPositionY(), fishingHole.GetPositionZ());
        // Water spot is in range, and we have LOS to it. Do not move
        if (distance  >= MIN_DISTANCE_TO_WATER &&
            distance <=  MAX_DISTANCE_TO_WATER && hasLOS)
        {
            return false;
        }
        // Water spot is out of range, lets look for a spot to move to for the fishing hole.
        if (distance > MAX_DISTANCE_TO_WATER || distance < MIN_DISTANCE_TO_WATER)
        {
            float angle = bot->GetAngle(fishingHole.GetPositionX(), fishingHole.GetPositionY());
            landSpot = findLandRadialFromPosition(bot, fishingHole, MIN_DISTANCE_TO_WATER, MAX_DISTANCE_TO_WATER, SEARCH_INCREMENT, 32);
            if (IsValid(landSpot))
            {
                if (master && botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
                {
                    return (master->GetExactDist2d(bot) < MAX_MASTER_DISTANCE);
                }
                return true;
            }
        }
    }
    // Lets find some water where we can fish.
    WorldPosition water = findWaterRadial(
        bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        bot->GetMap(), bot->GetPhaseMask(),
        MIN_WATER_SEARCH_DISTANCE,
        MAX_WATER_SEARCH_DISTANCE,
        SEARCH_INCREMENT,
        false);

    if (IsValid(water))
    {
        float distance = bot->GetExactDist2d(&water);
        bool hasLOS = bot->IsWithinLOS(water.GetPositionX(), water.GetPositionY(), water.GetPositionZ());
        if (distance >= MIN_DISTANCE_TO_WATER &&
            distance <= MAX_DISTANCE_TO_WATER && hasLOS)
        {
            return false;
        }
        float angle = bot->GetAngle(water.GetPositionX(), water.GetPositionY());
        landSpot = findLandFromPosition(bot, 0.0f,
            MAX_DISTANCE_TO_WATER, 1.0f,
            angle, water, false);

        if (IsValid(landSpot))
        {
            if (master && botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
            {
                return (master->GetExactDist2d(bot) < MAX_MASTER_DISTANCE);
            }
            return true;
        }
    }
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
    WorldPosition target = WorldPosition();
    WorldPosition fishingHole = findFishingHole(botAI);
    if (IsValid(fishingHole))
    {
        Position pos = fishingHole; 
        float distance = bot->GetExactDist2d(&pos);
        bool hasLOS = bot->IsWithinLOS(fishingHole.GetPositionX(), fishingHole.GetPositionY(), fishingHole.GetPositionZ());

        if (distance < MAX_DISTANCE_TO_WATER &&
            distance > MIN_DISTANCE_TO_WATER && hasLOS)
        {
            target = fishingHole;
        }
    }
    if (!IsValid(target))
    {
        target = findWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(),
                bot->GetPositionZ(), bot->GetMap(), bot->GetPhaseMask(), 
                MAX_DISTANCE_TO_WATER, MAX_DISTANCE_TO_WATER, SEARCH_INCREMENT, true);
        if (!IsValid(target))
        {
            return false;
        }
    }
    Position pos = target;

    if (!bot->HasInArc(1.0, &pos, 5.0))
    {
        float angle = bot->GetAngle(pos.GetPositionX(), pos.GetPositionY());
        bot->SetOrientation(angle);
        bot->SendMovementFlagUpdate();
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
    return (AI_VALUE(bool, "can fish"));
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

bool EndFishing::isUseful()
{
    WorldPosition nearwater = findWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMap(), bot->GetPhaseMask(), 30.0f, 31.0f, 10.0f);
    return (!IsValid(nearwater));

}
bool RemoveBobberStrategyAction::Execute(Event event)
{
    botAI->ChangeStrategy("-use bobber", BOT_STATE_NON_COMBAT);
    return true;
}