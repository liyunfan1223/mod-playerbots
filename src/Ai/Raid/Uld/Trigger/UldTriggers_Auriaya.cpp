/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_Auriaya.h"

#include "GameObject.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldEncounter_Auriaya.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include "Vehicle.h"
#include <MovementActions.h>
#include <FollowMasterStrategy.h>

#include <cmath>

// Every boss lookup here sweeps every unit in sight, so the cheap tests in each trigger go first.

bool AuriayaFallFromFloorTrigger::IsActive()
{
    return bot->GetPositionZ() < ULDUAR_AURIAYA_AXIS_Z_PATHING_ISSUE_DETECT && AuriayaEncounterActive(botAI);
}

bool AuriayaSeepingEssenceTrigger::IsActive()
{
    // 7 yd is a cell or two, so the pool search goes ahead of the boss.
    return !CollectAuriayaEssencePools(bot, ULDUAR_AURIAYA_SEEPING_ESSENCE_RADIUS).empty() &&
           AuriayaEncounterActive(botAI);
}

bool AuriayaRaidPositionTrigger::IsActive()
{
    // Only the main tank and the ranged half get an anchor.
    if (!PlayerbotAI::IsMainTank(bot) && !PlayerbotAI::IsRanged(bot))
        return false;

    // Combat-gated, or the raid walks to its spots before anyone pulls.
    Unit* boss = GetAuriaya(botAI);
    if (!boss || !boss->IsInCombat())
        return false;

    // Standing in a pool beats standing on a spot. No stand-down for Sonic Screech, the cast is when
    // the stack most needs its anchors. CanFreeMove already covers fear and stuns.
    if (!CollectAuriayaEssencePools(bot, ULDUAR_AURIAYA_SEEPING_ESSENCE_RADIUS).empty())
        return false;

    // Off the boss, not the bot: the bot is off its anchor here, so the pool fouling it can be out of reach.
    std::vector<Unit*> const roomPools = GetAuriayaRoomPools(botAI);

    Position anchor;
    float tolerance = 0.0f;
    if (!GetAuriayaAnchor(botAI, bot, boss, &roomPools, anchor, tolerance))
        return false;

    // Stand down on a fouled anchor, or this and the dodge take turns walking the bot in and out of the pool.
    for (Unit* pool : roomPools)
        if (pool->GetExactDist2d(&anchor) < ULDUAR_AURIAYA_SEEPING_ESSENCE_RADIUS)
            return false;

    return bot->GetExactDist2d(&anchor) > tolerance;
}

bool AuriayaSetDpsPriorityTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    // The action calls Attack() directly, so without this the first bot to see her pulls the room.
    // Whoever pulls flips it for everyone, so the raid engages together.
    return IsAuriayaEngaged(botAI);
}

bool AuriayaSentryTauntTrigger::IsActive()
{
    // IsAssistTankOfIndex repeats this, but it walks the group, so it stays last and this goes first.
    if (!PlayerbotAI::IsTank(bot))
        return false;

    // A taunt is a pull, so the off-tank waits for the encounter to be live like everyone else.
    if (!IsAuriayaEngaged(botAI))
        return false;

    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return false;

    // A sentry already on the off-tank stays in melee, where its Savage Pounce (8-25 yd) cannot fire
    return GetAuriayaLooseSentry(botAI, bot) != nullptr;
}
