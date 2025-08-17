/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "LootTriggers.h"

#include "LootObjectStack.h"
#include "Playerbots.h"
#include "ServerFacade.h"

bool LootAvailableTrigger::IsActive()
{
    bool distanceCheck = false;
    if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT))
    {
        distanceCheck =
            sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "loot target"), CONTACT_DISTANCE);
    }
    else
    {
        distanceCheck = sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "loot target"),
                                                                 INTERACTION_DISTANCE - 2.0f);
    }

    // if loot target if empty, always pass distance check
    return AI_VALUE(bool, "has available loot") &&
        (distanceCheck || AI_VALUE(GuidVector, "all targets").empty());
}

bool FarFromCurrentLootTrigger::IsActive()
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (!loot.IsLootPossible(bot))
        return false;

    return AI_VALUE2(float, "distance", "loot target") >= INTERACTION_DISTANCE - 2.0f;
}

bool CanLootTrigger::IsActive() { return AI_VALUE(bool, "can loot"); }
