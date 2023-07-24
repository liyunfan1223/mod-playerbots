/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HEALDRUIDSTRATEGY_H
#define _PLAYERBOT_HEALDRUIDSTRATEGY_H

#include "GenericDruidStrategy.h"
#include "Strategy.h"

class PlayerbotAI;

class HealDruidStrategy : public GenericDruidStrategy
{
    public:
        HealDruidStrategy(PlayerbotAI* botAI);

        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
        std::string const getName() override { return "heal"; }
        uint32 GetType() const override { return STRATEGY_TYPE_RANGED | STRATEGY_TYPE_HEAL; }
};

#endif
