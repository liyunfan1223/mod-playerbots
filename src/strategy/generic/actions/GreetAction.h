/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GREETACTION_H
#define _PLAYERBOT_GREETACTION_H

#include "Action.h"

class PlayerbotAI;

class GreetAction : public Action
{
public:
    GreetAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

#endif
