/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "IsBehindValue.h"

#include "Playerbots.h"

bool IsBehindValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);
    if (!target)
        return false;

    float targetOrientation = target->GetOrientation();
    float orientation = bot->GetOrientation();
    float distance = bot->GetDistance(target);

    return distance <= ATTACK_DISTANCE && abs(targetOrientation - orientation) < M_PI / 2;
}
