/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RtiTriggers.h"

#include "Playerbots.h"

bool NoRtiTrigger::IsActive() { return !AI_VALUE(Unit*, "rti target"); }
