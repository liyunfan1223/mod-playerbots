/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AVAILABLELOOTVALUE_H
#define PLAYERBOTS_AVAILABLELOOTVALUE_H

#include "LootObjectStack.h"
#include "Value.h"

class PlayerbotAI;

class AvailableLootValue : public ManualSetValue<LootObjectStack*>
{
public:
    AvailableLootValue(PlayerbotAI* botAI, std::string const name = "available loot");
    virtual ~AvailableLootValue();
};

class LootTargetValue : public ManualSetValue<LootObject>
{
public:
    LootTargetValue(PlayerbotAI* botAI, std::string const name = "loot target");
};

class CanLootValue : public BoolCalculatedValue
{
public:
    CanLootValue(PlayerbotAI* botAI, std::string const name = "can loot") : BoolCalculatedValue(botAI, name, 100) {}

    bool Get() override;
    bool Calculate() override;

private:
    ObjectGuid _lootGuid;
};

#endif
