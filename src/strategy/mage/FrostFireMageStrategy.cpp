/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FrostFireMageStrategy.h"
#include "Playerbots.h"

// ===== Action Node Factory =====
class FrostFireMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FrostFireMageStrategyActionNodeFactory()
    {
        creators["frostfire bolt"] = &frostfire_bolt;
        creators["fire blast"] = &fire_blast;
        creators["pyroblast"] = &pyroblast;
        creators["combustion"] = &combustion;
        creators["icy veins"] = &icy_veins;
        creators["scorch"] = &scorch;
        creators["living bomb"] = &living_bomb;
    }

private:
    static ActionNode* frostfire_bolt(PlayerbotAI*) { return new ActionNode("frostfire bolt", nullptr, nullptr, nullptr); }
    static ActionNode* fire_blast(PlayerbotAI*) { return new ActionNode("fire blast", nullptr, nullptr, nullptr); }
    static ActionNode* pyroblast(PlayerbotAI*) { return new ActionNode("pyroblast", nullptr, nullptr, nullptr); }
    static ActionNode* combustion(PlayerbotAI*) { return new ActionNode("combustion", nullptr, nullptr, nullptr); }
    static ActionNode* icy_veins(PlayerbotAI*) { return new ActionNode("icy veins", nullptr, nullptr, nullptr); }
    static ActionNode* scorch(PlayerbotAI*) { return new ActionNode("scorch", nullptr, nullptr, nullptr); }
    static ActionNode* living_bomb(PlayerbotAI*) { return new ActionNode("living bomb", nullptr, nullptr, nullptr); }
};

// ===== Single Target Strategy =====
FrostFireMageStrategy::FrostFireMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    actionNodeFactories.Add(new FrostFireMageStrategyActionNodeFactory());
}

// ===== Default Actions =====
NextAction** FrostFireMageStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("frostfire bolt", 5.2f),
                                new NextAction("fire blast", 5.1f),  // cast during movement
                                new NextAction("shoot", 5.0f), nullptr);
}

// ===== Trigger Initialization =====
void FrostFireMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    // Debuff Triggers
    triggers.push_back(new TriggerNode("improved scorch", NextAction::array(0, new NextAction("scorch", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("living bomb", NextAction::array(0, new NextAction("living bomb", 18.5f), nullptr)));

    // Proc Trigger
    triggers.push_back(new TriggerNode("hot streak", NextAction::array(0, new NextAction("pyroblast", 25.0f), nullptr)));
}
