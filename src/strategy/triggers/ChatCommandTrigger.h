/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHATCOMMANDTRIGGER_H
#define _PLAYERBOT_CHATCOMMANDTRIGGER_H

#include "Trigger.h"

class Event;
class Player;
class PlayerbotAI;

class ChatCommandTrigger : public Trigger
{
public:
    ChatCommandTrigger(PlayerbotAI* botAI, std::string const command);

    void ExternalEvent(std::string const param, Player* owner = nullptr) override;
    Event Check() override;
    void Reset() override;

private:
    std::string param;
    bool triggered;
    Player* owner;
};

#endif
