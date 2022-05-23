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
        }

    private:
        static ActionNode* magma_totem([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("magma totem",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ NextAction::array(0, new NextAction("fire nova"), nullptr));
        }
};

CasterShamanStrategy::CasterShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new CasterShamanStrategyActionNodeFactory());
}

NextAction** CasterShamanStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("lightning bolt", 10.0f), nullptr);
}

void CasterShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("enemy out of spell", NextAction::array(0, new NextAction("reach spell", ACTION_NORMAL + 9), nullptr)));
    triggers.push_back(new TriggerNode("shaman weapon", NextAction::array(0, new NextAction("flametongue weapon", 23.0f), nullptr)));
    triggers.push_back(new TriggerNode("searing totem", NextAction::array(0, new NextAction("searing totem", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("shock", NextAction::array(0, new NextAction("earth shock", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("frost shock snare", NextAction::array(0, new NextAction("frost shock", 21.0f), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("flametongue totem", ACTION_LIGHT_HEAL), nullptr)));
}

void CasterAoeShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("light aoe", NextAction::array(0, new NextAction("chain lightning", 25.0f), nullptr)));
}
