/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICHUNTERSTRATEGY_H
#define _PLAYERBOT_GENERICHUNTERSTRATEGY_H

#include "CombatStrategy.h"
#include "Strategy.h"

class PlayerbotAI;

class GenericHunterStrategy : public CombatStrategy
{
public:
    GenericHunterStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "hunter"; }
    uint32 GetType() const override { return CombatStrategy::GetType() | STRATEGY_TYPE_RANGED | STRATEGY_TYPE_DPS; }
};


class AoEHunterStrategy : public CombatStrategy
{
public:
    AoEHunterStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "aoe"; }
};

class HunterBoostStrategy : public Strategy
{
public:
    HunterBoostStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "boost"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class HunterCcStrategy : public Strategy
{
public:
    HunterCcStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cc"; }
};

class HunterTrapWeaveStrategy : public Strategy
{
public:
    HunterTrapWeaveStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "trap weave"; }
};


#endif
