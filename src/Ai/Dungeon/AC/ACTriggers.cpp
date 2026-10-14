/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ACTriggers.h"
#include "Playerbots.h"

// Shirrak the Dead Watcher

bool ShirrakTankPositionBossTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) &&
        AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
}

bool ShirrakFleeFocusFireTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "shirrak the dead watcher"))
        return false;

    return bot->FindNearestCreature(NPC_FOCUS_FIRE, FLARE_SEARCH_RADIUS);
}

bool ShirrakRangedKeepDistanceTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) &&
        AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
}
