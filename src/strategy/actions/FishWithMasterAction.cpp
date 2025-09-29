/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */
#include "FishWithMasterAction.h"
#include "Event.h"
#include "Playerbots.h"
#include "Common.h"

const std::set<uint32> FISHING_SPELLS = {7620, 7731, 7732, 18248, 33095, 51294};

bool FishWithMasterAction::Execute(Event event)
{
    if (!botAI)
        return false;

    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint8 castcount = 0;
    uint32 spellId = 0; 
    p >> castcount >> spellId;
    
    if (FISHING_SPELLS.find(spellId) != FISHING_SPELLS.end())
    {
        if (AI_VALUE(bool, "can fish"))
        {
            botAI->ChangeStrategy("+masterfishing", BOT_STATE_NON_COMBAT);
            return true;
        }
    }
    return false;
}
