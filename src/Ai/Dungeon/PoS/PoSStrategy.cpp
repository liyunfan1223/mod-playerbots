/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PoSStrategy.h"
#include "PoSMultipliers.h"

void WotlkDungeonPoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("ick and krick",
        { NextAction("ick and krick", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("tyrannus",
        { NextAction("tyrannus", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("garfrost saronite",
        { NextAction("garfrost dodge saronite", ACTION_EMERGENCY + 5) }));
    triggers.push_back(new TriggerNode("garfrost tank position",
        { NextAction("garfrost tank position", ACTION_RAID + 3) }));
    triggers.push_back(new TriggerNode("garfrost permafrost",
        { NextAction("garfrost take cover", ACTION_RAID + 4) }));
    triggers.push_back(new TriggerNode("pos ground hazard",
        { NextAction("pos avoid ground hazard", ACTION_EMERGENCY + 4) }));
    triggers.push_back(new TriggerNode("tyrannus overlords brand",
        { NextAction("tyrannus brand stop", ACTION_EMERGENCY + 3) }));
    triggers.push_back(new TriggerNode("tyrannus overlords brand faded",
        { NextAction("tyrannus brand restore pets", ACTION_RAID) }));
}

void WotlkDungeonPoSStrategy::InitReactionTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("tyrannus brand stop cast",
        { NextAction("tyrannus brand stop cast", ACTION_EMERGENCY + 3) }));
}

void WotlkDungeonPoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new IckAndKrickMultiplier(botAI));
    multipliers.push_back(new PoSHazardMultiplier(botAI));
    multipliers.push_back(new TyrannusBrandMultiplier(botAI));
    multipliers.push_back(new GarfrostMultiplier(botAI));
}
