/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AvailableLootValue.h"

#include "LootObjectStack.h"
#include "Playerbots.h"
#include "ServerFacade.h"

AvailableLootValue::AvailableLootValue(PlayerbotAI* botAI, std::string const name)
    : ManualSetValue<LootObjectStack*>(botAI, nullptr, name)
{
    value = new LootObjectStack(botAI->GetBot());
}

AvailableLootValue::~AvailableLootValue() { delete value; }

LootTargetValue::LootTargetValue(PlayerbotAI* botAI, std::string const name)
    : ManualSetValue<LootObject>(botAI, LootObject(), name)
{
}

bool CanLootValue::Calculate()
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    LootObjectStack* availableLoot = AI_VALUE(LootObjectStack*, "available loot");
    return !loot.IsEmpty() && availableLoot->CanAttemptLoot(loot.guid) && loot.IsLootPossible(bot) &&
           ServerFacade::instance().IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "loot target"),
                                                              INTERACTION_DISTANCE - 2);
}
