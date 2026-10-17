/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOLYPRIESTSTRATEGY_H
#define PLAYERBOTS_HOLYPRIESTSTRATEGY_H

#include "GenericPriestStrategy.h"

class PlayerbotAI;

class HolyDpsPriestStrategy : public GenericPriestStrategy
{
public:
    HolyDpsPriestStrategy(PlayerbotAI* botAI);

    std::vector<NextAction> getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "holy dps"; }
    uint32 GetType() const override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
};

class HolyHealPriestStrategy : public GenericPriestStrategy
{
public:
    HolyHealPriestStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::vector<NextAction> getDefaultActions() override;
    std::string const getName() override { return "holy heal"; }
    uint32 GetType() const override { return STRATEGY_TYPE_HEAL | STRATEGY_TYPE_RANGED; }
};

#endif
