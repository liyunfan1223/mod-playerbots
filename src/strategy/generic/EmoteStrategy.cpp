/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "EmoteStrategy.h"

#include "Playerbots.h"

void EmoteStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    if (sPlayerbotAIConfig->randomBotEmote)
    {
        triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("talk", 1.0f), nullptr)));
        triggers.push_back(new TriggerNode("seldom", NextAction::array(0, new NextAction("emote", 1.0f), nullptr)));
        triggers.push_back(
            new TriggerNode("receive text emote", NextAction::array(0, new NextAction("emote", 10.0f), nullptr)));
        triggers.push_back(
            new TriggerNode("receive emote", NextAction::array(0, new NextAction("emote", 10.0f), nullptr)));
    }

    if (sPlayerbotAIConfig->randomBotTalk)
    {
        triggers.push_back(new TriggerNode(
            "often",
            NextAction::array(0, new NextAction("suggest what to do", 10.0f), new NextAction("suggest dungeon", 3.0f),
                              new NextAction("suggest trade", 3.0f), nullptr)));
    }

    if (sPlayerbotAIConfig->enableGreet)
        triggers.push_back(
            new TriggerNode("new player nearby", NextAction::array(0, new NextAction("greet", 1.0f), nullptr)));

    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("rpg mount anim", 1.0f), nullptr)));
}
