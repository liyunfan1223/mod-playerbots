/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DpsWarlockStrategy.h"
#include "Playerbots.h"

class DpsWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        DpsWarlockStrategyActionNodeFactory()
        {
            creators["shadow bolt"] = &shadow_bolt;
        }

    private:
        static ActionNode* shadow_bolt([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("shadow bolt",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("shoot"), nullptr),
                /*C*/ nullptr);
        }
};

DpsWarlockStrategy::DpsWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new DpsWarlockStrategyActionNodeFactory());
}

NextAction** DpsWarlockStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("incinirate", 10.0f), new NextAction("shadow bolt", 10.0f), nullptr);
}

void DpsWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

	triggers.push_back(new TriggerNode("backlash", NextAction::array(0, new NextAction("shadow bolt", 20.0f), nullptr)));
}

void DpsAoeWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("rain of fire", 37.0f), nullptr)));
    triggers.push_back(new TriggerNode("corruption on attacker", NextAction::array(0, new NextAction("corruption on attacker", 27.0f), nullptr)));
    triggers.push_back(new TriggerNode("curse of agony on attacker", NextAction::array(0, new NextAction("curse of agony on attacker", 28.0f), nullptr)));
    triggers.push_back(new TriggerNode("siphon life on attacker", NextAction::array(0, new NextAction("siphon life on attacker", 29.0f), nullptr)));
}

void DpsWarlockDebuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("corruption", NextAction::array(0, new NextAction("corruption", 22.0f), nullptr)));
    triggers.push_back(new TriggerNode("curse of agony", NextAction::array(0, new NextAction("curse of agony", 21.0f), nullptr)));
    triggers.push_back(new TriggerNode("siphon life", NextAction::array(0, new NextAction("siphon life", 23.0f), nullptr)));
}
