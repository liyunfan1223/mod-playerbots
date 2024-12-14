/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SHADOWPRIESTSTRATEGY_H
#define _PLAYERBOT_SHADOWPRIESTSTRATEGY_H

#include "GenericPriestStrategy.h"

class PlayerbotAI;

class ShadowPriestStrategy : public GenericPriestStrategy
{
public:
    ShadowPriestStrategy(PlayerbotAI* botAI);

    NextAction** getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "shadow"; }
    uint32 GetType() const override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
};

class ShadowPriestAoeStrategy : public CombatStrategy
{
public:
    ShadowPriestAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "shadow aoe"; }
};

class ShadowPriestDebuffStrategy : public CombatStrategy
{
public:
    ShadowPriestDebuffStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "shadow debuff"; }
};

#endif
