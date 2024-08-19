/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_VEHICLEACTIONS_H
#define _PLAYERBOT_VEHICLEACTIONS_H

#include "Event.h"
#include "MovementActions.h"

class PlayerbotAI;

class EnterVehicleAction : public MovementAction
{
public:
    EnterVehicleAction(PlayerbotAI* botAI, std::string const& name = "enter vehicle") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
protected:
    bool EnterVehicle(Unit* vehicleBase, bool moveIfFar);
};

class LeaveVehicleAction : public MovementAction
{
public:
    LeaveVehicleAction(PlayerbotAI* botAI, std::string const& name = "leave vehicle") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
