/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TotemsShamanStrategy.h"

#include "Playerbots.h"

TotemsShamanStrategy::TotemsShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}

void TotemsShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("no air totem", NextAction::array(0, new NextAction("wrath of air totem", 8.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("no water totem", NextAction::array(0, new NextAction("mana spring totem", 7.0f),
                                                            new NextAction("healing stream totem", 6.0f), nullptr)));

    triggers.push_back(new TriggerNode("strength of earth totem",
                                       NextAction::array(0, new NextAction("strength of earth totem", 6.0f), nullptr)));
}
