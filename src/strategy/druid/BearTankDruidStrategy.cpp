/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "BearTankDruidStrategy.h"
#include "Playerbots.h"

class BearTankDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        BearTankDruidStrategyActionNodeFactory()
        {
            creators["melee"] = &melee;
            creators["feral charge - bear"] = &feral_charge_bear;
            creators["swipe (bear)"] = &swipe_bear;
            creators["faerie fire (feral)"] = &faerie_fire_feral;
            creators["bear form"] = &bear_form;
            creators["dire bear form"] = &dire_bear_form;
            creators["mangle (bear)"] = &mangle_bear;
            creators["maul"] = &maul;
            creators["bash"] = &bash;
            creators["swipe"] = &swipe;
            creators["lacerate"] = &lacerate;
            creators["demoralizing roar"] = &demoralizing_roar;
            creators["taunt spell"] = &growl;
        }

    private:
        static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("melee",
                /*P*/ NextAction::array(0, new NextAction("feral charge - bear"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* feral_charge_bear([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("feral charge - bear",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("reach melee"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* swipe_bear([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("swipe (bear)",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* faerie_fire_feral([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("faerie fire (feral)",
                /*P*/ NextAction::array(0, new NextAction("feral charge - bear"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* bear_form([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("bear form",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* dire_bear_form([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("dire bear form",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("bear form"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* mangle_bear([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("mangle (bear)",
                /*P*/ nullptr,
                // /*A*/ NextAction::array(0, new NextAction("lacerate"), nullptr),
                nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* maul([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("maul",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* bash([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("bash",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* swipe([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("swipe",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* lacerate([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("lacerate",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("maul"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* growl([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("growl",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* demoralizing_roar([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("demoralizing roar",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }
};

BearTankDruidStrategy::BearTankDruidStrategy(PlayerbotAI* botAI) : FeralDruidStrategy(botAI)
{
    actionNodeFactories.Add(new BearTankDruidStrategyActionNodeFactory());
}

NextAction** BearTankDruidStrategy::getDefaultActions()
{
    return NextAction::array(0,
            new NextAction("mangle (bear)", ACTION_DEFAULT + 0.5f),
            new NextAction("faerie fire (feral)", ACTION_DEFAULT + 0.4f),
            new NextAction("lacerate", ACTION_DEFAULT + 0.3f),
            new NextAction("maul", ACTION_DEFAULT + 0.2f),
            new NextAction("enrage", ACTION_DEFAULT + 0.1f),
            new NextAction("melee", ACTION_DEFAULT),
            nullptr);
}

void BearTankDruidStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("feral charge - bear", ACTION_NORMAL + 8), nullptr)));
    triggers.push_back(new TriggerNode("thorns", NextAction::array(0, new NextAction("thorns", ACTION_HIGH + 9), nullptr)));
    triggers.push_back(new TriggerNode("bear form", NextAction::array(0, new NextAction("dire bear form", ACTION_HIGH + 8), nullptr)));
    triggers.push_back(new TriggerNode("faerie fire (feral)", NextAction::array(0, new NextAction("faerie fire (feral)", ACTION_HIGH + 7), nullptr)));
    triggers.push_back(new TriggerNode("lose aggro", NextAction::array(0, new NextAction("growl", ACTION_HIGH + 8), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("demoralizing roar", ACTION_HIGH + 6), new NextAction("swipe (bear)", ACTION_HIGH + 6), nullptr)));
    triggers.push_back(new TriggerNode("light aoe", NextAction::array(0, new NextAction("swipe (bear)", ACTION_HIGH + 5), nullptr)));
    triggers.push_back(new TriggerNode("bash", NextAction::array(0, new NextAction("bash", ACTION_INTERRUPT + 2), nullptr)));
    triggers.push_back(new TriggerNode("bash on enemy healer", NextAction::array(0, new NextAction("bash on enemy healer", ACTION_INTERRUPT + 1), nullptr)));
}
