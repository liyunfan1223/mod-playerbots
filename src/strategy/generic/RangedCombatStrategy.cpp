/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RangedCombatStrategy.h"
#include "Playerbots.h"

void RangedCombatStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    CombatStrategy::InitTriggers(triggers);

    //triggers.push_back(new TriggerNode("enemy too close for shoot", NextAction::array(0, new NextAction("flee", ACTION_HIGH), nullptr)));
    triggers.push_back(new TriggerNode("enemy too close for spell", NextAction::array(0, new NextAction("flee", ACTION_HIGH), nullptr)));
}
