/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericWarlockNonCombatStrategy.h"
#include "AiFactory.h"
#include "Playerbots.h"

class GenericWarlockNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarlockNonCombatStrategyActionNodeFactory()
    {
        creators["fel armor"] = &fel_armor;
        creators["demon armor"] = &demon_armor;
        creators["summon voidwalker"] = &summon_voidwalker;
        creators["summon felguard"] = &summon_felguard;
        creators["summon succubus"] = &summon_succubus;
        creators["summon felhunter"] = &summon_felhunter;
    }

    // Pet skills are setup in pass-through fashion, so if one fails, it attempts to cast the next one
    // The order goes Felguard -> Felhunter -> Succubus -> Voidwalker -> Imp
    // Pets are summoned based on the non-combat strategy you have active, the warlock's level, and if they have a soul shard available

private:
    static ActionNode* fel_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("fel armor",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("demon armor"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* demon_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("demon armor",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("demon skin"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* summon_voidwalker([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("summon voidwalker",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("summon imp"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* summon_succubus([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("summon succubus",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("summon voidwalker"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* summon_felhunter([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("summon felhunter",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("summon succubus"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* summon_felguard([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("summon felguard",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("summon felhunter"), nullptr),
                              /*C*/ nullptr);
    }
};

GenericWarlockNonCombatStrategy::GenericWarlockNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarlockNonCombatStrategyActionNodeFactory());
}

void GenericWarlockNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("has pet", NextAction::array(0, new NextAction("toggle pet spell", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("fel domination", 30.0f), nullptr)));
    triggers.push_back(new TriggerNode("no soul shard", NextAction::array(0, new NextAction("create soul shard", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("too many soul shards", NextAction::array(0, new NextAction("destroy soul shard", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("soul link", NextAction::array(0, new NextAction("soul link", 28.0f), nullptr)));
    triggers.push_back(new TriggerNode("demon armor", NextAction::array(0, new NextAction("fel armor", 27.0f), nullptr)));
    triggers.push_back(new TriggerNode("no healthstone", NextAction::array(0, new NextAction("create healthstone", 26.0f), nullptr)));
    triggers.push_back(new TriggerNode("no soulstone", NextAction::array(0, new NextAction("create soulstone", 25.0f), nullptr)));
    triggers.push_back(new TriggerNode("life tap", NextAction::array(0, new NextAction("life tap", 23.0f), nullptr)));
}

// Non-combat strategy for summoning a Imp
// Enabled by default for the Destruction spec
// To enable, type "nc +imp"
// To disable, type "nc -imp"
SummonImpStrategy::SummonImpStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SummonImpStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon imp", 29.0f), NULL)));
    triggers.push_back(new TriggerNode("wrong pet", NextAction::array(0, new NextAction("summon imp", 29.0f), NULL)));
}

// Non-combat strategy for summoning a Voidwalker
// Disabled by default
// To enable, type "nc +voidwalker"
// To disable, type "nc -voidwalker"
SummonVoidwalkerStrategy::SummonVoidwalkerStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SummonVoidwalkerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon voidwalker", 29.0f), NULL)));
    triggers.push_back(new TriggerNode("wrong pet", NextAction::array(0, new NextAction("summon voidwalker", 29.0f), NULL)));
}

// Non-combat strategy for summoning a Succubus
// Disabled by default
// To enable, type "nc +succubus"
// To disable, type "nc -succubus"
SummonSuccubusStrategy::SummonSuccubusStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SummonSuccubusStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon succubus", 29.0f), NULL)));
    triggers.push_back(new TriggerNode("wrong pet", NextAction::array(0, new NextAction("summon succubus", 29.0f), NULL)));
}

// Non-combat strategy for summoning a Felhunter
// Enabled by default for the Affliction spec
// To enable, type "nc +felhunter"
// To disable, type "nc -felhunter"
SummonFelhunterStrategy::SummonFelhunterStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SummonFelhunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon felhunter", 29.0f), NULL)));
    triggers.push_back(new TriggerNode("wrong pet", NextAction::array(0, new NextAction("summon felhunter", 29.0f), NULL)));
}

// Non-combat strategy for summoning a Felguard
// Enabled by default for the Demonology spec
// To enable, type "nc +felguard"
// To disable, type "nc -felguard"
SummonFelguardStrategy::SummonFelguardStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SummonFelguardStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon felguard", 29.0f), NULL)));
    triggers.push_back(new TriggerNode("wrong pet", NextAction::array(0, new NextAction("summon felguard", 29.0f), NULL)));
}

// Non-combat strategy for selecting themselves to receive soulstone
// Disabled by default
// To enable, type "nc +ss self"
// To disable, type "nc -ss self"
SoulstoneSelfStrategy::SoulstoneSelfStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SoulstoneSelfStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("soulstone", NextAction::array(0, new NextAction("soulstone self", 24.0f), NULL)));
}

// Non-combat strategy for selecting the master to receive soulstone
// Disabled by default
// To enable, type "nc +ss master"
// To disable, type "nc -ss master"
SoulstoneMasterStrategy::SoulstoneMasterStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SoulstoneMasterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("soulstone", NextAction::array(0, new NextAction("soulstone master", 24.0f), NULL)));
}

// Non-combat strategy for selecting tanks to receive soulstone
// Disabled by default
// To enable, type "nc +ss tank"
// To disable, type "nc -ss tank"
SoulstoneTankStrategy::SoulstoneTankStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SoulstoneTankStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("soulstone", NextAction::array(0, new NextAction("soulstone tank", 24.0f), NULL)));
}

// Non-combat strategy for selecting healers to receive soulstone
// Disabled by default
// To enable, type "nc +ss healer"
// To disable, type "nc -ss healer"
SoulstoneHealerStrategy::SoulstoneHealerStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SoulstoneHealerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("soulstone", NextAction::array(0, new NextAction("soulstone healer", 24.0f), NULL)));
}

// Non-combat strategy for using Spellstone
// Enabled by default for Affliction and Demonology specs
// To enable, type "nc +spellstone"
// To disable, type "nc -spellstone"
UseSpellstoneStrategy::UseSpellstoneStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void UseSpellstoneStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no spellstone", NextAction::array(0, new NextAction("create spellstone", 24.0f), nullptr)));
    triggers.push_back(new TriggerNode("spellstone", NextAction::array(0, new NextAction("spellstone", 24.0f), nullptr)));
}

// Non-combat strategy for using Firestone
// Enabled by default for the Destruction spec
// To enable, type "nc +firestone"
// To disable, type "nc -firestone"
UseFirestoneStrategy::UseFirestoneStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void UseFirestoneStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no firestone", NextAction::array(0, new NextAction("create firestone", 24.0f), nullptr)));
    triggers.push_back(new TriggerNode("firestone", NextAction::array(0, new NextAction("firestone", 24.0f), nullptr)));
}
