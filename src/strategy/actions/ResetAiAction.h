/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RESETAIACTION_H
#define _PLAYERBOT_RESETAIACTION_H

#include "Action.h"

class PlayerbotAI;

class ResetAiAction : public Action
{
public:
    ResetAiAction(PlayerbotAI* botAI) : Action(botAI, "reset botAI") {}

    bool Execute(Event event) override;
};

#endif
