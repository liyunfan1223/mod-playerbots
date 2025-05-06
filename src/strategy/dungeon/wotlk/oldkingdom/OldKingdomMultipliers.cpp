#include "OldKingdomMultipliers.h"
#include "OldKingdomActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "OldKingdomTriggers.h"
#include "Action.h"

float ElderNadoxMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "elder nadox");
    if (!boss) { return 1.0f; }

    Unit* guardian = AI_VALUE2(Unit*, "find target", "ahn'kahar guardian");
    if (guardian)
    {
        if (dynamic_cast<DpsAssistAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float JedogaShadowseekerMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "jedoga shadowseeker");
    if (!boss) { return 1.0f; }

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
    
    if (volunteer)
    {
        if (dynamic_cast<DpsAssistAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float ForgottenOneMultiplier::GetValue(Action* action)
{
    Unit* unit = AI_VALUE2(Unit*, "find target", "forgotten one");
    if (!unit) { return 1.0f; }

    if (bot->isMoving())
    {
        if (dynamic_cast<MovementAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}
