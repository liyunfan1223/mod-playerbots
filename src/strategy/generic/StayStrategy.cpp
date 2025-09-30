/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "StayStrategy.h"

#include "Playerbots.h"

void StayStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "return to stay position",
        NextAction::array(0, new NextAction("return to stay position", ACTION_MOVE), nullptr)));
}

NextAction** StayStrategy::getDefaultActions() { return NextAction::array(0, new NextAction("stay", 1.0f), nullptr); }

void SitStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("sit", NextAction::array(0, new NextAction("sit", 1.5f), nullptr)));
}
