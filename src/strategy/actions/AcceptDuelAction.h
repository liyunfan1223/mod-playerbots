/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ACCEPTDUELACTION_H
#define _PLAYERBOT_ACCEPTDUELACTION_H

#include "Action.h"

class PlayerbotAI;

class AcceptDuelAction : public Action
{
public:
    AcceptDuelAction(PlayerbotAI* botAI) : Action(botAI, "accept duel") {}

    bool Execute(Event event) override;
};

#endif
