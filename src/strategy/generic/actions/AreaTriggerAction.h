/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AREATRIGGERACTION_H
#define _PLAYERBOT_AREATRIGGERACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class ReachAreaTriggerAction : public MovementAction
{
public:
    ReachAreaTriggerAction(PlayerbotAI* botAI) : MovementAction(botAI, "reach area trigger") {}

    bool Execute(Event event) override;
};

class AreaTriggerAction : public MovementAction
{
public:
    AreaTriggerAction(PlayerbotAI* botAI) : MovementAction(botAI, "area trigger") {}

    bool Execute(Event event) override;
};

#endif
