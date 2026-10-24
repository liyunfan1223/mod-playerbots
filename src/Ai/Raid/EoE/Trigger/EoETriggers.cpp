/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EoETriggers.h"
#include "EoEEncounter_Drakes.h"
#include "EoEEncounter_Malygos.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "Vehicle.h"

bool MalygosTrigger::IsActive()
{
    MalygosPhase const phase = GetMalygosPhase(bot);
    return phase == MalygosPhase::P1 || phase == MalygosPhase::P2 || phase == MalygosPhase::Transition;
}

bool PowerSparkTrigger::IsActive() { return GetMalygosPhase(bot) == MalygosPhase::P1 && AnyLivePowerSpark(bot); }

bool MalygosBubbleTrigger::IsActive()
{
    if (GetMalygosPhase(bot) != MalygosPhase::P2)
        return false;

    // Disk riders are already immune to both Arcane Overload and Surge of Power.
    if (bot->GetVehicle())
        return false;

    // Fires again once the current bubble is nearly spent, so the bot moves before it despawns.
    if (IsSafelySheltered(bot))
        return false;

    if (!PlayerbotAI::IsRanged(bot) && !PlayerbotAI::IsHeal(bot))
    {
        // Melee and tanks owe the raid a dead Nexus Lord and then a disk ride first; the disk half
        // only counts for bots MalygosFreeDiskTrigger will actually let board.
        if (GetNearestEoECreature(bot, NPC_NEXUS_LORD, BUBBLE_SEARCH_RADIUS))
            return false;
        if (IsEligibleDiskRider(bot) && AnyScionAlive(bot) && FindFreeHoverDisk(bot))
            return false;
    }

    return GetNearestEoECreature(bot, NPC_ARCANE_OVERLOAD, BUBBLE_SEARCH_RADIUS) != nullptr;
}

bool MalygosFreeDiskTrigger::IsActive()
{
    if (GetMalygosPhase(bot) != MalygosPhase::P2)
        return false;
    if (bot->GetVehicle())
        return false;
    if (!IsEligibleDiskRider(bot))
        return false;

    // Don't climb back onto a disk the bot just got off because the Scions are dead.
    if (!AnyScionAlive(bot))
        return false;

    return FindFreeHoverDisk(bot) != nullptr;
}

bool MalygosOnDiskTrigger::IsActive()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    return vehicleBase && vehicleBase->GetEntry() == NPC_HOVER_DISK;
}

bool SurgeOfPowerTrigger::IsActive()
{
    if (GetMalygosPhase(bot) != MalygosPhase::P2)
        return false;

    if (GetNearestEoECreature(bot, NPC_SURGE_OF_POWER, EOE_SURGE_SEARCH_RADIUS))
        return true;

    Unit* boss = GetMalygos(bot);
    return boss && bool(boss->FindCurrentSpellBySpellId(SPELL_SURGE_OF_POWER_P2));
}

bool MalygosDrakeFlightTrigger::IsActive()
{
    Unit* drake = bot->GetVehicleBase();
    return drake && drake->GetEntry() == NPC_WYRMREST_SKYTALON;
}

bool DrakeSurgeTrigger::IsActive()
{
    if (GetMalygosPhase(bot) != MalygosPhase::P3)
        return false;

    // The healer rotation reads the same helper, so the two cannot disagree about who is hit.
    return IsDrakeSurgeTarget(botAI);
}
