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
            creators["unstable affliction"] = &unstable_affliction;
            creators["unstable affliction on attacker"] = &unstable_affliction_on_attacker;
        }

    private:
        static ActionNode* shadow_bolt([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("shadow bolt",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("shoot"), nullptr),
                /*C*/ nullptr);
        }
        static ActionNode* unstable_affliction(PlayerbotAI* ai)
        {
            return new ActionNode ("unstable affliction",
                /*P*/ NULL,
                /*A*/ NextAction::array(0, new NextAction("immolate"), NULL),
                /*C*/ NULL);
        }
        static ActionNode* unstable_affliction_on_attacker(PlayerbotAI* ai)
        {
            return new ActionNode ("unstable affliction on attacker",
                /*P*/ NULL,
                /*A*/ NextAction::array(0, new NextAction("immolate on attacker"), NULL),
                /*C*/ NULL);
        }
};

DpsWarlockStrategy::DpsWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new DpsWarlockStrategyActionNodeFactory());
}

NextAction** DpsWarlockStrategy::getDefaultActions()
{
    return NextAction::array(0, 
        new NextAction("haunt", 14.0f), 
        new NextAction("demonic empowerment", 13.0f),
		new NextAction("shadow bolt", 10.0f), 
        nullptr);
}

void DpsWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

	triggers.push_back(new TriggerNode("backlash", NextAction::array(0, new NextAction("shadow bolt", 20.0f), nullptr)));

    triggers.push_back(new TriggerNode(
		"haunt",
		NextAction::array(0, new NextAction("haunt", 26.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"shadow trance",
		NextAction::array(0, new NextAction("shadow bolt", 15.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"backlash",
		NextAction::array(0, new NextAction("shadow bolt", 15.0f), NULL)));

    triggers.push_back(new TriggerNode(
		"molten core",
		NextAction::array(0, new NextAction("incinerate", 15.0f), NULL)));

    triggers.push_back(new TriggerNode(
		"decimation",
		NextAction::array(0, new NextAction("soul fire", 16.0f), NULL)));
}

void DpsAoeWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("rain of fire", 37.0f), nullptr)));
    triggers.push_back(new TriggerNode("corruption on attacker", NextAction::array(0, new NextAction("corruption on attacker", 27.0f), nullptr)));
    // triggers.push_back(new TriggerNode("curse of agony on attacker", NextAction::array(0, new NextAction("curse of agony on attacker", 28.0f), nullptr)));
    triggers.push_back(new TriggerNode("unstable affliction on attacker", NextAction::array(0, new NextAction("unstable affliction on attacker", 22.0f), NULL)));
}

void DpsWarlockDebuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("corruption", NextAction::array(0, new NextAction("corruption", 22.0f), nullptr)));
    triggers.push_back(new TriggerNode("curse of agony", NextAction::array(0, new NextAction("curse of agony", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("unstable affliction", NextAction::array(0, new NextAction("unstable affliction", 21.0f), NULL)));
}
