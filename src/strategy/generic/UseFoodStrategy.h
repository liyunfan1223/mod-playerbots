/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_USEFOODSTRATEGY_H
#define _PLAYERBOT_USEFOODSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class UseFoodStrategy : public Strategy
{
public:
    UseFoodStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "food"; }
};

#endif
