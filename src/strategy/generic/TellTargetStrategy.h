/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TELLTARGETSTRATEGY_H
#define _PLAYERBOT_TELLTARGETSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class TellTargetStrategy : public Strategy
{
public:
    TellTargetStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "TellTarget"; }
};

#endif
