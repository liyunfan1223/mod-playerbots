/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATDRUIDSTRATEGY_H
#define PLAYERBOTS_CATDRUIDSTRATEGY_H

#include "FeralDruidStrategy.h"

class PlayerbotAI;

class CatDruidStrategy : public FeralDruidStrategy
{
public:
    CatDruidStrategy(PlayerbotAI* botAI);

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cat"; }
    std::vector<NextAction> getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
};

// Optional additive strategy. Layers emergency heals on top of the "cat" strategy.
// Enable : co +offheal
// Disable: co -offheal
class CatOffhealStrategy : public CombatStrategy
{
public:
    CatOffhealStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "offheal"; }
};

#endif
