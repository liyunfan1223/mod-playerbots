/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RestoDruidStrategy.h"
#include "Playerbots.h"

RestoDruidStrategy::RestoDruidStrategy(PlayerbotAI* botAI) : GenericDruidStrategy(botAI)
{
    // No custom ActionNodeFactory needed
}

void RestoDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("no healer dps strategy",
                                       { NextAction("tree form", 5.0f) }));

    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        { NextAction("reach party member to heal", 39.0f) }));

    triggers.push_back(
        new TriggerNode("party member critical health",
                        {
                            NextAction("tree form",              34.1f),
                            NextAction("swiftmend on party",     34.0f),
                            NextAction("wild growth on party",   33.0f),
                            NextAction("nourish on party",       32.0f),
                            NextAction("regrowth on party",      31.0f),
                            NextAction("healing touch on party", 30.0f),
                        }));

    triggers.push_back(
        new TriggerNode("party member critical health",
                        { NextAction("nature's swiftness", 58.0f) }));

    triggers.push_back(new TriggerNode(
        "nature's swiftness active",
        { NextAction("healing touch on party", 55.0f) }));

    triggers.push_back(new TriggerNode("clearcasting",
        { NextAction("lifebloom on main tank", 13.0f) }));

    // LOW
    triggers.push_back(
        new TriggerNode("party member low health",
                        {
                            NextAction("tree form",              21.5f),
                            NextAction("swiftmend on party",     21.4f),
                            NextAction("wild growth on party",   21.3f),
                            NextAction("nourish on party",       21.2f),
                            NextAction("regrowth on party",      21.1f),
                            NextAction("healing touch on party", 21.0f),
                        }));

    // MEDIUM
    triggers.push_back(
        new TriggerNode("party member medium health",
                        {
                            NextAction("tree form",              20.5f),
                            NextAction("swiftmend on party",     20.4f),
                            NextAction("wild growth on party",   20.3f),
                            NextAction("nourish on party",       20.2f),
                            NextAction("regrowth on party",      20.1f),
                            NextAction("healing touch on party", 20.0f),
                        }));

    // ALMOST FULL
    triggers.push_back(
        new TriggerNode("party member almost full health",
                        {
                            NextAction("wild growth on party",  10.3f),
                            NextAction("rejuvenation on party", 10.2f),
                            NextAction("regrowth on party",     10.1f),
                        }));

    triggers.push_back(
        new TriggerNode("medium mana", { NextAction("innervate", 25.0f) }));

    triggers.push_back(new TriggerNode("enemy too close for spell",
                                       { NextAction("flee", 39.0f) }));
}

void DruidTranquilityStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("medium group heal setting",
                                       { NextAction("tree form", 30.6f), NextAction("tranquility", 30.5f) }));
}

class DruidBlanketStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DruidBlanketStrategyActionNodeFactory()
    {
        creators["wild growth blanket"] = &wild_growth_blanket;
        creators["rejuvenation blanket"] = &rejuvenation_blanket;
    }

private:
    static ActionNode* wild_growth_blanket([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("wild growth blanket",
                              /*P*/ { NextAction("tree form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* rejuvenation_blanket([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rejuvenation blanket",
                              /*P*/ { NextAction("tree form") },
                              /*A*/ {},
                              /*C*/ {});
    }
};

DruidBlanketStrategy::DruidBlanketStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new DruidBlanketStrategyActionNodeFactory());
}

void DruidBlanketStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("wild growth blanket", { NextAction("wild growth blanket", 8.0f) }));

    triggers.push_back(new TriggerNode("rejuvenation blanket", { NextAction("rejuvenation blanket", 6.0f) }));
}
