#include "Playerbots.h"
#include "GundrakActions.h"
#include "GundrakStrategy.h"

bool AvoidPoisonNovaAction::Execute(Event event)
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

bool AttackSnakeWrapAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "slad'ran");
    if (!boss) { return false; }

    Unit* snakeWrap = nullptr;
    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_SNAKE_WRAP)
        {
            Unit* currentTarget = AI_VALUE(Unit*, "current target");
            if (!currentTarget || currentTarget->GetEntry() != NPC_SNAKE_WRAP)
            {
            return Attack(unit);
            }
        }
    }

    return false;
}

bool AvoidWhirlingSlashAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "gal'darah");
    if (!boss) { return false; }

    float distance = bot->GetExactDist2d(boss->GetPosition());
    float radius = 5.0f;
    float distanceExtra = 2.0f;

    if (distance < radius + distanceExtra)
    {
        if (botAI->IsTank(bot))
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
