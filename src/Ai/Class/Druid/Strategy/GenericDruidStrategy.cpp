/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericDruidStrategy.h"
#include "AiFactory.h"
#include "FeralDruidStrategy.h"
#include "Playerbots.h"

class GenericDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDruidStrategyActionNodeFactory()
    {
        creators["entangling roots on cc"] = &entangling_roots_on_cc;
        creators["cyclone on cc"] = &cyclone_on_cc;
        creators["hibernate on cc"] = &hibernate_on_cc;
    }

private:
    static ActionNode* entangling_roots_on_cc([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("entangling roots on cc",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* cyclone_on_cc([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cyclone on cc",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* hibernate_on_cc([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("hibernate on cc",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }
};

GenericDruidStrategy::GenericDruidStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDruidStrategyActionNodeFactory());
}

void GenericDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("almost full health", { NextAction("barkskin", 40.0f) }));

    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == DRUID_TAB_FERAL)
    {
        if (!bot->HasAura(16931) /*thick hide — bear spec*/)
        {
            triggers.push_back(new TriggerNode("predator's swiftness and combat party member dead",
                                               { NextAction("rebirth", 29.0f) }));
            triggers.push_back(new TriggerNode("combat party member dead",
                                               { NextAction("rebirth", 28.5f) }));
        }
    }
    else
    {
        triggers.push_back(new TriggerNode("combat party member dead",
                                           { NextAction("rebirth", 29.0f) }));
    }

    triggers.push_back(new TriggerNode("being attacked",
                                       { NextAction("nature's grasp", 39.0f) }));
}

void DruidCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("party member cure poison",
                        { NextAction("abolish poison on party", 51.0f) }));

    triggers.push_back(
        new TriggerNode("party member remove curse",
                        { NextAction("remove curse on party", 57.0f) }));

}

void DruidBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == DRUID_TAB_BALANCE)
    {
        triggers.push_back(new TriggerNode("force of nature", { NextAction("force of nature", 29.0f) }));
        triggers.push_back(new TriggerNode("new pet", { NextAction("set pet stance", 60.0f) }));
    }

    if (tab == DRUID_TAB_FERAL)
    {
        triggers.push_back(new TriggerNode("berserk", { NextAction("berserk", 27.5f) }));
    }
}

void DruidCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == DRUID_TAB_BALANCE || tab == DRUID_TAB_RESTORATION)
    {
        triggers.push_back(new TriggerNode(
            "cyclone", { NextAction("cyclone on cc", 42.0f) }));
        triggers.push_back(new TriggerNode(
            "hibernate", { NextAction("hibernate on cc", 41.0f) }));
        triggers.push_back(new TriggerNode(
            "entangling roots", { NextAction("entangling roots on cc", 40.0f) }));
    }
    if (tab == DRUID_TAB_FERAL)
    {
        if (bot->HasSpell(SPELL_CAT_FORM) && !bot->HasAura(AURA_THICK_HIDE))
        {
            triggers.push_back(new TriggerNode(
                "predator's swiftness and cyclone", { NextAction("cyclone on cc", 42.0f) }));
            triggers.push_back(new TriggerNode(
                "predator's swiftness and hibernate", { NextAction("hibernate on cc", 41.0f) }));
            triggers.push_back(new TriggerNode(
                "predator's swiftness and entangling roots", { NextAction("entangling roots on cc", 40.0f) }));
        }
        else
        {
            triggers.push_back(new TriggerNode(
                "cyclone", { NextAction("cyclone on cc", 42.0f) }));
            triggers.push_back(new TriggerNode(
                "hibernate", { NextAction("hibernate on cc", 41.0f) }));
            triggers.push_back(new TriggerNode(
                "entangling roots", { NextAction("entangling roots on cc", 40.0f) }));
        }
    }
}

void DruidHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("healer should attack and not blanketing",
                        {
                            NextAction("cancel tree form", 5.4f),
                            NextAction("moonfire",         5.3f),
                            NextAction("wrath",            5.2f),
                            NextAction("starfire",         5.1f),
                        }));
}

void DruidAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == DRUID_TAB_BALANCE)
    {
        triggers.push_back(new TriggerNode("hurricane channel check", { NextAction("cancel channel", 22.0f) }));
        triggers.push_back(new TriggerNode("starfall", { NextAction("starfall", 28.5f) }));
        triggers.push_back(new TriggerNode("medium aoe", { NextAction("hurricane", 23.0f) }));
        triggers.push_back(new TriggerNode("enemy within melee", { NextAction("typhoon", 40.0f) }));
        triggers.push_back(new TriggerNode("insect swarm on attacker", { NextAction("insect swarm on attacker", 5.2f) }));
        triggers.push_back(new TriggerNode("moonfire on attacker", { NextAction("moonfire on attacker", 5.1f) }));
    }

    if (tab == DRUID_TAB_RESTORATION)
    {
        triggers.push_back(new TriggerNode("hurricane channel check", { NextAction("cancel channel", 22.0f) }));
        triggers.push_back(new TriggerNode("medium aoe", { NextAction("hurricane", 23.0f) }));
        triggers.push_back(new TriggerNode("insect swarm on attacker", { NextAction("insect swarm on attacker", 5.2f) }));
        triggers.push_back(new TriggerNode("moonfire on attacker", { NextAction("moonfire on attacker", 5.1f) }));
    }

    if (tab == DRUID_TAB_FERAL && bot->HasSpell(SPELL_CAT_FORM) && !bot->HasAura(AURA_THICK_HIDE))
    {
        triggers.push_back(new TriggerNode("clearcasting and medium aoe", { NextAction("swipe (cat)", 25.5f) }));
        triggers.push_back(new TriggerNode("medium aoe", { NextAction("swipe (cat)", 25.0f) }));
    }
}
