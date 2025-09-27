/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RemoveAuraAction.h"

#include "Event.h"
#include "Playerbots.h"

RemoveAuraAction::RemoveAuraAction(PlayerbotAI* botAI) : Action(botAI, "ra") {}

bool RemoveAuraAction::Execute(Event event)
{
    botAI->RemoveAura(event.getParam());
    return true;
}
