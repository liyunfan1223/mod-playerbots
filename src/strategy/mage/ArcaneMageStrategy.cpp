/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ArcaneMageStrategy.h"
#include "Playerbots.h"

// ===== Action Node Factory =====
class ArcaneMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ArcaneMageStrategyActionNodeFactory()
    {
        creators["arcane blast"] = &arcane_blast;
        creators["arcane barrage"] = &arcane_barrage;
        creators["arcane missiles"] = &arcane_missiles;
        creators["fire blast"] = &fire_blast;
        creators["frostbolt"] = &frostbolt;
        creators["arcane power"] = &arcane_power;
        creators["icy veins"] = &icy_veins;
    }

private:
    static ActionNode* arcane_blast(PlayerbotAI*) { return new ActionNode("arcane blast", nullptr, nullptr, nullptr); }
    static ActionNode* arcane_barrage(PlayerbotAI*) { return new ActionNode("arcane barrage", nullptr, nullptr, nullptr); }
    static ActionNode* arcane_missiles(PlayerbotAI*) { return new ActionNode("arcane missiles", nullptr, nullptr, nullptr); }
    static ActionNode* fire_blast(PlayerbotAI*) { return new ActionNode("fire blast", nullptr, nullptr, nullptr); }
    static ActionNode* frostbolt(PlayerbotAI*) { return new ActionNode("frostbolt", nullptr, nullptr, nullptr); }
    static ActionNode* arcane_power(PlayerbotAI*) { return new ActionNode("arcane power", nullptr, nullptr, nullptr); }
    static ActionNode* icy_veins(PlayerbotAI*) { return new ActionNode("icy veins", nullptr, nullptr, nullptr); }
};

// ===== Single Target Strategy =====
ArcaneMageStrategy::ArcaneMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    actionNodeFactories.Add(new ArcaneMageStrategyActionNodeFactory());
}

// ===== Default Actions =====
NextAction** ArcaneMageStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("arcane blast", 5.6f),
                                new NextAction("arcane missiles", 5.5f),
                                new NextAction("arcane barrage", 5.4f),   // cast while moving
                                new NextAction("fire blast", 5.3f),       // cast while moving if arcane barrage isn't available/learned
                                new NextAction("frostbolt", 5.2f),        // for arcane immune targets
                                new NextAction("shoot", 5.1f), nullptr);
}

// ===== Trigger Initialization ===
void ArcaneMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    // Proc Trigger
    triggers.push_back(new TriggerNode("arcane blast 4 stacks and missile barrage", NextAction::array(0, new NextAction("arcane missiles", 15.0f), nullptr)));
}
