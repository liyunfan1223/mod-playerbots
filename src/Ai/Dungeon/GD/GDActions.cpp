/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GDActions.h"
#include "Playerbots.h"

bool AvoidPoisonNovaAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "slad'ran");
    if (!boss) { return false; }

    float distance = bot->GetExactDist2d(boss->GetPosition());
    float radius = 15.0f;
    float distanceExtra = 2.0f;

    if (distance < radius + distanceExtra)
    {
        return MoveAway(boss, radius + distanceExtra - distance);
    }

    return false;
}

bool AttackSnakeWrapAction::Execute(Event /*event*/)
{
    Unit* snakeWrap = GundrakSladran::GetAssignedSnakeWrap(botAI);
    if (!snakeWrap) { return false; }

    if (AI_VALUE(Unit*, "current target") == snakeWrap) { return false; }

    return Attack(snakeWrap);
}

bool SladranStackOnTankAction::Execute(Event /*event*/)
{
    Player* tank = GundrakSladran::GetStackTank(botAI);
    if (!tank) { return false; }

    return MoveTo(tank, GundrakSladran::STACK_CLOSE_TO_YD, MovementPriority::MOVEMENT_COMBAT);
}

bool SladranTankHoldAction::Execute(Event /*event*/)
{
    Unit* boss = GundrakSladran::GetTankHoldTarget(botAI);
    if (!boss) { return false; }

    return Attack(boss);
}

bool AvoidWhirlingSlashAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "gal'darah");
    if (!boss) { return false; }

    float distance = bot->GetExactDist2d(boss->GetPosition());
    float radius = 5.0f;
    float distanceExtra = 2.0f;

    if (distance < radius + distanceExtra)
    {
        if (PlayerbotAI::IsTank(bot))
        {
            // The boss chases tank during this, leads to jittery stutter-stepping
            // by the tank if we don't pre-move additional range. 2*radius seems ok
            return MoveAway(boss, (2.0f * radius) + distanceExtra - distance);
        }
        // else
        return MoveAway(boss, radius + distanceExtra - distance);
    }

    return false;
}
