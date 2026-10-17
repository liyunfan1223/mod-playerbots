/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEXSTRATEGY_H
#define PLAYERBOTS_NEXSTRATEGY_H

#include "Strategy.h"

class WotlkDungeonNexStrategy : public Strategy
{
public:
    WotlkDungeonNexStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "wotlk-nex"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
