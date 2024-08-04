/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GuildStrategy.h"

#include "Playerbots.h"

void GuildStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("often", NextAction::array(0, new NextAction("offer petition nearby", 4.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("often", NextAction::array(0, new NextAction("guild manage nearby", 4.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("petition signed", NextAction::array(0, new NextAction("turn in petition", 10.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("buy tabard", NextAction::array(0, new NextAction("buy tabard", 10.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("leave large guild", NextAction::array(0, new NextAction("guild leave", 4.0f), nullptr)));
}
