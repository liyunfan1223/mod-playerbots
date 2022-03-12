/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RpgStrategy.h"
#include "Playerbots.h"
#include "RpgSubActions.h"

float RpgActionMultiplier::GetValue(Action* action)
{
    if (action == nullptr)
        return 1.0f;

    std::string const nextAction = AI_VALUE(std::string, "next rpg action");
    std::string const name = action->getName();

    if (!nextAction.empty() && dynamic_cast<RpgEnabled*>(action) && name != nextAction)
        return 0.0f;

    return 1.0f;
}

RpgStrategy::RpgStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
}

NextAction** RpgStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("rpg", 1.1f), nullptr);
}

void RpgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no rpg target", NextAction::array(0, new NextAction("choose rpg target", 5.0f), nullptr)));
    triggers.push_back(new TriggerNode("far from rpg target", NextAction::array(0, new NextAction("move to rpg target", 5.0f), nullptr)));

    //Sub actions
    triggers.push_back(new TriggerNode("rpg", NextAction::array(0, new NextAction("rpg stay", 1.001f), nullptr)));
    triggers.push_back(new TriggerNode("rpg", NextAction::array(0, new NextAction("rpg work", 1.001f), nullptr)));
    triggers.push_back(new TriggerNode("rpg", NextAction::array(0, new NextAction("rpg emote", 1.001f), nullptr)));
    triggers.push_back(new TriggerNode("has rpg target", NextAction::array(0, new NextAction("rpg cancel", 1.001f), nullptr)));
    triggers.push_back(new TriggerNode("rpg taxi", NextAction::array(0, new NextAction("rpg taxi", 1.005f), nullptr)));
    triggers.push_back(new TriggerNode("rpg discover", NextAction::array(0, new NextAction("rpg discover", 1.110f), nullptr)));
    triggers.push_back(new TriggerNode("rpg start quest", NextAction::array(0, new NextAction("rpg start quest", 1.080f), nullptr)));
    triggers.push_back(new TriggerNode("rpg end quest", NextAction::array(0, new NextAction("rpg end quest", 1.090f), nullptr)));
    triggers.push_back(new TriggerNode("rpg buy", NextAction::array(0, new NextAction("rpg buy", 1.030f), nullptr)));
    triggers.push_back(new TriggerNode("rpg sell", NextAction::array(0, new NextAction("rpg sell", 1.100f), nullptr)));
    triggers.push_back(new TriggerNode("rpg repair", NextAction::array(0, new NextAction("rpg repair", 1.095f), nullptr)));
    triggers.push_back(new TriggerNode("rpg train", NextAction::array(0, new NextAction("rpg train", 1.080f), nullptr)));
    triggers.push_back(new TriggerNode("rpg heal", NextAction::array(0, new NextAction("rpg heal", 1.025f), nullptr)));
    triggers.push_back(new TriggerNode("rpg home bind", NextAction::array(0, new NextAction("rpg home bind", 1.060f), nullptr)));
    triggers.push_back(new TriggerNode("rpg queue bg", NextAction::array(0, new NextAction("rpg queue bg", 1.085f), nullptr)));
    triggers.push_back(new TriggerNode("rpg buy petition", NextAction::array(0, new NextAction("rpg buy petition", 1.040f), nullptr)));
    triggers.push_back(new TriggerNode("rpg use", NextAction::array(0, new NextAction("rpg use", 1.002f), nullptr)));
    //triggers.push_back(new TriggerNode("rpg spell", NextAction::array(0, new NextAction("rpg spell", 1.001f), nullptr)));
    //triggers.push_back(new TriggerNode("rpg craft", NextAction::array(0, new NextAction("rpg craft", 1.001f), nullptr)));
    triggers.push_back(new TriggerNode("rpg trade useful", NextAction::array(0, new NextAction("rpg trade useful", 1.030f), nullptr)));
    triggers.push_back(new TriggerNode("rpg duel", NextAction::array(0, new NextAction("rpg duel", 1.010f), nullptr)));
}

void RpgStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new RpgActionMultiplier(botAI));
}
