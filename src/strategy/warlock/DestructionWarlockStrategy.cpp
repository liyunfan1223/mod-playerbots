/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DestructionWarlockStrategy.h"
#include "Playerbots.h"

// ===== Action Node Factory =====
class DestructionWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DestructionWarlockStrategyActionNodeFactory()
    {
        creators["immolate"] = &immolate;
        creators["conflagrate"] = &conflagrate;
        creators["chaos bolt"] = &chaos_bolt;
        creators["incinerate"] = &incinerate;
        creators["corruption"] = &corruption;
        creators["corruption on attacker"] = &corruption_on_attacker;
        creators["curse of agony"] = &curse_of_agony;
        creators["curse of agony on attacker"] = &curse_of_agony_on_attacker;
        creators["shadow bolt"] = &shadow_bolt;
        creators["shadowburn"] = &shadowburn;
        creators["life tap"] = &life_tap;
        creators["shadowfury"] = &shadowfury;
        creators["shadowflame"] = &shadowflame;
        creators["seed of corruption"] = &seed_of_corruption;
        creators["seed of corruption on attacker"] = &seed_of_corruption;
        creators["rain of fire"] = &rain_of_fire;
    }

private:
    static ActionNode* immolate(PlayerbotAI*) { return new ActionNode("immolate", nullptr, nullptr, nullptr); }
    static ActionNode* conflagrate(PlayerbotAI*) { return new ActionNode("conflagrate", nullptr, nullptr, nullptr); }
    static ActionNode* chaos_bolt(PlayerbotAI*) { return new ActionNode("chaos bolt", nullptr, nullptr, nullptr); }
    static ActionNode* incinerate(PlayerbotAI*) { return new ActionNode("incinerate", nullptr, nullptr, nullptr); }
    static ActionNode* corruption(PlayerbotAI*) { return new ActionNode("corruption", nullptr, nullptr, nullptr); }
    static ActionNode* corruption_on_attacker(PlayerbotAI*) { return new ActionNode("corruption on attacker", nullptr, nullptr, nullptr); }
    static ActionNode* curse_of_agony(PlayerbotAI*) { return new ActionNode("curse of agony", nullptr, nullptr, nullptr); }
    static ActionNode* curse_of_agony_on_attacker(PlayerbotAI*) { return new ActionNode("curse of agony on attacker", nullptr, nullptr, nullptr); }
    static ActionNode* shadow_bolt(PlayerbotAI*) { return new ActionNode("shadow bolt", nullptr, nullptr, nullptr); }
    static ActionNode* shadowburn(PlayerbotAI*) { return new ActionNode("shadowburn", nullptr, nullptr, nullptr); }
    static ActionNode* life_tap(PlayerbotAI*) { return new ActionNode("life tap", nullptr, nullptr, nullptr); }
    static ActionNode* shadowfury(PlayerbotAI*) { return new ActionNode("shadowfury", nullptr, nullptr, nullptr); }
    static ActionNode* shadowflame(PlayerbotAI*) { return new ActionNode("shadowflame", nullptr, nullptr, nullptr); }
    static ActionNode* seed_of_corruption(PlayerbotAI*) { return new ActionNode("seed of corruption", nullptr, nullptr, nullptr); }
    static ActionNode* seed_of_corruption_on_attacker(PlayerbotAI*) { return new ActionNode("seed of corruption on attacker", nullptr, nullptr, nullptr); }
    static ActionNode* rain_of_fire(PlayerbotAI*) { return new ActionNode("rain of fire", nullptr, nullptr, nullptr); }
};

// ===== Single Target Strategy =====
DestructionWarlockStrategy::DestructionWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new DestructionWarlockStrategyActionNodeFactory());
}

// ===== Default Actions =====
NextAction** DestructionWarlockStrategy::getDefaultActions()
{
    return NextAction::array( 0,
       new NextAction("immolate", 6.1f),
       new NextAction("conflagrate", 6.0f),
       new NextAction("chaos bolt", 5.9f),
       new NextAction("incinerate", 5.8f),
       new NextAction("corruption", 5.4f),      // Note: Corruption, Curse of Agony, and Shadow Bolt won't be used after                                              
       new NextAction("curse of agony", 5.3f),  // the character learns Incinerate at level 64
       new NextAction("shadow bolt", 5.2f),
       new NextAction("shoot", 5.0f), nullptr);
}

// ===== Trigger Initialization ===
void DestructionWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    // Main DoT triggers for high uptime + high priority cooldowns
    triggers.push_back(new TriggerNode("immolate", NextAction::array(0, new NextAction("immolate", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("conflagrate", NextAction::array(0, new NextAction("conflagrate", 19.5f), nullptr)));
    triggers.push_back(new TriggerNode("chaos bolt", NextAction::array(0, new NextAction("chaos bolt", 19.0f), nullptr)));

    // Note: These DoTs won't be used after the character learns Incinerate at level 64
    triggers.push_back(new TriggerNode("corruption on attacker", NextAction::array(0, new NextAction("corruption on attacker", 5.6f), nullptr)));
    triggers.push_back(new TriggerNode("curse of agony on attacker", NextAction::array(0, new NextAction("curse of agony on attacker", 5.5f), nullptr)));
    triggers.push_back(new TriggerNode("corruption", NextAction::array(0, new NextAction("corruption", 5.4f), nullptr)));
    triggers.push_back(new TriggerNode("curse of agony", NextAction::array(0, new NextAction("curse of agony", 5.3f), nullptr)));

    // Shadowburn as execute if target is low HP
    triggers.push_back(new TriggerNode("target critical health", NextAction::array(0, new NextAction("shadowburn", 18.0f), nullptr)));

    // Life Tap glyph buff, and Life Tap as filler
    triggers.push_back(new TriggerNode("life tap glyph buff", NextAction::array(0, new NextAction("life tap", 29.0f), nullptr)));
    triggers.push_back(new TriggerNode("life tap", NextAction::array(0, new NextAction("life tap", 5.1f), nullptr)));

    triggers.push_back(new TriggerNode("enemy too close for spell", NextAction::array(0, new NextAction("flee", 39.0f), nullptr)));
}

// ===== AoE Strategy, 3+ enemies =====
DestructionWarlockAoeStrategy::DestructionWarlockAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

void DestructionWarlockAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0,
                       new NextAction("shadowfury", 23.0f),
                       new NextAction("shadowflame", 22.5f),
                       new NextAction("seed of corruption on attacker", 22.0f),
                       new NextAction("seed of corruption", 21.5f),
                       new NextAction("rain of fire", 21.0f), nullptr)));
}
