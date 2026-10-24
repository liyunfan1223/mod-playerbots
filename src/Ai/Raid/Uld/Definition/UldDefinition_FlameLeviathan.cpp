/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldDefinitions.h"

#include "Player.h"
#include "PlayerbotAI.h"
#include "Strategy.h"
#include "UldActions_FlameLeviathan.h"
#include "UldData.h"
#include "UldEncounterGate.h"
#include "UldEncounter_FlameLeviathan.h"
#include "UldScripts.h"
#include "UldTriggers_FlameLeviathan.h"

namespace Role = RaidEncounterRules::Role;

namespace
{
bool RidingRaidVehicle(Player* bot)
{
    Unit* vehicleBase = bot->GetVehicleBase();
    if (!vehicleBase)
        return false;

    switch (vehicleBase->GetEntry())
    {
        case NPC_SALVAGED_SIEGE_ENGINE:
        case NPC_SALVAGED_SIEGE_ENGINE_TURRET:
        case NPC_SALVAGED_DEMOLISHER:
        case NPC_SALVAGED_DEMOLISHER_TURRET:
        case NPC_VEHICLE_CHOPPER:
            return true;
        default:
            return false;
    }
}

// FlameLeviathanEngaged walks the target list, so it goes last.
bool FlameLeviathanRiding(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return bot->GetMapId() == ULDUAR_MAP_ID && RidingRaidVehicle(bot) && FlameLeviathanEngaged(botAI);
}

void DefineFlameLeviathan(EncounterBuilder& e)
{
    e.Node<FlameLeviathanFlameVentsTrigger, FlameLeviathanInterruptVentsAction>(ACTION_RAID + 4);

    // Same action as the routine row: actions are cached by name, so both rows share one instance and one
    // latched destination, just promoted while chased or in a hazard.
    e.Node<FlameLeviathanDriveUrgentTrigger, FlameLeviathanDriveAction>(ACTION_RAID + 3);

    e.Node<FlameLeviathanVehicleNearTrigger, FlameLeviathanEnterVehicleAction>(ACTION_RAID + 2);
    e.Node<FlameLeviathanOnVehicleTrigger, FlameLeviathanVehicleAction>(ACTION_RAID + 1);
    e.Node<FlameLeviathanOnVehicleTrigger, FlameLeviathanDriveAction>(ACTION_RAID + 0.5f);

    // Only the drive moves a vehicle, two movers on one MotionMaster bounce. Boarding and leaving stay, or a
    // bot that lost its hull never boards again and nobody gets out after the kill.
    e.Exclusive("flame leviathan vehicle movement", Role::Any, FlameLeviathanRiding, 0,
                {FlameLeviathanDriveAction::Name, FlameLeviathanEnterVehicleAction::Name, "leave vehicle"});

    e.Tick(FlameLeviathanTick);
}
}  // namespace

EncounterDefinition const& UldFlameLeviathanDefinition()
{
    static EncounterDefinition const definition(ULD_BOSS_LEVIATHAN, &BossStateGateOpen, &DefineFlameLeviathan);
    return definition;
}
