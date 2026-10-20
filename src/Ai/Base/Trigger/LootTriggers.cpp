/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LootTriggers.h"
#include "LootObjectStack.h"
#include "Playerbots.h"
#include "ServerFacade.h"

bool LootAvailableTrigger::IsActive()
{
    if (!AI_VALUE(bool, "has available loot"))
        return false;

    LootObject nearest = AI_VALUE(LootObject, "nearest loot");
    if (nearest.IsEmpty())
        return false;

    LootObject lootTarget = AI_VALUE(LootObject, "loot target");
    bool distanceCheck = false;
    if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT))
    {
        distanceCheck =
            ServerFacade::instance().IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "loot target"), CONTACT_DISTANCE);
    }
    else
    {
        distanceCheck = lootTarget.IsAtInteractDistance(bot);
    }

    // Loot target in range, or no hostile targets to deal with first.
    if (distanceCheck || AI_VALUE(GuidVector, "all targets").empty())
        return true;

    if (lootTarget.IsEmpty() || !lootTarget.IsLootPossible(bot))
        return true;

    // Re-run the loot action when the nearest entry is clearly closer than the target, instead
    // of chasing the old target past fresh corpses. The margin stops two equidistant corpses on
    // either side of an obstacle from swapping places on every tick.
    if (nearest.guid == lootTarget.guid)
        return false;

    WorldObject* nearestObject = nearest.GetWorldObject(bot);
    WorldObject* currentObject = lootTarget.GetWorldObject(bot);
    if (!nearestObject || !currentObject)
        return true;

    constexpr float lootSwitchMargin = 3.0f;
    return bot->GetDistance(nearestObject) + lootSwitchMargin < bot->GetDistance(currentObject);
}

bool FarFromCurrentLootTrigger::IsActive()
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (!loot.IsLootPossible(bot))
        return false;

    return !loot.IsAtInteractDistance(bot);
}

bool CanLootTrigger::IsActive() { return AI_VALUE(bool, "can loot"); }
