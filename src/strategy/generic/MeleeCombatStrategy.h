/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MELEECOMBATSTRATEGY_H
#define _PLAYERBOT_MELEECOMBATSTRATEGY_H

#include "CombatStrategy.h"

class PlayerbotAI;

class MeleeCombatStrategy : public CombatStrategy
{
public:
    MeleeCombatStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_MELEE; }
    std::string const getName() override { return "close"; }
};

class SetBehindCombatStrategy : public CombatStrategy
{
public:
    SetBehindCombatStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "behind"; }
};

#endif
