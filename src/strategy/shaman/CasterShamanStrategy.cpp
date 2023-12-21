/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CasterShamanStrategy.h"
#include "Playerbots.h"

class CasterShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        CasterShamanStrategyActionNodeFactory()
        {
            creators["magma totem"] = &magma_totem;
            creators["totem of wrath"] = &totem_of_wrath;
        }

    private:
        static ActionNode* magma_totem([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("magma totem",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ NextAction::array(0, new NextAction("fire nova"), nullptr));
        }
        static ActionNode* totem_of_wrath(PlayerbotAI* botAI)
        {
            return new ActionNode ("totem of wrath",
                /*P*/ NULL,
                /*A*/ NextAction::array(0, new NextAction("flametongue totem"), NULL),
                /*C*/ NULL);
        }
};

CasterShamanStrategy::CasterShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new CasterShamanStrategyActionNodeFactory());
}

NextAction** CasterShamanStrategy::getDefaultActions()
{
    return NextAction::array(0, 
        new NextAction("lava burst", ACTION_DEFAULT + 0.2f),
        new NextAction("thunderstorm", ACTION_DEFAULT + 0.1f),
        new NextAction("lightning bolt", ACTION_DEFAULT), 
        NULL);
}

void CasterShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    // triggers.push_back(new TriggerNode("enemy out of spell", NextAction::array(0, new NextAction("reach spell", ACTION_NORMAL + 9), nullptr)));
    triggers.push_back(new TriggerNode("shaman weapon", NextAction::array(0, new NextAction("flametongue weapon", 23.0f), nullptr)));
    // triggers.push_back(new TriggerNode("searing totem", NextAction::array(0, new NextAction("searing totem", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("flame shock", NextAction::array(0, new NextAction("flame shock", 20.0f), nullptr)));
    // triggers.push_back(new TriggerNode("frost shock snare", NextAction::array(0, new NextAction("frost shock", 21.0f), nullptr)));
    triggers.push_back(new TriggerNode(
        "no fire totem",
        NextAction::array(0, 
            // new NextAction("fire elemental totem", 16.0f), 
            new NextAction("totem of wrath", 15.0f), NULL)));

    triggers.push_back(new TriggerNode("enemy too close for spell", NextAction::array(0, new NextAction("flee", ACTION_MOVE + 9), nullptr)));
}

void CasterAoeShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("light aoe", NextAction::array(0, new NextAction("chain lightning", 25.0f), nullptr)));
}
