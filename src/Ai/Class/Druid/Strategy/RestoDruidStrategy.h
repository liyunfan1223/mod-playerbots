/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RESTODRUIDSTRATEGY_H
#define PLAYERBOTS_RESTODRUIDSTRATEGY_H

#include "GenericDruidStrategy.h"
#include "Strategy.h"

class PlayerbotAI;

class RestoDruidStrategy : public GenericDruidStrategy
{
public:
    RestoDruidStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "resto"; }
    uint32 GetType() const override { return STRATEGY_TYPE_RANGED | STRATEGY_TYPE_HEAL; }
};

class DruidBlanketStrategy : public Strategy
{
public:
    DruidBlanketStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "blanketing"; }
};

class DruidTranquilityStrategy : public Strategy
{
public:
    DruidTranquilityStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "tranquility"; }
};

#endif
