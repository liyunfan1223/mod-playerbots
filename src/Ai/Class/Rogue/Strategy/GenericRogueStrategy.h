/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GENERICROGUESTRATEGY_H
#define PLAYERBOTS_GENERICROGUESTRATEGY_H

#include "CombatStrategy.h"

class PlayerbotAI;

class GenericRogueStrategy : public CombatStrategy
{
public:
    GenericRogueStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "rogue"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    uint32 GetType() const override
    {
        return CombatStrategy::GetType() | STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE;
    }
};

#endif
