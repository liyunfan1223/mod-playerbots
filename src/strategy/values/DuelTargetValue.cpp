/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DuelTargetValue.h"

#include "Playerbots.h"

Unit* DuelTargetValue::Calculate() { return bot->duel ? bot->duel->Opponent : nullptr; }
