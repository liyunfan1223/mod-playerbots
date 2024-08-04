/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TellTargetStrategy.h"

#include "Playerbots.h"

void TellTargetStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("target changed", NextAction::array(0, new NextAction("tell target", 51.0f), nullptr)));
}
