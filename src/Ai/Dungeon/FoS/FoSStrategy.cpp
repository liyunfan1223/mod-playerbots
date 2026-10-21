/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FoSStrategy.h"
#include "FoSMultipliers.h"

void WotlkDungeonFoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("move from bronjahm",
        { NextAction("move from bronjahm", ACTION_MOVE + 5) }));
    triggers.push_back(new TriggerNode("switch to soul fragment",
        { NextAction("attack corrupted soul fragment", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("bronjahm position",
        { NextAction("bronjahm group position", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("devourer wailing souls",
        { NextAction("devourer wailing souls", ACTION_EMERGENCY + 5) }));
    triggers.push_back(new TriggerNode("devourer well of souls",
        { NextAction("devourer well of souls", ACTION_EMERGENCY + 4) }));
    triggers.push_back(new TriggerNode("devourer mirrored soul",
        { NextAction("devourer stop damage", ACTION_EMERGENCY + 3) }));
    triggers.push_back(new TriggerNode("devourer phantom blast",
        { NextAction("devourer interrupt phantom blast", ACTION_EMERGENCY + 2) }));
    triggers.push_back(new TriggerNode("devourer mirrored soul faded",
        { NextAction("devourer restore pets", ACTION_RAID) }));
}

void WotlkDungeonFoSStrategy::InitReactionTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("devourer stop cast",
        { NextAction("devourer stop cast", ACTION_EMERGENCY + 3) }));
}

void WotlkDungeonFoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new BronjahmMultiplier(botAI));
    multipliers.push_back(new DevourerOfSoulsMultiplier(botAI));
}
