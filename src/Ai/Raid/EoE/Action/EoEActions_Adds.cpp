/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EoEActions_Adds.h"
#include "EoEData.h"
#include "EoEEncounter_Malygos.h"
#include "MotionMaster.h"
#include "Playerbots.h"
#include "Vehicle.h"
#include "WorldSession.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

Unit* MalygosSpellstealAction::GetHastedLord()
{
    if (!bot->IsClass(CLASS_MAGE) || bot->GetVehicle())
        return nullptr;
    if (GetMalygosPhase(bot) != MalygosPhase::P2)
        return nullptr;

    Unit* best = nullptr;
    float closest = std::numeric_limits<float>::max();

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");
    for (ObjectGuid const& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (!unit || unit->GetEntry() != NPC_NEXUS_LORD || !unit->IsAlive())
            continue;
        if (!unit->HasAura(SPELL_HASTE))
            continue;

        if (!bot->IsWithinCombatRange(unit, sPlayerbotAIConfig.SpellDistance))
            continue;

        float dist = bot->GetExactDist(unit);
        if (dist < closest)
        {
            closest = dist;
            best = unit;
        }
    }
    return best;
}

bool MalygosSpellstealAction::isUseful()
{
    Unit* lord = GetHastedLord();
    if (!lord || !botAI->CanCastSpell("spellsteal", lord))
        return false;

    _lordGuid = lord->GetGUID();
    return true;
}

bool MalygosSpellstealAction::Execute(Event /*event*/)
{
    Unit* lord = botAI->GetUnit(_lordGuid);
    return lord && botAI->CastSpell("spellsteal", lord);
}

bool MalygosSeekBubbleAction::isUseful() { return !bot->GetVehicle() && !IsSafelySheltered(bot); }

bool MalygosSeekBubbleAction::Execute(Event /*event*/)
{
    // Arcane Overload is non-attackable, so it never shows up in "possible targets".
    std::vector<Unit*> found;
    GetEoECreatures(bot, NPC_ARCANE_OVERLOAD, found);

    std::vector<Unit*> bubbles;
    for (Unit* bubble : found)
    {
        if (bot->GetExactDist2d(bubble) <= BUBBLE_SEARCH_RADIUS)
            bubbles.push_back(bubble);
    }
    if (bubbles.empty())
    {
        _assignedBubbleGuid.Clear();
        return false;
    }

    std::sort(bubbles.begin(), bubbles.end(), [](Unit* a, Unit* b)
    {
        return a->GetGUID().GetRawValue() < b->GetGUID().GetRawValue();
    });

    std::vector<Unit*> usable;
    for (Unit* bubble : bubbles)
    {
        if (GetBubbleShrinkFactor(bubble) >= BUBBLE_MIN_USABLE_FACTOR)
            usable.push_back(bubble);
    }

    if (usable.empty())
    {
        usable.push_back(*std::max_element(bubbles.begin(), bubbles.end(), [](Unit* a, Unit* b)
        {
            return GetBubbleShrinkFactor(a) < GetBubbleShrinkFactor(b);
        }));
    }

    Unit* target = nullptr;
    if (!_assignedBubbleGuid.IsEmpty())
    {
        for (Unit* bubble : usable)
        {
            if (bubble->GetGUID() == _assignedBubbleGuid)
            {
                target = bubble;
                break;
            }
        }
    }

    if (!target)
    {
        uint32 index = static_cast<uint32>(std::max(0, botAI->GetGroupSlotIndex(bot)));
        target = usable[index % usable.size()];

        if (bot->GetExactDist2d(target) > BUBBLE_SEARCH_RADIUS / 2.0f)
        {
            for (Unit* bubble : usable)
            {
                if (bot->GetExactDist2d(bubble) < bot->GetExactDist2d(target))
                    target = bubble;
            }
        }
        _assignedBubbleGuid = target->GetGUID();
    }

    return MoveTo(EOE_MAP_ID, target->GetPositionX(), target->GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
}

bool MalygosBoardDiskAction::Execute(Event /*event*/)
{
    Unit* disk = FindFreeHoverDisk(bot);
    if (!disk)
        return false;

    return EnterVehicle(disk, true);
}

bool MalygosRideDiskAction::isPossible()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    return vehicleBase && vehicleBase->GetEntry() == NPC_HOVER_DISK;
}

