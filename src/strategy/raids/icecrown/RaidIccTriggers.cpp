#include "RaidIccTriggers.h"

bool IccInCannonTrigger::IsActive()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicleBase || !vehicle)
        return false;

    uint32 entry = vehicleBase->GetEntry();
    return entry == 36838 || entry == 36839;
}

bool IccGunshipCannonNearTrigger::IsActive()
{
    if (bot->GetVehicle())
        return false;
    
    if (!botAI->IsDps(bot))
        return false;
    // Player* master = botAI->GetMaster();
    // if (!master)
    //     return false;
    
    // if (!master->GetVehicle())
    //     return false;

    return true;
}
