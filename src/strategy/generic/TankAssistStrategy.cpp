/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TankAssistStrategy.h"

#include "Playerbots.h"

void TankAssistStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("tank assist", NextAction::array(0, new NextAction("tank assist", 50.0f), nullptr)));
}
