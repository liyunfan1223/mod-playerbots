/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CheckValuesAction.h"

#include "Event.h"
#include "Playerbots.h"
#include "ServerFacade.h"

CheckValuesAction::CheckValuesAction(PlayerbotAI* botAI) : Action(botAI, "check values") {}

bool CheckValuesAction::Execute(Event event)
{
    if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
    {
        botAI->Ping(bot->GetPositionX(), bot->GetPositionY());
    }

    if (botAI->HasStrategy("map", BOT_STATE_NON_COMBAT) || botAI->HasStrategy("map full", BOT_STATE_NON_COMBAT))
    {
        sTravelNodeMap->manageNodes(bot, botAI->HasStrategy("map full", BOT_STATE_NON_COMBAT));
    }

    GuidVector possible_targets = *context->GetValue<GuidVector>("possible targets");
    GuidVector all_targets = *context->GetValue<GuidVector>("all targets");
    GuidVector npcs = *context->GetValue<GuidVector>("nearest npcs");
    GuidVector corpses = *context->GetValue<GuidVector>("nearest corpses");
    GuidVector gos = *context->GetValue<GuidVector>("nearest game objects");
    GuidVector nfp = *context->GetValue<GuidVector>("nearest friendly players");

    return true;
}
