/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HealDruidStrategy.h"
#include "Playerbots.h"

class HealDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        HealDruidStrategyActionNodeFactory() { }
};

HealDruidStrategy::HealDruidStrategy(PlayerbotAI* botAI) : GenericDruidStrategy(botAI)
{
    actionNodeFactories.Add(new HealDruidStrategyActionNodeFactory());
}

void HealDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("enemy out of spell", NextAction::array(0, new NextAction("reach spell", ACTION_NORMAL + 9), nullptr)));
    triggers.push_back(new TriggerNode("tree form", NextAction::array(0, new NextAction("tree form", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL + 2), nullptr)));
    triggers.push_back(new TriggerNode("party member medium health", NextAction::array(0, new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 1), nullptr)));
    triggers.push_back(new TriggerNode("almost full health", NextAction::array(0, new NextAction("rejuvenation", ACTION_LIGHT_HEAL + 2), nullptr)));
    triggers.push_back(new TriggerNode("party member almost full health", NextAction::array(0, new NextAction("rejuvenation on party", ACTION_LIGHT_HEAL + 1), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe heal", NextAction::array(0, new NextAction("tranquility", ACTION_MEDIUM_HEAL + 3), nullptr)));
    triggers.push_back(new TriggerNode("party member to heal out of spell range", NextAction::array(0, new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 1), nullptr)));
}
