/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NewRpgStrategy.h"

#include "Playerbots.h"

NewRpgStrategy::NewRpgStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

NextAction** NewRpgStrategy::getDefaultActions()
{
    // the releavance should be greater than grind
    return NextAction::array(0,
        new NextAction("new rpg status update", 11.0f), 
        nullptr);
}

void NewRpgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("go grind status", NextAction::array(0, new NextAction("new rpg go grind", 3.0f), nullptr)));
    
    triggers.push_back(
        new TriggerNode("go innkeeper status", NextAction::array(0, new NextAction("new rpg go innkeeper", 3.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("near random status", NextAction::array(0, new NextAction("new rpg move random", 3.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("near npc status", NextAction::array(0, new NextAction("new rpg move npc", 3.0f), nullptr)));
    
    triggers.push_back(
        new TriggerNode("do quest status", NextAction::array(0, new NextAction("new rpg do quest", 3.0f), nullptr)));
}

void NewRpgStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // multipliers.push_back(new RpgActionMultiplier(botAI));
}
