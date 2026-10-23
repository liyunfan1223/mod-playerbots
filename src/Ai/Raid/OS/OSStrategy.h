/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OSSTRATEGY_H
#define PLAYERBOTS_OSSTRATEGY_H

#include "Strategy.h"

class RaidOsStrategy : public Strategy
{
public:
    RaidOsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    virtual std::string const getName() override { return "wotlk-os"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
