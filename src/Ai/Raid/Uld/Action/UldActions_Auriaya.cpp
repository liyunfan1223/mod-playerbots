/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldActions_Auriaya.h"
#include "UldActions_Shared.h"

#include <CombatStrategy.h>
#include <FollowMasterStrategy.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "AiObjectContext.h"
#include "CellImpl.h"
#include "DBCEnums.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "LastMovementValue.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Position.h"
#include "UldEncounter_Auriaya.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "ServerFacade.h"
#include "Unit.h"
#include "Vehicle.h"
#include <TankAssistStrategy.h>

using namespace EncounterHelpers;

namespace
{
// The check behind "nearest npcs", with the entry tested first.
struct AnyUnitOfEntriesInRangeCheck
{
    Acore::AnyUnitInObjectRangeCheck inRange;
    uint32 const* entries;
    size_t count;

    bool operator()(Unit* unit)
    {
        return std::find(entries, entries + count, unit->GetEntry()) != entries + count && inRange(unit);
    }
};
}  // namespace

bool AuriayaFallFromFloorAction::Execute(Event /*event*/)
{
    // Anchor first, it's a known-good spot in the room and the master may be out in the corridor.
    Position anchor;
    float tolerance = 0.0f;
    if (GetAuriayaAnchor(botAI, bot, anchor, tolerance))
    {
        bot->NearTeleportTo(anchor.GetPositionX(), anchor.GetPositionY(), anchor.GetPositionZ(), bot->GetOrientation());
        return true;
    }

    Player* master = botAI->GetMaster();
    if (!master || master->GetMap() != bot->GetMap())
        return false;

    bot->NearTeleportTo(master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(),
                        master->GetOrientation());
    return true;
}

bool AuriayaFallFromFloorAction::isUseful()
{
    AuriayaFallFromFloorTrigger auriayaFallFromFloorTrigger(botAI);
    return auriayaFallFromFloorTrigger.IsActive();
}

bool AuriayaSeepingEssenceAction::isUseful()
{
    AuriayaSeepingEssenceTrigger auriayaSeepingEssenceTrigger(botAI);
    return auriayaSeepingEssenceTrigger.IsActive();
}

