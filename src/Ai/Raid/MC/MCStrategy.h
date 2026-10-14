/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MCSTRATEGY_H
#define PLAYERBOTS_MCSTRATEGY_H

#include "Strategy.h"

class RaidMcStrategy : public Strategy
{
public:
    RaidMcStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "moltencore"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;
    bool HasTargetExclusions() const override { return true; }
};

#endif
