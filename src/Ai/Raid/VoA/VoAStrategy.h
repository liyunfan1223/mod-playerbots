/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VOASTRATEGY_H
#define PLAYERBOTS_VOASTRATEGY_H

#include "Strategy.h"

class RaidVoAStrategy : public Strategy
{
public:
    RaidVoAStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    virtual std::string const getName() override { return "voa"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
