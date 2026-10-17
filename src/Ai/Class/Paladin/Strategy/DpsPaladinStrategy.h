/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DPSPALADINSTRATEGY_H
#define PLAYERBOTS_DPSPALADINSTRATEGY_H

#include "GenericPaladinStrategy.h"

class PlayerbotAI;

class DpsPaladinStrategy : public GenericPaladinStrategy
{
public:
    DpsPaladinStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "ret"; }
    std::vector<NextAction> getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
};

#endif
