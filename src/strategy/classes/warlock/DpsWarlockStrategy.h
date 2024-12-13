/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DPSWARLOCKSTRATEGY_H
#define _PLAYERBOT_DPSWARLOCKSTRATEGY_H

#include "GenericWarlockStrategy.h"
#include "Strategy.h"

class PlayerbotAI;

class DpsWarlockStrategy : public GenericWarlockStrategy
{
public:
    DpsWarlockStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "dps"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    NextAction** getDefaultActions() override;
    uint32 GetType() const override { return GenericWarlockStrategy::GetType() | STRATEGY_TYPE_DPS; }
};

class DpsAoeWarlockStrategy : public CombatStrategy
{
public:
    DpsAoeWarlockStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "aoe"; }
};

class DpsWarlockDebuffStrategy : public CombatStrategy
{
public:
    DpsWarlockDebuffStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "dps debuff"; }
};

#endif
