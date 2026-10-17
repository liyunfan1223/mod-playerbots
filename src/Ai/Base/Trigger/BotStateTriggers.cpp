/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BotStateTriggers.h"

#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

bool CombatStartTrigger::IsActive()
{
    // "attackers" covers the bot's own attackers and those of nearby group members, so it
    // also catches the master pulling while the bot itself is still out of combat.
    // Edge-triggered: fire once when attackers first appear, not on every tick they persist,
    // or the reaction would keep clearing the main AI delay for a bot that is idling on purpose.
    bool const hasAttackers = !AI_VALUE(GuidVector, "attackers").empty();
    bool const started = hasAttackers && !hadAttackers;
    hadAttackers = hasAttackers;

    if (botAI->GetState() == BOT_STATE_COMBAT || botAI->GetState() == BOT_STATE_DEAD)
        return false;

    return started;
}
