/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SETHOMEACTION_H
#define _PLAYERBOT_SETHOMEACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class SetHomeAction : public MovementAction
{
public:
    SetHomeAction(PlayerbotAI* botAI) : MovementAction(botAI, "home") {}

    bool Execute(Event event) override;
};

#endif
