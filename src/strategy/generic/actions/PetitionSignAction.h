/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PETITIONSIGNACTION_H
#define _PLAYERBOT_PETITIONSIGNACTION_H

#include "Action.h"

class PlayerbotAI;

class PetitionSignAction : public Action
{
public:
    PetitionSignAction(PlayerbotAI* botAI) : Action(botAI, "petition sign") {}

    bool Execute(Event event) override;
};

#endif
