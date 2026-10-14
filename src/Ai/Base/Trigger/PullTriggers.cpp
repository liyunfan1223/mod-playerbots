/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

/*
 * Ported from the CMaNGOS playerbots project (https://github.com/cmangos/playerbots), GPL v2,
 * with modifications for AzerothCore.
 * Original authors:
 *   David Parra Ausina (davidonete/Flekz) <davidparraausina@gmail.com> - original author
 *   Sebastiaan Keek (mostlikely4r) <sebastiaan.keek@gmail.com>
 *   Cyberium <cyberium@users.noreply.github.com>
 */

#include "PullTriggers.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "PullStrategy.h"

bool PullStartTrigger::IsActive()
{
    PullStrategy const* strategy = PullStrategy::Get(botAI);
    return strategy && strategy->IsPullPendingToStart();
}

bool PullEndTrigger::IsActive()
{
    PullStrategy const* strategy = PullStrategy::Get(botAI);

    if (!strategy || !strategy->HasPullStarted())
        return false;

    Unit* target = strategy->GetTarget();
    if (!target || !target->IsInWorld() || !target->IsAlive())
        return true;

    time_t const secondsSincePullStarted = time(nullptr) - strategy->GetPullStartTime();
    if (secondsSincePullStarted >= PullStrategy::GetMaxPullTime())
        return true;

    float distanceToPullTarget = bot->GetDistance(target);
    if (distanceToPullTarget > ATTACK_DISTANCE && !target->IsNonMeleeSpellCast(false, false, true) &&
        (!PlayerbotAI::IsRanged(bot) || distanceToPullTarget > botAI->GetRange("spell")))
        return false;

    if (!botAI->HasStrategy("pull back", BOT_STATE_COMBAT))
        return true;

    PositionInfo pullPosition = AI_VALUE(PositionMap&, "position")["pull"];
    if (!pullPosition.isSet() || pullPosition.mapId != bot->GetMapId())
        return true;

    return bot->GetDistance(pullPosition.x, pullPosition.y, pullPosition.z) <= botAI->GetRange("follow");
}

bool ReturnToPullPositionTrigger::IsActive()
{
    PullStrategy const* strategy = PullStrategy::Get(botAI);

    Unit* target = strategy ? strategy->GetTarget() : nullptr;
    if (!strategy || !strategy->HasPullStarted() || !target || !target->IsInCombat() ||
        !botAI->HasStrategy("pull back", BOT_STATE_COMBAT))
        return false;

    PositionInfo pullPosition = AI_VALUE(PositionMap&, "position")["pull"];
    return pullPosition.isSet() && pullPosition.mapId == bot->GetMapId() &&
           bot->GetDistance(pullPosition.x, pullPosition.y, pullPosition.z) > sPlayerbotAIConfig.FollowDistance;
}
