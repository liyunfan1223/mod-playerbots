/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MeleeShamanStrategy.h"
#include "Playerbots.h"

class MeleeShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        MeleeShamanStrategyActionNodeFactory()
        {
            creators["stormstrike"] = &stormstrike;
            creators["lava lash"] = &lava_lash;
            creators["magma totem"] = &magma_totem;
        }

    private:
        static ActionNode* stormstrike([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("stormstrike",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("lava lash"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* lava_lash([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("lava lash",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* magma_totem([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("magma totem",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ NextAction::array(0, new NextAction("fire nova"), nullptr));
        }
};

MeleeShamanStrategy::MeleeShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new MeleeShamanStrategyActionNodeFactory());
}

NextAction** MeleeShamanStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("stormstrike", 10.0f), nullptr);
}

void MeleeShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("shaman weapon", NextAction::array(0, new NextAction("windfury weapon", 22.0f), nullptr)));
    triggers.push_back(new TriggerNode("searing totem", NextAction::array(0, new NextAction("reach melee", 22.0f), new NextAction("searing totem", 22.0f), nullptr)));
    triggers.push_back(new TriggerNode("shock", NextAction::array(0, new NextAction("earth shock", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("not facing target", NextAction::array(0, new NextAction("set facing", ACTION_NORMAL + 7), nullptr)));
    triggers.push_back(new TriggerNode("enemy too close for melee", NextAction::array(0, new NextAction("move out of enemy contact", ACTION_NORMAL + 8), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("strength of earth totem", ACTION_LIGHT_HEAL), nullptr)));
}

void MeleeAoeShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("reach melee", ACTION_NORMAL + 8), nullptr)));
    triggers.push_back(new TriggerNode("magma totem", NextAction::array(0, new NextAction("magma totem", 26.0f), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("fire nova", 25.0f), nullptr)));
}
