/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OCSTRATEGY_H
#define PLAYERBOTS_OCSTRATEGY_H

#include "Strategy.h"

class WotlkDungeonOccStrategy : public Strategy
{
public:
    WotlkDungeonOccStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "wotlk-occ"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
