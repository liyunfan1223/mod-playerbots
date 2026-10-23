/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FishingAction.h"
#include "Event.h"
#include "FishValues.h"
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

uint32 const FISHING_SPELL = 7620;
uint32 const FISHING_POLE = 6256;
uint32 const FISHING_BOBBER = 35591;
float const MIN_DISTANCE_TO_WATER = 10.0f; // Minimum spell distance
float const MAX_DISTANCE_TO_WATER = 20.0f; // Maximum spell distance
float const HEIGHT_ABOVE_WATER_TOLERANCE = 1.0f; // Can stand in up to 1 unit of water and still fish.
float const SEARCH_INCREMENT = 2.5f;
float const HEIGHT_SEARCH_BUFFER = 10.0f; // Height buffer to prevent potentially missing the model the bot is standing on.
float const SEARCH_LAND_BUFFER = 0.5f;
uint32 const FISHING_LOCATION_TIMEOUT = 180000; //Three minutes

static bool IsFishingPole(Item* const item)
{
    if (!item)
        return false;
    ItemTemplate const* proto = item->GetTemplate();
    return proto && proto->Class == ITEM_CLASS_WEAPON &&
        proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE;
}

float HasFishableWaterOrLand(float x, float y, float z,  Map* map, uint32 phaseMask, bool checkForLand=false)
{
    if (!map)
        return INVALID_HEIGHT;

    LiquidData const& liq = map->GetLiquidData(phaseMask, x, y, z+HEIGHT_ABOVE_WATER_TOLERANCE, DEFAULT_COLLISION_HEIGHT, MAP_ALL_LIQUIDS);
    float ground = map->GetHeight(phaseMask, x, y, z + HEIGHT_SEARCH_BUFFER, true);
    if (liq.Entry == MAP_LIQUID_TYPE_NO_WATER)
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
        if (abs(liq.DepthLevel) < 0.5f) // too shallow to fish in.
            return INVALID_HEIGHT;
        return liq.Level;
    }
    return INVALID_HEIGHT;
}

bool HasLosToWater(Player* bot, float wx, float wy, float waterZ)
{
    float z = bot->GetCollisionHeight() + bot->GetPositionZ();
    return bot->GetMap()->isInLineOfSight(
        bot->GetPositionX(), bot->GetPositionY(), z,
        wx, wy, waterZ,
        bot->GetPhaseMask(),
        LINEOFSIGHT_ALL_CHECKS,
        VMAP::ModelIgnoreFlags::Nothing);
}

WorldPosition FindLandFromPosition(PlayerbotAI* botAI, float startDistance, float endDistance, float increment, float orientation, WorldPosition targetPos, float fishingSearchWindow, bool checkLOS = true)
{
    Player* bot = botAI->GetBot();
    Map* map = bot->GetMap();
    uint32 phaseMask = bot->GetPhaseMask();
    Player* master = botAI->GetMaster();

    float targetX = targetPos.GetPositionX();
    float targetY = targetPos.GetPositionY();
    float targetZ = targetPos.GetPositionZ();

    for (float dist = startDistance; dist <= endDistance; dist += increment)
    {
        //step backwards from position to bot to find edge of shore.
        float checkX = targetX - dist * cos(orientation);
        float checkY = targetY - dist * sin(orientation);

        float groundZ = map->GetHeight(phaseMask, checkX, checkY, targetZ + HEIGHT_SEARCH_BUFFER, true);

        if (groundZ == INVALID_HEIGHT)
            continue;

        LiquidData const& liq = map->GetLiquidData(phaseMask, checkX, checkY, targetZ, DEFAULT_COLLISION_HEIGHT, MAP_ALL_LIQUIDS);
        if (liq.Entry == MAP_LIQUID_TYPE_NO_WATER || groundZ > liq.DepthLevel + HEIGHT_ABOVE_WATER_TOLERANCE)
        {
            if (checkLOS)
            {
                bool hasLOS = map->isInLineOfSight(checkX, checkY, groundZ, targetX, targetY, targetZ, phaseMask, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing);
                if (!hasLOS)
                    continue;
            }
            // Add a distance check for the position to prevent the bot from moving out of range to the master.
            if (master && botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT) && master->GetDistance(checkX, checkY, groundZ) > fishingSearchWindow - SEARCH_LAND_BUFFER)
                continue;

            return WorldPosition(bot->GetMapId(), checkX, checkY, groundZ);
        }
    }

    return WorldPosition();
}

