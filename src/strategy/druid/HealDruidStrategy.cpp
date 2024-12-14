/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HealDruidStrategy.h"

#include "Playerbots.h"

class HealDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HealDruidStrategyActionNodeFactory() {
        creators["nourish on party"] = &nourtish_on_party;
        // creators["wild growth on party"] = &wild_growth_on_party;
        // creators["rejuvenation on party"] = &rejuvenation_on_party;
        // creators["regrowth on party"] = &regrowth_on_party;
    }

private:
    static ActionNode* nourtish_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("nourish on party",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("healing touch on party"), nullptr),
                              /*C*/ nullptr);
    }
    // static ActionNode* wild_growth_on_party([[maybe_unused]] PlayerbotAI* botAI)
    // {
    //     return new ActionNode("wild growth on party",
    //                           /*P*/ NextAction::array(0, new NextAction("tree form"), nullptr),
    //                           /*A*/ nullptr,
    //                           /*C*/ nullptr);
    // }
    // static ActionNode* rejuvenation_on_party([[maybe_unused]] PlayerbotAI* botAI)
    // {
    //     return new ActionNode("rejuvenation on party",
    //                           /*P*/ NextAction::array(0, new NextAction("tree form"), nullptr),
    //                           /*A*/ nullptr,
    //                           /*C*/ nullptr);
    // }
    // static ActionNode* regrowth_on_party([[maybe_unused]] PlayerbotAI* botAI)
    // {
    //     return new ActionNode("regrowth on party",
    //                           /*P*/ NextAction::array(0, new NextAction("tree form"), nullptr),
    //                           /*A*/ nullptr,
    //                           /*C*/ nullptr);
    // }
};

HealDruidStrategy::HealDruidStrategy(PlayerbotAI* botAI) : GenericDruidStrategy(botAI)
{
    actionNodeFactories.Add(new HealDruidStrategyActionNodeFactory());
}

void HealDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    // triggers.push_back(
    //     new TriggerNode("tree form", NextAction::array(0, new NextAction("tree form", ACTION_HIGH + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        NextAction::array(0, new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 9), nullptr)));

    // CRITICAL
    triggers.push_back(
        new TriggerNode("party member critical health",
                        NextAction::array(0,
                                          new NextAction("tree form", ACTION_CRITICAL_HEAL + 4.1f),
                                          new NextAction("swiftmend on party", ACTION_CRITICAL_HEAL + 4),
                                          new NextAction("regrowth on party", ACTION_CRITICAL_HEAL + 3),
                                          new NextAction("wild growth on party", ACTION_CRITICAL_HEAL + 2),
                                          new NextAction("nourish on party", ACTION_CRITICAL_HEAL + 1),
                                          // new NextAction("healing touch on party", ACTION_CRITICAL_HEAL + 0),
                                          nullptr)));

    triggers.push_back(
        new TriggerNode("party member critical health",
                        NextAction::array(0, new NextAction("nature's swiftness", ACTION_CRITICAL_HEAL + 4), nullptr)));

    triggers.push_back(new TriggerNode(
        "group heal setting",
        NextAction::array(0, 
                          new NextAction("tree form", ACTION_MEDIUM_HEAL + 2.3f),
                          new NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 2.2f),
                          new NextAction("rejuvenation on not full", ACTION_MEDIUM_HEAL + 2.1f),
                          nullptr)));

    triggers.push_back(
        new TriggerNode("medium group heal setting",
                        NextAction::array(0, 
                        new NextAction("tree form", ACTION_CRITICAL_HEAL + 0.6f),
                        new NextAction("tranquility", ACTION_CRITICAL_HEAL + 0.5f), nullptr)));

    // LOW
    triggers.push_back(
        new TriggerNode("party member low health",
                        NextAction::array(0, new NextAction("tree form", ACTION_MEDIUM_HEAL + 1.5f),
                                          new NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 1.4f),
                                          new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 1.3f),
                                          new NextAction("swiftmend on party", ACTION_MEDIUM_HEAL + 1.2),
                                          new NextAction("nourish on party", ACTION_MEDIUM_HEAL + 1.1f),
                                          nullptr)));

    // MEDIUM
    triggers.push_back(
        new TriggerNode("party member medium health",
                        NextAction::array(0, 
                                          new NextAction("tree form", ACTION_MEDIUM_HEAL + 0.5f),
                                          new NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 0.4f),
                                          new NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 0.3f),
                                          new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 0.2f),
                                          new NextAction("nourish on party", ACTION_MEDIUM_HEAL + 0.1f), nullptr)));

    // almost full
    triggers.push_back(
        new TriggerNode("party member almost full health",
                        NextAction::array(0, new NextAction("wild growth on party", ACTION_LIGHT_HEAL + 0.3f),
                                          new NextAction("rejuvenation on party", ACTION_LIGHT_HEAL + 0.2f),
                                          new NextAction("regrowth on party", ACTION_LIGHT_HEAL + 0.1f), nullptr)));

    triggers.push_back(
        new TriggerNode("medium mana", NextAction::array(0, new NextAction("innervate", ACTION_HIGH + 5), nullptr)));

    triggers.push_back(new TriggerNode("enemy too close for spell",
                                       NextAction::array(0, new NextAction("flee", ACTION_MOVE + 9), nullptr)));
}
