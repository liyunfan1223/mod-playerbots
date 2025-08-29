/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ARENATEAMACTION_H
#define _PLAYERBOT_ARENATEAMACTION_H

#include "Action.h"

class PlayerbotAI;

class ArenaTeamAcceptAction : public Action
{
public:
    ArenaTeamAcceptAction(PlayerbotAI* botAI) : Action(botAI, "arena team accept") {}

    bool Execute(Event event) override;
};

#endif
