/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DpsPaladinStrategy.h"
#include "Playerbots.h"
#include "Strategy.h"

class DpsPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        DpsPaladinStrategyActionNodeFactory()
        {
            creators["sanctity aura"] = &sanctity_aura;
            creators["retribution aura"] = &retribution_aura;
            creators["seal of corruption"] = &seal_of_corruption;
            creators["seal of vengeance"] = &seal_of_vengeance;
            creators["seal of command"] = &seal_of_command;
            creators["blessing of might"] = &blessing_of_might;
            creators["crusader strike"] = &crusader_strike;
            creators["repentance"] = &repentance;
            creators["repentance on enemy healer"] = &repentance_on_enemy_healer;
            creators["repentance on snare target"] = &repentance_on_snare_target;
            creators["repentance of shield"] = &repentance_or_shield;
        }

    private:
        static ActionNode* seal_of_corruption([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("seal of corruption",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("seal of vengeance"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* seal_of_vengeance([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("seal of vengeance",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("seal of righteousness"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* seal_of_command([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("seal of command",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("seal of corruption"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* blessing_of_might([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("blessing of might",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("blessing of kings"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* crusader_strike([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("crusader strike",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                /*C*/ nullptr);
        }
    ACTION_NODE_A(repentance, "repentance", "hammer of justice");
    ACTION_NODE_A(repentance_on_enemy_healer, "repentance on enemy healer", "hammer of justice on enemy healer");
    ACTION_NODE_A(repentance_on_snare_target, "repentance on snare target", "hammer of justice on snare target");
    ACTION_NODE_A(sanctity_aura, "sanctity aura", "retribution aura");
    ACTION_NODE_A(retribution_aura, "retribution aura", "devotion aura");
    ACTION_NODE_A(repentance_or_shield, "repentance", "divine shield");
};

DpsPaladinStrategy::DpsPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new DpsPaladinStrategyActionNodeFactory());
}

NextAction** DpsPaladinStrategy::getDefaultActions()
{
    return NextAction::array(0, 
        new NextAction("judgement of wisdom", ACTION_DEFAULT + 0.4f),
        new NextAction("crusader strike", ACTION_DEFAULT + 0.3f),
        new NextAction("divine storm", ACTION_DEFAULT + 0.2f),
        new NextAction("consecration", ACTION_DEFAULT + 0.1f),
        new NextAction("melee", ACTION_DEFAULT), 
        NULL);
}

void DpsPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "seal",
        NextAction::array(0, new NextAction("seal of command", ACTION_HIGH), NULL)));
    // triggers.push_back(new TriggerNode("seal", NextAction::array(0, new NextAction("seal of command", 90.0f), nullptr)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("seal of wisdom", ACTION_HIGH + 5), nullptr)));
    // triggers.push_back(new TriggerNode("sanctity aura", NextAction::array(0, new NextAction("sanctity aura", 90.0f), nullptr)));
    // triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("repentance or shield", ACTION_CRITICAL_HEAL + 3), new NextAction("holy light", ACTION_CRITICAL_HEAL + 2), nullptr)));
    // triggers.push_back(new TriggerNode("judgement of wisdom", NextAction::array(0, new NextAction("judgement of wisdom", ACTION_NORMAL + 10), nullptr)));
    // triggers.push_back(new TriggerNode("judgement", NextAction::array(0, new NextAction("judgement", ACTION_HIGH + 10), nullptr)));
    // triggers.push_back(new TriggerNode("enemy is close", NextAction::array(0, new NextAction("consecration", ACTION_INTERRUPT), nullptr)));
    // triggers.push_back(new TriggerNode("repentance on enemy healer", NextAction::array(0, new NextAction("repentance on enemy healer", ACTION_INTERRUPT + 2), nullptr)));
    // triggers.push_back(new TriggerNode("repentance on snare target", NextAction::array(0, new NextAction("repentance on snare target", ACTION_INTERRUPT + 2), nullptr)));
    // triggers.push_back(new TriggerNode("repentance", NextAction::array(0, new NextAction("repentance", ACTION_INTERRUPT + 2), nullptr)));
	triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("consecration", ACTION_HIGH + 3), nullptr)));
	triggers.push_back(new TriggerNode("art of war", NextAction::array(0, new NextAction("exorcism", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode("target critical health", NextAction::array(0, new NextAction("hammer of wrath", ACTION_HIGH), nullptr)));
    // triggers.push_back(new TriggerNode(
    //     "not facing target",
    //     NextAction::array(0, new NextAction("set facing", ACTION_NORMAL + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("reach melee", ACTION_NORMAL + 8), NULL)));
}
