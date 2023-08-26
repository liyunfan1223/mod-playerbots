/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CurrentTargetValue.h"
#include "Playerbots.h"

Unit* CurrentTargetValue::Get()
{
    if (selection.IsEmpty())
        return nullptr;

    Unit* unit = ObjectAccessor::GetUnit(*bot, selection);
    // if (unit && !bot->IsWithinLOSInMap(unit))
    //     return nullptr;

    return unit;
}

void CurrentTargetValue::Set(Unit* target)
{
    selection = target ? target->GetGUID() : ObjectGuid::Empty;
}
