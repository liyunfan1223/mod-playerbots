/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "UsePotionsStrategy.h"

#include "Playerbots.h"

class UsePotionsStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    UsePotionsStrategyActionNodeFactory() { creators["healthstone"] = &healthstone; }

private:
    static ActionNode* healthstone(PlayerbotAI* botAI)
    {
        return new ActionNode("healthstone",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("healing potion"), nullptr),
                              /*C*/ nullptr);
    }
};

UsePotionsStrategy::UsePotionsStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new UsePotionsStrategyActionNodeFactory());
}

void UsePotionsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Strategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health", NextAction::array(0, new NextAction("healthstone", ACTION_MEDIUM_HEAL + 1), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("mana potion", ACTION_EMERGENCY), nullptr)));
}
