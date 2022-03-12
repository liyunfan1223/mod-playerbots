/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PriestActions.h"
#include "Event.h"
#include "Playerbots.h"

bool CastRemoveShadowformAction::isUseful()
{
    return botAI->HasAura("shadowform", AI_VALUE(Unit*, "self target"));
}

bool CastRemoveShadowformAction::isPossible()
{
    return true;
}

bool CastRemoveShadowformAction::Execute(Event event)
{
    botAI->RemoveAura("shadowform");
    return true;
}
