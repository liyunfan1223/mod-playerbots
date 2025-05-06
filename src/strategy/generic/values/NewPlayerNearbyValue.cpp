/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NewPlayerNearbyValue.h"

#include "Playerbots.h"

ObjectGuid NewPlayerNearbyValue::Calculate()
{
    GuidVector players = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get();
    GuidSet& alreadySeenPlayers = botAI->GetAiObjectContext()->GetValue<GuidSet&>("already seen players")->Get();
    for (ObjectGuid const guid : players)
    {
        if (alreadySeenPlayers.find(guid) == alreadySeenPlayers.end())
            return guid;
    }

    return ObjectGuid::Empty;
}
