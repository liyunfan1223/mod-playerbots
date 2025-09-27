/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MaintenanceStrategy.h"

#include "Playerbots.h"

NextAction** MaintenanceStrategy::getDefaultActions() { return nullptr; }

void MaintenanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("random", NextAction::array(0, new NextAction("clean quest log", 6.0f), nullptr)));
    triggers.push_back(new TriggerNode("random", NextAction::array(0, new NextAction("use random recipe", 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("random", NextAction::array(0, new NextAction("disenchant random item", 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("random", NextAction::array(0, new NextAction("enchant random item", 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("random", NextAction::array(0, new NextAction("smart destroy item", 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("move stuck", NextAction::array(0, new NextAction("reset", 1.0f), nullptr)));
    // triggers.push_back(new TriggerNode("move long stuck", NextAction::array(0, new NextAction("hearthstone", 0.9f), new NextAction("repop", 0.8f), nullptr)));
    triggers.push_back(new TriggerNode("random", NextAction::array(0, new NextAction("use random quest item", 0.9f), nullptr)));
    triggers.push_back(new TriggerNode("random", NextAction::array(0, new NextAction("auto share quest", 0.9f), nullptr)));
}
