/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UNHOLYDKSTRATEGY_H
#define PLAYERBOTS_UNHOLYDKSTRATEGY_H

#include "GenericDKStrategy.h"

class PlayerbotAI;

class UnholyDKStrategy : public GenericDKStrategy
{
public:
    UnholyDKStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "unholy"; }
    std::vector<NextAction> getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
};

class UnholyDKAoeStrategy : public CombatStrategy
{
public:
    UnholyDKAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "unholy aoe"; }
};

#endif
