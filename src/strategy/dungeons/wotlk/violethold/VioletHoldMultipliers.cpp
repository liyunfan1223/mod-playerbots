#include "VioletHoldMultipliers.h"
#include "VioletHoldActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "VioletHoldTriggers.h"
#include "Action.h"

float ErekemMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "erekem");
    if (!boss || !botAI->IsDps(bot)) { return 1.0f; }
    
    if (dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
    }
    if (action->getThreatType() == Action::ActionThreatType::Aoe)
    {
        return 0.0f;
    }
    return 1.0f;
}

float IchoronMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ichoron");
    if (!boss) { return 1.0f; }

    if (dynamic_cast<DpsAssistAction*>(action)
        || dynamic_cast<TankAssistAction*>(action)
        || dynamic_cast<DropTargetAction*>(action))
    {
        return 0.0f;
    }
    return 1.0f;
}

float ZuramatMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zuramat the obliterator");
    if (!boss) { return 1.0f; }

    if (bot->HasAura(SPELL_VOID_SHIFTED))
    {
        if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action))
        {
            return 0.0f;
        }
    }
    
    if (boss->HasAura(SPELL_SHROUD_OF_DARKNESS) && dynamic_cast<AttackAction*>(action))
    {
        return 0.0f;
    }
    return 1.0f;
}
