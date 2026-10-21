/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HoRStrategy.h"
#include "HoRMultipliers.h"

void WotlkDungeonHoRStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/) {}

void WotlkDungeonHoRStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MarwynSharedSufferingMultiplier(botAI));
}
