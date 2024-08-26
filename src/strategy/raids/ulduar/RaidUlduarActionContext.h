// /*
//  * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
//  and/or modify it under version 2 of the License, or (at your option), any later version.
//  */

#ifndef _PLAYERBOT_RAIDULDUARACTIONCONTEXT_H
#define _PLAYERBOT_RAIDULDUARACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidUlduarActions.h"

class RaidUlduarActionContext : public NamedObjectContext<Action>
{
public:
    RaidUlduarActionContext()
    {
        creators["flame leviathan vehicle"] = &RaidUlduarActionContext::flame_leviathan_vehicle;
        creators["flame leviathan enter vehicle"] = &RaidUlduarActionContext::flame_leviathan_enter_vehicle;
    }

private:
    static Action* flame_leviathan_vehicle(PlayerbotAI* ai) { return new FlameLeviathanVehicleAction(ai); }
    static Action* flame_leviathan_enter_vehicle(PlayerbotAI* ai) { return new FlameLeviathanEnterVehicleAction(ai); }
};

#endif