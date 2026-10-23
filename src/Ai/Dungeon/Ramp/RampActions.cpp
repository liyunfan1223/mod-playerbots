/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RampActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "RampShared.h"

using namespace RampShared;
using namespace EncounterHelpers;

// Watchkeeper Gargolmar

// Hellfire Watchers will be marked with skull

bool GargolmarMarkHellfireWatchersAction::Execute(Event /*event*/)
{
    Unit* watcher = AI_VALUE2(Unit*, "find target", "hellfire watcher");
    if (!watcher)
        return false;

    return MarkTargetWithSkull(bot, watcher);
}

// Omor the Unscarred

// Flee from other players if you have Treacherous Aura or Bane of Treachery
bool OmorTreacheryAuraFleeFromPlayersAction::Execute(Event /*event*/)
{
    if (!GetNearestPlayerInRadius(bot, OMOR_TREACHERY_AURA_SAFE_DISTANCE))
        return false;

    bot->CastStop();

    return MoveFromGroup(OMOR_TREACHERY_AURA_SAFE_DISTANCE);
}

// Nearby bots should flee from the tank if it has Treacherous Aura or Bane of Treachery
bool OmorTreacheryAuraFleeFromTankAction::Execute(Event /*event*/)
{
    Unit* omor = AI_VALUE2(Unit*, "find target", "omor the unscarred");

    if (!omor)
        return false;

    Unit* tank = GetGroupMainTank(bot);

    if (!tank)
        return false;

    if (bot->GetExactDist2d(tank) >= OMOR_TREACHERY_AURA_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return MoveAway(tank, OMOR_TREACHERY_AURA_SAFE_DISTANCE);
}

// Ranged spread out from each other
bool OmorRangedSpreadAction::Execute(Event /*event*/)
{
    constexpr float minDistance = OMOR_TREACHERY_AURA_SAFE_DISTANCE;

    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance))
        return FleePosition(nearestPlayer->GetPosition(), minDistance);

    return false;
}

// Mark Fiendish Hound with skull
bool OmorMarkFiendishHoundAction::Execute(Event /*event*/)
{
    Unit* hound = AI_VALUE2(Unit*, "find target", "fiendish hound");
    if (!hound)
        return false;

    return MarkTargetWithSkull(bot, hound);
}

// Vazruden & Nazan

// Tank Vazruden in the middle of the platform
bool VazrudenTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* vazruden = AI_VALUE2(Unit*, "find target", "vazruden");
    if (!vazruden)
        return false;

    if (AI_VALUE(Unit*, "current target") != vazruden)
        return Attack(vazruden);

    if (vazruden->GetVictim() != bot || !bot->IsWithinMeleeRange(vazruden) || bot->GetHealthPct() <= 25.0f)
        return false;

    Position const& position = VAZRUDEN_TANK_POSITION;
    constexpr float arrivalDist = 10.0f;
    float distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= arrivalDist)
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, vazruden, moveX, moveY, backwards))
        return false;

    return MoveTo(RAMP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Mark Vazruden with 'Skull'
bool VazrudenMarkBossAction::Execute(Event /*event*/)
{
    Unit* vazruden = AI_VALUE2(Unit*, "find target", "vazruden");
    if (!vazruden)
        return false;

    return MarkTargetWithSkull(bot, vazruden);
}

// Shamans use Tremor totem when Nazan is active
bool NazanSetTremorTotemAction::Execute(Event /*event*/)
{
    return  !AI_VALUE2(bool, "has totem", "tremor totem") &&
            botAI->CanCastSpell("tremor totem", bot) &&
            botAI->CastSpell("tremor totem", bot);
}

// Shamans use Fire Resistance totem when Nazan is active
bool NazanSetFireResistanceTotemAction::Execute(Event /*event*/)
{
    return  !AI_VALUE2(bool, "has totem", "fire resistance totem") &&
            botAI->CanCastSpell("fire resistance totem", bot) &&
            botAI->CastSpell("fire resistance totem", bot);
}

// Paladins use Fire Resistance aura when Nazan is active
bool NazanSetFireResistanceAuraAction::Execute(Event /*event*/)
{
    return  !botAI->HasAura("fire resistance aura", bot) &&
            botAI->CanCastSpell("fire resistance aura", bot) &&
            botAI->CastSpell("fire resistance aura", bot);
}
