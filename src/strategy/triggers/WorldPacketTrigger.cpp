/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WorldPacketTrigger.h"

#include "Playerbots.h"

void WorldPacketTrigger::ExternalEvent(WorldPacket& revData, Player* eventOwner)
{
    packet = revData;
    owner = eventOwner;
    triggered = true;
}

Event WorldPacketTrigger::Check()
{
    if (!triggered)
        return Event();

    return Event(getName(), packet, owner);
}

void WorldPacketTrigger::Reset() { triggered = false; }
