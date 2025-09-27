/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericPriestStrategy.h"

#include "GenericPriestStrategyActionNodeFactory.h"
#include "HealPriestStrategy.h"
#include "Playerbots.h"

GenericPriestStrategy::GenericPriestStrategy(PlayerbotAI* botAI) : RangedCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPriestStrategyActionNodeFactory());
}

void GenericPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    // triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("greater heal", 25.0f),
    // nullptr))); triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("power word:
    // shield", 61.0f), new NextAction("greater heal", 60.0f), nullptr))); triggers.push_back(new TriggerNode("critical
    // health", NextAction::array(0, new NextAction("remove shadowform", 72.0f), new NextAction("power word:
    // shield", 71.0f), new NextAction("flash heal", 70.0f), nullptr))); triggers.push_back(new TriggerNode("party
    // member critical health", NextAction::array(0, new NextAction("remove shadowform", 62.0f), new NextAction("power
    // word: shield on party", 61.0f), new NextAction("flash heal on party", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("medium threat", NextAction::array(0, new NextAction("fade", 55.0f), nullptr)));
    // triggers.push_back(new TriggerNode("enemy is close", NextAction::array(0, new NextAction("psychic
    // scream", 50.0f), nullptr))); triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new
    // NextAction("inner focus", 42.0f), nullptr))); triggers.push_back(new TriggerNode("medium mana",
    // NextAction::array(0, new NextAction("symbol of hope", ACTION_EMERGENCY), nullptr))); triggers.push_back(new
    // TriggerNode("low mana", NextAction::array(0, new NextAction("consume magic", 10.0f), nullptr)));
    // triggers.push_back(
    //     new TriggerNode("inner focus", NextAction::array(0, new NextAction("inner focus", 42.0f), nullptr)));
    triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("desperate prayer",
        ACTION_HIGH + 5), nullptr)));
    // triggers.push_back(new TriggerNode("enemy is close", NextAction::array(0, new
    // NextAction("elune's grace", ACTION_EMERGENCY), nullptr))); triggers.push_back(new TriggerNode("chastise",
    // NextAction::array(0, new NextAction("chastise", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode(
        "critical health", NextAction::array(0, new NextAction("power word: shield", ACTION_NORMAL), NULL)));

    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("power word: shield", ACTION_HIGH), NULL)));

    triggers.push_back(
        new TriggerNode("medium mana",
            NextAction::array(0,
                new NextAction("shadowfiend", ACTION_HIGH + 2),
                new NextAction("inner focus", ACTION_HIGH + 1), nullptr)));

    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("hymn of hope", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode("enemy too close for spell",
                                       NextAction::array(0, new NextAction("flee", ACTION_MOVE + 9), nullptr)));
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply oil", 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("being attacked",
        NextAction::array(0, new NextAction("power word: shield", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 60.0f), nullptr)));
}

PriestCureStrategy::PriestCureStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new CurePriestStrategyActionNodeFactory());
}

void PriestCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("dispel magic", NextAction::array(0, new NextAction("dispel magic", 41.0f), nullptr)));
    triggers.push_back(new TriggerNode("dispel magic on party",
                                       NextAction::array(0, new NextAction("dispel magic on party", 40.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("cure disease", NextAction::array(0, new NextAction("abolish disease", 31.0f), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member cure disease", NextAction::array(0, new NextAction("abolish disease on party", 30.0f), nullptr)));
}

void PriestBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("power infusion", NextAction::array(0, new NextAction("power infusion", 41.0f), nullptr)));
    triggers.push_back(new TriggerNode("boost", NextAction::array(0, new NextAction("shadowfiend", 20.0f), nullptr)));
}

void PriestCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("shackle undead", NextAction::array(0, new NextAction("shackle undead", 31.0f), nullptr)));
}

void PriestHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("healer should attack",
                        NextAction::array(0,
                            new NextAction("shadow word: pain", ACTION_DEFAULT + 0.5f),
                            new NextAction("holy fire", ACTION_DEFAULT + 0.4f),
                            new NextAction("smite", ACTION_DEFAULT + 0.3f),
                            new NextAction("mind blast", ACTION_DEFAULT + 0.2f),
                            new NextAction("shoot", ACTION_DEFAULT),
                            nullptr)));

    triggers.push_back(
        new TriggerNode("medium aoe and healer should attack",
                        NextAction::array(0,
                            new NextAction("mind sear", ACTION_DEFAULT + 0.5f),
                            nullptr)));
}

