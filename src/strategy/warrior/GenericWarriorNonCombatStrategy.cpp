/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericWarriorNonCombatStrategy.h"

#include "Playerbots.h"

void GenericWarriorNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply stone", 1.0f), nullptr)));
    // triggers.push_back(new TriggerNode("battle stance", NextAction::array(0, new NextAction("battle stance", 1.0f),
    // nullptr)));
}
