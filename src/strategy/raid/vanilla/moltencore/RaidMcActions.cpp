#include "RaidMcActions.h"

#include "Playerbots.h"

bool McCheckShouldMoveFromGroupAction::Execute(Event event)
{
    if (bot->HasAura(20475))  // barron geddon's living bomb
    {
        if (!botAI->HasStrategy("move from group", BotState::BOT_STATE_COMBAT))
        {
            // add/remove from both for now as it will make it more obvious to
            // player if this strat remains on after fight somehow
            botAI->ChangeStrategy("+move from group", BOT_STATE_NON_COMBAT);
            botAI->ChangeStrategy("+move from group", BOT_STATE_COMBAT);
            return true;
        }
    }
    else if (botAI->HasStrategy("move from group", BotState::BOT_STATE_COMBAT))
    {
        // add/remove from both for now as it will make it more obvious to
        // player if this strat remains on after fight somehow
        botAI->ChangeStrategy("-move from group", BOT_STATE_NON_COMBAT);
        botAI->ChangeStrategy("-move from group", BOT_STATE_COMBAT);
        return true;
    }
    return false;
}

bool McMoveFromBaronGeddonAction::Execute(Event event)
{
    const float radius = 25.0f;  // more than should be needed but bots keep trying to run back in
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "baron geddon"))
    {
        long distToTravel = radius - bot->GetDistance(boss);
        if (distToTravel > 0)
        {
            // float angle = bot->GetAngle(boss) + M_PI;
            // return Move(angle, distToTravel);
            return MoveAway(boss, distToTravel); 
        }
    }
    return false;
}
