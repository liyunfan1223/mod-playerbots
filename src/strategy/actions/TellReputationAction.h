/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TELLREPUTATIONACTION_H
#define _PLAYERBOT_TELLREPUTATIONACTION_H

#include "Action.h"

class PlayerbotAI;

class TellReputationAction : public Action
{
public:
    TellReputationAction(PlayerbotAI* botAI) : Action(botAI, "reputation") {}

    bool Execute(Event event) override;
};

#endif
