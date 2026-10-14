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

    bool distanceCheck = false;
    if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT))
    {
        distanceCheck =
            ServerFacade::instance().IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "loot target"), CONTACT_DISTANCE);
    }
    else
    {
        distanceCheck = ServerFacade::instance().IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "loot target"),
                                                                 INTERACTION_DISTANCE - 2.0f);
    }

    // Loot target in range, or no hostile targets to deal with first.
    if (distanceCheck || AI_VALUE(GuidVector, "all targets").empty())
        return true;

    LootObject lootTarget = AI_VALUE(LootObject, "loot target");
    return !lootTarget.IsEmpty() && !lootTarget.IsLootPossible(bot);
}

bool FarFromCurrentLootTrigger::IsActive()
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (!loot.IsLootPossible(bot))
        return false;

    return AI_VALUE2(float, "distance", "loot target") >= INTERACTION_DISTANCE - 2.0f;
}

bool CanLootTrigger::IsActive() { return AI_VALUE(bool, "can loot"); }
