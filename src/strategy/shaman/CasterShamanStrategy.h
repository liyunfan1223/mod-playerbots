/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CASTSHAMANSTRATEGY_H
#define _PLAYERBOT_CASTSHAMANSTRATEGY_H

#include "GenericShamanStrategy.h"

class PlayerbotAI;

class CasterShamanStrategy : public GenericShamanStrategy
{
public:
    CasterShamanStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    NextAction** getDefaultActions() override;
    std::string const getName() override { return "caster"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
};

class CasterAoeShamanStrategy : public CombatStrategy
{
public:
    CasterAoeShamanStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "caster aoe"; }
};

#endif
