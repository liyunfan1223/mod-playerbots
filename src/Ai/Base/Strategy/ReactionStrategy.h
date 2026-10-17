/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _PLAYERBOT_REACTIONSTRATEGY_H
#define _PLAYERBOT_REACTIONSTRATEGY_H

#include "Strategy.h"

class ReactionStrategy : public Strategy
{
public:
    ReactionStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "react"; }
    uint32 GetType() const override { return STRATEGY_TYPE_REACTION; }

protected:
    void InitReactionTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
