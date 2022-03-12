/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TotemsShamanStrategy.h"
#include "Playerbots.h"

TotemsShamanStrategy::TotemsShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
}

void TotemsShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("grace of air totem", NextAction::array(0, new NextAction("grace of air totem", 16.0f), nullptr)));
    triggers.push_back(new TriggerNode("mana spring totem", NextAction::array(0, new NextAction("mana spring totem", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("strength of earth totem", NextAction::array(0, new NextAction("strength of earth totem", 18.0f), nullptr)));
    triggers.push_back(new TriggerNode("flametongue totem", NextAction::array(0, new NextAction("flametongue totem", 17.0f), nullptr)));
}
