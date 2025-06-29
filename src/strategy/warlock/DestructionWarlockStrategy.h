/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DESTRUCTIONWARLOCKSTRATEGY_H
#define _PLAYERBOT_DESTRUCTIONWARLOCKSTRATEGY_H

#include "GenericWarlockStrategy.h"
#include "CombatStrategy.h" 

class PlayerbotAI;

class DestructionWarlockStrategy : public GenericWarlockStrategy
{
public:
    DestructionWarlockStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "destro"; }
    NextAction** getDefaultActions() override;
    uint32 GetType() const override { return GenericWarlockStrategy::GetType() | STRATEGY_TYPE_DPS; }
};

class DestructionWarlockAoeStrategy : public CombatStrategy
{
public:
    DestructionWarlockAoeStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "destro aoe"; }
};

#endif
