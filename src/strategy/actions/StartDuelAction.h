/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOTS_STARTDUELACTION_H
#define _PLAYERBOTS_STARTDUELACTION_H

#include "Action.h"

class Player;

class StartDuelAction : public Action
{
public:
    StartDuelAction(PlayerbotAI* botAI) : Action(botAI, "start duel") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif