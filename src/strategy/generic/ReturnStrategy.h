/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RETURNSTRATEGY_H
#define _PLAYERBOT_RETURNSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class ReturnStrategy : public NonCombatStrategy
{
public:
    ReturnStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "return"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
