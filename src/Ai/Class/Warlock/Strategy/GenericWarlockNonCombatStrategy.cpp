/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
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
                              /*P*/ {},
                              /*A*/ { NextAction("demon armor") },
                              /*C*/ {});
    }

    static ActionNode* demon_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("demon armor",
                              /*P*/ {},
                              /*A*/ { NextAction("demon skin") },
                              /*C*/ {});
    }
    static ActionNode* summon_voidwalker([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("summon voidwalker",
                              /*P*/ {},
                              /*A*/ { NextAction("summon imp") },
                              /*C*/ {});
    }
    static ActionNode* summon_succubus([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("summon succubus",
                              /*P*/ {},
                              /*A*/ { NextAction("summon voidwalker") },
                              /*C*/ {});
    }
    static ActionNode* summon_felhunter([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("summon felhunter",
                              /*P*/ {},
                              /*A*/ { NextAction("summon succubus") },
                              /*C*/ {});
    }
    static ActionNode* summon_felguard([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("summon felguard",
                              /*P*/ {},
                              /*A*/ { NextAction("summon felhunter") },
                              /*C*/ {});
    }
};

GenericWarlockNonCombatStrategy::GenericWarlockNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarlockNonCombatStrategyActionNodeFactory());
}

void GenericWarlockNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("has pet", { NextAction("toggle pet spell", 60.0f) }));
    triggers.push_back(new TriggerNode("new pet", { NextAction("set pet stance", 60.0f) }));
    triggers.push_back(new TriggerNode("no pet", { NextAction("fel domination", 30.0f) }));
    triggers.push_back(new TriggerNode("no soul shard", { NextAction("create soul shard", 60.0f) }));
    triggers.push_back(new TriggerNode("too many soul shards", { NextAction("destroy soul shard", 60.0f) }));
    triggers.push_back(new TriggerNode("soul link", { NextAction("soul link", 28.0f) }));
    triggers.push_back(new TriggerNode("demon armor", { NextAction("fel armor", 27.0f) }));
    triggers.push_back(new TriggerNode("unending breath", { NextAction("unending breath", 12.0f) }));
    triggers.push_back(new TriggerNode("unending breath on party", { NextAction("unending breath on party", 11.0f) }));
    triggers.push_back(new TriggerNode("no healthstone", { NextAction("create healthstone", 26.0f) }));
    triggers.push_back(new TriggerNode("no soulstone", { NextAction("create soulstone", 25.0f) }));
    triggers.push_back(new TriggerNode("life tap", { NextAction("life tap", 23.0f) }));
}

// Non-combat strategy for summoning a Imp
// Enabled by default for the Destruction spec
// To enable, type "nc +imp"
// To disable, type "nc -imp"
SummonImpStrategy::SummonImpStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

void SummonImpStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", { NextAction("summon imp", 29.0f) }));
    triggers.push_back(new TriggerNode("wrong pet", { NextAction("summon imp", 29.0f) }));
}

// Non-combat strategy for summoning a Voidwalker
// Disabled by default
// To enable, type "nc +voidwalker"
// To disable, type "nc -voidwalker"
SummonVoidwalkerStrategy::SummonVoidwalkerStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

void SummonVoidwalkerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", { NextAction("summon voidwalker", 29.0f) }));
    triggers.push_back(new TriggerNode("wrong pet", { NextAction("summon voidwalker", 29.0f) }));
}

// Non-combat strategy for summoning a Succubus
// Disabled by default
// To enable, type "nc +succubus"
// To disable, type "nc -succubus"
SummonSuccubusStrategy::SummonSuccubusStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

void SummonSuccubusStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", { NextAction("summon succubus", 29.0f) }));
    triggers.push_back(new TriggerNode("wrong pet", { NextAction("summon succubus", 29.0f) }));
}

// Non-combat strategy for summoning a Felhunter
// Enabled by default for the Affliction spec
// To enable, type "nc +felhunter"
// To disable, type "nc -felhunter"
SummonFelhunterStrategy::SummonFelhunterStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

void SummonFelhunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", { NextAction("summon felhunter", 29.0f) }));
    triggers.push_back(new TriggerNode("wrong pet", { NextAction("summon felhunter", 29.0f) }));
}

// Non-combat strategy for summoning a Felguard
// Enabled by default for the Demonology spec
// To enable, type "nc +felguard"
// To disable, type "nc -felguard"
SummonFelguardStrategy::SummonFelguardStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

void SummonFelguardStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", { NextAction("summon felguard", 29.0f) }));
    triggers.push_back(new TriggerNode("wrong pet", { NextAction("summon felguard", 29.0f) }));
}

// Non-combat strategy for selecting themselves to receive soulstone
// Disabled by default
// To enable, type "nc +ss self"
// To disable, type "nc -ss self"
SoulstoneSelfStrategy::SoulstoneSelfStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

void SoulstoneSelfStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("soulstone", { NextAction("soulstone self", 24.0f) }));
}

// Non-combat strategy for selecting the master to receive soulstone
// Disabled by default
// To enable, type "nc +ss master"
// To disable, type "nc -ss master"
SoulstoneMasterStrategy::SoulstoneMasterStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

void SoulstoneMasterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("soulstone", { NextAction("soulstone master", 24.0f) }));
}

// Non-combat strategy for selecting tanks to receive soulstone
// Disabled by default
// To enable, type "nc +ss tank"
// To disable, type "nc -ss tank"
SoulstoneTankStrategy::SoulstoneTankStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

void SoulstoneTankStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("soulstone", { NextAction("soulstone tank", 24.0f) }));
}

// Non-combat strategy for selecting healers to receive soulstone
// Disabled by default
// To enable, type "nc +ss healer"
// To disable, type "nc -ss healer"
SoulstoneHealerStrategy::SoulstoneHealerStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

void SoulstoneHealerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("soulstone", { NextAction("soulstone healer", 24.0f) }));
}

// Non-combat strategy for using Spellstone
// Enabled by default for Affliction and Demonology specs
// To enable, type "nc +spellstone"
// To disable, type "nc -spellstone"
UseSpellstoneStrategy::UseSpellstoneStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

void UseSpellstoneStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no spellstone", { NextAction("create spellstone", 24.0f) }));
    triggers.push_back(new TriggerNode("spellstone", { NextAction("spellstone", 24.0f) }));
}

// Non-combat strategy for using Firestone
// Enabled by default for the Destruction spec
// To enable, type "nc +firestone"
// To disable, type "nc -firestone"
UseFirestoneStrategy::UseFirestoneStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

void UseFirestoneStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no firestone", { NextAction("create firestone", 24.0f) }));
    triggers.push_back(new TriggerNode("firestone", { NextAction("firestone", 24.0f) }));
}
