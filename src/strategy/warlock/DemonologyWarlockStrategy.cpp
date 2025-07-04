/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DemonologyWarlockStrategy.h"
#include "Playerbots.h"

// ===== Action Node Factory =====
class DemonologyWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DemonologyWarlockStrategyActionNodeFactory()
    {
        creators["metamorphosis"] = &metamorphosis;
        creators["demonic empowerment"] = &demonic_empowerment;
        creators["curse of agony"] = &curse_of_agony;
        creators["curse of agony on attacker"] = &curse_of_agony_on_attacker;
        creators["corruption"] = &corruption;
        creators["corruption on attacker"] = &corruption_on_attacker;
        creators["immolate"] = &immolate;
        creators["immolate on attacker"] = &immolate_on_attacker;
        creators["incinerate"] = &incinerate;
        creators["soul fire"] = &soul_fire;
        creators["shadow bolt"] = &shadow_bolt;
        creators["life tap"] = &life_tap;
        creators["immolation aura"] = &immolation_aura;
        creators["shadowflame"] = &shadowflame;
        creators["seed of corruption on attacker"] = &seed_of_corruption_on_attacker;
        creators["seed of corruption"] = &seed_of_corruption;
        creators["rain of fire"] = &rain_of_fire;
        creators["demon charge"] = &demon_charge;
    }

private:
    static ActionNode* metamorphosis(PlayerbotAI*) { return new ActionNode("metamorphosis", nullptr, nullptr, nullptr); }
    static ActionNode* demonic_empowerment(PlayerbotAI*) { return new ActionNode("demonic empowerment", nullptr, nullptr, nullptr); }
    static ActionNode* curse_of_agony(PlayerbotAI*) { return new ActionNode("curse of agony", nullptr, nullptr, nullptr); }
    static ActionNode* curse_of_agony_on_attacker(PlayerbotAI*) { return new ActionNode("curse of agony on attacker", nullptr, nullptr, nullptr); }
    static ActionNode* corruption(PlayerbotAI*) { return new ActionNode("corruption", nullptr, nullptr, nullptr); }
    static ActionNode* corruption_on_attacker(PlayerbotAI*) { return new ActionNode("corruption on attacker", nullptr, nullptr, nullptr); }
    static ActionNode* immolate(PlayerbotAI*) { return new ActionNode("immolate", nullptr, nullptr, nullptr); }
    static ActionNode* immolate_on_attacker(PlayerbotAI*) { return new ActionNode("immolate on attacker", nullptr, nullptr, nullptr); }
    static ActionNode* incinerate(PlayerbotAI*) { return new ActionNode("incinerate", nullptr, nullptr, nullptr); }
    static ActionNode* soul_fire(PlayerbotAI*) { return new ActionNode("soul fire", nullptr, nullptr, nullptr); }
    static ActionNode* shadow_bolt(PlayerbotAI*) { return new ActionNode("shadow bolt", nullptr, nullptr, nullptr); }
    static ActionNode* life_tap(PlayerbotAI*) { return new ActionNode("life tap", nullptr, nullptr, nullptr); }
    static ActionNode* immolation_aura(PlayerbotAI*) { return new ActionNode("immolation aura", nullptr, nullptr, nullptr); }
    static ActionNode* shadowflame(PlayerbotAI*) { return new ActionNode("shadowflame", nullptr, nullptr, nullptr); }
    static ActionNode* seed_of_corruption_on_attacker(PlayerbotAI*) { return new ActionNode("seed of corruption on attacker", nullptr, nullptr, nullptr); }
    static ActionNode* seed_of_corruption(PlayerbotAI*) { return new ActionNode("seed of corruption", nullptr, nullptr, nullptr); }
    static ActionNode* rain_of_fire(PlayerbotAI*) { return new ActionNode("rain of fire", nullptr, nullptr, nullptr); }
    static ActionNode* demon_charge(PlayerbotAI*) { return new ActionNode("demon charge", nullptr, nullptr, nullptr); }
};

// ===== Single Target Strategy =====
DemonologyWarlockStrategy::DemonologyWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new DemonologyWarlockStrategyActionNodeFactory());
}

// ===== Default Actions =====
NextAction** DemonologyWarlockStrategy::getDefaultActions()
{
    return NextAction::array(0,
       new NextAction("corruption", 5.6f),
       new NextAction("curse of agony", 5.5f),
       new NextAction("immolate", 5.4f),
       new NextAction("shadow bolt", 5.3f),
       new NextAction("incinerate", 5.2f),
       new NextAction("shoot", 5.0f), nullptr);
}

// ===== Trigger Initialization ===
void DemonologyWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    // High priority cooldowns
    triggers.push_back(new TriggerNode("metamorphosis", NextAction::array(0, new NextAction("metamorphosis", 28.5f), nullptr)));
    triggers.push_back(new TriggerNode("demonic empowerment", NextAction::array(0, new NextAction("demonic empowerment", 28.0f), nullptr)));

    // Main DoT triggers for high uptime
    triggers.push_back(new TriggerNode("corruption on attacker", NextAction::array(0, new NextAction("corruption on attacker", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("curse of agony on attacker", NextAction::array(0, new NextAction("curse of agony on attacker", 19.5f), nullptr)));
    triggers.push_back(new TriggerNode("immolate on attacker", NextAction::array(0, new NextAction("immolate on attacker", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("corruption", NextAction::array(0, new NextAction("corruption", 18.5f), nullptr)));
    triggers.push_back(new TriggerNode("curse of agony", NextAction::array(0, new NextAction("curse of agony", 18.0f), nullptr)));
    triggers.push_back(new TriggerNode("immolate", NextAction::array(0, new NextAction("immolate", 17.5f), nullptr)));

    // Procs
    triggers.push_back(new TriggerNode("decimation", NextAction::array(0, new NextAction("soul fire", 17.0f), nullptr)));
    triggers.push_back(new TriggerNode("molten core", NextAction::array(0, new NextAction("incinerate", 16.5f), nullptr)));

    // Life Tap glyph buff, and Life Tap as filler
    triggers.push_back(new TriggerNode("life tap glyph buff", NextAction::array(0, new NextAction("life tap", 29.0f), nullptr)));
    triggers.push_back(new TriggerNode("life tap", NextAction::array(0, new NextAction("life tap", 5.1f), nullptr)));

    triggers.push_back(new TriggerNode("meta melee flee check", NextAction::array(0, new NextAction("flee", 39.0f), nullptr)));
}

// ===== AoE Strategy, 3+ enemies =====
DemonologyWarlockAoeStrategy::DemonologyWarlockAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

void DemonologyWarlockAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0,
                       new NextAction("immolation aura", 26.0f),
                       new NextAction("shadowflame", 22.5f),
                       new NextAction("seed of corruption on attacker", 22.0f),
                       new NextAction("seed of corruption", 21.5f),
                       new NextAction("rain of fire", 21.0f), nullptr)));
}

// Combat strategy to run to melee for Immolation Aura
// Enabled by default for the Demonology spec
// To enable, type "co +meta melee"
// To disable, type "co -meta melee"
MetaMeleeAoeStrategy::MetaMeleeAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

void MetaMeleeAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("immolation aura active", NextAction::array(0,
                       new NextAction("reach melee", 25.5f),
                       new NextAction("demon charge", 25.0f), nullptr)));
}
