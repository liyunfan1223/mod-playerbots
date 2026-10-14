/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MgTActions.h"
#include "AiObjectContext.h"
#include "EncounterHelpers.h"
#include "MgTShared.h"
#include "Playerbots.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using namespace EncounterHelpers;

bool MgTEscapeAction::Execute(Event)
{
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    auto const& spots = AI_VALUE_REF(MagistersTerrace::EscapeSpots, _value);
    for (MagistersTerrace::EscapeSpot const& spot : spots)
    {
        if (MoveTo(bot->GetMapId(), spot.x, spot.y, spot.z, false, false, false, false,
                   MovementPriority::MOVEMENT_COMBAT, true, false))
            return true;
    }

    return false;
}

bool MgTFocusAction::Execute(Event)
{
    Unit* target = botAI->GetUnit(AI_VALUE(ObjectGuid, _value));
    if (!target)
        return false;

    if (IsMechanicTrackerBot(bot, MagistersTerrace::MAP_MAGISTERS_TERRACE))
        MarkTargetWithSkull(bot, target);

    if (AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}

bool MgTKillCrystalAction::Execute(Event)
{
    Unit* crystal = botAI->GetUnit(AI_VALUE(ObjectGuid, "mgt crystal target"));
    if (!crystal || AI_VALUE(Unit*, "current target") == crystal)
        return false;

    return Attack(crystal);
}

bool MgTReturnToRoomAction::Execute(Event)
{
    float x = MagistersTerrace::SELIN_REGROUP_X;
    float y = bot->GetPositionY();
    MagistersTerrace::ClampIntoRoom(x, y);

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const bz = bot->GetPositionZ();

    float const dist = bot->GetExactDist2d(x, y);
    if (dist < 1.0f)
        return false;

    float const step = std::min(8.0f, dist);
    float const moveX = bx + ((x - bx) / dist) * step;
    float const moveY = by + ((y - by) / dist) * step;

    float moveZ = bot->GetMapHeight(moveX, moveY, bz + MagistersTerrace::GROUND_SEARCH_UP, true,
                                    MagistersTerrace::GROUND_SEARCH_DOWN);
    if (std::fabs(moveZ - bz) > MagistersTerrace::GROUND_TIER_STEP)
        moveZ = bz;

    return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool MgTCloseOnMageGuardAction::Execute(Event)
{
    Unit* guard = botAI->GetUnit(AI_VALUE(ObjectGuid, "mgt mage guard target"));
    if (!guard)
        return false;

    return ReachCombatTo(guard, 0.0f);
}

bool MgTPriorityInterruptAction::Execute(Event)
{
    Unit* target = botAI->GetUnit(AI_VALUE(ObjectGuid, "mgt interrupt target"));
    if (!target)
        return false;

    uint8 const urgency = MagistersTerrace::GetInterruptUrgency(target);
    if (!urgency)
        return false;

    return !MagistersTerrace::CastInterrupt(botAI, target, urgency).empty();
}

bool MgTTauntEnragedWretchedAction::Execute(Event)
{
    Unit* wretched = botAI->GetUnit(AI_VALUE(ObjectGuid, "mgt enraged wretched"));
    if (!wretched)
        return false;

    for (std::string const& spell : MagistersTerrace::TauntSpellNames())
    {
        if (botAI->CanCastSpell(spell, wretched) && botAI->CastSpell(spell, wretched))
            return true;
    }

    return false;
}

bool MgTDelrissaInterruptAction::Execute(Event)
{
    auto const& order = AI_VALUE_REF(GuidVector, "mgt delrissa interrupt order");
    for (ObjectGuid const guid : order)
    {
        Unit* target = botAI->GetUnit(guid);
        if (!target)
            continue;

        uint8 const urgency = MagistersTerrace::GetRetinueInterruptUrgency(target);
        if (!urgency)
            continue;

        if (!MagistersTerrace::CastInterrupt(botAI, target, urgency).empty())
            return true;
    }

    return false;
}

bool MgTDelrissaTremorTotemAction::Execute(Event)
{
    return botAI->CanCastSpell("tremor totem", bot) && botAI->CastSpell("tremor totem", bot);
}

bool MgTKaelInterruptAction::Execute(Event)
{
    Unit* target = MagistersTerrace::GetKaelInterruptTarget(bot);
    if (!target)
        return false;

    return !MagistersTerrace::CastInterrupt(botAI, target, MagistersTerrace::CONTROL_INTERRUPT_URGENCY).empty();
}

bool MgTTakeLapseSpotAction::Execute(Event)
{
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    MagistersTerrace::LapseWorld world;
    MagistersTerrace::CollectLapseWorld(bot, world);

    MagistersTerrace::LapseSpot spot = { bot->GetPositionX(), bot->GetPositionY() };
    MagistersTerrace::LapseSpot station = spot;
    Unit* kael = MagistersTerrace::GetKaelthas(bot);
    bool move = false;

    if (MagistersTerrace::GetGravityLapseKite(bot, world, spot, _kite) ||
        MagistersTerrace::GetGravityLapseDodge(bot, world, spot))
    {
        move = true;
    }
    else if (MagistersTerrace::GetGravityLapseSpot(bot, world, station) &&
             !MagistersTerrace::OnLapseStation(bot, kael, station))
    {
        move = MagistersTerrace::GetLapseApproach(bot, world, station,
                                                  MagistersTerrace::LapseThreatened(bot, world), spot);
    }

    if (!move)
        return false;

    float z = bot->GetPositionZ();
    if (kael && MagistersTerrace::IsLapseMeleeSlot(bot))
        z = kael->GetPositionZ();

    return MoveTo(bot->GetMapId(), spot.x, spot.y, z, false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}
