/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DeadStrategy.h"
#include "Playerbots.h"

void DeadStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassThroughStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("often", { NextAction("auto release", relevance) }));
    triggers.push_back(
        new TriggerNode("bg active", { NextAction("auto release", relevance) }));
    triggers.push_back(
        new TriggerNode("dead", { NextAction("find corpse", relevance) }));
    triggers.push_back(new TriggerNode("dungeon corpse run", {NextAction("dungeon corpse run", relevance + 0.5f)}));
    triggers.push_back(new TriggerNode(
        "corpse near", { NextAction("revive from corpse", relevance - 1.0f) }));
    triggers.push_back(new TriggerNode("resurrect request",
                                       { NextAction("accept resurrect", relevance) }));
    triggers.push_back(
        new TriggerNode("falling far", { NextAction("repop", relevance + 1.f) }));
    triggers.push_back(
        new TriggerNode("location stuck", { NextAction("repop", relevance + 1) }));
    triggers.push_back(new TriggerNode(
        "can self resurrect", { NextAction("self resurrect", relevance + 2.0f) }));
}

DeadStrategy::DeadStrategy(PlayerbotAI* botAI) : PassThroughStrategy(botAI) {}
