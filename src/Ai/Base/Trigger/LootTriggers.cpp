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

    LootObject lootTarget = AI_VALUE(LootObject, "loot target");
    if (lootTarget.IsEmpty())
        return true;

    // A different nearest entry may be worth switching to; LootAction::isUseful applies the margin.
    if (AI_VALUE(LootObject, "nearest loot").guid != lootTarget.guid)
        return true;

    bool const inRange = botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT)
                             ? ServerFacade::instance().IsDistanceLessOrEqualThan(
                                   AI_VALUE2(float, "distance", "loot target"), CONTACT_DISTANCE)
                             : AI_VALUE(bool, "can loot");
    if (inRange || !AI_VALUE(LootObjectStack*, "available loot")->CanAttemptLoot(lootTarget.guid) ||
        !lootTarget.IsLootPossible(bot))
        return true;

    // No hostile targets to deal with first. Last because "all targets" is an uncached grid search.
    return AI_VALUE(GuidVector, "all targets").empty();
}

bool FarFromCurrentLootTrigger::IsActive()
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (loot.IsEmpty() || AI_VALUE(bool, "can loot") ||
        !AI_VALUE(LootObjectStack*, "available loot")->CanAttemptLoot(loot.guid))
        return false;

    return loot.IsLootPossible(bot);
}

bool CanLootTrigger::IsActive() { return AI_VALUE(bool, "can loot"); }
