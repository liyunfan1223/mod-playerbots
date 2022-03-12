/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FrostMageStrategy.h"
#include "Playerbots.h"

FrostMageStrategy::FrostMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
}

NextAction** FrostMageStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("frostbolt", 7.0f), nullptr);
}

void FrostMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("icy veins", NextAction::array(0, new NextAction("icy veins", 50.0f), nullptr)));
}

void FrostMageAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("blizzard", 40.0f), nullptr)));
}
