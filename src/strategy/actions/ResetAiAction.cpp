/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ResetAiAction.h"
#include "Event.h"
#include "Playerbots.h"
#include "PlayerbotDbStore.h"

bool ResetAiAction::Execute(Event event)
{
    sPlayerbotDbStore->Reset(botAI);
    botAI->ResetStrategies(false);
    botAI->TellMaster("AI was reset to defaults");
    return true;
}
