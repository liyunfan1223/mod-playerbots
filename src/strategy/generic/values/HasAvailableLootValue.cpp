/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HasAvailableLootValue.h"

#include "LootObjectStack.h"
#include "Playerbots.h"

bool HasAvailableLootValue::Calculate()
{
    return !AI_VALUE(bool, "can loot") &&
           AI_VALUE(LootObjectStack*, "available loot")->CanLoot(sPlayerbotAIConfig->lootDistance);
}
