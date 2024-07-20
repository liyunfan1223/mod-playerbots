/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FrostMageStrategy.h"
#include "Playerbots.h"

class FrostMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FrostMageStrategyActionNodeFactory()
    {
        creators["cold snap"] = &cold_snap;
        creators["ice barrier"] = &ice_barrier;
        creators["summon water elemental"] = &summon_water_elemental;
        creators["deep freeze"] = &deep_freeze;
        creators["frostbolt and deep freeze"] = &frostbolt_and_deep_freeze;
    }
private:
    static ActionNode* cold_snap([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("cold snap",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }
    
    static ActionNode* ice_barrier([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("ice barrier",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }
    
    static ActionNode* summon_water_elemental([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("summon water elemental",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }
    
    static ActionNode* deep_freeze([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("deep freeze",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("ice lance"), nullptr),
                /*C*/ nullptr);
        }
    
    static ActionNode* frostbolt_and_deep_freeze([[maybe_unused]] PlayerbotAI* botAI)
    // Combo cast the last charge of fingers of frost for double crits.
    // Should only do this on the final charge of FoF.
    {
        return new ActionNode ("frostbolt",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ NextAction::array(0, new NextAction("deep freeze"), nullptr));
    }
};

FrostMageStrategy::FrostMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    actionNodeFactories.Add(new FrostMageStrategyActionNodeFactory());
}

NextAction** FrostMageStrategy::getDefaultActions()
{
    return NextAction::array(0, 
        new NextAction("frostbolt", ACTION_DEFAULT + 0.1f),
        new NextAction("shoot", ACTION_DEFAULT),
        nullptr);

}

void FrostMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("icy veins", NextAction::array(0, new NextAction("icy veins", 50.0f), nullptr)));
    // No logic currently for cold snap usage.. possibly use right after icy veins drops off?
    // triggers.push_back(new TriggerNode("cold snap", NextAction::array(0, new NextAction("cold snap", 50.0f), nullptr)));

    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon water elemental", 70.0f), nullptr)));
    triggers.push_back(new TriggerNode("ice barrier", NextAction::array(0, new NextAction("ice barrier", 30.0f), nullptr)));
    
    triggers.push_back(new TriggerNode("brain freeze", NextAction::array(0, new NextAction("frostfire bolt", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("fingers of frost single", NextAction::array(0, new NextAction("frostbolt and deep freeze", 20.0f), nullptr)));
    // May not need this, frostbolt is the default action so probably don't need to specify.
    // Maybe uncomment if you find the mage is prioritising auxillary spells while this buff is up, and wasting the proc.
    // triggers.push_back(new TriggerNode("fingers of frost double", NextAction::array(0, new NextAction("frostbolt", 10.0f), nullptr)));
}

void FrostMageAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("blizzard", 40.0f), nullptr)));
    triggers.push_back(new TriggerNode("light aoe", NextAction::array(0, new NextAction("cone of cold", 45.0f), nullptr)));
}
