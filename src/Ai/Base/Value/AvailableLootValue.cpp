/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AvailableLootValue.h"

#include "LootObjectStack.h"
#include "Playerbots.h"

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

bool CanLootValue::Get()
{
    ObjectGuid const guid = AI_VALUE(LootObject, "loot target").guid;
    if (guid != _lootGuid)
    {
        _lootGuid = guid;
        Reset();
    }

    return BoolCalculatedValue::Get();
}

bool CanLootValue::Calculate()
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    LootObjectStack* availableLoot = AI_VALUE(LootObjectStack*, "available loot");
    return !loot.IsEmpty() && availableLoot->CanAttemptLoot(loot.guid) && loot.IsAtInteractDistance(bot) &&
           loot.IsLootPossible(bot);
}
