/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PlayerbotAI.h"
#include "WipeAction.h"

bool WipeAction::Execute(Event event)
{
    Player* master = event.getOwner();

    if (botAI->GetMaster()->GetGUID() != event.getOwner()->GetGUID())
        return false;

    bot->Kill(bot, bot);
    return true;
}
