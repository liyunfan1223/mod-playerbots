/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FROSTDKSTRATEGY_H
#define PLAYERBOTS_FROSTDKSTRATEGY_H

#include "GenericDKStrategy.h"

class PlayerbotAI;

class FrostDKStrategy : public GenericDKStrategy
{
public:
    FrostDKStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "frost"; }
    std::vector<NextAction> getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
};

class FrostDKAoeStrategy : public CombatStrategy
{
public:
    FrostDKAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "frost aoe"; }
};

#endif
