/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericDruidNonCombatStrategy.h"
#include "Playerbots.h"

class GenericDruidNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        GenericDruidNonCombatStrategyActionNodeFactory()
        {
            creators["thorns"] = &thorns;
            creators["thorns on party"] = &thorns_on_party;
            creators["mark of the wild"] = &mark_of_the_wild;
            creators["mark of the wild on party"] = &mark_of_the_wild_on_party;
            // creators["innervate"] = &innervate;
            creators["regrowth_on_party"] = &regrowth_on_party;
            creators["rejuvenation on party"] = &rejuvenation_on_party;
            creators["remove curse on party"] = &remove_curse_on_party;
            creators["abolish poison on party"] = &abolish_poison_on_party;
        }

    private:
        static ActionNode* thorns([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode("thorns",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* thorns_on_party([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode("thorns on party",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* mark_of_the_wild([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("mark of the wild",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* mark_of_the_wild_on_party([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("mark of the wild on party",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }
        static ActionNode* regrowth_on_party(PlayerbotAI* ai)
        {
            return new ActionNode ("regrowth on party",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ NULL,
                /*C*/ NULL);
        }
        static ActionNode* rejuvenation_on_party(PlayerbotAI* ai)
        {
            return new ActionNode ("rejuvenation on party",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ NULL,
                /*C*/ NULL);
        }
        static ActionNode* remove_curse_on_party(PlayerbotAI* ai)
        {
            return new ActionNode ("remove curse on party",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ NULL,
                /*C*/ NULL);
        }
        static ActionNode* abolish_poison_on_party(PlayerbotAI* ai)
        {
            return new ActionNode ("abolish poison on party",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ NULL,
                /*C*/ NULL);
        }
        // static ActionNode* innervate([[maybe_unused]] PlayerbotAI* botAI)
        // {
        //     return new ActionNode ("innervate",
        //         /*P*/ nullptr,
        //         /*A*/ NextAction::array(0, new NextAction("drink"), nullptr),
        //         /*C*/ nullptr);
        // }
};

GenericDruidNonCombatStrategy::GenericDruidNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDruidNonCombatStrategyActionNodeFactory());
}

void GenericDruidNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("mark of the wild", NextAction::array(0, new NextAction("mark of the wild", 14.0f), nullptr)));
    // triggers.push_back(new TriggerNode("thorns", NextAction::array(0, new NextAction("thorns", 12.0f), nullptr)));
    // triggers.push_back(new TriggerNode("cure poison", NextAction::array(0, new NextAction("abolish poison", 21.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member cure poison", NextAction::array(0, new NextAction("abolish poison on party", 20.0f), nullptr)));
	triggers.push_back(new TriggerNode("party member dead", NextAction::array(0, new NextAction("revive", ACTION_CRITICAL_HEAL + 10), nullptr)));
    // triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("innervate", ACTION_EMERGENCY + 5), nullptr)));
    // triggers.push_back(new TriggerNode("swimming", NextAction::array(0, new NextAction("aquatic form", 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply oil", 1.0f), nullptr)));

    triggers.push_back(new TriggerNode("party member critical health",
        NextAction::array(0, new NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 5),
                                           new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 6), NULL)));

    triggers.push_back(new TriggerNode("party member low health",
        NextAction::array(0, new NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 3), 
                                           new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 4), NULL)));

    triggers.push_back(new TriggerNode("party member medium health",
        NextAction::array(0, new NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 1),
                                           new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 2), NULL)));

    triggers.push_back(new TriggerNode("party member almost full health",
        NextAction::array(0, new NextAction("rejuvenation on party", ACTION_LIGHT_HEAL + 2), NULL)));
    
    triggers.push_back(new TriggerNode("party member remove curse",
        NextAction::array(0, new NextAction("remove curse on party", ACTION_DISPEL + 7), NULL)));
}

GenericDruidBuffStrategy::GenericDruidBuffStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDruidNonCombatStrategyActionNodeFactory());
}

void GenericDruidBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("mark of the wild on party", NextAction::array(0, new NextAction("mark of the wild on party", 13.0f), nullptr)));
    triggers.push_back(new TriggerNode("thorns on main tank", NextAction::array(0, new NextAction("thorns on main tank", 11.0f), nullptr)));
}
