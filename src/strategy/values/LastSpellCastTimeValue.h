/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LASTSPELLCASTTIMEVALUE_H
#define _PLAYERBOT_LASTSPELLCASTTIMEVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class LastSpellCastTimeValue : public ManualSetValue<time_t>, public Qualified
{
public:
    LastSpellCastTimeValue(PlayerbotAI* botAI) : ManualSetValue<time_t>(botAI, 0), Qualified() {}
};

#endif
