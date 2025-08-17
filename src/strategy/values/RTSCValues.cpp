/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RTSCValues.h"

#include "Playerbots.h"

bool SeeSpellLocationValue::EqualToLast(WorldPosition value) { return value == lastValue; }

WorldPosition SeeSpellLocationValue::Calculate() { return value; }

std::string const RTSCSavedLocationValue::Save() { return value.to_string(); }

bool RTSCSavedLocationValue::Load(std::string const text)
{
    value = WorldPosition(text);
    return true;
}
