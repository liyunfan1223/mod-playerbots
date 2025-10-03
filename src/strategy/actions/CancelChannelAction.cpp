/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "CancelChannelAction.h"
#include "Player.h"
#include "PlayerbotAI.h"

bool CancelChannelAction::Execute(Event event)
{
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        bot->InterruptSpell(CURRENT_CHANNELED_SPELL);
        return true;
    }
    return false;
}
