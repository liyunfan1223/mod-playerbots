/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MarkRtiStrategy.h"

#include "Playerbots.h"

void MarkRtiStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("no rti target", NextAction::array(0, new NextAction("mark rti", ACTION_NORMAL), nullptr)));
}
