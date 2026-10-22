/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRINDINGSTRATEGY_H
#define PLAYERBOTS_GRINDINGSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class GrindingStrategy : public NonCombatStrategy
{
public:
    GrindingStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "grind"; }
    uint32 GetType() const override { return STRATEGY_TYPE_DPS; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class MoveRandomStrategy : public NonCombatStrategy
{
public:
    MoveRandomStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}
    std::string const getName() override { return "move random"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};
#endif
