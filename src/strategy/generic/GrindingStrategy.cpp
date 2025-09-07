/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GrindingStrategy.h"

#include "Playerbots.h"

NextAction** GrindingStrategy::getDefaultActions()
{
    return NextAction::array(0,
        new NextAction("drink", 4.2f),
        new NextAction("food", 4.1f),
        nullptr);
}

void GrindingStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // reduce lower than loot
    triggers.push_back(
        new TriggerNode("no target", NextAction::array(0, new NextAction("attack anything", 4.0f), nullptr)));
}

void MoveRandomStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("move random", 1.5f), NULL)));
}