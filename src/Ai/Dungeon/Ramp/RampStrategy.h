/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAMPSTRATEGY_H
#define PLAYERBOTS_RAMPSTRATEGY_H

#include "Strategy.h"

class TbcDungeonHellfireRampartsStrategy : public Strategy
{
public:
    TbcDungeonHellfireRampartsStrategy(PlayerbotAI* botAI) : Strategy(botAI)
    {
    }

    std::string const getName() override { return "tbc-ramp"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
