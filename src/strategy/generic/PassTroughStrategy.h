/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PASSTHROUGHSTRATEGY_H
#define _PLAYERBOT_PASSTHROUGHSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class PassTroughStrategy : public Strategy
{
public:
    PassTroughStrategy(PlayerbotAI* botAI, float relevance = 100.0f) : Strategy(botAI), relevance(relevance) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;

protected:
    std::vector<std::string> supported;
    float relevance;
};

#endif
