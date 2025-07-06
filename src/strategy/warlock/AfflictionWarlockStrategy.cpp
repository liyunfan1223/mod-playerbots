/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AfflictionWarlockStrategy.h"
#include "Playerbots.h"

// ===== Action Node Factory =====
class AfflictionWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    AfflictionWarlockStrategyActionNodeFactory()
    {
        creators["corruption"] = &corruption;
        creators["corruption on attacker"] = &corruption;
        creators["unstable affliction"] = &unstable_affliction;
        creators["unstable affliction on attacker"] = &unstable_affliction;
        creators["curse of agony"] = &curse_of_agony;
        creators["curse of agony on attacker"] = &curse_of_agony;
        creators["haunt"] = &haunt;
        creators["shadow bolt"] = &shadow_bolt;
        creators["drain soul"] = &drain_soul;
        creators["life tap"] = &life_tap;
        creators["shadowflame"] = &shadowflame;
        creators["seed of corruption on attacker"] = &seed_of_corruption;
        creators["seed of corruption"] = &seed_of_corruption;
        creators["rain of fire"] = &rain_of_fire;
    }

private:
    static ActionNode* corruption(PlayerbotAI*) { return new ActionNode("corruption", nullptr, nullptr, nullptr); }
    static ActionNode* corruption_on_attacker(PlayerbotAI*) { return new ActionNode("corruption on attacker", nullptr, nullptr, nullptr); }
    static ActionNode* unstable_affliction(PlayerbotAI*) { return new ActionNode("unstable affliction", nullptr, nullptr, nullptr); }
    static ActionNode* unstable_affliction_on_attacker(PlayerbotAI*) { return new ActionNode("unstable affliction on attacker", nullptr, nullptr, nullptr); }
    static ActionNode* curse_of_agony(PlayerbotAI*) { return new ActionNode("curse of agony", nullptr, nullptr, nullptr); }
    static ActionNode* curse_of_agony_on_attacker(PlayerbotAI*) { return new ActionNode("curse of agony on attacker", nullptr, nullptr, nullptr); }
    static ActionNode* haunt(PlayerbotAI*) { return new ActionNode("haunt", nullptr, nullptr, nullptr); }
    static ActionNode* shadow_bolt(PlayerbotAI*) { return new ActionNode("shadow bolt", nullptr, nullptr, nullptr); }
    static ActionNode* drain_soul(PlayerbotAI*) { return new ActionNode("drain soul", nullptr, nullptr, nullptr); }
    static ActionNode* life_tap(PlayerbotAI*) { return new ActionNode("life tap", nullptr, nullptr, nullptr); }
    static ActionNode* shadowflame(PlayerbotAI*) { return new ActionNode("shadowflame", nullptr, nullptr, nullptr); }
    static ActionNode* seed_of_corruption_on_attacker(PlayerbotAI*) { return new ActionNode("seed of corruption on attacker", nullptr, nullptr, nullptr); }
    static ActionNode* seed_of_corruption(PlayerbotAI*) { return new ActionNode("seed of corruption", nullptr, nullptr, nullptr); }
    static ActionNode* rain_of_fire(PlayerbotAI*) { return new ActionNode("rain of fire", nullptr, nullptr, nullptr); }
};

// ===== Single Target Strategy =====
AfflictionWarlockStrategy::AfflictionWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new AfflictionWarlockStrategyActionNodeFactory());
}

// ===== Default Actions =====
NextAction** AfflictionWarlockStrategy::getDefaultActions()
{
    return NextAction::array( 0,
       new NextAction("corruption", 5.6f),
       new NextAction("unstable affliction", 5.5f),
       new NextAction("curse of agony", 5.4f),
       new NextAction("haunt", 5.3f),
       new NextAction("shadow bolt", 5.2f),
       new NextAction("shoot", 5.0f), nullptr);
}

// ===== Trigger Initialization ===
void AfflictionWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    // Main DoT triggers for high uptime
    triggers.push_back(new TriggerNode("corruption on attacker", NextAction::array(0, new NextAction("corruption on attacker", 19.5f), nullptr)));
    triggers.push_back(new TriggerNode("unstable affliction on attacker", NextAction::array(0, new NextAction("unstable affliction on attacker", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("curse of agony on attacker", NextAction::array(0, new NextAction("curse of agony on attacker", 18.5f), nullptr)));
    triggers.push_back(new TriggerNode("corruption", NextAction::array(0, new NextAction("corruption", 18.0f), nullptr)));
    triggers.push_back(new TriggerNode("unstable affliction", NextAction::array(0, new NextAction("unstable affliction", 17.5f), nullptr)));
    triggers.push_back(new TriggerNode("curse of agony", NextAction::array(0, new NextAction("curse of agony", 17.0f), nullptr)));
    triggers.push_back(new TriggerNode("haunt", NextAction::array(0, new NextAction("haunt", 16.5f), nullptr)));

    // Drain Soul as execute if target is low HP // Shadow Trance for free casts
    triggers.push_back(new TriggerNode("shadow trance", NextAction::array(0, new NextAction("shadow bolt", 16.0f), nullptr)));
    triggers.push_back(new TriggerNode("target critical health", NextAction::array(0, new NextAction("drain soul", 15.5f), nullptr)));
    
    // Life Tap glyph buff, and Life Tap as filler
    triggers.push_back(new TriggerNode("life tap glyph buff", NextAction::array(0, new NextAction("life tap", 29.0f), nullptr)));
    triggers.push_back(new TriggerNode("life tap", NextAction::array(0, new NextAction("life tap", 5.1f), nullptr)));

    triggers.push_back(new TriggerNode("enemy too close for spell", NextAction::array(0, new NextAction("flee", 39.0f), nullptr)));
}

// ===== AoE Strategy, 3+ enemies =====
AfflictionWarlockAoeStrategy::AfflictionWarlockAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

void AfflictionWarlockAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0,
                       new NextAction("shadowflame", 22.5f),
                       new NextAction("seed of corruption on attacker", 22.0f),
                       new NextAction("seed of corruption", 21.5f),
                       new NextAction("rain of fire", 21.0f), nullptr)));
}
