/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRAVELSTRATEGY_H
#define _PLAYERBOT_TRAVELSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class TravelStrategy : public Strategy
{
public:
    TravelStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "travel"; }

    NextAction** getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class ExploreStrategy : public Strategy
{
public:
    ExploreStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    std::string const getName() override { return "explore"; }
};

class MapStrategy : public Strategy
{
public:
    MapStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    std::string const getName() override { return "map"; }
};

class MapFullStrategy : public Strategy
{
public:
    MapFullStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    std::string const getName() override { return "map full"; }
};

#endif
