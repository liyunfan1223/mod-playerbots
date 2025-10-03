/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BEARTANKDRUIDSTRATEGY_H
#define _PLAYERBOT_BEARTANKDRUIDSTRATEGY_H

#include "FeralDruidStrategy.h"

class PlayerbotAI;

class BearTankDruidStrategy : public FeralDruidStrategy
{
public:
    BearTankDruidStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bear"; }
    NextAction** getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_TANK | STRATEGY_TYPE_MELEE; }
};

#endif
