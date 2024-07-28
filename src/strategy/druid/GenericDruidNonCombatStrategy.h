/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICDRUIDNONCOMBATSTRATEGY_H
#define _PLAYERBOT_GENERICDRUIDNONCOMBATSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class GenericDruidNonCombatStrategy : public NonCombatStrategy
{
public:
    GenericDruidNonCombatStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "nc"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class GenericDruidBuffStrategy : public NonCombatStrategy
{
public:
    GenericDruidBuffStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "buff"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
