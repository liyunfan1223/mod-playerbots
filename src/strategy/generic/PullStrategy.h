/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PULLSTRATEGY_H
#define _PLAYERBOT_PULLSTRATEGY_H

#include "CombatStrategy.h"

class PlayerbotAI;

class PullStrategy : public CombatStrategy
{
public:
    PullStrategy(PlayerbotAI* botAI, std::string const action) : CombatStrategy(botAI), action(action) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
    std::string const getName() override { return "pull"; }
    NextAction** getDefaultActions() override;

private:
    std::string const action;
};

class PossibleAddsStrategy : public Strategy
{
public:
    PossibleAddsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "adds"; }
};

#endif
