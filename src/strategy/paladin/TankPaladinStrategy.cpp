/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TankPaladinStrategy.h"
#include "Playerbots.h"

class TankPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        TankPaladinStrategyActionNodeFactory()
        {
            creators["seal of vengeance"] = &seal_of_vengeance;
            creators["hand of reckoning"] = &hand_of_reckoning;
        }

    private:
        static ActionNode* seal_of_vengeance([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode("seal of vengeance",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("seal of righteousness"), nullptr),
                /*C*/ nullptr);
        }

        ACTION_NODE_A(hand_of_reckoning, "hand of reckoning", "righteous defense");
};

TankPaladinStrategy::TankPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new TankPaladinStrategyActionNodeFactory());
}

NextAction** TankPaladinStrategy::getDefaultActions()
{
    return NextAction::array(0,
        new NextAction("shield of righteousness", ACTION_NORMAL + 6),
        new NextAction("hammer of the righteous", ACTION_NORMAL + 5),
        new NextAction("judgement of wisdom", ACTION_NORMAL + 4),
        // new NextAction("avenger's shield", ACTION_NORMAL + 3), 
        // new NextAction("consecration", ACTION_NORMAL + 2),
        new NextAction("melee", ACTION_NORMAL), 
        NULL);
}

void TankPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("seal", NextAction::array(0, new NextAction("seal of command", 90.0f), NULL)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("seal of wisdom", 91.0f), nullptr)));
    triggers.push_back(new TriggerNode(
		"devotion aura",
		NextAction::array(0, new NextAction("devotion aura", 90.0f), NULL)));

    // triggers.push_back(new TriggerNode("judgement of light", NextAction::array(0, new NextAction("judgement of light", ACTION_HIGH + 6), nullptr)));
    // triggers.push_back(new TriggerNode("medium mana", NextAction::array(0, new NextAction("judgement of wisdom", ACTION_HIGH + 6), nullptr)));
    // triggers.push_back(new TriggerNode("judgement", NextAction::array(0, new NextAction("judgement", ACTION_HIGH + 6), nullptr)));
    // triggers.push_back(new TriggerNode("judgement", NextAction::array(0, new NextAction("exorcism", ACTION_HIGH + 6), nullptr)));
    triggers.push_back(new TriggerNode("light aoe", NextAction::array(0, new NextAction("hammer of the righteous", ACTION_HIGH + 8), new NextAction("avenger's shield", ACTION_HIGH + 7), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("consecration", ACTION_HIGH + 1), new NextAction("avenger's shield", ACTION_HIGH + 3), NULL)));
    // triggers.push_back(new TriggerNode("avenger's shield", NextAction::array(0, new NextAction("avenger's shield", ACTION_HIGH + 7), nullptr)));
    triggers.push_back(new TriggerNode("lose aggro", NextAction::array(0, new NextAction("hand of reckoning", ACTION_HIGH + 7), nullptr)));
    triggers.push_back(new TriggerNode("holy shield", NextAction::array(0, new NextAction("holy shield", ACTION_HIGH + 7), nullptr)));
    // triggers.push_back(new TriggerNode("blessing", NextAction::array(0, new NextAction("blessing of sanctuary", ACTION_HIGH + 9), nullptr)));
    triggers.push_back(new TriggerNode("target critical health", NextAction::array(0, new NextAction("hammer of wrath", ACTION_CRITICAL_HEAL), nullptr)));
    triggers.push_back(new TriggerNode(
        "righteous fury",
        NextAction::array(0, new NextAction("righteous fury", ACTION_HIGH + 8), NULL)));
    triggers.push_back(new TriggerNode(
        "not facing target",
        NextAction::array(0, new NextAction("set facing", ACTION_NORMAL + 7), NULL)));
    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("reach melee", ACTION_NORMAL + 8), NULL)));
}
