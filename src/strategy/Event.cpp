/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "Event.h"

#include "Playerbots.h"

Event::Event(std::string const source, ObjectGuid object, Player* owner) : source(source), owner(owner)
{
    packet << object;
}

ObjectGuid Event::getObject()
{
    if (packet.empty())
        return ObjectGuid::Empty;

    WorldPacket p(packet);
    p.rpos(0);

    ObjectGuid guid;
    p >> guid;

    return guid;
}
