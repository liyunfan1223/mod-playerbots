#include "Playerbots.h"
#include "OldKingdomTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"


bool NadoxGuardianTrigger::IsActive()
{
    if (botAI->IsHeal(bot)) { return false; }

    Unit* boss = AI_VALUE2(Unit*, "find target", "elder nadox");
    Unit* guardian = AI_VALUE2(Unit*, "find target", "ahn'kahar guardian");

    return boss && guardian;
}

bool JedogaVolunteerTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "jedoga shadowseeker");
    // Unit* volunteer = AI_VALUE2(Unit*, "find target", "twilight volunteer");
    Unit* volunteer = nullptr;
    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_TWILIGHT_VOLUNTEER)
        {
            volunteer = unit;
            break;
        }
    }
    
    return boss && volunteer;
}

bool ShadowCrashTrigger::IsActive()
{
    if (botAI->IsMelee(bot)) { return false; }
    return !botAI->IsMelee(bot) && AI_VALUE2(Unit*, "find target", "forgotten one");
}
