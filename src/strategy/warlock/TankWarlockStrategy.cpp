/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TankWarlockStrategy.h"
#include "Playerbots.h"

// Combat strategy for a Warlock Tank, for certain bosses like Twin Emperors
// Priority is set to spam Searing Pain and use Shadow Ward on CD
// Disabled by default
// To enable, type "co +tank"
// To disable, type "co -tank"

// ===== Action Node Factory =====
class TankWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TankWarlockStrategyActionNodeFactory()
    {
        creators["shadow ward"] = &shadow_ward;
        creators["searing pain"] = &searing_pain;
    }

private:
    static ActionNode* shadow_ward(PlayerbotAI*) { return new ActionNode("shadow ward", nullptr, nullptr, nullptr); }
    static ActionNode* searing_pain(PlayerbotAI*) { return new ActionNode("searing pain", nullptr, nullptr, nullptr); }
};

// ===== Warlock Tank Combat Strategy =====
TankWarlockStrategy::TankWarlockStrategy(PlayerbotAI* botAI) : GenericWarlockStrategy(botAI)
{
    actionNodeFactories.Add(new TankWarlockStrategyActionNodeFactory());
}

NextAction** TankWarlockStrategy::getDefaultActions()
{
    // Shadow Ward is the highest priority, Searing Pain next.
    return NextAction::array(0, new NextAction("shadow ward", 27.5f), new NextAction("searing pain", 27.0f), nullptr);
}

void TankWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
}
