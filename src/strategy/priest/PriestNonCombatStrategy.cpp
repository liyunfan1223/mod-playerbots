/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PriestNonCombatStrategy.h"

#include "Playerbots.h"
#include "PriestNonCombatStrategyActionNodeFactory.h"

PriestNonCombatStrategy::PriestNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new PriestNonCombatStrategyActionNodeFactory());
}

void PriestNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    // triggers.push_back(new TriggerNode("power word: fortitude", NextAction::array(0, new NextAction("power word:
    // fortitude", 12.0f), nullptr))); triggers.push_back(new TriggerNode("divine spirit", NextAction::array(0, new
    // NextAction("divine spirit", 14.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("inner fire", NextAction::array(0, new NextAction("inner fire", 10.0f), nullptr)));
    // triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("greater heal", 70.0f),
    // nullptr))); triggers.push_back(new TriggerNode("party member critical health", NextAction::array(0, new
    // NextAction("greater heal on party", 60.0f), nullptr))); triggers.push_back(new TriggerNode("medium health",
    // NextAction::array(0, new NextAction("renew", 41.0f), nullptr))); triggers.push_back(new TriggerNode("party member
    // medium health", NextAction::array(0, new NextAction("renew on party", 40.0f), nullptr))); triggers.push_back(new
    // TriggerNode("medium aoe heal", NextAction::array(0, new NextAction("lightwell", 42.f), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member dead", NextAction::array(0, new NextAction("remove shadowform", ACTION_CRITICAL_HEAL + 11),
                                               new NextAction("resurrection", ACTION_CRITICAL_HEAL + 10), nullptr)));
    // triggers.push_back(new TriggerNode("swimming", NextAction::array(0, new NextAction("levitate", 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply oil", 1.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("party member critical health",
                        NextAction::array(0, new NextAction("renew on party", ACTION_CRITICAL_HEAL + 3),
                                          new NextAction("penance on party", ACTION_CRITICAL_HEAL + 2),
                                          new NextAction("greater heal on party", ACTION_CRITICAL_HEAL + 1), NULL)));

    triggers.push_back(
        new TriggerNode("party member low health",
                        NextAction::array(0, new NextAction("renew on party", ACTION_MEDIUM_HEAL + 3),
                                          new NextAction("penance on party", ACTION_MEDIUM_HEAL + 2),
                                          new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(
        new TriggerNode("party member medium health",
                        NextAction::array(0, new NextAction("renew on party", ACTION_LIGHT_HEAL + 9),
                                          new NextAction("penance on party", ACTION_LIGHT_HEAL + 8), NULL)));

    triggers.push_back(
        new TriggerNode("party member almost full health",
                        NextAction::array(0, new NextAction("renew on party", ACTION_LIGHT_HEAL + 3), NULL)));

    triggers.push_back(
        new TriggerNode("group heal setting", NextAction::array(0, new NextAction("circle of healing on party", 27.0f), NULL)));
}

void PriestBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("prayer of fortitude on party",
                        NextAction::array(0, new NextAction("prayer of fortitude on party", 12.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("prayer of spirit on party",
                        NextAction::array(0, new NextAction("prayer of spirit on party", 14.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("power word: fortitude on party",
                        NextAction::array(0, new NextAction("power word: fortitude on party", 11.0f), nullptr)));
    triggers.push_back(new TriggerNode("divine spirit on party",
                                       NextAction::array(0, new NextAction("divine spirit on party", 13.0f), nullptr)));
    // triggers.push_back(new TriggerNode("fear ward", NextAction::array(0, new NextAction("fear ward", 10.0f),
    // nullptr))); triggers.push_back(new TriggerNode("touch of weakness", NextAction::array(0, new NextAction("touch of
    // weakness", 10.0f), nullptr))); triggers.push_back(new TriggerNode("shadowguard", NextAction::array(0, new
    // NextAction("shadowguard", 10.0f), nullptr)));
}

void PriestShadowResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("shadow protection",
                                       NextAction::array(0, new NextAction("shadow protection", 12.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("shadow protection on party",
                        NextAction::array(0, new NextAction("shadow protection on party", 11.0f), nullptr)));
    // triggers.push_back(new TriggerNode("shadow protection on party", NextAction::array(0, new NextAction("shadow
    // protection on party", 10.0f), nullptr)));
}
