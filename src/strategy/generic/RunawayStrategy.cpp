/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RunawayStrategy.h"

#include "Playerbots.h"

void RunawayStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("enemy too close for spell", NextAction::array(0, new NextAction("runaway", 50.0f), nullptr)));
}
