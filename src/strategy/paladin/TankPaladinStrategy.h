/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TANKPALADINSTRATEGY_H
#define _PLAYERBOT_TANKPALADINSTRATEGY_H

#include "GenericPaladinStrategy.h"

class PlayerbotAI;

class TankPaladinStrategy : public GenericPaladinStrategy
{
public:
    TankPaladinStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "tank"; }
    NextAction** getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_TANK | STRATEGY_TYPE_MELEE; }
};

#endif
