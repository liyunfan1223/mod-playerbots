/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericDruidStrategy.h"
#include "Playerbots.h"

class GenericDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        GenericDruidStrategyActionNodeFactory()
        {
            creators["melee"] = &melee;
            creators["caster form"] = &caster_form;
            creators["cure poison"] = &cure_poison;
            creators["cure poison on party"] = &cure_poison_on_party;
            creators["abolish poison"] = &abolish_poison;
            creators["abolish poison on party"] = &abolish_poison_on_party;
            creators["rebirth"] = &rebirth;
            creators["entangling roots on cc"] = &entangling_roots_on_cc;
            creators["innervate"] = &innervate;
        }

    private:
        static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("melee",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* caster_form([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("caster form",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* cure_poison([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("cure poison",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* cure_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("cure poison on party",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* abolish_poison([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("abolish poison",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* abolish_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("abolish poison on party",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* rebirth([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("rebirth",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* entangling_roots_on_cc([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("entangling roots on cc",
                /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* innervate([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("innervate",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("mana potion"), nullptr),
                /*C*/ nullptr);
        }
};

GenericDruidStrategy::GenericDruidStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDruidStrategyActionNodeFactory());
}

void GenericDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    // triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("regrowth", ACTION_MEDIUM_HEAL + 2), nullptr)));
    // triggers.push_back(new TriggerNode("party member low health", NextAction::array(0, new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 1), nullptr)));
    // triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("regrowth", ACTION_CRITICAL_HEAL + 2), new NextAction("healing touch", ACTION_CRITICAL_HEAL + 2), nullptr)));
    // triggers.push_back(new TriggerNode("party member critical health", NextAction::array(0,  new NextAction("regrowth on party", ACTION_CRITICAL_HEAL + 1), new NextAction("healing touch on party", ACTION_CRITICAL_HEAL + 1), nullptr)));
	// triggers.push_back(new TriggerNode("party member dead", NextAction::array(0, new NextAction("rebirth", ACTION_HIGH + 1), nullptr)));
    // triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("innervate", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode(
		"combat party member dead",
		NextAction::array(0, new NextAction("rebirth", ACTION_HIGH + 9), NULL)));
}

void DruidCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // triggers.push_back(new TriggerNode("cure poison", NextAction::array(0, new NextAction("abolish poison", ACTION_DISPEL + 2), nullptr)));
    triggers.push_back(new TriggerNode("party member cure poison", NextAction::array(0, new NextAction("abolish poison on party", ACTION_DISPEL + 1), nullptr)));
}

void DruidBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("nature's swiftness", NextAction::array(0, new NextAction("nature's swiftness", ACTION_HIGH + 9), nullptr)));
}

void DruidCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("entangling roots", NextAction::array(0, new NextAction("entangling roots on cc", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode("entangling roots kite", NextAction::array(0, new NextAction("entangling roots", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode("hibernate", NextAction::array(0, new NextAction("hibernate on cc", ACTION_HIGH + 3), nullptr)));
}
