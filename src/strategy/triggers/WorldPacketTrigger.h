/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WORLDPACKETTRIGGER_H
#define _PLAYERBOT_WORLDPACKETTRIGGER_H

#include "Trigger.h"

class Event;
class Player;
class PlayerbotAI;
class WorldPacket;

class WorldPacketTrigger : public Trigger
{
public:
    WorldPacketTrigger(PlayerbotAI* botAI, std::string const command) : Trigger(botAI, command), triggered(false) {}

    void ExternalEvent(WorldPacket& packet, Player* owner = nullptr) override;
    Event Check() override;
    void Reset() override;

private:
    WorldPacket packet;
    bool triggered;
    Player* owner;
};

#endif
