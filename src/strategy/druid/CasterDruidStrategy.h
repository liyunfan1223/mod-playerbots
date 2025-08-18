/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CASTERDRUIDSTRATEGY_H
#define _PLAYERBOT_CASTERDRUIDSTRATEGY_H

#include "GenericDruidStrategy.h"

class PlayerbotAI;

class CasterDruidStrategy : public GenericDruidStrategy
{
public:
    CasterDruidStrategy(PlayerbotAI* botAI);

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "caster"; }
    NextAction** getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
};

class CasterDruidAoeStrategy : public CombatStrategy
{
public:
    CasterDruidAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "caster aoe"; }
};

class CasterDruidDebuffStrategy : public CombatStrategy
{
public:
    CasterDruidDebuffStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "caster debuff"; }
};

#endif
