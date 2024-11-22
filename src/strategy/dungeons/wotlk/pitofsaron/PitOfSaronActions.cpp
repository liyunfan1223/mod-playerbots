#include "Playerbots.h"
#include "PitOfSaronActions.h"
#include "PitOfSaronStrategy.h"

bool MoveFromIckPursuitAction::Execute(Event event)
{    
    Unit* boss = AI_VALUE2(Unit*, "find target", "ick");
    if (!boss)
        return false;

    
    float distance = bot->GetExactDist2d(boss->GetPosition());
    float targetDis = 20.0f;
    if (distance >= targetDis)
        return false;
    return MoveAway(boss, targetDis - distance);
}

bool MoveFromPoisonNovaAction::Execute(Event event)
{    
    Unit* boss = AI_VALUE2(Unit*, "find target", "ick");
    if (!boss)
        return false;

    
    float distance = bot->GetExactDist2d(boss->GetPosition());
    float targetDis = 20.0f;
    if (distance >= targetDis)
        return false;
    return MoveAway(boss, targetDis - distance);
}

bool MoveFromExplodingOrbAction::Execute(Event event)
{
    Unit* orb = nullptr;
    float minDistance = 1000.0f; // Initialize with a large value

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto &target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_EXPLODING_ORB)
        {
            float distance = bot->GetExactDist2d(unit->GetPosition());
            if (distance < minDistance)
            {
                minDistance = distance;
                orb = unit;
            }
        }
    }

    if (!orb)
        return false;

    float targetDis = 20.0f;
    if (minDistance >= targetDis)
        return false;
    return MoveAway(orb, targetDis - minDistance);
}
