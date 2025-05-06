#include "Playerbots.h"
#include "CullingOfStratholmeTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"


bool ExplodeGhoulTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "salramm the fleshcrafter");
    if (!boss) { return false; }

    float distance = 10.0f;
    float distanceExtra = 2.0f;
    GuidVector corpses = AI_VALUE(GuidVector, "nearest corpses");
    for (auto i = corpses.begin(); i != corpses.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_RISEN_GHOUL)
        {
            if (bot->GetExactDist2d(unit) < distance + distanceExtra)
            {
                return true;
            }
        }
    }
    return false;
}

bool EpochRangedTrigger::IsActive()
{
    return !botAI->IsMelee(bot) && AI_VALUE2(Unit*, "find target", "chrono-lord epoch");
}
