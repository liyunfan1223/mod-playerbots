/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ACCEPTRESURRECTACTION_H
#define _PLAYERBOT_ACCEPTRESURRECTACTION_H

#include "Action.h"

class PlayerbotAI;

class AcceptResurrectAction : public Action
{
public:
    AcceptResurrectAction(PlayerbotAI* botAI) : Action(botAI, "accept resurrect") {}

    bool Execute(Event event) override;
};

#endif
