/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_COMBATSTRATEGY_H
#define _PLAYERBOT_COMBATSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class CombatStrategy : public Strategy
{
    public:
        CombatStrategy(PlayerbotAI* botAI) : Strategy(botAI) { }

        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
        uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
};

#endif
