#include "RaidEoEMultipliers.h"

#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "RaidEoEActions.h"
#include "RaidEoETriggers.h"
#include "ReachTargetActions.h"
#include "ScriptedCreature.h"
#include "WarriorActions.h"

float MalygosMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "malygos");

    uint8 phase = MalygosTrigger::getPhase(bot, boss);
    if (phase == 0) { return 1.0f; }

    if (phase == 1)
    {
        if (dynamic_cast<FollowAction*>(action))
        {
            return 0.0f;
        }

        if (botAI->IsDps(bot) && dynamic_cast<DpsAssistAction*>(action))
        {
            return 0.0f;
        }

        if (botAI->IsRangedDps(bot) && dynamic_cast<DropTargetAction*>(action))
        {
            return 0.0f;
        }

        if (!botAI->IsMainTank(bot) && dynamic_cast<TankAssistAction*>(action))
        {
            return 0.0f;
        }

        // if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<MalygosPositionAction*>(action))
        // {
        //     return 0.0f;
        // }
    }
    else if (phase == 2)
    {
        if (botAI->IsDps(bot) && dynamic_cast<DpsAssistAction*>(action))
        {
            return 0.0f;
        }

        if (dynamic_cast<FleeAction*>(action))
        {
            return 0.0f;
        }

        if (dynamic_cast<TankAssistAction*>(action))
        {
            Unit* target = action->GetTarget();
            if (target && target->GetEntry() == NPC_SCION_OF_ETERNITY)
            return 0.0f;
        }
    }
    else if (phase == 3)
    {
        // Suppresses FollowAction as well as some attack-based movements
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<EoEFlyDrakeAction*>(action))
        {
            return 0.0f;
        }
    }
    
    return 1.0f;
}
