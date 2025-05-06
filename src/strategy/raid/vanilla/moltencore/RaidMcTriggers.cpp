#include "RaidMcTriggers.h"

#include "SharedDefines.h"

bool McLivingBombDebuffTrigger::IsActive()
{
    // if bot has barron geddon's living bomb, we need to add strat, otherwise we need to remove
    // only do when fighting baron geddon (to avoid modifying strat set by player outside this fight)
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "baron geddon"))
    {
        if (boss->IsInCombat())
            return bot->HasAura(20475) != botAI->HasStrategy("move from group", BotState::BOT_STATE_COMBAT);
    }
    return false;
}

bool McBaronGeddonInfernoTrigger::IsActive()
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "baron geddon"))
        return boss->HasAura(19695);
    return false;
}
