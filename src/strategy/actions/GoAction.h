/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GOACTION_H
#define _PLAYERBOT_GOACTION_H

#include "MovementActions.h"

class PlayerbotAI;
class TravelDestination;
class WorldPosition;

class GoAction : public MovementAction
{
public:
    GoAction(PlayerbotAI* botAI) : MovementAction(botAI, "Go") {}

    bool Execute(Event event) override;
};

#endif
