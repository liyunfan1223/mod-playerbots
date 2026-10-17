/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BALANCEDRUIDSTRATEGY_H
#define PLAYERBOTS_BALANCEDRUIDSTRATEGY_H

#include "GenericDruidStrategy.h"

class PlayerbotAI;

class BalanceDruidStrategy : public GenericDruidStrategy
{
public:
    BalanceDruidStrategy(PlayerbotAI* botAI);

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "balance"; }
    std::vector<NextAction> getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
};

#endif
