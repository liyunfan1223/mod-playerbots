#include "Playerbots.h"
#include "UtgardePinnacleActions.h"
#include "UtgardePinnacleStrategy.h"

bool AvoidFreezingCloudAction::Execute(Event event)
{
    Unit* closestTrigger = nullptr;
    GuidVector objects = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (auto i = objects.begin(); i != objects.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_BREATH_TRIGGER)
        {
            if (!closestTrigger || bot->GetExactDist2d(unit) < bot->GetExactDist2d(closestTrigger))
            {
                closestTrigger = unit;
            }
        }
    }

    if (!closestTrigger) { return false; }

    float distance = bot->GetExactDist2d(closestTrigger->GetPosition());
    float radius = 3.0f;
    // Large buffer for this - the radius of the breath is a lot smaller than the graphic, but it looks dumb
    // if the bot stands just outside the hitbox but still visibly in the cloud patches.
    float distanceExtra = 3.0f;

    if (distance < radius + distanceExtra - 1.0f)
    {
        // bot->Yell("MOVING", LANG_UNIVERSAL);
        return MoveAway(closestTrigger, radius + distanceExtra - distance);
    }

    return false;
}

bool AvoidSkadiWhirlwindAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "skadi the ruthless");
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
