/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HealPaladinStrategy.h"
#include "Playerbots.h"

class HealPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HealPaladinStrategyActionNodeFactory()
    {
        creators["concentration aura"] = &concentration_aura;
    }

private:
    ACTION_NODE_A(concentration_aura, "concentration aura", "devotion aura");
};

HealPaladinStrategy::HealPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new HealPaladinStrategyActionNodeFactory());
}

NextAction** HealPaladinStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("reach party member to heal", ACTION_NORMAL + 1), new NextAction("judgement", ACTION_NORMAL), nullptr);
}

void HealPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("concentration aura", NextAction::array(0, new NextAction("concentration aura", 90.0f), nullptr)));
    triggers.push_back(new TriggerNode("seal", NextAction::array(0, new NextAction("seal of light", ACTION_HIGH), nullptr)));
    triggers.push_back(new TriggerNode("enemy is close", NextAction::array(0, new NextAction("judgement of light", ACTION_HIGH + 10), nullptr)));
    triggers.push_back(new TriggerNode("enemy is close", NextAction::array(0, new NextAction("judgement", ACTION_HIGH + 10), nullptr)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("divine favor", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode("almost full health", NextAction::array(0, new NextAction("flash of light", ACTION_LIGHT_HEAL + 2), nullptr)));
    triggers.push_back(new TriggerNode("party member almost full health", NextAction::array(0, new NextAction("flash of light on party", ACTION_LIGHT_HEAL + 1), nullptr)));
    triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("flash of light", ACTION_MEDIUM_HEAL + 2), nullptr)));
    triggers.push_back(new TriggerNode("party member medium health", NextAction::array(0, new NextAction("flash of light on party", ACTION_MEDIUM_HEAL + 1), nullptr)));
    triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("divine favor", ACTION_MEDIUM_HEAL + 4), new NextAction("holy shock", ACTION_MEDIUM_HEAL + 3), new NextAction("holy light", ACTION_MEDIUM_HEAL + 2), nullptr)));
    triggers.push_back(new TriggerNode("party member low health", NextAction::array(0, new NextAction("divine favor", ACTION_MEDIUM_HEAL + 7), new NextAction("holy shock on party", ACTION_MEDIUM_HEAL + 6), new NextAction("holy light on party", ACTION_MEDIUM_HEAL + 5), nullptr)));
    triggers.push_back(new TriggerNode("blessing", NextAction::array(0, new NextAction("blessing of sanctuary", ACTION_HIGH + 9), nullptr)));
    triggers.push_back(new TriggerNode("party member to heal out of spell range", NextAction::array(0, new NextAction("reach party member to heal", ACTION_EMERGENCY + 3), nullptr)));
}
