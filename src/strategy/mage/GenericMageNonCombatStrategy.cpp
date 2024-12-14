/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericMageNonCombatStrategy.h"

#include "Playerbots.h"

class GenericMageNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageNonCombatStrategyActionNodeFactory()
    {
        creators["molten armor"] = &molten_armor;
        creators["mage armor"] = &mage_armor;
        creators["ice armor"] = &ice_armor;
    }

private:
    static ActionNode* molten_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("molten armor",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("mage armor"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* mage_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mage armor",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("ice armor"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* ice_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("ice armor",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("frost armor"), nullptr),
                              /*C*/ nullptr);
    }
};

GenericMageNonCombatStrategy::GenericMageNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericMageNonCombatStrategyActionNodeFactory());
}

void GenericMageNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("arcane intellect", NextAction::array(0, new NextAction("arcane intellect", 21.0f), nullptr)));
    // triggers.push_back(new TriggerNode("no drink", NextAction::array(0, new NextAction("conjure water", 16.0f),
    // nullptr))); triggers.push_back(new TriggerNode("no food", NextAction::array(0, new NextAction("conjure
    // food", 15.0f), nullptr)));
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply oil", 1.0f), nullptr)));
}

void MageBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("mage armor", NextAction::array(0, new NextAction("mage armor", 19.0f), nullptr)));
}

void MageBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("mage armor", NextAction::array(0, new NextAction("molten armor", 19.0f), nullptr)));
}

void MageBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("arcane intellect on party",
                        NextAction::array(0, new NextAction("arcane intellect on party", 20.0f), nullptr)));
    // triggers.push_back(new TriggerNode("give water", NextAction::array(0, new NextAction("give water", 14.0f),
    // nullptr))); triggers.push_back(new TriggerNode("give food", NextAction::array(0, new NextAction("give
    // food", 13.0f), nullptr)));
}
