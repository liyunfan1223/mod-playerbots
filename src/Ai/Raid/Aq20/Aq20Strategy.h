/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ20STRATEGY_H
#define PLAYERBOTS_AQ20STRATEGY_H

#include "Strategy.h"

class RaidAq20Strategy : public Strategy
{
public:
    RaidAq20Strategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    virtual std::string const getName() override { return "aq20"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    // virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
