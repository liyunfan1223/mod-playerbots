/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CATDPSDRUIDSTRATEGY_H
#define _PLAYERBOT_CATDPSDRUIDSTRATEGY_H

#include "FeralDruidStrategy.h"

class PlayerbotAI;

class CatDpsDruidStrategy : public FeralDruidStrategy
{
public:
    CatDpsDruidStrategy(PlayerbotAI* botAI);

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cat"; }
    NextAction** getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_MELEE; }
};

class CatAoeDruidStrategy : public CombatStrategy
{
public:
    CatAoeDruidStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cat aoe"; }
};

#endif
