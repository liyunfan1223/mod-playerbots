/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PaladinPullStrategy.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

std::string PaladinPullStrategy::GetPullActionName() const
{
    Unit* target = GetTarget();
    if (!target ||
        (!botAI->HasStrategy("prot", BOT_STATE_COMBAT) && !botAI->HasStrategy("prot", BOT_STATE_NON_COMBAT)))
    {
        return PullStrategy::GetPullActionName();
    }

    if (botAI->CanCastSpell("avenger's shield", target))
        return "avenger's shield";

    if (botAI->CanCastSpell("hand of reckoning", target))
        return "hand of reckoning";

    return PullStrategy::GetPullActionName();
}

std::string PaladinPullStrategy::GetPreActionName() const
{
    if (botAI->HasStrategy("prot", BOT_STATE_COMBAT) || botAI->HasStrategy("prot", BOT_STATE_NON_COMBAT))
        return "";

    return PullStrategy::GetPreActionName();
}
