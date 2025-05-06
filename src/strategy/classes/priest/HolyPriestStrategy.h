/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HOLYPRIESTSTRATEGY_H
#define _PLAYERBOT_HOLYPRIESTSTRATEGY_H

#include "GenericPriestStrategyActionNodeFactory.h"
#include "HealPriestStrategy.h"

class PlayerbotAI;

class HolyPriestStrategy : public HealPriestStrategy
{
public:
    HolyPriestStrategy(PlayerbotAI* botAI);

    NextAction** getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "holy dps"; }
    uint32 GetType() const override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
};

class HolyHealPriestStrategy : public GenericPriestStrategy
{
public:
    HolyHealPriestStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    NextAction** getDefaultActions() override;
    std::string const getName() override { return "holy heal"; }
    uint32 GetType() const override { return STRATEGY_TYPE_HEAL | STRATEGY_TYPE_RANGED; }
};

#endif
