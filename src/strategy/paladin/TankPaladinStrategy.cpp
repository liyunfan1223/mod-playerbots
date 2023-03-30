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
    return NextAction::array(0, new NextAction("melee", ACTION_NORMAL), nullptr);
}

void TankPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("seal", NextAction::array(0, new NextAction("seal of vengeance", 90.0f), NULL)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("seal of wisdom", 91.0f), nullptr)));
    triggers.push_back(new TriggerNode("judgement of light", NextAction::array(0, new NextAction("judgement of light", ACTION_HIGH + 6), nullptr)));
    triggers.push_back(new TriggerNode("medium mana", NextAction::array(0, new NextAction("judgement of wisdom", ACTION_HIGH + 6), nullptr)));
    triggers.push_back(new TriggerNode("judgement", NextAction::array(0, new NextAction("judgement", ACTION_HIGH + 6), nullptr)));
    triggers.push_back(new TriggerNode("judgement", NextAction::array(0, new NextAction("exorcism", ACTION_HIGH + 6), nullptr)));
    triggers.push_back(new TriggerNode("light aoe", NextAction::array(0, new NextAction("hammer of the righteous", ACTION_HIGH + 8), new NextAction("avenger's shield", ACTION_HIGH + 7), nullptr)));
    triggers.push_back(new TriggerNode("avenger's shield", NextAction::array(0, new NextAction("avenger's shield", ACTION_HIGH + 7), nullptr)));
    triggers.push_back(new TriggerNode("enemy is close", NextAction::array(0, new NextAction("consecration", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode("lose aggro", NextAction::array(0, new NextAction("hand of reckoning", ACTION_HIGH + 7), nullptr)));
    triggers.push_back(new TriggerNode("lose aggro", NextAction::array(0, new NextAction("exorcism", ACTION_HIGH + 8), nullptr)));
    triggers.push_back(new TriggerNode("holy shield", NextAction::array(0, new NextAction("holy shield", ACTION_HIGH + 7), nullptr)));
    triggers.push_back(new TriggerNode("blessing", NextAction::array(0, new NextAction("blessing of sanctuary", ACTION_HIGH + 9), nullptr)));
    triggers.push_back(new TriggerNode("target critical health", NextAction::array(0, new NextAction("hammer of wrath", ACTION_CRITICAL_HEAL), nullptr)));
}
