/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */
#include "FishWithMasterAction.h"
#include "FishingAction.h"
#include "Event.h"
#include "Playerbots.h"
#include "Common.h"

bool FishWithMasterAction::Execute(Event event)
{
    if (!botAI)
        return false;

    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint8 castcount = 0;
    uint32 spellId = 0; 
    p >> castcount >> spellId;
    
    if (spellId == FISHING_SPELL)
    {
        if (AI_VALUE(bool, "can fish"))
        {
            botAI->ChangeStrategy("+masterfishing", BOT_STATE_NON_COMBAT);
            return true;
        }
    }
    return false;
}
