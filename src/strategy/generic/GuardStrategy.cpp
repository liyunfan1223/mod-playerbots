/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GuardStrategy.h"

#include "Playerbots.h"

NextAction** GuardStrategy::getDefaultActions() { return NextAction::array(0, new NextAction("guard", 4.0f), nullptr); }

void GuardStrategy::InitTriggers(std::vector<TriggerNode*>& triggers) {}
