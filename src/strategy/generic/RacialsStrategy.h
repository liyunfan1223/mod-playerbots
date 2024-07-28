/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RACIALSSTRATEGY_H
#define _PLAYERBOT_RACIALSSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class RacialsStrategy : public Strategy
{
public:
    RacialsStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "racials"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
