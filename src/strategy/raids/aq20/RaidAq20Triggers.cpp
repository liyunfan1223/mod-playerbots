#include "RaidAq20Triggers.h"

#include "SharedDefines.h"
#include "RaidAq20Utils.h"


bool Aq20MoveToCrystalTrigger::IsActive()
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "ossirian the unscarred"))
    {
        if (boss->IsInCombat())
        {
            // if buff is active move to crystal
            if (RaidAq20Utils::IsOssirianBuffActive(boss))
                return true;

            // if buff is not active a debuff will be, buff becomes active once debuff expires
            // so move to crystal when debuff almost done, or based debuff time left and
            // distance bot is from crystal (ie: start moving early enough to make it)
            int32 debuffTimeRemaining = RaidAq20Utils::GetOssirianDebuffTimeRemaining(boss);
            if (debuffTimeRemaining < 5000)
                return true;
            if (debuffTimeRemaining < 30000)
            {
                if (GameObject* crystal = RaidAq20Utils::GetNearestCrystal(boss))
                {
                    float botDist = bot->GetDistance(crystal);
                    float timeToReach = botDist / bot->GetSpeed(MOVE_RUN);
                    // bot should ideally activate crystal a ~5 seconds early (due to time it takes for crystal
                    // to activate) so aim to get there in time to do so
                    return debuffTimeRemaining - 5000 < timeToReach * 1000.0f;
                }
            }
        }
    }
    return false;
}
