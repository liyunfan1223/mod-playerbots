/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "LastSpellCastValue.h"

#include "Playerbots.h"

LastSpellCast::LastSpellCast() : id(0), timer(0) {}

void LastSpellCast::Set(uint32 newId, ObjectGuid newTarget, time_t newTime)
{
    id = newId;
    target = newTarget;
    timer = newTime;
}

void LastSpellCast::Reset()
{
    id = 0;
    target.Clear();
    timer = 0;
}
