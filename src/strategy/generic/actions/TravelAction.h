/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRAVELACTION_H
#define _PLAYERBOT_TRAVELACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class TravelAction : public MovementAction
{
public:
    TravelAction(PlayerbotAI* botAI) : MovementAction(botAI, "travel") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MoveToDarkPortalAction : public MovementAction
{
public:
    MoveToDarkPortalAction(PlayerbotAI* botAI) : MovementAction(botAI, "move to dark portal") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class DarkPortalAzerothAction : public MovementAction
{
public:
    DarkPortalAzerothAction(PlayerbotAI* botAI) : MovementAction(botAI, "dark portal azeroth") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MoveFromDarkPortalAction : public MovementAction
{
public:
    MoveFromDarkPortalAction(PlayerbotAI* botAI) : MovementAction(botAI, "move from dark portal") {}

    bool Execute(Event event) override;
};

#endif
