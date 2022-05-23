/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HealShamanStrategy.h"
#include "Playerbots.h"

class HealShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        HealShamanStrategyActionNodeFactory()
        {
            creators["earthliving weapon"] = &earthliving_weapon;
            creators["mana tide totem"] = &mana_tide_totem;
        }

    private:
        static ActionNode* earthliving_weapon([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("earthliving weapon",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("flametongue weapon"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* mana_tide_totem([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("mana tide totem",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("mana potion"), nullptr),
                /*C*/ nullptr);
        }
};

HealShamanStrategy::HealShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new HealShamanStrategyActionNodeFactory());
}

void HealShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("enemy out of spell", NextAction::array(0, new NextAction("reach spell", ACTION_NORMAL + 9), nullptr)));
    triggers.push_back(new TriggerNode("shaman weapon", NextAction::array(0, new NextAction("earthliving weapon", 22.0f), nullptr)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("mana tide totem", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("healing stream totem", ACTION_LIGHT_HEAL), nullptr)));
    triggers.push_back(new TriggerNode("party member to heal out of spell range", NextAction::array(0, new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 1), nullptr)));
}
