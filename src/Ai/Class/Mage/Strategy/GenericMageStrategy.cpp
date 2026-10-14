/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericMageStrategy.h"
#include "AiFactory.h"
#include "Playerbots.h"
#include "RangedCombatStrategy.h"

namespace
{
constexpr uint32 SPELL_CONJURE_MANA_SAPPHIRE = 42985;
constexpr uint32 SPELL_CONJURE_MANA_EMERALD = 27101;
constexpr uint32 SPELL_CONJURE_MANA_RUBY = 10054;
constexpr uint32 SPELL_CONJURE_MANA_CITRINE = 10053;
constexpr uint32 SPELL_CONJURE_MANA_JADE = 3552;
constexpr uint32 SPELL_CONJURE_MANA_AGATE = 759;
constexpr uint32 SPELL_FROSTFIRE_BOLT = 44614;
constexpr uint32 SPELL_ICE_SHARDS = 15047;
}

class GenericMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageStrategyActionNodeFactory()
    {
        creators["frostbolt"] = &frostbolt;
        creators["frostfire bolt"] = &frostfire_bolt;
        creators["scorch"] = &scorch;
        creators["remove curse"] = &remove_curse;
        creators["remove curse on party"] = &remove_curse_on_party;
        creators["fireball"] = &fireball;
    }

private:
    static ActionNode* frostbolt([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frostbolt",
                              /*P*/ {},
                              /*A*/ { NextAction("shoot") },
                              /*C*/ {});
    }

    static ActionNode* frostfire_bolt([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frostfire bolt",
                              /*P*/ {},
                              /*A*/ { NextAction("fireball") },
                              /*C*/ {});
    }

    static ActionNode* scorch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("scorch",
                              /*P*/ {},
                              /*A*/ { NextAction("shoot") },
                              /*C*/ {});
    }

    static ActionNode* remove_curse([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("remove curse",
                              /*P*/ {},
                              /*A*/ { NextAction("remove lesser curse") },
                              /*C*/ {});
    }

    static ActionNode* remove_curse_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("remove curse on party",
                              /*P*/ {},
                              /*A*/ { NextAction("remove lesser curse on party") },
                              /*C*/ {});
    }
    static ActionNode* fireball([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("fireball",
                              /*P*/ {},
                              /*A*/ { NextAction("shoot") },
                              /*C*/ {});
    }
};

GenericMageStrategy::GenericMageStrategy(PlayerbotAI* botAI) : RangedCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericMageStrategyActionNodeFactory());
}

void GenericMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);

    // Threat Triggers
    triggers.push_back(new TriggerNode("high threat", { NextAction("mirror image", 60.0f) }));
    triggers.push_back(new TriggerNode("medium threat", { NextAction("invisibility", 30.0f) }));

    // Defensive Triggers
    triggers.push_back(new TriggerNode("critical health", { NextAction("ice block", 90.0f) }));
    triggers.push_back(new TriggerNode("low health", { NextAction("mana shield", 85.0f) }));
    triggers.push_back(new TriggerNode("fire ward", { NextAction("fire ward", 90.0f) }));
    triggers.push_back(new TriggerNode("frost ward", { NextAction("frost ward", 90.0f) }));
    triggers.push_back(new TriggerNode("enemy is close and no firestarter strategy", { NextAction("frost nova", 50.0f) }));
    triggers.push_back(new TriggerNode("enemy too close for spell and no firestarter strategy", { NextAction("blink back", 35.0f) }));

    // Mana Threshold Triggers
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(SPELL_CONJURE_MANA_SAPPHIRE))
        triggers.push_back(new TriggerNode("high mana", { NextAction("use mana sapphire", 90.0f) }));
    else if (bot->HasSpell(SPELL_CONJURE_MANA_EMERALD))
        triggers.push_back(new TriggerNode("high mana", { NextAction("use mana emerald", 90.0f) }));
    else if (bot->HasSpell(SPELL_CONJURE_MANA_RUBY))
        triggers.push_back(new TriggerNode("high mana", { NextAction("use mana ruby", 90.0f) }));
    else if (bot->HasSpell(SPELL_CONJURE_MANA_CITRINE))
        triggers.push_back(new TriggerNode("high mana", { NextAction("use mana citrine", 90.0f) }));
    else if (bot->HasSpell(SPELL_CONJURE_MANA_JADE))
        triggers.push_back(new TriggerNode("high mana", { NextAction("use mana jade", 90.0f) }));
    else if (bot->HasSpell(SPELL_CONJURE_MANA_AGATE))
        triggers.push_back(new TriggerNode("high mana", { NextAction("use mana agate", 90.0f) }));

    triggers.push_back(new TriggerNode("low mana", { NextAction("evocation", 90.0f) }));

    // Counterspell / Spellsteal Triggers
    triggers.push_back(new TriggerNode("spellsteal", { NextAction("spellsteal", 40.0f) }));
    triggers.push_back(new TriggerNode("counterspell on enemy healer", { NextAction("counterspell on enemy healer", 40.0f) }));
}

void MageCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("remove curse", { NextAction("remove curse", 41.0f) }));
    triggers.push_back(new TriggerNode("remove curse on party", { NextAction("remove curse on party", 40.0f) }));
}

void MageBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == MAGE_TAB_ARCANE)
    {
        triggers.push_back(new TriggerNode("arcane power", { NextAction("arcane power", 29.0f) }));
        triggers.push_back(new TriggerNode("icy veins", { NextAction("icy veins", 28.5f) }));
        triggers.push_back(new TriggerNode("mirror image", { NextAction("mirror image", 28.0f) }));
    }
    else if (tab == MAGE_TAB_FIRE)
    {
        if (bot->HasSpell(SPELL_FROSTFIRE_BOLT) && bot->HasAura(SPELL_ICE_SHARDS))
        { // Frostfire
            triggers.push_back(new TriggerNode("combustion", { NextAction("combustion", 18.0f) }));
            triggers.push_back(new TriggerNode("icy veins", { NextAction("icy veins", 17.5f) }));
            triggers.push_back(new TriggerNode("mirror image", { NextAction("mirror image", 17.0f) }));
        }
        else
        { // Fire
            triggers.push_back(new TriggerNode("combustion", { NextAction("combustion", 18.0f) }));
            triggers.push_back(new TriggerNode("mirror image", { NextAction("mirror image", 17.5f) }));
        }
    }
    else if (tab == MAGE_TAB_FROST)  // Frost
    {
        triggers.push_back(new TriggerNode("cold snap", { NextAction("cold snap", 28.0f) }));
        triggers.push_back(new TriggerNode("icy veins", { NextAction("icy veins", 27.5f) }));
        triggers.push_back(new TriggerNode("mirror image", { NextAction("mirror image", 26.0f) }));
    }
}

void MageCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("polymorph", { NextAction("polymorph", 30.0f) }));
}

void MageAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("blizzard channel check", { NextAction("cancel channel", 26.0f) }));

    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == MAGE_TAB_ARCANE)
    {
        triggers.push_back(new TriggerNode("flamestrike active and medium aoe", { NextAction("blizzard", 24.0f) }));
        triggers.push_back(new TriggerNode("medium aoe", {
                                                     NextAction("flamestrike", 23.0f),
                                                     NextAction("blizzard", 22.0f) }));
    }
    else if (tab == MAGE_TAB_FIRE)
    {
        triggers.push_back(
            new TriggerNode("medium aoe", {
                                      NextAction("dragon's breath", 39.0f),
                                      NextAction("blast wave", 38.0f),
                                      NextAction("flamestrike", 23.0f),
                                      NextAction("blizzard", 22.0f) }));

        triggers.push_back(new TriggerNode("flamestrike active and medium aoe", { NextAction("blizzard", 24.0f) }));
        triggers.push_back(new TriggerNode("firestarter", { NextAction("flamestrike", 40.0f) }));
        triggers.push_back(new TriggerNode("living bomb on attackers", { NextAction("living bomb on attackers", 21.0f) }));
    }
    else if (tab == MAGE_TAB_FROST)
    {
        triggers.push_back(new TriggerNode("flamestrike active and medium aoe", { NextAction("blizzard", 24.0f) }));
        triggers.push_back(new TriggerNode("medium aoe", {
                                                     NextAction("flamestrike", 23.0f),
                                                     NextAction("blizzard", 22.0f) }));
        triggers.push_back(new TriggerNode("light aoe", { NextAction("cone of cold", 21.0f) }));
    }
}
