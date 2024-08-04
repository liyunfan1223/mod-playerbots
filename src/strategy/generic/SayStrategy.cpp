/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SayStrategy.h"

#include "Playerbots.h"

void SayStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("critical health",
                                       NextAction::array(0, new NextAction("say::critical health", 99.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("say::low health", 99.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("say::low mana", 99.0f), nullptr)));
    triggers.push_back(new TriggerNode("tank aoe", NextAction::array(0, new NextAction("say::taunt", 99.0f), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("say::aoe", 99.0f), nullptr)));
}
