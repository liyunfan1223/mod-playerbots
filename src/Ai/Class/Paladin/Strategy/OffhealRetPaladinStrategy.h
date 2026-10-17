/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

 #ifndef PLAYERBOTS_OFFHEALRETPALADINSTRATEGY_H
#define PLAYERBOTS_OFFHEALRETPALADINSTRATEGY_H

#include "GenericPaladinStrategy.h"

class PlayerbotAI;

class OffhealRetPaladinStrategy : public GenericPaladinStrategy
{
public:
    OffhealRetPaladinStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "offheal"; }
    std::vector<NextAction> getDefaultActions() override;
    uint32 GetType() const override
    {
        return STRATEGY_TYPE_DPS | STRATEGY_TYPE_HEAL | STRATEGY_TYPE_MELEE;
    }
};

#endif