WorldPosition FindLandRadialFromPosition (PlayerbotAI* botAI, WorldPosition targetPos, float startDistance, float endDistance, float increment, float fishingSearchWindow, int angles = 16)
{
    Player* bot = botAI->GetBot();
    const int numDirections = angles;
    std::vector<WorldPosition> boundaryPoints;
    Player* master = botAI->GetMaster();
    if (!master)
        return WorldPosition();

    Map* map = bot->GetMap();
    uint32 phaseMask = bot->GetPhaseMask();

    float targetX = targetPos.GetPositionX();
    float targetY = targetPos.GetPositionY();
    float targetZ = targetPos.GetPositionZ();

    for (float dist = startDistance; dist <= endDistance; dist += increment)
    {
        for (int i = 0; i < numDirections; ++i)
        {
            float angle = (2.0f * M_PI * i) / numDirections;
            float checkX = targetX - cos(angle) * dist;
            float checkY = targetY - sin(angle) * dist;

            float groundZ = HasFishableWaterOrLand(checkX, checkY, targetZ, map, phaseMask, true);

            if (groundZ == INVALID_HEIGHT)
                continue;

            if (map->isInLineOfSight(checkX, checkY, groundZ, targetX, targetY, targetZ, phaseMask, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing) && master->GetDistance(checkX, checkY, groundZ) > fishingSearchWindow - SEARCH_LAND_BUFFER)
                continue;

            boundaryPoints.emplace_back(WorldPosition(bot->GetMapId(), checkX, checkY, groundZ));
        }

        if (!boundaryPoints.empty())
            break;
    }

    if (boundaryPoints.empty())
        return WorldPosition();

    if (boundaryPoints.size() == 1)
        return boundaryPoints[0];

    float minDistance = FLT_MAX;
    WorldLocation closestPoint = WorldPosition();
    for (auto const& pos : boundaryPoints)
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

WorldPosition FindWaterRadial(Player* bot, float x, float y, float z, Map* map, uint32 phaseMask, float minDistance, float maxDistance, float increment, bool checkLOS, int numDirections)
{
    std::vector<WorldPosition> boundaryPoints;

    float dist = minDistance;
    while (dist <= maxDistance)
    {
        for (int i = 0; i < numDirections; ++i)
        {
            float angle = (2.0f * M_PI * i) / numDirections;
            float checkX = x + cos(angle) * dist;
            float checkY = y + sin(angle) * dist;

            float waterZ = HasFishableWaterOrLand(checkX, checkY, z, map, phaseMask);

            if (waterZ == INVALID_HEIGHT)
                continue;

            if (checkLOS && !HasLosToWater(bot, checkX, checkY, waterZ))
                continue;

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
    // return the central point in the identified positions in to try to be perpendicular to the shore.
    return boundaryPoints[boundaryPoints.size() / 2];
}

WorldPosition FindFishingHole(PlayerbotAI* botAI)
{
    Player* player = botAI->GetBot();
    GuidVector gos = PAI_VALUE(GuidVector, "nearest game objects no los");
    GameObject* nearestFishingHole = nullptr;
    float minDist = std::numeric_limits<float>::max();
    for (auto const& guid : gos)
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
        return WorldPosition(nearestFishingHole->GetMapId(), nearestFishingHole->GetPositionX(), nearestFishingHole->GetPositionY(), nearestFishingHole->GetPositionZ());

    return WorldPosition();
}

bool MoveNearWaterAction::Execute(Event /*event*/)
{
    WorldPosition landSpot = AI_VALUE(WorldPosition, "fishing spot");
    if (landSpot.IsValid())
        return MoveTo(landSpot.GetMapId(), landSpot.GetPositionX(), landSpot.GetPositionY(), landSpot.GetPositionZ());

    return false;
}

bool MoveNearWaterAction::isUseful()
{
    if (!AI_VALUE(bool, "can fish"))
        return false;
    FishingSpotValue* fishingSpotValueObject = (FishingSpotValue*)context->GetValue<WorldPosition>("fishing spot");
    WorldPosition pos = fishingSpotValueObject->Get();
    return !pos.IsValid() || fishingSpotValueObject->IsStale(FISHING_LOCATION_TIMEOUT) ||
           bot->GetExactDist(&pos) > 0.1f;

}

bool MoveNearWaterAction::isPossible()
{
    Player* master = botAI->GetMaster();
    float fishingSearchWindow;

    if (master)
        fishingSearchWindow = sPlayerbotAIConfig.FishingDistanceFromMaster;
    else
        fishingSearchWindow = sPlayerbotAIConfig.FishingDistance;

    WorldPosition fishingHole = FindFishingHole(botAI);

    if (fishingHole.IsValid())
    {
        float distance = bot->GetExactDist2d(&fishingHole);
        bool hasLOS = bot->IsWithinLOS(fishingHole.GetPositionX(), fishingHole.GetPositionY(), fishingHole.GetPositionZ());
        // Water spot is in range, and we have LOS to it. Set bot position to fishing spot and do not move
        if (distance >= MIN_DISTANCE_TO_WATER &&
            distance <= MAX_DISTANCE_TO_WATER && hasLOS)
        {
            SET_AI_VALUE(WorldPosition, "fishing spot", WorldPosition(WorldPosition(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())));
            return false;
        }
        // Water spot is out of range, lets look for a spot to move to for the fishing hole.
        if (distance > MAX_DISTANCE_TO_WATER || distance < MIN_DISTANCE_TO_WATER)
        {
            WorldPosition landSpot = FindLandRadialFromPosition(botAI, fishingHole, MIN_DISTANCE_TO_WATER, MAX_DISTANCE_TO_WATER, SEARCH_INCREMENT, fishingSearchWindow, 32);
            if (landSpot.IsValid())
            {
                SET_AI_VALUE(WorldPosition, "fishing spot", landSpot);
                return true;
            }
        }
    }
    // Can the bot fish from current position?
    WorldPosition waterAtCurrentPos =
        FindWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMap(),
                        bot->GetPhaseMask(), MIN_DISTANCE_TO_WATER, MAX_DISTANCE_TO_WATER, SEARCH_INCREMENT, true);
    if (waterAtCurrentPos.IsValid())
    {
        SET_AI_VALUE(WorldPosition, "fishing spot",
                     WorldPosition(WorldPosition(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                                                 bot->GetPositionZ())));
        return false;
    }
    // Lets find some water where we can fish.
    WorldPosition water = FindWaterRadial(
        bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        bot->GetMap(), bot->GetPhaseMask(),
        MIN_DISTANCE_TO_WATER,
        fishingSearchWindow + MAX_DISTANCE_TO_WATER,
        SEARCH_INCREMENT, false);

    if (!water.IsValid())
        return false;

    float angle = bot->GetAngle(water.GetPositionX(), water.GetPositionY());
    WorldPosition landSpot =
        FindLandFromPosition(botAI, 0.0f, MAX_DISTANCE_TO_WATER, 1.0f, angle, water, fishingSearchWindow, false);

    if (landSpot.IsValid())
    {
        SET_AI_VALUE(WorldPosition, "fishing spot", landSpot);
        return true;
    }
    return false;
}

bool EquipFishingPoleAction::Execute(Event /*event*/)
{
    if (!_pole)
        return false;

    WorldPacket eqPacket(CMSG_AUTOEQUIP_ITEM_SLOT, 2);
    eqPacket << _pole->GetGUID() << uint8(EQUIPMENT_SLOT_MAINHAND);
    WorldPackets::Item::AutoEquipItemSlot nicePacket(std::move(eqPacket));
    nicePacket.Read();
    bot->GetSession()->HandleAutoEquipItemSlotOpcode(nicePacket);
    return true;
}

bool EquipFishingPoleAction::isUseful()
{
    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (IsFishingPole(mainHand))
        return false;

    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            if (IsFishingPole(item))
            {
                _pole = item;
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
                    if (IsFishingPole(item))
                    {
                        _pole = item;
                        return true;
                    }
                }
            }
        }
    }

    if (sRandomPlayerbotMgr.IsRandomBot(bot))
    {
        bot->StoreNewItemInBestSlots(FISHING_POLE, 1);  // Try to get a fishing pole
        return true;
    }

    Player* master = botAI->GetMaster();
    if (!master)
        return false;

    std::string masterName = master->GetName();
    std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
    "no_fishing_pole_error", "I don't have a Fishing Pole",{});
    botAI->Whisper(text, masterName);

    return false;
}

