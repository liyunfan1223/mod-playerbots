/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LASTSPELLCASTVALUE_H
#define _PLAYERBOT_LASTSPELLCASTVALUE_H

#include "Value.h"

class PlayerbotAI;

class LastSpellCast
{
public:
    LastSpellCast();

    void Set(uint32 id, ObjectGuid target, time_t time);
    void Reset();

    uint32 id;
    ObjectGuid target;
    time_t timer;
};

class LastSpellCastValue : public ManualSetValue<LastSpellCast&>
{
public:
    LastSpellCastValue(PlayerbotAI* botAI, std::string const name = "last spell cast")
        : ManualSetValue<LastSpellCast&>(botAI, data, name)
    {
    }

private:
    LastSpellCast data;
};

#endif
