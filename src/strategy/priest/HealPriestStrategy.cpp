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
    return NextAction::array(0, new NextAction("shoot", ACTION_DEFAULT), nullptr);
}

void HealPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    // triggers.push_back(new TriggerNode(
    //     "enemy out of spell",
    //     NextAction::array(0, new NextAction("reach spell", ACTION_NORMAL + 9), NULL)));

	// triggers.push_back(new TriggerNode(
	// 	"medium aoe heal",
	// 	NextAction::array(0, 
    //         new NextAction("circle of healing", ACTION_MEDIUM_HEAL + 8), 
    //         // new NextAction("power word: shield on almost full health below", ACTION_MEDIUM_HEAL + 7),
    //         NULL)));

    triggers.push_back(new TriggerNode(
		"group heal occasion",
        NextAction::array(0, 
            new NextAction("circle of healing", ACTION_MEDIUM_HEAL + 8), 
            new NextAction("power word: shield on almost full health below", ACTION_MEDIUM_HEAL + 7),
            new NextAction("prayer of healing on party", ACTION_MEDIUM_HEAL + 6),
            NULL)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
		NextAction::array(0,
            new NextAction("power word: shield on party", ACTION_CRITICAL_HEAL + 6),
            new NextAction("penance on party", ACTION_CRITICAL_HEAL + 4), 
            new NextAction("flash heal on party", ACTION_CRITICAL_HEAL + 3),
            new NextAction("prayer of mending on party", ACTION_CRITICAL_HEAL + 2),
            NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
		NextAction::array(0,
            new NextAction("power word: shield on party", ACTION_MEDIUM_HEAL + 4),
            new NextAction("penance on party", ACTION_MEDIUM_HEAL + 2), 
            new NextAction("circle of healing", ACTION_MEDIUM_HEAL + 2),
            new NextAction("prayer of mending on party", ACTION_MEDIUM_HEAL + 1),
            new NextAction("flash heal on party", ACTION_MEDIUM_HEAL + 0),
            NULL)));

	triggers.push_back(new TriggerNode(
        "party member medium health",
		NextAction::array(0,
            new NextAction("power word: shield on party", ACTION_LIGHT_HEAL + 9),
            new NextAction("penance on party", ACTION_LIGHT_HEAL + 7), 
            new NextAction("circle of healing", ACTION_LIGHT_HEAL + 7),
            new NextAction("prayer of mending on party", ACTION_LIGHT_HEAL + 6),
            new NextAction("flash heal on party", ACTION_LIGHT_HEAL + 5),
            // new NextAction("renew on party", ACTION_LIGHT_HEAL + 8),
            NULL)));
    
    triggers.push_back(new TriggerNode(
        "party member almost full health",
		NextAction::array(0, 
            // new NextAction("penance on party", ACTION_LIGHT_HEAL + 3), 
            new NextAction("renew on party", ACTION_LIGHT_HEAL + 2),
            NULL)));

    // triggers.push_back(new TriggerNode("almost full health", NextAction::array(0, new NextAction("renew", 43.f), nullptr)));
    // triggers.push_back(new TriggerNode("party member almost full health", NextAction::array(0, new NextAction("heal on party", 41.0f), new NextAction("renew on party", 40.0f), nullptr)));
    // triggers.push_back(new TriggerNode("party member medium health", NextAction::array(0, new NextAction("greater heal on party", 47.0f), nullptr)));
    // triggers.push_back(new TriggerNode("party member low health", NextAction::array(0, new NextAction("power word: shield on party", 51.0f), new NextAction("greater heal on party", 50.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member to heal out of spell range", NextAction::array(0, new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 10), nullptr)));
    // triggers.push_back(new TriggerNode("medium aoe heal", NextAction::array(0, new NextAction("prayer of mending", 49.0f), nullptr)));
    // triggers.push_back(new TriggerNode("medium aoe heal", NextAction::array(0, new NextAction("circle of healing", 48.0f), nullptr)));
    // triggers.push_back(new TriggerNode("binding heal", NextAction::array(0, new NextAction("binding heal", 52.0f), nullptr)));
    // triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("shadowfiend", ACTION_HIGH), nullptr)));

    triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("pain suppression", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode("protect party member", NextAction::array(0, new NextAction("pain suppression on party", ACTION_EMERGENCY), nullptr)));
    
}
