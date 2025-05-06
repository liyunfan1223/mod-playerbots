/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RESETINSTANCESACTION_H
#define _PLAYERBOT_RESETINSTANCESACTION_H

#include "Action.h"

class PlayerbotAI;

class ResetInstancesAction : public Action
{
public:
    ResetInstancesAction(PlayerbotAI* botAI) : Action(botAI, "reset instances") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
