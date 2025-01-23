/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FireMageStrategy.h"

#include "Playerbots.h"

NextAction** FireMageStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("fireball", ACTION_DEFAULT + 0.2f),
                             new NextAction("fire blast", ACTION_DEFAULT + 0.1f), // cast during movement
                             new NextAction("shoot", ACTION_DEFAULT), NULL);
}

void FireMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    // triggers.push_back(new TriggerNode("pyroblast", NextAction::array(0, new NextAction("pyroblast", 10.0f),
    // nullptr)));
    triggers.push_back(
        new TriggerNode("hot streak", NextAction::array(0, new NextAction("pyroblast", 25.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("combustion", NextAction::array(0, new NextAction("combustion", 50.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("living bomb", NextAction::array(0, new NextAction("living bomb", 19.0f), nullptr)));
    // triggers.push_back(new TriggerNode("enemy too close for spell", NextAction::array(0, new NextAction("dragon's
    // breath", 70.0f), nullptr)));
}

void FireMageAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0,
            new NextAction("dragon's breath", 24.0f),
            new NextAction("flamestrike", 23.0f), 
            new NextAction("blast wave", 22.0f), 
            new NextAction("living bomb on attackers", 21.0f), 
            new NextAction("blizzard", 20.0f), nullptr)));
}
