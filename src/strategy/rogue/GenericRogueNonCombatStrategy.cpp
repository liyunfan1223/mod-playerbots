/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericRogueNonCombatStrategy.h"
#include "Playerbots.h"

void GenericRogueNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("player has flag", NextAction::array(0, new NextAction("sprint", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode("enemy flagcarrier near", NextAction::array(0, new NextAction("sprint", ACTION_EMERGENCY + 2), nullptr)));
    // triggers.push_back(new TriggerNode("unstealth", NextAction::array(0, new NextAction("unstealth", 1.0f), nullptr)));
    // triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply poison", 1.0f), nullptr)));

    triggers.push_back(new TriggerNode(
		"main hand weapon no enchant",
		NextAction::array(0, new NextAction("use instant poison on main hand", 20.0f), NULL)));
	
	triggers.push_back(new TriggerNode(
		"off hand weapon no enchant",
		NextAction::array(0, new NextAction("use deadly poison on off hand", 19.0f), NULL)));

	// triggers.push_back(new TriggerNode(
	// 	"off hand weapon no enchant",
	// 	NextAction::array(0, new NextAction("use instant poison", 18.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"often",
		NextAction::array(0, new NextAction("unstealth", 10.0f), NULL)));
}
