/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DPSHUNTERSTRATEGY_H
#define _PLAYERBOT_DPSHUNTERSTRATEGY_H

#include "GenericHunterStrategy.h"

class PlayerbotAI;

class DpsHunterStrategy : public GenericHunterStrategy
{
public:
    DpsHunterStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "dps"; }
    NextAction** getDefaultActions() override;
};

class DpsAoeHunterStrategy : public CombatStrategy
{
public:
    DpsAoeHunterStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "aoe"; }
};

class DpsHunterDebuffStrategy : public CombatStrategy
{
public:
    DpsHunterDebuffStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "dps debuff"; }
};

#endif
