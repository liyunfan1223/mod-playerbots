/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDSTRATEGY_H
#define PLAYERBOTS_ULDSTRATEGY_H

#include "Strategy.h"

class RaidUlduarStrategy : public Strategy
{
public:
    RaidUlduarStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    virtual std::string const getName() override { return "ulduar"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
