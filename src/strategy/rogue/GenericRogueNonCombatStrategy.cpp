/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericRogueNonCombatStrategy.h"

#include "Playerbots.h"

class GenericRogueNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericRogueNonCombatStrategyActionNodeFactory()
    {
        creators["use deadly poison on off hand"] = &use_deadly_poison_on_off_hand;
    }

private:
    static ActionNode* use_deadly_poison_on_off_hand(PlayerbotAI* botAI)
    {
        return new ActionNode("use deadly poison on off hand",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("use instant poison on off hand"), nullptr),
                              /*C*/ nullptr);
    }
};

GenericRogueNonCombatStrategy::GenericRogueNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericRogueNonCombatStrategyActionNodeFactory());
}

void GenericRogueNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("player has flag",
                                       NextAction::array(0, new NextAction("sprint", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode("enemy flagcarrier near",
                                       NextAction::array(0, new NextAction("sprint", ACTION_EMERGENCY + 2), nullptr)));
    // triggers.push_back(new TriggerNode("unstealth", NextAction::array(0, new NextAction("unstealth", 1.0f),
    // nullptr))); triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply
    // poison", 1.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("main hand weapon no enchant",
                        NextAction::array(0, new NextAction("use instant poison on main hand", 20.0f), NULL)));

    triggers.push_back(
        new TriggerNode("off hand weapon no enchant",
                        NextAction::array(0, new NextAction("use deadly poison on off hand", 19.0f), NULL)));

    // triggers.push_back(new TriggerNode(
    // 	"off hand weapon no enchant",
    // 	NextAction::array(0, new NextAction("use instant poison", 18.0f), NULL)));

    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("unstealth", 30.0f), NULL)));
}
