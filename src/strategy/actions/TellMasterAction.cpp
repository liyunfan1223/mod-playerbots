/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TellMasterAction.h"

#include "Event.h"
#include "Playerbots.h"

bool TellMasterAction::Execute(Event event)
{
    botAI->TellMaster(text);
    return true;
}

bool OutOfReactRangeAction::Execute(Event event)
{
    botAI->TellMaster("Wait for me!");
    return true;
}

bool OutOfReactRangeAction::isUseful()
{
    bool canFollow = Follow(AI_VALUE(Unit*, "master target"));
    if (!canFollow)
    {
        return false;
    }

    return true;
}
