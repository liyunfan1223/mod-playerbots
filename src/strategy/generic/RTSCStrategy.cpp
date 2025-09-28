/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RTSCStrategy.h"

#include "Playerbots.h"

RTSCStrategy::RTSCStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

void RTSCStrategy::InitTriggers([[maybe_unused]] std::vector<TriggerNode*>& triggers) {}
