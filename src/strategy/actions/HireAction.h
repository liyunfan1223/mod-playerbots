/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HIREACTION_H
#define _PLAYERBOT_HIREACTION_H

#include "Action.h"

class PlayerbotAI;

class HireAction : public Action
{
public:
    HireAction(PlayerbotAI* botAI) : Action(botAI, "hire") {}

    bool Execute(Event event) override;
};

#endif
