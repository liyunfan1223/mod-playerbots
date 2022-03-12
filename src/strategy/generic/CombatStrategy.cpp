/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CombatStrategy.h"
#include "Playerbots.h"

void CombatStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode("enemy out of spell", NextAction::array(0, new NextAction("reach spell", ACTION_NORMAL + 9), nullptr)));
    triggers.push_back(new TriggerNode("invalid target", NextAction::array(0, new NextAction("drop target", 55), nullptr)));
    triggers.push_back(new TriggerNode("mounted", NextAction::array(0, new NextAction("check mount state", 54), nullptr)));
    triggers.push_back(new TriggerNode("out of react range", NextAction::array(0, new NextAction("flee to master", 55), nullptr)));
    triggers.push_back(new TriggerNode("combat stuck", NextAction::array(0, new NextAction("reset", 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("combat long stuck", NextAction::array(0, new NextAction("hearthstone", 0.9f), new NextAction("repop", 0.8f), nullptr)));
}
