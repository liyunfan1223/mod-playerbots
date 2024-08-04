/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "VehicleActions.h"

#include "ItemVisitors.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Vehicle.h"

bool EnterVehicleAction::Execute(Event event)
{
    // do not switch vehicles yet
    if (bot->GetVehicle())
        return false;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles");
    for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* vehicleBase = botAI->GetUnit(*i);
        if (!vehicleBase)
            return false;

        if (!vehicleBase->IsFriendlyTo(bot))
            return false;

        if (!vehicleBase->GetVehicleKit()->GetAvailableSeatCount())
            return false;

        if (fabs(bot->GetPositionZ() - vehicleBase->GetPositionZ()) < 20.0f)

            // if (sServerFacade->GetDistance2d(bot, vehicle) > 100.0f)
            //     continue;

            if (sServerFacade->GetDistance2d(bot, vehicleBase) > 10.0f)
                return MoveTo(vehicleBase, INTERACTION_DISTANCE);

        bot->EnterVehicle(vehicleBase);

        if (!bot->IsOnVehicle(vehicleBase))
            return false;

        // dismount because bots can enter vehicle on mount
        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
        return true;
    }

    return false;
}

bool LeaveVehicleAction::Execute(Event event)
{
    Vehicle* myVehicle = bot->GetVehicle();
    if (!myVehicle)
        return false;

    VehicleSeatEntry const* seat = myVehicle->GetSeatForPassenger(bot);
    if (!seat || !seat->CanEnterOrExit())
        return false;

    WorldPacket p;
    bot->GetSession()->HandleRequestVehicleExit(p);

    return true;
}
