/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FOSSTRATEGY_H
#define PLAYERBOTS_FOSSTRATEGY_H

#include "Strategy.h"

class WotlkDungeonFoSStrategy : public Strategy
{
public:
    WotlkDungeonFoSStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "wotlk-fos"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
