/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EmoteStrategy.h"
#include "Playerbots.h"

void EmoteStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    if (sPlayerbotAIConfig.RandomBotEmote)
    {
        triggers.push_back(new TriggerNode("often", { NextAction("talk", 1.0f) }));
        triggers.push_back(new TriggerNode("seldom", { NextAction("emote", 1.0f) }));
        triggers.push_back(
            new TriggerNode("receive text emote", { NextAction("emote", 10.0f) }));
        triggers.push_back(
            new TriggerNode("receive emote", { NextAction("emote", 10.0f) }));
    }

    if (sPlayerbotAIConfig.RandomBotTalk)
    {
        triggers.push_back(new TriggerNode(
            "often",
            { NextAction("suggest what to do", 10.0f), NextAction("suggest dungeon", 3.0f),
                              NextAction("suggest trade", 3.0f) }));
    }

    if (sPlayerbotAIConfig.EnableGreet)
        triggers.push_back(
            new TriggerNode("new player nearby", { NextAction("greet", 1.0f) }));

    triggers.push_back(new TriggerNode("often", { NextAction("rpg mount anim", 1.0f) }));
}