bool AuriayaSeepingEssenceAction::Execute(Event /*event*/)
{
    Unit* boss = GetAuriaya(botAI);
    if (!boss)
        return false;

    // Room-wide off the boss, so every pool near a candidate counts however far the bot has drifted.
    std::vector<Unit*> const pools = GetAuriayaRoomPools(botAI);
    if (pools.empty())
        return false;

    // Melee ride the boss and are not anchored, so they are leashed to her instead.
    Position anchor;
    float tolerance = 0.0f;
    if (!GetAuriayaAnchor(botAI, bot, boss, &pools, anchor, tolerance))
        anchor = Position(boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ());

    constexpr int directions = 8;
    constexpr float increment = 3.0f;

    bool found = false;
    bool bestIsCurrent = false;
    float bestX = 0.0f;
    float bestY = 0.0f;
    int bestFouling = std::numeric_limits<int>::max();
    float bestClearance = -1.0f;
    float bestDisplacement = std::numeric_limits<float>::max();

    // Nothing past the leash, so a boxed-in bot takes the least bad point instead of crossing the room.
    auto const consider = [&](float candX, float candY, bool isCurrent)
    {
        if (anchor.GetExactDist2d(candX, candY) > ULDUAR_AURIAYA_ESSENCE_LEASH)
            return;

        int fouling = 0;
        float clearance = std::numeric_limits<float>::max();
        for (Unit* pool : pools)
        {
            float const distance = pool->GetExactDist2d(candX, candY);
            if (distance < ULDUAR_AURIAYA_SEEPING_ESSENCE_RADIUS)
                ++fouling;

            clearance = std::min(clearance, distance);
        }

        // Fewest pools, then furthest from the rest, then shortest walk, so a bot on the best ground stays put.
        float const displacement = bot->GetExactDist2d(candX, candY);
        bool const better = fouling != bestFouling      ? fouling < bestFouling
                            : clearance != bestClearance ? clearance > bestClearance
                                                         : displacement < bestDisplacement;

        if (found && !better)
            return;

        // LOS last. It's a raycast, and it only matters for a spot that would take over as the best.
        if (!bot->IsWithinLOS(candX, candY, bot->GetPositionZ()))
            return;

        found = true;
        bestIsCurrent = isCurrent;
        bestX = candX;
        bestY = candY;
        bestFouling = fouling;
        bestClearance = clearance;
        bestDisplacement = displacement;
    };

    consider(bot->GetPositionX(), bot->GetPositionY(), true);
    consider(anchor.GetPositionX(), anchor.GetPositionY(), false);

    for (int i = 0; i < directions; ++i)
    {
        float const angle = (i * 2.0f * static_cast<float>(M_PI)) / directions;
        for (float distance = increment; distance <= ULDUAR_AURIAYA_ESSENCE_LEASH; distance += increment)
            consider(bot->GetPositionX() + distance * std::cos(angle),
                     bot->GetPositionY() + distance * std::sin(angle), false);
    }

    // Already on the best ground it can reach, so hand the tick back to the rotation.
    if (!found || bestIsCurrent)
        return false;

    return MoveTo(bot->GetMapId(), bestX, bestY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool AuriayaSentryTauntAction::isUseful()
{
    AuriayaSentryTauntTrigger auriayaSentryTauntTrigger(botAI);
    return auriayaSentryTauntTrigger.IsActive();
}

bool AuriayaSentryTauntAction::Execute(Event /*event*/)
{
    return CastClassTaunt(botAI, GetAuriayaLooseSentry(botAI, bot));
}

bool AuriayaRaidPositionAction::isUseful()
{
    AuriayaRaidPositionTrigger auriayaRaidPositionTrigger(botAI);
    return auriayaRaidPositionTrigger.IsActive();
}

bool AuriayaRaidPositionAction::Execute(Event /*event*/)
{
    Position anchor;
    float tolerance = 0.0f;
    if (!GetAuriayaAnchor(botAI, bot, anchor, tolerance))
        return false;

    // Parked. Returning false hands the tick back so the rotation still runs.
    if (bot->GetExactDist2d(&anchor) <= tolerance)
        return false;

    return MoveTo(bot->GetMapId(), anchor.GetPositionX(), anchor.GetPositionY(), anchor.GetPositionZ(), false,
                  false, false, false, MovementPriority::MOVEMENT_COMBAT);
}

bool AuriayaSetDpsPriorityAction::isUseful()
{
    AuriayaSetDpsPriorityTrigger auriayaSetDpsPriorityTrigger(botAI);
    return auriayaSetDpsPriorityTrigger.IsActive();
}

bool AuriayaSetDpsPriorityAction::IsAllowedPriorityTarget(Unit* boss, Unit* candidate)
{
    if (!candidate || !candidate->IsAlive())
        return false;

    if (candidate->GetEntry() == NPC_AURIAYA_FERAL_DEFENDER)
    {
        // Between lives it lies feigned at 1 HP and unselectable, so it is "alive" but unhittable.
        if (IsDownOrFeigning(candidate))
            return false;

        // It re-rolls aggro and roams, so melee only take it near the boss or they chase it all fight.
        if (PlayerbotAI::IsMelee(bot) &&
            candidate->GetExactDist2d(boss) > ULDUAR_AURIAYA_MELEE_DEFENDER_RANGE)
            return false;
    }

    return true;
}

bool AuriayaSetDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* boss = GetAuriaya(botAI);
    if (!boss)
        return false;

    // Sentries first: they stay dead and their Strength of the Pack buffs the boss while they live.
    static uint32 const priorityOrder[] = {NPC_AURIAYA_SANCTUM_SENTRY, NPC_AURIAYA_FERAL_DEFENDER, NPC_AURIAYA};
    constexpr size_t priorityCount = sizeof(priorityOrder) / sizeof(priorityOrder[0]);
    constexpr float targetSwitchDistance = 10.0f;

    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();

    // Nearest live candidate of each entry.
    Unit* perEntry[priorityCount] = {nullptr};

    // "nearest npcs" cut down to these entries before its LOS test instead of after. Same units in the
    // same order, without a raycast for every pet and totem in the raid.
    float const range = sPlayerbotAIConfig.SightDistance;
    std::vector<Unit*> candidates;
    AnyUnitOfEntriesInRangeCheck check{Acore::AnyUnitInObjectRangeCheck(bot, range), priorityOrder, priorityCount};
    Acore::UnitListSearcher<AnyUnitOfEntriesInRangeCheck> searcher(bot, candidates, check);
    Cell::VisitObjects(bot, searcher, range);

    for (Unit* unit : candidates)
    {
        if (!IsAllowedPriorityTarget(boss, unit) || !bot->IsWithinLOSInMap(unit))
            continue;

        for (size_t index = 0; index < priorityCount; ++index)
        {
            if (unit->GetEntry() != priorityOrder[index])
                continue;

            Unit*& selected = perEntry[index];
            if (!selected || unit->GetExactDist2d(bot) < selected->GetExactDist2d(bot))
                selected = unit;

            break;
        }
    }

    Unit* target = nullptr;
    size_t desiredPriority = priorityCount;
    for (size_t index = 0; index < priorityCount; ++index)
    {
        if (perEntry[index])
        {
            target = perEntry[index];
            desiredPriority = index;
            break;
        }
    }

    size_t currentPriority = priorityCount;
    if (currentTarget && IsAllowedPriorityTarget(boss, currentTarget))
    {
        for (size_t index = 0; index < priorityCount; ++index)
        {
            if (currentTarget->GetEntry() == priorityOrder[index])
            {
                currentPriority = index;
                break;
            }
        }
    }

    if (currentPriority < priorityCount)
    {
        // Never drop to a lower priority, and within one entry switch only for something 10 yd
        // closer, or two sentries ping-pong the raid.
        if (currentPriority < desiredPriority)
            target = currentTarget;
        else if (currentPriority == desiredPriority && target &&
                 target->GetExactDist2d(bot) + targetSwitchDistance >= currentTarget->GetExactDist2d(bot))
            target = currentTarget;
    }

    if (!target)
        target = AI_VALUE(Unit*, "dps target");

    if (!target)
        return false;

    bool needsAttack = currentTarget != target;
    if (PlayerbotAI::IsMelee(bot))
        needsAttack = needsAttack || !bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING);

    // Already on the right target, so yield the tick to the lower nodes.
    if (!needsAttack)
        return false;

    return Attack(target);
}
