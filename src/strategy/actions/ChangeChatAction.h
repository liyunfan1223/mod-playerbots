/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHANGECHATACTION_H
#define _PLAYERBOT_CHANGECHATACTION_H

#include "Action.h"

class PlayerbotAI;

class ChangeChatAction : public Action
{
public:
    ChangeChatAction(PlayerbotAI* botAI) : Action(botAI, "chat") {}

    bool Execute(Event event) override;
};

#endif
