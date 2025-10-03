/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
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

    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("life tap", 95.0f), nullptr)));
    triggers.push_back(new TriggerNode("medium threat", NextAction::array(0, new NextAction("soulshatter", 55.0f), nullptr)));
    triggers.push_back(new TriggerNode("spell lock", NextAction::array(0, new NextAction("spell lock", 40.0f), nullptr)));
    triggers.push_back(new TriggerNode("no soul shard", NextAction::array(0, new NextAction("create soul shard", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("too many soul shards", NextAction::array(0, new NextAction("destroy soul shard", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("devour magic purge", NextAction::array(0, new NextAction("devour magic purge", 50.0f), nullptr)));
    triggers.push_back(new TriggerNode("devour magic cleanse", NextAction::array(0, new NextAction("devour magic cleanse", 50.0f), nullptr)));
}

// ===== AoE Strategy, 3+ enemies =====

void AoEWarlockStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0,
                                                 new NextAction("immolation aura", 26.0f),
                                                 new NextAction("shadowfury", 23.0f),
                                                 new NextAction("shadowflame", 22.5f),
                                                 new NextAction("seed of corruption on attacker", 22.0f),
                                                 new NextAction("seed of corruption", 21.5f),
                                                 new NextAction("rain of fire", 21.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("rain of fire channel check", NextAction::array(0, new NextAction("cancel channel", 21.5f), nullptr)));
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

// Combat strategy for using Curse of Agony
// Enabled by default for the Affliction spec
// To enable, type "co +curse of agony"
// To disable, type "co -curse of agony"
void WarlockCurseOfAgonyStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("curse of agony on attacker", NextAction::array(0, new NextAction("curse of agony on attacker", 18.5f), nullptr)));
    triggers.push_back(new TriggerNode("curse of agony", NextAction::array(0, new NextAction("curse of agony", 17.0f), nullptr)));
}

// Combat strategy for using Curse of the Elements
// Enabled by default for the Destruction spec
// To enable, type "co +curse of elements"
// To disable, type "co -curse of elements"
void WarlockCurseOfTheElementsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("curse of the elements", NextAction::array(0, new NextAction("curse of the elements", 29.0f), nullptr)));
}

// Combat strategy for using Curse of Doom
// Disabled by default
// To enable, type "co +curse of doom"
// To disable, type "co -curse of doom"
void WarlockCurseOfDoomStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("curse of doom", NextAction::array(0, new NextAction("curse of doom", 29.0f), nullptr)));
}

// Combat strategy for using Curse of Exhaustion
// Disabled by default
// To enable, type "co +curse of exhaustion"
// To disable, type "co -curse of exhaustion"
void WarlockCurseOfExhaustionStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("curse of exhaustion", NextAction::array(0, new NextAction("curse of exhaustion", 29.0f), nullptr)));
}

// Combat strategy for using Curse of Tongues
// Disabled by default
// To enable, type "co +curse of tongues"
// To disable, type "co -curse of tongues"
void WarlockCurseOfTonguesStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("curse of tongues", NextAction::array(0, new NextAction("curse of tongues", 29.0f), nullptr)));
}

// Combat strategy for using Curse of Weakness
// Disabled by default
// To enable, type "co +curse of weakness"
// To disable, type "co -curse of weakness"
void WarlockCurseOfWeaknessStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("curse of weakness", NextAction::array(0, new NextAction("curse of weakness", 29.0f), nullptr)));
}
