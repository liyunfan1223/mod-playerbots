/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HealPriestStrategy.h"
#include "GenericPriestStrategyActionNodeFactory.h"
#include "Playerbots.h"

HealPriestStrategy::HealPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPriestStrategyActionNodeFactory());
}

NextAction** HealPriestStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("reach party member to heal", 1.0f), nullptr);
}

void HealPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("almost full health", NextAction::array(0, new NextAction("renew", 43.f), nullptr)));
    triggers.push_back(new TriggerNode("party member almost full health", NextAction::array(0, new NextAction("heal on party", 41.0f), new NextAction("renew on party", 40.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member medium health", NextAction::array(0, new NextAction("greater heal on party", 47.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member low health", NextAction::array(0, new NextAction("power word: shield on party", 51.0f), new NextAction("greater heal on party", 50.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member to heal out of spell range", NextAction::array(0, new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe heal", NextAction::array(0, new NextAction("prayer of mending", 49.0f), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe heal", NextAction::array(0, new NextAction("circle of healing", 48.0f), nullptr)));
    triggers.push_back(new TriggerNode("binding heal", NextAction::array(0, new NextAction("binding heal", 52.0f), nullptr)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("shadowfiend", ACTION_HIGH), nullptr)));
}
