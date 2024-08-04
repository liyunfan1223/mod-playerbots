/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AcceptDuelAction.h"

#include "Event.h"
#include "Playerbots.h"

bool AcceptDuelAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());

    ObjectGuid flagGuid;
    p >> flagGuid;
    ObjectGuid playerGuid;
    p >> playerGuid;

    // do not auto duel with low hp
    if ((!botAI->HasRealPlayerMaster() || (botAI->GetMaster() && botAI->GetMaster()->GetGUID() != playerGuid)) &&
        AI_VALUE2(uint8, "health", "self target") < 90)
    {
        WorldPacket packet(CMSG_DUEL_CANCELLED, 8);
        packet << flagGuid;
        bot->GetSession()->HandleDuelCancelledOpcode(packet);
        return false;
    }

    WorldPacket packet(CMSG_DUEL_ACCEPTED, 8);
    packet << flagGuid;
    bot->GetSession()->HandleDuelAcceptedOpcode(packet);

    botAI->ResetStrategies();
    return true;
}
