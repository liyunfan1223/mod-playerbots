/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ElementalShamanStrategy.h"

#include "Playerbots.h"

// ===== Action Node Factory =====
class ElementalShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ElementalShamanStrategyActionNodeFactory()
    {
        creators["flame shock"] = &flame_shock;
        creators["earth shock"] = &earth_shock;
        creators["lava burst"] = &lava_burst;
        creators["lightning bolt"] = &lightning_bolt;
        creators["call of the elements"] = &call_of_the_elements;
        creators["elemental mastery"] = &elemental_mastery;
        creators["stoneclaw totem"] = &stoneclaw_totem;
        creators["water shield"] = &water_shield;
        creators["thunderstorm"] = &thunderstorm;
    }

private:
    static ActionNode* flame_shock(PlayerbotAI*) { return new ActionNode("flame shock", nullptr, nullptr, nullptr); }
    static ActionNode* earth_shock(PlayerbotAI*) { return new ActionNode("earth shock", nullptr, nullptr, nullptr); }
    static ActionNode* lava_burst(PlayerbotAI*) { return new ActionNode("lava burst", nullptr, nullptr, nullptr); }
    static ActionNode* lightning_bolt(PlayerbotAI*) { return new ActionNode("lightning bolt", nullptr, nullptr, nullptr); }
    static ActionNode* call_of_the_elements(PlayerbotAI*) { return new ActionNode("call of the elements", nullptr, nullptr, nullptr); }
    static ActionNode* elemental_mastery(PlayerbotAI*) { return new ActionNode("elemental mastery", nullptr, nullptr, nullptr); }
    static ActionNode* stoneclaw_totem(PlayerbotAI*) { return new ActionNode("stoneclaw totem", nullptr, nullptr, nullptr); }
    static ActionNode* water_shield(PlayerbotAI*) { return new ActionNode("water shield", nullptr, nullptr, nullptr); }
    static ActionNode* thunderstorm(PlayerbotAI*) { return new ActionNode("thunderstorm", nullptr, nullptr, nullptr); }
};

// ===== Single Target Strategy =====
ElementalShamanStrategy::ElementalShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new ElementalShamanStrategyActionNodeFactory());
}

// ===== Default Actions =====
NextAction** ElementalShamanStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("lava burst", 5.2f),
                                new NextAction("lightning bolt", 5.0f),
                                nullptr);
}

// ===== Trigger Initialization ===
void ElementalShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    // Totem Triggers
    triggers.push_back(new TriggerNode("call of the elements", NextAction::array(0, new NextAction("call of the elements", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("stoneclaw totem", 40.0f), nullptr)));

    // Cooldown Trigger
    triggers.push_back(new TriggerNode("elemental mastery", NextAction::array(0, new NextAction("elemental mastery", 29.0f), nullptr)));

    // Damage Triggers
    triggers.push_back(new TriggerNode("earth shock execute", NextAction::array(0, new NextAction("earth shock", 5.5f), nullptr)));
    triggers.push_back(new TriggerNode("flame shock", NextAction::array(0, new NextAction("flame shock", 5.3f), nullptr)));

    // Mana Triggers
    triggers.push_back(new TriggerNode("water shield", NextAction::array(0, new NextAction("water shield", 19.5f), nullptr)));
    triggers.push_back(new TriggerNode("high mana", NextAction::array(0, new NextAction("thunderstorm", 19.0f), nullptr)));

    // Range Triggers
    triggers.push_back(new TriggerNode("enemy is close", NextAction::array(0, new NextAction("thunderstorm", 19.0f), nullptr)));
}

