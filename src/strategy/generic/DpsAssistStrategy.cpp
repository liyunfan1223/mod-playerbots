/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DpsAssistStrategy.h"

#include "Playerbots.h"

void DpsAssistStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("not dps target active", NextAction::array(0, new NextAction("dps assist", 50.0f), nullptr)));
}

void DpsAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("not dps aoe target active", NextAction::array(0, new NextAction("dps aoe", 50.0f), nullptr)));
}
