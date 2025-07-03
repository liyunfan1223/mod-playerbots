/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericWarlockStrategy.h"
#include "Strategy.h"
#include "Playerbots.h"

class GenericWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarlockStrategyActionNodeFactory()
    {
        creators["banish on cc"] = &banish_on_cc;
        creators["fear on cc"] = &fear_on_cc;
        creators["spell lock"] = &spell_lock;
        creators["devour magic purge"] = &devour_magic_purge;
        creators["devour magic cleanse"] = &devour_magic_cleanse;
    }

private:
    static ActionNode* banish_on_cc(PlayerbotAI*) { return new ActionNode("banish on cc", nullptr, nullptr, nullptr); }
    static ActionNode* fear_on_cc(PlayerbotAI*) { return new ActionNode("fear on cc", nullptr, nullptr, nullptr); }
    static ActionNode* spell_lock(PlayerbotAI*) { return new ActionNode("spell lock", nullptr, nullptr, nullptr); }
    static ActionNode* devour_magic_purge(PlayerbotAI*) { return new ActionNode("devour magic purge", nullptr, nullptr, nullptr); }
    static ActionNode* devour_magic_cleanse(PlayerbotAI*) { return new ActionNode("devour magic cleanse", nullptr, nullptr, nullptr); }
};

GenericWarlockStrategy::GenericWarlockStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarlockStrategyActionNodeFactory());
}

NextAction** GenericWarlockStrategy::getDefaultActions() { return NextAction::array(0, nullptr); }

void GenericWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("life tap", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode("medium threat", NextAction::array(0, new NextAction("soulshatter", 55.0f), nullptr)));
    triggers.push_back(new TriggerNode("spell lock", NextAction::array(0, new NextAction("spell lock", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode("devour magic purge", NextAction::array(0, new NextAction("devour magic purge", ACTION_DISPEL), nullptr)));
    triggers.push_back(new TriggerNode("devour magic cleanse", NextAction::array(0, new NextAction("devour magic cleanse", ACTION_DISPEL), nullptr)));
}

void WarlockBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Placeholder for future boost triggers
}

void WarlockPetStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Placeholder for future pet triggers
}

void WarlockCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("banish", NextAction::array(0, new NextAction("banish on cc", 33.0f), nullptr)));
    triggers.push_back(new TriggerNode("fear", NextAction::array(0, new NextAction("fear on cc", 32.0f), nullptr)));
}

// Combat strategy for using Curse of the Elements
// Enabling this will turn off their use of Curse of Agony
// Enabled by default for the Destruction spec
// To enable, type "co +curse of elements"
// To disable, type "co -curse of elements"

void WarlockCurseOfTheElementsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("curse of the elements", NextAction::array(0, new NextAction("curse of the elements", 30.0f), nullptr)));
}
