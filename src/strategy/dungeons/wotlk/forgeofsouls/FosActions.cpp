#include "Playerbots.h"
#include "FosActions.h"
#include "FosStrategy.h"

bool MoveFromBronjahmAction::Execute(Event event)
{
    Unit* boss = nullptr;
    boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    float distance = bot->GetExactDist2d(boss->GetPosition());
    float targetDis = 30.0f;
    if (distance >= targetDis)
        return false;
    return MoveAway(boss, targetDis - distance);
}

bool AttackCorruptedSoulFragmentAction::Execute(Event event)
{
    Unit* fragment = nullptr;
    fragment = AI_VALUE2(Unit*, "find target", "corrupted soul fragment");
    if (!fragment)
        return false;

    if (AI_VALUE(Unit*, "current target") == fragment)
        return false;

    return Attack(fragment);
}