bool FishingAction::Execute(Event event)
{
    WorldPosition target = WorldPosition();
    WorldPosition fishingHole = FindFishingHole(botAI);
    if (fishingHole.IsValid())
    {
        Position pos = fishingHole;
        float distance = bot->GetExactDist2d(&pos);
        bool hasLOS = bot->IsWithinLOS(fishingHole.GetPositionX(), fishingHole.GetPositionY(), fishingHole.GetPositionZ());
        if (distance < MAX_DISTANCE_TO_WATER &&
            distance > MIN_DISTANCE_TO_WATER && hasLOS)
            target = fishingHole;
    }
    if (!target.IsValid())
    {
        target = FindWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(),
                bot->GetPositionZ(), bot->GetMap(), bot->GetPhaseMask(),
                MIN_DISTANCE_TO_WATER, MAX_DISTANCE_TO_WATER, SEARCH_INCREMENT, true, 32);
        if (!target.IsValid())
            return false;
    }
    Position pos = target;

    if (!bot->HasInArc(1.0, &pos, 1.0))
    {
        float angle = bot->GetAngle(pos.GetPositionX(), pos.GetPositionY());
        bot->SetOrientation(angle);
        if (!bot->IsRooted())
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
    if (!AI_VALUE(bool, "can fish"))
        return false;

    FishingSpotValue* fishingSpotValueObject = (FishingSpotValue*)context->GetValue<WorldPosition>("fishing spot");
    WorldPosition pos = fishingSpotValueObject->Get();

    if (!pos.IsValid() || fishingSpotValueObject->IsStale(FISHING_LOCATION_TIMEOUT))
        return false;

    return bot->GetExactDist(&pos) < 0.1f;
}

