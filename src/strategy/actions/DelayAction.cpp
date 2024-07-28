/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DelayAction.h"

#include "Event.h"
#include "Playerbots.h"

bool DelayAction::Execute(Event event)
{
    uint32 delay = sPlayerbotAIConfig->passiveDelay + sPlayerbotAIConfig->globalCoolDown;

    botAI->SetNextCheckDelay(delay);

    return true;
}

bool DelayAction::isUseful() { return !botAI->AllowActivity(ALL_ACTIVITY); }