bool MalygosRideDiskAction::Execute(Event /*event*/)
{
    Unit* disk = bot->GetVehicleBase();
    if (!disk)
        return false;

    Unit* scion = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    std::vector<Unit*> scions;
    GetEoECreatures(bot, NPC_SCION_OF_ETERNITY, scions);
    for (Unit* candidate : scions)
    {
        float dist = disk->GetExactDist(candidate);
        if (dist < closestDist)
        {
            closestDist = dist;
            scion = candidate;
        }
    }

    MotionMaster* mm = disk->GetMotionMaster();

    // The core keeps the P2 summons until both add types are dead, so the ride is ended by hand.
    // Fly back down first; stepping off at Scion altitude is a long drop.
    if (!scion)
    {
        if (disk->GetPositionZ() > MALYGOS_PLATFORM_Z + DISK_DISMOUNT_MAX_HEIGHT)
        {
            // Stamped over the approach in progress rather than waiting for POINT to clear.
            if (!_descending || mm->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
            {
                mm->Clear(false);
                mm->MovePoint(0, MALYGOS_CENTER_POSITION.first, MALYGOS_CENTER_POSITION.second,
                              MALYGOS_PLATFORM_Z, FORCED_MOVEMENT_NONE, 0.0f, 0.0f,
                              /*generatePath*/ false, /*forceDestination*/ true);
                disk->SendMovementFlagUpdate();
                _descending = true;
            }
            return true;
        }

        Vehicle* myVehicle = bot->GetVehicle();
        VehicleSeatEntry const* seat = myVehicle ? myVehicle->GetSeatForPassenger(bot) : nullptr;
        if (!seat || !seat->CanEnterOrExit())
            return false;

        WorldPacket p;
        bot->GetSession()->HandleRequestVehicleExit(p);
        return true;
    }

    _descending = false;

    float const reach = DISK_APPROACH_REACH;

    if (closestDist > reach + DISK_APPROACH_TOLERANCE)
    {
        if (mm->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
            return true;

        float dx = disk->GetPositionX() - scion->GetPositionX();
        float dy = disk->GetPositionY() - scion->GetPositionY();
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < EOE_DIRECTION_EPSILON)
        {
            dx = std::cos(disk->GetOrientation());
            dy = std::sin(disk->GetOrientation());
            len = 1.0f;
        }

        float tx = scion->GetPositionX() + dx / len * reach;
        float ty = scion->GetPositionY() + dy / len * reach;

        // Straight 3d spline: a generated path is 2d and drops the destination onto the platform.
        mm->Clear(false);
        mm->MovePoint(0, tx, ty, scion->GetPositionZ(), FORCED_MOVEMENT_NONE, 0.0f, 0.0f,
                      /*generatePath*/ false, /*forceDestination*/ true);
        disk->SendMovementFlagUpdate();
        return true;
    }

    mm->MoveIdle();
    disk->SetFacingToObject(scion);

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget || currentTarget->GetGUID() != scion->GetGUID())
        return Attack(scion);

    return false;
}

bool AvoidSurgeOfPowerAction::isUseful()
{
    // Riders and sheltered bots are already covered; peeling would only walk them out of it.
    if (bot->GetVehicle() || IsSafelySheltered(bot))
        return false;

    // The beam runs from Malygos through the surge focus; stepping off that line clears it.
    Unit* surge = GetNearestEoECreature(bot, NPC_SURGE_OF_POWER, EOE_SURGE_SEARCH_RADIUS);
    if (!surge || bot->GetExactDist2d(surge) >= SURGE_BEAM_CLEAR_DISTANCE)
        return false;

    _surgeGuid = surge->GetGUID();
    return true;
}

bool AvoidSurgeOfPowerAction::Execute(Event /*event*/)
{
    Unit* surge = botAI->GetUnit(_surgeGuid);
    return surge && MoveAway(surge, SURGE_BEAM_SIDESTEP);
}
