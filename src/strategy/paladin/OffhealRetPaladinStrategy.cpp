/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "OffhealRetPaladinStrategy.h"

#include "Playerbots.h"
#include "Strategy.h"

class OffhealRetPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    OffhealRetPaladinStrategyActionNodeFactory()
    {
        creators["retribution aura"] = &retribution_aura;
        creators["seal of corruption"] = &seal_of_corruption;
        creators["seal of vengeance"] = &seal_of_vengeance;
        creators["seal of command"] = &seal_of_command;
        creators["blessing of might"] = &blessing_of_might;
        creators["crusader strike"] = &crusader_strike;
        creators["divine plea"] = &divine_plea;
    }

private:
    static ActionNode* retribution_aura([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("retribution aura",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("devotion aura"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* seal_of_corruption([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("seal of corruption",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("seal of vengeance"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* seal_of_vengeance([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("seal of vengeance",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("seal of command"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* seal_of_command([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("seal of command",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("seal of righteousness"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* blessing_of_might([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("blessing of might",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("blessing of kings"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* crusader_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("crusader strike",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* divine_plea([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("divine plea",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
};

OffhealRetPaladinStrategy::OffhealRetPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new OffhealRetPaladinStrategyActionNodeFactory());
}

NextAction** OffhealRetPaladinStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("hammer of wrath", ACTION_DEFAULT + 0.6f),
                             new NextAction("judgement of wisdom", ACTION_DEFAULT + 0.5f),
                             new NextAction("crusader strike", ACTION_DEFAULT + 0.4f),
                             new NextAction("divine storm", ACTION_DEFAULT + 0.3f),
                             new NextAction("melee", ACTION_DEFAULT), nullptr);
}

void OffhealRetPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    // Damage Triggers
    triggers.push_back(
        new TriggerNode("seal", NextAction::array(0, new NextAction("seal of corruption", ACTION_HIGH), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("seal of wisdom", ACTION_HIGH + 5),
                                                      new NextAction("divine plea", ACTION_HIGH + 4), nullptr)));
    triggers.push_back(
        new TriggerNode("art of war", NextAction::array(0, new NextAction("exorcism", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "avenging wrath", NextAction::array(0, new NextAction("avenging wrath", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0, new NextAction("divine storm", ACTION_HIGH + 4),
                                                        new NextAction("consecration", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(new TriggerNode("enemy out of melee",
                                       NextAction::array(0, new NextAction("reach melee", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "retribution aura", NextAction::array(0, new NextAction("retribution aura", ACTION_NORMAL), nullptr)));
    triggers.push_back(new TriggerNode(
        "blessing of might", NextAction::array(0, new NextAction("blessing of might", ACTION_NORMAL + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "low health", NextAction::array(0, new NextAction("holy light", ACTION_CRITICAL_HEAL + 2), nullptr)));

    // Healing Triggers
    triggers.push_back(
        new TriggerNode("party member critical health",
                        NextAction::array(0, new NextAction("holy shock on party", ACTION_CRITICAL_HEAL + 6),
                                          new NextAction("holy light on party", ACTION_CRITICAL_HEAL + 4), nullptr)));
    triggers.push_back(
        new TriggerNode("party member low health",
                        NextAction::array(0, new NextAction("holy light on party", ACTION_MEDIUM_HEAL + 5), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_LIGHT_HEAL + 8), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("flash of light on party", ACTION_LIGHT_HEAL + 3), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        NextAction::array(0, new NextAction("reach party member to heal", ACTION_EMERGENCY + 3), nullptr)));
    triggers.push_back(new TriggerNode(
        "beacon of light on main tank",
        NextAction::array(0, new NextAction("beacon of light on main tank", ACTION_CRITICAL_HEAL + 7), nullptr)));
}
