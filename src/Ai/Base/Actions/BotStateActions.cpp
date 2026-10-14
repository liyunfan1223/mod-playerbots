/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BotStateActions.h"

#include "PlayerbotAI.h"
#include "Playerbots.h"

bool WakeOnCombatStartAction::Execute(Event /*event*/)
{
    // Do not switch engines here. The combat engine entered without a current target fires
    // "invalid target" -> "drop target" on its first tick and drops back to non-combat.
    // Clearing the main AI delay lets the non-combat engine run now; its "dps assist" /
    // "tank assist" / "attack" actions select a target and switch engines the normal way.
    botAI->ResetActionDuration();
    return true;
}

bool WakeOnCombatStartAction::isUseful()
{
    return botAI->GetState() != BOT_STATE_COMBAT && botAI->IsActionDurationActive();
}
