/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "IsMovingValue.h"

#include "Playerbots.h"

bool IsMovingValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);

    if (!target)
        return false;

    return target->isMoving();
}

bool IsSwimmingValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);

    if (!target)
        return false;

    return target->IsUnderWater() || (target->IsInWater() && target->CanSwim());
}
