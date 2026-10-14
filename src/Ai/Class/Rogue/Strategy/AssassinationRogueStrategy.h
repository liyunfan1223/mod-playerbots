/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ASSASSINATIONROGUESTRATEGY_H
#define PLAYERBOTS_ASSASSINATIONROGUESTRATEGY_H

#include "GenericRogueStrategy.h"

class AssassinationRogueStrategy : public GenericRogueStrategy
{
public:
    AssassinationRogueStrategy(PlayerbotAI* botAI);

public:
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    virtual std::string const getName() override { return "assassin"; }
    virtual std::vector<NextAction> getDefaultActions() override;
};

#endif
