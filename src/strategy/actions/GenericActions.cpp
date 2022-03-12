/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericActions.h"
#include "Playerbots.h"

bool MeleeAction::isUseful()
{
    // do not allow if can't attack from vehicle
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    return true;
}
