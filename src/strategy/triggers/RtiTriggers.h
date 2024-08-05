/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RTITRIGGERS_H
#define _PLAYERBOT_RTITRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

class NoRtiTrigger : public Trigger
{
public:
    NoRtiTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no rti target") {}

    bool IsActive() override;
};

#endif
