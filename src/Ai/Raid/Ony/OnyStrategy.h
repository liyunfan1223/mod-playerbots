/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// RaidOnyxiaStrategy.h
#ifndef PLAYERBOTS_ONYSTRATEGY_H
#define PLAYERBOTS_ONYSTRATEGY_H

#include "Strategy.h"

class RaidOnyxiaStrategy : public Strategy
{
public:
    RaidOnyxiaStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "onyxia"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
