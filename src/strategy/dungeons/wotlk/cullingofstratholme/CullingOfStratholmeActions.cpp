#include "Playerbots.h"
#include "CullingOfStratholmeActions.h"
#include "CullingOfStratholmeStrategy.h"


bool ExplodeGhoulSpreadAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "salramm the fleshcrafter");
    if (!boss) { return false; }

    float distance = 10.0f;
    float distanceExtra = 2.0f;
    GuidVector corpses = AI_VALUE(GuidVector, "nearest corpses");
    for (auto i = corpses.begin(); i != corpses.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_GHOUL_MINION)
        {
            float currentDistance = bot->GetExactDist2d(unit);
            if (currentDistance < distance + distanceExtra)
            {
                return MoveAway(unit, distance + distanceExtra - currentDistance);
            }
        }
    }
    return false;
}

bool EpochStackAction::isUseful()
{
    // Minimum hunter range is 5, but values too close to this seem to cause issues..
    // Hunter bots will try and melee in between ranged attacks, or just melee entirely at 5 as they are in range.
    // 7.5 or 8.0 solves this for this boss.
    // Unfortunately at this range the boss will charge. So I guess just don't stack as a hunter..
    // if(bot->getClass() == CLASS_HUNTER)
    // {
    //     return AI_VALUE2(float, "distance", "current target") > 7.5f;
    // }
    // else
    return !(bot->getClass() == CLASS_HUNTER) && AI_VALUE2(float, "distance", "current target") > 5.0f;
}
bool EpochStackAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono-lord epoch");
    if (!boss) { return false; }

    float maxMovement = 10.0f;
    // if(bot->getClass() == CLASS_HUNTER)
    // {
    //     return Move(bot->GetAngle(boss), fmin(bot->GetExactDist2d(boss) - 6.5f, maxMovement));
    // }
    // else
    return Move(bot->GetAngle(boss), fmin(bot->GetExactDist2d(boss), maxMovement));
}
