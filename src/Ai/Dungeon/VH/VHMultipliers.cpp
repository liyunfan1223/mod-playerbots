/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "VHMultipliers.h"
#include "ChooseTargetActions.h"
#include "Playerbots.h"
#include "VHActions.h"
#include "VHTriggers.h"

float ErekemMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "erekem");
    if (!boss || !PlayerbotAI::IsDps(bot)) { return 1.0f; }

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
