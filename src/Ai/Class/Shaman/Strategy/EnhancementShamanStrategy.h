/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ENHANCEMENTSHAMANSTRATEGY_H
#define PLAYERBOTS_ENHANCEMENTSHAMANSTRATEGY_H

#include "GenericShamanStrategy.h"

class PlayerbotAI;

class EnhancementShamanStrategy : public GenericShamanStrategy
{
public:
    EnhancementShamanStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::vector<NextAction> getDefaultActions() override;
    std::string const getName() override { return "enh"; }
    uint32 GetType() const override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
};

#endif
