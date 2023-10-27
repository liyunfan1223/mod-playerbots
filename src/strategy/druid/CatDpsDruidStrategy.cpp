/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CatDpsDruidStrategy.h"
#include "AiObjectContext.h"

class CatDpsDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        CatDpsDruidStrategyActionNodeFactory()
        {
            creators["faerie fire (feral)"] = &faerie_fire_feral;
            creators["melee"] = &melee;
            creators["feral charge - cat"] = &feral_charge_cat;
            creators["cat form"] = &cat_form;
            creators["claw"] = &claw;
            creators["mangle (cat)"] = &mangle_cat;
            creators["rake"] = &rake;
            creators["ferocious bite"] = &ferocious_bite;
            creators["rip"] = &rip;
            creators["pounce"] = &pounce;
            creators["ravage"] = &ravage;
        }

    private:
        static ActionNode* faerie_fire_feral([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("faerie fire (feral)",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("melee",
                /*P*/ NextAction::array(0, new NextAction("feral charge - cat"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* feral_charge_cat([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("feral charge - cat",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("reach melee"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* cat_form([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("cat form",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* claw([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("claw",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* mangle_cat([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("mangle (cat)",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("claw"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* rake([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("rake",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* ferocious_bite([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("ferocious bite",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("rip"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* rip([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("rip",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* pounce([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode("pounce",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("ravage"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* ravage([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode("ravage",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("shred"), nullptr),
                /*C*/ nullptr);
        }
};

CatDpsDruidStrategy::CatDpsDruidStrategy(PlayerbotAI* botAI) : FeralDruidStrategy(botAI)
{
    actionNodeFactories.Add(new CatDpsDruidStrategyActionNodeFactory());
}

NextAction** CatDpsDruidStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("mangle (cat)", ACTION_DEFAULT + 0.1f), nullptr);
}

void CatDpsDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("cat form", NextAction::array(0, new NextAction("cat form", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode("rake", NextAction::array(0, new NextAction("rake", ACTION_NORMAL + 5), nullptr)));
    triggers.push_back(new TriggerNode("combo points available", NextAction::array(0, new NextAction("ferocious bite", ACTION_NORMAL + 9), nullptr)));
    triggers.push_back(new TriggerNode("medium threat", NextAction::array(0, new NextAction("cower", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode("faerie fire (feral)", NextAction::array(0, new NextAction("faerie fire (feral)", ACTION_HIGH), nullptr)));
	triggers.push_back(new TriggerNode("tiger's fury", NextAction::array(0, new NextAction("tiger's fury", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode("behind target", NextAction::array(0, new NextAction("pounce", ACTION_HIGH + 1), nullptr)));
    //triggers.push_back(new TriggerNode("player has no flag", NextAction::array(0, new NextAction("prowl", ACTION_HIGH), nullptr)));
    //triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("prowl", ACTION_INTERRUPT + 1), nullptr)));
    triggers.push_back(new TriggerNode("player has flag", NextAction::array(0, new NextAction("dash", ACTION_EMERGENCY), nullptr)));
    triggers.push_back(new TriggerNode("enemy flagcarrier near", NextAction::array(0, new NextAction("dash", ACTION_EMERGENCY), nullptr)));
}

void CatAoeDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("swipe (cat)", ACTION_HIGH + 2), nullptr)));
}

