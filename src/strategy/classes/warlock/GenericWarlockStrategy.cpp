/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericWarlockStrategy.h"

#include "Playerbots.h"

class GenericWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarlockStrategyActionNodeFactory()
    {
        // creators["summon voidwalker"] = &summon_voidwalker;
        creators["banish"] = &banish;
    }

private:
    // static ActionNode* summon_voidwalker([[maybe_unused]] PlayerbotAI* botAI)
    //{
    // return new ActionNode ("summon voidwalker",
    /*P*/  // nullptr,
    /*A*/  // NextAction::array(0, new NextAction("drain soul"), nullptr),
    /*C*/  // nullptr);
    //}
    static ActionNode* banish([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("banish",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("fear"), nullptr),
                              /*C*/ nullptr);
    }
};

GenericWarlockStrategy::GenericWarlockStrategy(PlayerbotAI* botAI) : RangedCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarlockStrategyActionNodeFactory());
}

NextAction** GenericWarlockStrategy::getDefaultActions() { return NextAction::array(0, nullptr); }

void GenericWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);

    // triggers.push_back(new TriggerNode("shadow trance", NextAction::array(0, new NextAction("shadow bolt", 20.0f),
    // nullptr))); triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("drain
    // life", 40.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("life tap", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(
        new TriggerNode("target critical health", NextAction::array(0, new NextAction("drain soul", 30.0f), nullptr)));
    // triggers.push_back(new TriggerNode("immolate", NextAction::array(0, new NextAction("immolate", 13.0f), new
    // NextAction("conflagrate", 13.0f), nullptr))); triggers.push_back(new TriggerNode("enemy too close for spell",
    // NextAction::array(0, new NextAction("flee", 49.0f), NULL)));
}

void WarlockBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("amplify curse", NextAction::array(0, new NextAction("amplify curse", 41.0f), nullptr)));
}

void WarlockCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("banish", NextAction::array(0, new NextAction("banish on cc", 32.0f), nullptr)));
    triggers.push_back(new TriggerNode("fear", NextAction::array(0, new NextAction("fear on cc", 33.0f), nullptr)));
}
