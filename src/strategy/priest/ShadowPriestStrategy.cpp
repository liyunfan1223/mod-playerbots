/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ShadowPriestStrategy.h"

#include "Playerbots.h"
#include "ShadowPriestStrategyActionNodeFactory.h"

ShadowPriestStrategy::ShadowPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    actionNodeFactories.Add(new ShadowPriestStrategyActionNodeFactory());
}

NextAction** ShadowPriestStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("mind blast", ACTION_DEFAULT + 0.3f),
                             new NextAction("mind flay", ACTION_DEFAULT + 0.2f),
                             new NextAction("shadow word: death", ACTION_DEFAULT + 0.1f), // cast during movement
                             new NextAction("shoot", ACTION_DEFAULT), nullptr);
}

void ShadowPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    // triggers.push_back(new TriggerNode("enemy out of spell", NextAction::array(0, new NextAction("reach spell",
    // ACTION_MOVE + 9), nullptr)));
    triggers.push_back(
        new TriggerNode("shadowform", NextAction::array(0, new NextAction("shadowform", ACTION_HIGH), nullptr)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("dispersion", ACTION_HIGH
        + 5), nullptr)));
    triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("dispersion", ACTION_HIGH
        + 5), nullptr)));
    triggers.push_back(
        new TriggerNode("vampiric embrace", NextAction::array(0, new NextAction("vampiric embrace", 16.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("silence", NextAction::array(0, new NextAction("silence", ACTION_INTERRUPT + 1), nullptr)));
    triggers.push_back(
        new TriggerNode("silence on enemy healer",
                        NextAction::array(0, new NextAction("silence on enemy healer", ACTION_INTERRUPT), nullptr)));
    // triggers.push_back(new TriggerNode("shadowfiend", NextAction::array(0, new NextAction("shadowfiend",
    // ACTION_HIGH), nullptr))); triggers.push_back(new TriggerNode("medium mana", NextAction::array(0, new
    // NextAction("shadowfiend", ACTION_HIGH), nullptr))); triggers.push_back(new TriggerNode("low mana",
    // NextAction::array(0, new NextAction("mana burn", ACTION_HIGH), nullptr)));
}

void ShadowPriestAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "shadow word: pain on attacker",
        NextAction::array(0, new NextAction("shadow word: pain on attacker", ACTION_NORMAL + 5), nullptr)));
    triggers.push_back(new TriggerNode(
        "vampiric touch on attacker",
        NextAction::array(0, new NextAction("vampiric touch on attacker", ACTION_NORMAL + 4), nullptr)));
    triggers.push_back(
        new TriggerNode("mind sear channel check", NextAction::array(0, new NextAction("cancel channel", ACTION_HIGH + 5), nullptr)));
    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0, new NextAction("mind sear", ACTION_HIGH + 4), nullptr)));
}

void ShadowPriestDebuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "vampiric touch", NextAction::array(0, new NextAction("vampiric touch", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(new TriggerNode(
        "devouring plague", NextAction::array(0, new NextAction("devouring plague", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode(
        "shadow word: pain", NextAction::array(0, new NextAction("shadow word: pain", ACTION_HIGH + 1), nullptr)));
    // triggers.push_back(new TriggerNode("feedback", NextAction::array(0, new NextAction("feedback", 80.0f),
    // nullptr))); triggers.push_back(new TriggerNode("hex of weakness", NextAction::array(0, new NextAction("hex of
    // weakness", 10.0f), nullptr)));
}
