#include "Playerbots.h"
#include "HallsOfStoneActions.h"
#include "HallsOfStoneStrategy.h"

bool ShatterSpreadAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "krystallus");
    float radius = 40.0f;
    if (!boss) { return false; }

    Unit* closestMember = nullptr;
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        if (bot->GetGUID() == member)
        {
            continue;
        }
        if (!closestMember ||
            bot->GetExactDist2d(botAI->GetUnit(member)) < bot->GetExactDist2d(closestMember))
        {
            closestMember = botAI->GetUnit(member);
        }
    }

    if (closestMember && bot->GetExactDist2d(closestMember) < radius)
    {
        // Move in small increments so course can be corrected, otherwise two bots may
        // run in the same direction and not adjust until they reach the destination
        // return MoveAway(closestMember, radius - bot->GetExactDist2d(closestMember));
        return MoveAway(closestMember, 5.0f);
    }
    
    return false;
}

bool AvoidLightningRingAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sjonnir the ironshaper");
    if (!boss) { return false; }

    float distance = bot->GetExactDist2d(boss->GetPosition());
    float radius = 10.0f;
    float distanceExtra = 2.0f;

    if (distance < radius + distanceExtra)
    {
        return MoveAway(boss, radius + distanceExtra - distance);
    }

    return false;
}
