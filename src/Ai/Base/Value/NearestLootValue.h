/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEARESTLOOTVALUE_H
#define PLAYERBOTS_NEARESTLOOTVALUE_H

#include "LootObjectStack.h"
#include "Value.h"

class PlayerbotAI;

class NearestLootValue : public CalculatedValue<LootObject>
{
public:
    // 500ms cache: the stack walk is heavy and the loot trigger evaluates this every tick.
    NearestLootValue(PlayerbotAI* botAI) : CalculatedValue<LootObject>(botAI, "nearest loot", 500) {}

    LootObject Calculate() override;
};

#endif
