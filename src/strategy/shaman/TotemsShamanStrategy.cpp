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

    triggers.push_back(new TriggerNode("fire elemental totem", NextAction::array(0, new NextAction("fire elemental totem", 32.0f), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "no air totem",
        NextAction::array(0, new NextAction("wrath of air totem", 8.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "no water totem",
        NextAction::array(0, 
            new NextAction("mana spring totem", 7.0f), 
            new NextAction("healing stream totem", 6.0f), 
            nullptr)));

    triggers.push_back(new TriggerNode(
        "no fire totem",
        NextAction::array(0, 
            new NextAction("flametongue totem", 7.0f), 
            new NextAction("searing totem", 6.0f),
            nullptr)));

    triggers.push_back(new TriggerNode(
        "strength of earth totem",
        NextAction::array(0, new NextAction("strength of earth totem", 6.0f), NULL)));

}
