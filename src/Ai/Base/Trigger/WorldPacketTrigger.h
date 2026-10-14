/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WORLDPACKETTRIGGER_H
#define PLAYERBOTS_WORLDPACKETTRIGGER_H

#include "Trigger.h"

class Event;
class Player;
class PlayerbotAI;
class WorldPacket;

class WorldPacketTrigger : public Trigger
{
public:
    WorldPacketTrigger(PlayerbotAI* botAI, std::string const command) : Trigger(botAI, command), triggered(false) {}

    using Trigger::ExternalEvent;  // keep the std::string overload visible
    void ExternalEvent(WorldPacket& packet, Player* owner = nullptr) override;
    Event Check() override;
    void Reset() override;

private:
    WorldPacket packet;
    bool triggered;
    Player* owner;
};

#endif
