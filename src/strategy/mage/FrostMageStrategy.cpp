/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FrostMageStrategy.h"

#include "Playerbots.h"

// ===== Action Node Factory =====
class FrostMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FrostMageStrategyActionNodeFactory()
    {
        creators["cold snap"] = &cold_snap;
        creators["ice barrier"] = &ice_barrier;
        creators["summon water elemental"] = &summon_water_elemental;
        creators["deep freeze"] = &deep_freeze;
        creators["icy veins"] = &icy_veins;
        creators["frostbolt"] = &frostbolt;
        creators["ice lance"] = &ice_lance;
        creators["fire blast"] = &fire_blast;
        creators["fireball"] = &fireball;
        creators["frostfire bolt"] = &frostfire_bolt;
    }

private:
    static ActionNode* cold_snap(PlayerbotAI*) { return new ActionNode("cold snap", nullptr, nullptr, nullptr); }
    static ActionNode* ice_barrier(PlayerbotAI*) { return new ActionNode("ice barrier", nullptr, nullptr, nullptr); }
    static ActionNode* summon_water_elemental(PlayerbotAI*) { return new ActionNode("summon water elemental", nullptr, nullptr, nullptr); }
    static ActionNode* deep_freeze(PlayerbotAI*) { return new ActionNode("deep freeze", nullptr, nullptr, nullptr); }
    static ActionNode* icy_veins(PlayerbotAI*) { return new ActionNode("icy veins", nullptr, nullptr, nullptr); }
    static ActionNode* frostbolt(PlayerbotAI*) { return new ActionNode("frostbolt", nullptr, nullptr, nullptr); }
    static ActionNode* ice_lance(PlayerbotAI*) { return new ActionNode("ice lance", nullptr, nullptr, nullptr); }
    static ActionNode* fire_blast(PlayerbotAI*) { return new ActionNode("fire blast", nullptr, nullptr, nullptr); }
    static ActionNode* fireball(PlayerbotAI*) { return new ActionNode("fireball", nullptr, nullptr, nullptr); }
    static ActionNode* frostfire_bolt(PlayerbotAI*) { return new ActionNode("frostfire bolt", nullptr, nullptr, nullptr); }
};

// ===== Single Target Strategy =====
FrostMageStrategy::FrostMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    actionNodeFactories.Add(new FrostMageStrategyActionNodeFactory());
}

// ===== Default Actions =====
NextAction** FrostMageStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("frostbolt", 5.4f),
                                new NextAction("ice lance", 5.3f),   // cast during movement
                                new NextAction("fire blast", 5.2f),  // cast during movement if ice lance is not learned
                                new NextAction("shoot", 5.1f),
                                new NextAction("fireball", 5.0f), nullptr);
}

// ===== Trigger Initialization ===
void FrostMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    // Pet/Defensive triggers
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon water elemental", 30.0f), nullptr)));
    triggers.push_back(new TriggerNode("has pet", NextAction::array(0, new NextAction("toggle pet spell", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("ice barrier", 29.0f), nullptr)));
    triggers.push_back(new TriggerNode("being attacked", NextAction::array(0, new NextAction("ice barrier", 29.0f), nullptr)));

    // Proc/Freeze triggers
    triggers.push_back(new TriggerNode("brain freeze", NextAction::array(0, new NextAction("frostfire bolt", 19.5f), nullptr)));
    triggers.push_back(new TriggerNode("fingers of frost", NextAction::array(0,
                                                       new NextAction("deep freeze", 19.0f),
                                                       new NextAction("frostbolt", 18.0f), nullptr)));

    triggers.push_back(new TriggerNode("frostbite on target", NextAction::array(0,
                                                          new NextAction("deep freeze", 19.0f),
                                                          new NextAction("frostbolt", 18.0f), nullptr)));

    triggers.push_back(new TriggerNode("frost nova on target", NextAction::array(0,
                                                           new NextAction("deep freeze", 19.0f),
                                                           new NextAction("frostbolt", 18.0f), nullptr)));

}
