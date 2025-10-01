/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ReturnStrategy.h"

#include "Playerbots.h"

void ReturnStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("return", NextAction::array(0, new NextAction("set return position", 1.5f),
                                                                   new NextAction("return", 1.0f), nullptr)));
}
