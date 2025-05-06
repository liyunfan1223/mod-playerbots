/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LASTSAIDVALUE_H
#define _PLAYERBOT_LASTSAIDVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class LastSaidValue : public ManualSetValue<time_t>, public Qualified
{
public:
    LastSaidValue(PlayerbotAI* botAI) : ManualSetValue<time_t>(botAI, time(nullptr) - 120, "last said") {}
};

class LastEmoteValue : public ManualSetValue<time_t>, public Qualified
{
public:
    LastEmoteValue(PlayerbotAI* botAI) : ManualSetValue<time_t>(botAI, time(nullptr) - 120, "last emote") {}
};

#endif
