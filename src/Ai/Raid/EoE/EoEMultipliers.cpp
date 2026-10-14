/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EoEMultipliers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "EoEActions.h"
#include "EoETriggers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
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

        if (PlayerbotAI::IsDps(bot) && dynamic_cast<DpsAssistAction*>(action))
        {
            return 0.0f;
        }

        if (PlayerbotAI::IsRangedDps(bot) && dynamic_cast<DropTargetAction*>(action))
        {
            return 0.0f;
        }

        if (!PlayerbotAI::IsMainTank(bot) && dynamic_cast<TankAssistAction*>(action))
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
        if (PlayerbotAI::IsDps(bot) && dynamic_cast<DpsAssistAction*>(action))
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