bool UseBobberAction::isUseful()
{
    return AI_VALUE(bool, "can use fishing bobber");
}

bool UseBobberAction::Execute(Event /*event*/)
{
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects no los");
    for (auto const& guid : gos)
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

bool EndMasterFishingAction::Execute(Event /*event*/)
{
    botAI->ChangeStrategy("-master fishing", BOT_STATE_NON_COMBAT);
    return true;
}

bool EndMasterFishingAction::isUseful()
{
    FishingSpotValue* fishingSpotValueObject = (FishingSpotValue*)context->GetValue<WorldPosition>("fishing spot");
    WorldPosition pos = fishingSpotValueObject->Get();
    if (pos.IsValid() && !fishingSpotValueObject->IsStale(FISHING_LOCATION_TIMEOUT) && pos == bot->GetPosition())
        return false;

    WorldPosition nearWater = FindWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        bot->GetMap(), bot->GetPhaseMask(), MIN_DISTANCE_TO_WATER, sPlayerbotAIConfig.EndFishingWithMaster, 10.0f);
    return !nearWater.IsValid();
}

bool RemoveBobberStrategyAction::Execute(Event /*event*/)
{
    botAI->ChangeStrategy("-use bobber", BOT_STATE_NON_COMBAT);
    return true;
}
