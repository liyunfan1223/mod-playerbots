/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICPRIESTSTRATEGY_H
#define _PLAYERBOT_GENERICPRIESTSTRATEGY_H

#include "CombatStrategy.h"
#include "RangedCombatStrategy.h"

class PlayerbotAI;

class GenericPriestStrategy : public RangedCombatStrategy
{
public:
    GenericPriestStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class PriestCureStrategy : public Strategy
{
public:
    PriestCureStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cure"; }
};

class PriestBoostStrategy : public Strategy
{
public:
    PriestBoostStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "boost"; }
};

class PriestCcStrategy : public Strategy
{
public:
    PriestCcStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cc"; }
};

class PriestHealerDpsStrategy : public Strategy
{
public:
    PriestHealerDpsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "healer dps"; }
};

#endif
