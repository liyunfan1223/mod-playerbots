/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_KITESTRATEGY_H
#define _PLAYERBOT_KITESTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class KiteStrategy : public Strategy
{
public:
    KiteStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "kite"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
