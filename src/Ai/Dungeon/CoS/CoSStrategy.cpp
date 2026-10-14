/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CoSStrategy.h"
#include "CoSMultipliers.h"

void WotlkDungeonCoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Meathook
    // Can tank this in a fixed position to allow healer to LoS the stun, probably not necessary

    // Salramm the Fleshcrafter
    triggers.push_back(new TriggerNode("explode ghoul",
        { NextAction("explode ghoul spread", ACTION_MOVE + 5) }));

    // Chrono-Lord Epoch
    // Not sure if this actually works, I think I've seen him charge melee characters..?
    triggers.push_back(new TriggerNode("epoch ranged",
        { NextAction("epoch stack", ACTION_MOVE + 5) }));

    // Mal'Ganis

    // Infinite Corruptor (Heroic only)
}

void WotlkDungeonCoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new EpochMultiplier(botAI));
}
