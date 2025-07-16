/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */
#include "FishWithMasterAction.h"
#include "FishingAction.h"
#include "Event.h"
#include "Playerbots.h"

bool FishWithMasterAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint8 castCount = 0, castFlags = 0;
    uint32 spellId = 0;
    p >> castCount >> spellId >> castFlags;

    if (!botAI)
        return false;
   
    return FishingAction(botAI).Execute(event);
}
