/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RTSCSTRATEGY_H
#define _PLAYERBOT_RTSCSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class RTSCStrategy : public Strategy
{
public:
    RTSCStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "RTSC"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
