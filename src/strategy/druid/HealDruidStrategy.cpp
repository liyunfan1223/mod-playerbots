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

    // triggers.push_back(new TriggerNode("enemy out of spell", NextAction::array(0, new NextAction("reach spell", ACTION_NORMAL + 9), nullptr)));
    triggers.push_back(new TriggerNode("tree form", NextAction::array(0, new NextAction("tree form", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode("party member to heal out of spell range", NextAction::array(0, new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 9), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member remove curse",
        NextAction::array(0, new NextAction("remove curse on party", ACTION_DISPEL + 7), NULL)));
    // CRITICAL
    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, 
            new NextAction("swiftmend on party", ACTION_CRITICAL_HEAL + 4), 
            new NextAction("wild growth", ACTION_CRITICAL_HEAL + 3),
            new NextAction("regrowth on party", ACTION_CRITICAL_HEAL + 2),
            new NextAction("nourish on party", ACTION_CRITICAL_HEAL + 1),
            new NextAction("healing touch on party", ACTION_CRITICAL_HEAL + 0),
            NULL)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("nature's swiftness", ACTION_CRITICAL_HEAL + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "medium group heal occasion",
        NextAction::array(0, new NextAction("tranquility", ACTION_CRITICAL_HEAL + 5), NULL)));

    // LOW
    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 9),
            new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 8),
            new NextAction("swiftmend on party", ACTION_MEDIUM_HEAL + 7),
            new NextAction("nourish on party", ACTION_MEDIUM_HEAL + 6),
            new NextAction("healing touch on party", ACTION_MEDIUM_HEAL + 5), 
            NULL)));

    // MEDIUM
    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 4), 
            new NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 3),
            new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 2),
            new NextAction("nourish on party", ACTION_MEDIUM_HEAL + 1),
            NULL)));

    // almost full
    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("wild growth on party", ACTION_LIGHT_HEAL + 3),
            new NextAction("rejuvenation on party", ACTION_LIGHT_HEAL + 2), 
            new NextAction("regrowth on party", ACTION_LIGHT_HEAL + 1),
            NULL)));
    
    triggers.push_back(new TriggerNode(
        "medium mana",
        NextAction::array(0, new NextAction("innervate", ACTION_HIGH + 5), NULL)));
    
    triggers.push_back(new TriggerNode("enemy too close for spell", NextAction::array(0, new NextAction("flee", ACTION_MOVE + 9), nullptr)));
}
