/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "StayStrategy.h"

#include "Playerbots.h"

NextAction** StayStrategy::getDefaultActions() { return NextAction::array(0, new NextAction("stay", 1.0f), nullptr); }

void SitStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("sit", NextAction::array(0, new NextAction("sit", 1.5f), nullptr)));
}
