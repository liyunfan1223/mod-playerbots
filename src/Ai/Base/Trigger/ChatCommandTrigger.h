/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CHATCOMMANDTRIGGER_H
#define PLAYERBOTS_CHATCOMMANDTRIGGER_H

#include "Trigger.h"

class Event;
class Player;
class PlayerbotAI;

class ChatCommandTrigger : public Trigger
{
public:
    ChatCommandTrigger(PlayerbotAI* botAI, std::string const command);

    using Trigger::ExternalEvent;  // keep the WorldPacket& overload visible
    void ExternalEvent(std::string const param, Player* owner = nullptr) override;
    Event Check() override;
    void Reset() override;

private:
    std::string param;
    bool triggered;
    Player* owner;
};

#endif
