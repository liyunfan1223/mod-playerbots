/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LOOTSTRATEGYVALUE_H
#define _PLAYERBOT_LOOTSTRATEGYVALUE_H

#include "Value.h"

class LootStrategy;
class PlayerbotAI;

class LootStrategyValue : public ManualSetValue<LootStrategy*>
{
public:
    LootStrategyValue(PlayerbotAI* botAI, std::string const name = "loot strategy")
        : ManualSetValue<LootStrategy*>(botAI, normal, name)
    {
    }
    virtual ~LootStrategyValue();

    std::string const Save() override;
    bool Load(std::string const value) override;

    static LootStrategy* normal;
    static LootStrategy* gray;
    static LootStrategy* all;
    static LootStrategy* disenchant;
    static LootStrategy* instance(std::string const name);
};

#endif
