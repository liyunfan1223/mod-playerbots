/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DTKStrategy.h"
#include "DTKMultipliers.h"

void WotlkDungeonDTKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Trollgore
    triggers.push_back(new TriggerNode("corpse explode",
        { NextAction("corpse explode spread", ACTION_MOVE + 5) }));

    // Novos the Summoner
    // TODO: Can be improved - it's a pretty easy fight but complex to program, revisit if needed
    triggers.push_back(new TriggerNode("arcane field",
        { NextAction("avoid arcane field", ACTION_MOVE + 5) }));
    triggers.push_back(new TriggerNode("arcane field",
        { NextAction("novos positioning", ACTION_MOVE + 4) }));
    triggers.push_back(new TriggerNode("arcane field",
        { NextAction("novos target priority", ACTION_NORMAL + 1) }));

    // King Dred
    // TODO: tremor totem, or general anti-fear strat development

    //The Prophet Tharon'ja
    triggers.push_back(new TriggerNode("gift of tharon'ja",
        { NextAction("touch of life", ACTION_NORMAL + 5) }));
    triggers.push_back(new TriggerNode("gift of tharon'ja",
        { NextAction("bone armor", ACTION_NORMAL + 4) }));
    // Run ranged chars (who would normally stand at range) into melee, to dps in skeleton form
    triggers.push_back(new TriggerNode("tharon'ja out of melee",
        { NextAction("reach melee", ACTION_NORMAL + 3) }));
    triggers.push_back(new TriggerNode("gift of tharon'ja",
        { NextAction("taunt", ACTION_NORMAL + 2) }));
    triggers.push_back(new TriggerNode("gift of tharon'ja",
        { NextAction("slaying strike", ACTION_NORMAL + 2) }));
}

void WotlkDungeonDTKStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new NovosMultiplier(botAI));
    multipliers.push_back(new TharonjaMultiplier(botAI));
}
