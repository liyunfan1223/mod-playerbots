/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HealPaladinStrategy.h"
#include "Playerbots.h"

HealPaladinStrategy::HealPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
}

NextAction** HealPaladinStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_NORMAL), nullptr);
}

void HealPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("flash of light", ACTION_MEDIUM_HEAL + 2), nullptr)));
    triggers.push_back(new TriggerNode("party member medium health", NextAction::array(0, new NextAction("flash of light on party", ACTION_MEDIUM_HEAL + 1), nullptr)));
    triggers.push_back(new TriggerNode("blessing", NextAction::array(0, new NextAction("blessing of sanctuary", ACTION_HIGH + 9), nullptr)));
    triggers.push_back(new TriggerNode("party member to heal out of spell range", NextAction::array(0, new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 1), nullptr)));
}
