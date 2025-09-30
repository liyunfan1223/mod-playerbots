/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PassTroughStrategy.h"

#include "Playerbots.h"

void PassTroughStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    for (std::vector<std::string>::iterator i = supported.begin(); i != supported.end(); i++)
        triggers.push_back(
            new TriggerNode(i->c_str(), NextAction::array(0, new NextAction(i->c_str(), relevance), nullptr)));
}
