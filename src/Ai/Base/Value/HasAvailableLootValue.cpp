/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HasAvailableLootValue.h"
#include "LootObjectStack.h"
#include "Playerbots.h"

bool HasAvailableLootValue::Calculate()
{
    return !AI_VALUE(bool, "can loot") &&
           AI_VALUE(LootObjectStack*, "available loot")->CanLoot(sPlayerbotAIConfig.LootDistance);
}
