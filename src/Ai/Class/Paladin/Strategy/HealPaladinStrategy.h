/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HEALPALADINSTRATEGY_H
#define PLAYERBOTS_HEALPALADINSTRATEGY_H

#include "GenericPaladinStrategy.h"

class PlayerbotAI;

class HealPaladinStrategy : public GenericPaladinStrategy
{
public:
    HealPaladinStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "holy"; }
    std::vector<NextAction> getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_HEAL | STRATEGY_TYPE_RANGED; }
};

#endif
