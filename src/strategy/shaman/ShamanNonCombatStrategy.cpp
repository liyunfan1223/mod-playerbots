/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ShamanNonCombatStrategy.h"

#include "Playerbots.h"

void ShamanNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("party member dead",
                        NextAction::array(0, new NextAction("ancestral spirit", ACTION_CRITICAL_HEAL + 10), nullptr)));
    triggers.push_back(
        new TriggerNode("water breathing", NextAction::array(0, new NextAction("water breathing", 12.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("water walking", NextAction::array(0, new NextAction("water walking", 12.0f), nullptr)));
    triggers.push_back(new TriggerNode(
        "water breathing on party", NextAction::array(0, new NextAction("water breathing on party", 11.0f), nullptr)));
    triggers.push_back(new TriggerNode("water walking on party",
                                       NextAction::array(0, new NextAction("water walking on party", 11.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member critical health",
                                       NextAction::array(0, new NextAction("riptide on party", 31.0f),
                                                         new NextAction("healing wave on party", 30.0f), NULL)));

    triggers.push_back(new TriggerNode("party member low health",
                                       NextAction::array(0, new NextAction("riptide on party", 29.0f),
                                                         new NextAction("healing wave on party", 28.0f), NULL)));

    triggers.push_back(new TriggerNode("party member medium health",
                                       NextAction::array(0, new NextAction("riptide on party", 27.0f),
                                                         new NextAction("healing wave on party", 26.0f), NULL)));

    triggers.push_back(new TriggerNode("party member almost full health",
                                       NextAction::array(0, new NextAction("riptide on party", 25.0f),
                                                         new NextAction("lesser healing wave on party", 24.0f), NULL)));

    triggers.push_back(
        new TriggerNode("group heal setting", NextAction::array(0, new NextAction("chain heal on party", 27.0f), NULL)));

    triggers.push_back(
        new TriggerNode("cure poison", NextAction::array(0, new NextAction("cure poison", 21.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member cure poison",
                                       NextAction::array(0, new NextAction("cure poison on party", 21.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("cure disease", NextAction::array(0, new NextAction("cure disease", 31.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member cure disease",
                                       NextAction::array(0, new NextAction("cure disease on party", 30.0f), nullptr)));
}

void ShamanNonCombatStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    NonCombatStrategy::InitMultipliers(multipliers);
}
