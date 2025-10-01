/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericPaladinNonCombatStrategy.h"

#include "GenericPaladinStrategyActionNodeFactory.h"
#include "Playerbots.h"
#include "AiFactory.h"

GenericPaladinNonCombatStrategy::GenericPaladinNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPaladinStrategyActionNodeFactory());
}

void GenericPaladinNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("party member dead", NextAction::array(0, new NextAction("redemption", ACTION_CRITICAL_HEAL + 10), nullptr)));
    triggers.push_back(new TriggerNode("party member almost full health", NextAction::array(0, new NextAction("flash of light on party", 25.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member medium health", NextAction::array(0, new NextAction("flash of light on party", 26.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member low health", NextAction::array(0, new NextAction("holy light on party", 27.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member critical health", NextAction::array(0, new NextAction("holy light on party", 28.0f), nullptr)));

    int specTab = AiFactory::GetPlayerSpecTab(botAI->GetBot());
    if (specTab == 0 || specTab == 1) // Holy or Protection
        triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply oil", 1.0f), nullptr)));
    if (specTab == 2) // Retribution
        triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply stone", 1.0f), nullptr)));
}
