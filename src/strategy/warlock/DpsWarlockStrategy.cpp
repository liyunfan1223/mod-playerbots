/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DpsWarlockStrategy.h"
#include "Playerbots.h"

// This strategy is designed for low-level Warlocks without talents.
// All of the important spells/cooldowns have been migrated to
// their respective specs.

// ===== Action Node Factory =====
class DpsWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsWarlockStrategyActionNodeFactory()
    {
        creators["corruption"] = &corruption;
        creators["curse of agony"] = &curse_of_agony;
        creators["immolate"] = &immolate;
        creators["shadow bolt"] = &shadow_bolt;
        creators["life tap"] = &life_tap;
        creators["shadowflame"] = &shadowflame;
        creators["seed of corruption"] = &seed_of_corruption;
        creators["rain of fire"] = &rain_of_fire;
        creators["drain soul"] = &drain_soul;
    }

private:
    static ActionNode* corruption(PlayerbotAI*) { return new ActionNode("corruption", nullptr, nullptr, nullptr); }
    static ActionNode* curse_of_agony(PlayerbotAI*){return new ActionNode("curse of agony", nullptr, nullptr, nullptr);}
    static ActionNode* immolate(PlayerbotAI*) { return new ActionNode("immolate", nullptr, nullptr, nullptr); }
    static ActionNode* shadow_bolt(PlayerbotAI*) { return new ActionNode("shadow bolt", nullptr, nullptr, nullptr); }
    static ActionNode* life_tap(PlayerbotAI*) { return new ActionNode("life tap", nullptr, nullptr, nullptr); }
    static ActionNode* shadowflame(PlayerbotAI*) { return new ActionNode("shadowflame", nullptr, nullptr, nullptr); }
    static ActionNode* seed_of_corruption(PlayerbotAI*){return new ActionNode("seed of corruption", nullptr, nullptr, nullptr);}
    static ActionNode* rain_of_fire(PlayerbotAI*) { return new ActionNode("rain of fire", nullptr, nullptr, nullptr); }
    static ActionNode* drain_soul(PlayerbotAI*) { return new ActionNode("drain soul", nullptr, nullptr, nullptr); }
};

// ===== Single Target Strategy =====
DpsWarlockStrategy::DpsWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new DpsWarlockStrategyActionNodeFactory());
}

// ===== Default Actions =====
NextAction** DpsWarlockStrategy::getDefaultActions()
{
    return NextAction::array(0,
       new NextAction("immolate", 5.5f),
       new NextAction("corruption", 5.4f),
       new NextAction("curse of agony", 5.3f),
       new NextAction("shadow bolt", 5.2f),
       new NextAction("shoot", 5.0f), nullptr);
}

// ===== Trigger Initialization ===
void DpsWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    // Main DoT triggers for high uptime
    triggers.push_back(new TriggerNode("corruption on attacker", NextAction::array(0, new NextAction("corruption on attacker", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("curse of agony on attacker", NextAction::array(0, new NextAction("curse of agony on attacker", 19.5f), nullptr)));
    triggers.push_back(new TriggerNode("immolate on attacker", NextAction::array(0, new NextAction("immolate on attacker", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("corruption", NextAction::array(0, new NextAction("corruption", 18.5f), nullptr)));
    triggers.push_back(new TriggerNode("curse of agony", NextAction::array(0, new NextAction("curse of agony", 18.0f), nullptr)));
    triggers.push_back(new TriggerNode("immolate", NextAction::array(0, new NextAction("immolate", 17.5f), nullptr)));

    // Drain Soul as execute if target is low HP
    triggers.push_back(new TriggerNode("target critical health", NextAction::array(0, new NextAction("drain soul", 17.0f), nullptr)));

    // Cast during movement or to activate glyph buff
    triggers.push_back(new TriggerNode("life tap", NextAction::array(0, new NextAction("life tap", ACTION_DEFAULT + 0.1f), nullptr)));
    triggers.push_back(new TriggerNode("life tap glyph buff", NextAction::array(0, new NextAction("life tap", 28.0f), NULL)));
}

// ===== AoE Strategy, 3+ enemies =====
void DpsAoeWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0,
                       new NextAction("shadowflame", 22.5f),
                       new NextAction("seed of corruption on attacker", 22.0f),
                       new NextAction("seed of corruption", 21.5f),
                       new NextAction("rain of fire", 21.0f), nullptr)));
}
