/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FishingTriggers.h"
#include "Playerbots.h"

bool CanFishTrigger::IsActive() { return AI_VALUE(bool, "can fish"); }

bool CanUseFishingBobberTrigger::IsActive() { return AI_VALUE(bool, "can use fishing bobber");}