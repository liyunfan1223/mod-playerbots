/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RacialsStrategy.h"

#include "Playerbots.h"

class RacialsStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    RacialsStrategyActionNodeFactory() { creators["lifeblood"] = &lifeblood; }

private:
    static ActionNode* lifeblood(PlayerbotAI* botAI)
    {
        return new ActionNode("lifeblood",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("gift of the naaru"), nullptr),
                              /*C*/ nullptr);
    }
};

void RacialsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("lifeblood", 71.0f), nullptr)));
    // triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("war stomp", 71.0f),
    // nullptr)));
    /*triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("war stomp", 71.0f),
     * nullptr)));*/
    /*triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("arcane torrent",
    ACTION_EMERGENCY + 6), nullptr))); triggers.push_back(new TriggerNode("medium mana", NextAction::array(0, new
    NextAction("mana tap", ACTION_EMERGENCY + 6), nullptr)));*/
}

RacialsStrategy::RacialsStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new RacialsStrategyActionNodeFactory());
}
