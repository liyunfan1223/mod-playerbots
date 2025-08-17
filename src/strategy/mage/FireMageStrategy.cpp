/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FireMageStrategy.h"
#include "Playerbots.h"
#include "Strategy.h"

// ===== Action Node Factory =====
class FireMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FireMageStrategyActionNodeFactory()
    {
        creators["fireball"] = &fireball;
        creators["frostbolt"] = &frostbolt;
        creators["fire blast"] = &fire_blast;
        creators["pyroblast"] = &pyroblast;
        creators["scorch"] = &scorch;
        creators["living bomb"] = &living_bomb;
        creators["combustion"] = &combustion;
    }

private:
    static ActionNode* fireball(PlayerbotAI*) { return new ActionNode("fireball", nullptr, nullptr, nullptr); }
    static ActionNode* frostbolt(PlayerbotAI*) { return new ActionNode("frostbolt", nullptr, nullptr, nullptr); }
    static ActionNode* fire_blast(PlayerbotAI*) { return new ActionNode("fire blast", nullptr, nullptr, nullptr); }
    static ActionNode* pyroblast(PlayerbotAI*) { return new ActionNode("pyroblast", nullptr, nullptr, nullptr); }
    static ActionNode* scorch(PlayerbotAI*) { return new ActionNode("scorch", nullptr, nullptr, nullptr); }
    static ActionNode* living_bomb(PlayerbotAI*) { return new ActionNode("living bomb", nullptr, nullptr, nullptr); }
    static ActionNode* combustion(PlayerbotAI*) { return new ActionNode("combustion", nullptr, nullptr, nullptr); }
};

// ===== Single Target Strategy =====
FireMageStrategy::FireMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    actionNodeFactories.Add(new FireMageStrategyActionNodeFactory());
}

// ===== Default Actions =====
NextAction** FireMageStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("fireball", 5.3f),
                                new NextAction("frostbolt", 5.2f),   // fire immune target
                                new NextAction("fire blast", 5.1f),  // cast during movement
                                new NextAction("shoot", 5.0f), nullptr);
}

// ===== Trigger Initialization =====
void FireMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    // Debuff Triggers
    triggers.push_back(new TriggerNode("improved scorch", NextAction::array(0, new NextAction("scorch", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("living bomb", NextAction::array(0, new NextAction("living bomb", 18.5f), nullptr)));

    // Proc Trigger
    triggers.push_back(new TriggerNode("hot streak", NextAction::array(0, new NextAction("pyroblast", 25.0f), nullptr)));
}

// Combat strategy to run to melee for Dragon's Breath and Blast Wave
// Disabled by default for the Fire/Frostfire spec
// To enable, type "co +firestarter"
// To disable, type "co -firestarter"
FirestarterStrategy::FirestarterStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

void FirestarterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "blast wave off cd and medium aoe",
        NextAction::array(0, new NextAction("reach melee", 25.5f), nullptr)));
}
