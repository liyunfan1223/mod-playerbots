/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_FlameLeviathan.h"

#include "GameObject.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include "UldEncounter_FlameLeviathan.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "Vehicle.h"

namespace
{
// Every vehicle a bot can usefully occupy on this fight, driver seats and gunner seats alike.
constexpr uint32 RIDEABLE_VEHICLES[] = {NPC_VEHICLE_CHOPPER, NPC_SALVAGED_DEMOLISHER, NPC_SALVAGED_DEMOLISHER_TURRET,
                                        NPC_SALVAGED_SIEGE_ENGINE, NPC_SALVAGED_SIEGE_ENGINE_TURRET};
}  // namespace

bool FlameLeviathanOnVehicleTrigger::IsActive()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicleBase || !vehicle)
        return false;

    uint32 entry = vehicleBase->GetEntry();
    for (uint32 rideable : RIDEABLE_VEHICLES)
        if (entry == rideable)
            return true;

    return false;
}

bool FlameLeviathanVehicleNearTrigger::IsActive()
{
    if (bot->GetVehicle())
        return false;

    Player* master = botAI->GetMaster();
    if (!master)
        return false;

    if (!master->GetVehicle())
        return false;

    return true;
}

bool FlameLeviathanFlameVentsTrigger::IsActive()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    if (!vehicleBase || vehicleBase->GetEntry() != NPC_SALVAGED_SIEGE_ENGINE)
        return false;

    if (!FlameLeviathanIsVentChanneling(FlameLeviathanBoss(botAI)))
        return false;

    // One siege engine per channel. The action and this share the helper, or the two would disagree
    // about whose job it is and either double up or leave the channel running.
    return FlameLeviathanIsVentInterrupter(botAI, bot);
}

bool FlameLeviathanDriveUrgentTrigger::IsActive()
{
    if (!FlameLeviathanIsDriver(bot) || !FlameLeviathanEngaged(botAI))
        return false;

    if (FlameLeviathanIsPursued(bot) || FlameLeviathanIsRamTarget(bot))
        return true;

    // Getting out of Battering Ram outranks holding station, same as a tower hazard does: both are
    // "you are standing somewhere that is about to hurt" rather than positioning preferences.
    if (FlameLeviathanShouldClearBatteringRam(botAI, bot))
        return true;

    uint32 towerMask = FlameLeviathanActiveTowerMask(botAI);
    if (!towerMask)
        return false;

    Unit* vehicleBase = bot->GetVehicleBase();
    return GetFlameLeviathanNearestTowerHazard(botAI, vehicleBase, towerMask) != nullptr;
}
