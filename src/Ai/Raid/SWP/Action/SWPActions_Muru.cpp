/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SWPEncounter_Muru.h"
#include "SWPShared.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <utility>
#include <vector>

using namespace SwpHelpers;
using namespace EncounterHelpers;

bool MuruMisdirectEnemyToTankAction::Execute(Event /*event*/)
{
    Unit* enemy = nullptr;
    Player* tank = nullptr;

    if (Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
        voidSentinel && voidSentinel->GetHealthPct() > MURU_MISDIRECT_MIN_TARGET_HP_PERCENT)
    {
        enemy = voidSentinel;
        tank = GetGroupAssistTank(bot, 0);
    }
    else if (Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
        entropius && entropius->GetHealthPct() > MURU_MISDIRECT_MIN_TARGET_HP_PERCENT)
    {
        enemy = entropius;
        tank = GetGroupMainTank(bot);
    }

    if (!enemy || !tank || !tank->IsAlive())
        return false;

    return MisdirectTargetToTank(botAI, enemy, tank);
}

bool MuruMainTankPickUpEntropiusAction::Execute(Event /*event*/)
{
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!entropius)
        return false;

    return AI_VALUE(Unit*, "current target") != entropius && Attack(entropius);
}

bool MuruPositionRangedByPhaseAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (IsMuruPhaseActive(muru))
    {
        _entropiusRangedPositionReached = false;

        Position const& position = MURU_STACK_POSITION;
        constexpr float rangedGroupRadius = 3.5f;
        float moveX;
        float moveY;
        bool backwards;
        if (!GetStepToPosition(bot, position, rangedGroupRadius, nullptr, moveX, moveY, backwards))
            return false;

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (TryGetMuruDarknessActiveState(bot, muru))
        return false;

    MuruEncounterTargets targets;
    GatherMuruEncounterTargets(botAI, targets);

    bool const hasActiveAdds =
        !targets.voidSentinels.empty() || !targets.furyMages.empty() || !targets.berserkers.empty();

    if (!hasActiveAdds && !_entropiusRangedPositionReached)
    {
        Position position;
        if (!TryGetEntropiusInitialRangedPosition(position))
            return false;

        constexpr float arrivalDistance = 2.0f;
        if (bot->GetExactDist2d(position) <= arrivalDistance)
        {
            _entropiusRangedPositionReached = true;
            return false;
        }

        float moveX;
        float moveY;
        bool backwards;
        if (!GetStepToPosition(bot, position, arrivalDistance, nullptr, moveX, moveY, backwards))
            return false;

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    constexpr float safeDistFromPlayer = 4.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);
}

bool MuruPositionRangedByPhaseAction::TryGetEntropiusInitialRangedPosition(
    Position& position) const
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SWP_MAP_ID)
            continue;

        if (GET_PLAYERBOT_AI(member) && PlayerbotAI::IsRanged(member))
            rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    auto const it = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    if (it == rangedMembers.end())
        return false;

    size_t const slotIndex = std::distance(rangedMembers.begin(), it);

    constexpr float spreadRadius = 25.0f;
    float const anchorAngle = std::atan2(
        MURU_STACK_POSITION.GetPositionY() - MURU_CENTER_POSITION.GetPositionY(),
        MURU_STACK_POSITION.GetPositionX() - MURU_CENTER_POSITION.GetPositionX());
    float const angleStep =
        2.0f * static_cast<float>(M_PI) / static_cast<float>(rangedMembers.size());
    float const angle = Position::NormalizeOrientation(anchorAngle + angleStep * slotIndex);

    float const destinationX = MURU_CENTER_POSITION.GetPositionX() + std::cos(angle) * spreadRadius;
    float const destinationY = MURU_CENTER_POSITION.GetPositionY() + std::sin(angle) * spreadRadius;

    position = Position{ destinationX, destinationY, MURU_CENTER_POSITION.GetPositionZ() };
    return true;
}

bool MuruAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    Unit* target = ResolveMuruDpsTarget(currentTarget);
    if (!target)
        return false;

    if (target->GetEntry() == Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER))
    {
        if (bot->getClass() == CLASS_ROGUE &&
            botAI->CanCastSpell("dismantle", target) && botAI->CastSpell("dismantle", target))
        {
            return true;
        }

        if (bot->getClass() == CLASS_WARRIOR &&
            botAI->CanCastSpell("disarm", target) && botAI->CastSpell("disarm", target))
        {
            return true;
        }
    }

    return currentTarget != target && Attack(target);
}

Unit* MuruAssignDpsPriorityAction::ResolveMuruDpsTarget(Unit* currentTarget)
{
    bool const isShadowPriest =
        bot->getClass() == CLASS_PRIEST && botAI->HasStrategy("shadow", BOT_STATE_COMBAT);
    bool const isOtherRanged = PlayerbotAI::IsRanged(bot) && !isShadowPriest;

    MuruEncounterTargets targets;
    GatherMuruEncounterTargets(botAI, targets);
    Unit* muru = targets.muru;
    Unit* entropius = targets.entropius;

    if (!muru && !entropius)
        return nullptr;

    if (muru && currentTarget == muru)
        context->GetValue<bool>("neglect threat")->Set(true);

    bool const isMuruPhase = IsMuruPhaseActive(muru);
    bool const darknessActive = isMuruPhase && TryGetMuruDarknessActiveState(bot, muru);

    bool const isMelee = PlayerbotAI::IsMelee(bot);
    Unit* addStickyTarget = currentTarget;
    if (isMelee && !targets.voidSentinels.empty())
    {
        auto const isUnsafe = [&targets](Unit* add)
        { return IsMuruAddInVoidSentinelPulse(add, targets.voidSentinels); };

        targets.furyMages.erase(
            std::remove_if(targets.furyMages.begin(), targets.furyMages.end(), isUnsafe),
            targets.furyMages.end());
        targets.berserkers.erase(
            std::remove_if(targets.berserkers.begin(), targets.berserkers.end(), isUnsafe),
            targets.berserkers.end());

        if (isUnsafe(addStickyTarget))
            addStickyTarget = nullptr;
    }

    Position const& origin = MURU_STACK_POSITION;
    Unit* voidSentinel = SelectNearestMuruTargetByEntry(
        currentTarget, Id(SwpNpcs::NPC_VOID_SENTINEL), targets.voidSentinels, origin);
    Unit* voidSpawn = SelectNearestMuruTargetByEntry(
        currentTarget, Id(SwpNpcs::NPC_VOID_SPAWN), targets.voidSpawns, origin);
    Unit* furyMage = SelectNearestMuruTargetByEntry(
        addStickyTarget, Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), targets.furyMages, origin);
    Unit* berserker = SelectNearestMuruTargetByEntry(
        addStickyTarget, Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), targets.berserkers, origin);

    Player* voidSentinelVictim = nullptr;
    if (voidSentinel)
    {
        if (Unit* victim = voidSentinel->GetVictim())
            voidSentinelVictim = victim->ToPlayer();
    }

    // Void Sentinel: Attack only if a tank has aggro or if it is below 10% health.
    bool const isVoidSentinelAllowed =
        ((voidSentinel && voidSentinel->GetHealthPct() < 10.0f) ||
         (voidSentinelVictim && PlayerbotAI::IsTank(voidSentinelVictim)));

    auto const isAllowedPriorityTarget = [&](Unit* unit) -> bool
    {
        if (!unit || !unit->IsAlive())
            return false;

        switch (unit->GetEntry())
        {
            // Melee will stay on M'uru as long as Darkness is not acive.
            // Shadow Priests stay on M'uru through all of phase 1.
            case Id(SwpNpcs::NPC_MURU):
            {
                if (!isMuruPhase)
                    return false;
                return isOtherRanged || isShadowPriest || !darknessActive;
            }
            case Id(SwpNpcs::NPC_ENTROPIUS):
                return true;
            case Id(SwpNpcs::NPC_VOID_SENTINEL):
            {
                if (isShadowPriest)
                    return !isMuruPhase && isVoidSentinelAllowed;
                if (isOtherRanged)
                    return isVoidSentinelAllowed;
                return false;
            }
            case Id(SwpNpcs::NPC_VOID_SPAWN):
                return isOtherRanged;
            case Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE):
            case Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER):
            {
                if (isMelee && IsMuruAddInVoidSentinelPulse(unit, targets.voidSentinels))
                    return false;
                if (isShadowPriest)
                    return !isMuruPhase;
                if (isOtherRanged)
                    return true;
                return darknessActive || !isMuruPhase;
            }
            default:
                return false;
        }
    };

    std::vector<std::pair<uint32, Unit*>> priorityTargets;
    if (isShadowPriest)
    {
        priorityTargets =
        {
            { Id(SwpNpcs::NPC_MURU), muru },
            { Id(SwpNpcs::NPC_VOID_SENTINEL), voidSentinel },
            { Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), furyMage },
            { Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), berserker },
            { Id(SwpNpcs::NPC_ENTROPIUS), entropius },
        };
    }
    else if (isOtherRanged)
    {
        priorityTargets =
        {
            { Id(SwpNpcs::NPC_VOID_SENTINEL), voidSentinel },
            { Id(SwpNpcs::NPC_VOID_SPAWN), voidSpawn },
            { Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), furyMage },
            { Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), berserker },
            { Id(SwpNpcs::NPC_MURU), muru },
            { Id(SwpNpcs::NPC_ENTROPIUS), entropius },
        };
    }
    else
    {
        priorityTargets =
        {
            { Id(SwpNpcs::NPC_MURU), muru },
            { Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), furyMage },
            { Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), berserker },
            { Id(SwpNpcs::NPC_VOID_SPAWN), voidSpawn },
            { Id(SwpNpcs::NPC_ENTROPIUS), entropius },
        };
    }

    Unit* target = nullptr;
    for (auto const& candidate : priorityTargets)
    {
        if (isAllowedPriorityTarget(candidate.second))
        {
            target = candidate.second;
            break;
        }
    }

    Unit* stickyTarget = currentTarget;

    auto const getPriorityIndex = [&](Unit* unit) -> size_t
    {
        if (!isAllowedPriorityTarget(unit))
            return priorityTargets.size();

        for (size_t index = 0; index < priorityTargets.size(); ++index)
        {
            if (priorityTargets[index].first == unit->GetEntry())
                return index;
        }

        return priorityTargets.size();
    };

    if (stickyTarget)
    {
        size_t const currentPriority = getPriorityIndex(stickyTarget);
        size_t const desiredPriority = getPriorityIndex(target);
        if (currentPriority < priorityTargets.size() && currentPriority <= desiredPriority)
            target = stickyTarget;
    }

    if (!target)
        target = isMelee ? (isMuruPhase ? muru : nullptr) : AI_VALUE(Unit*, "dps target");

    return target;
}

bool MuruKillDarkFiendsWithDispelAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!muru && !entropius)
        return false;

    bool const isMuruPhase = IsMuruPhaseActive(muru);

    Creature* darkFiendNearMuru = nullptr;
    constexpr float massDispelRange = 15.0f;
    std::vector<Creature*> const darkFiends = GetMuruDarkFiends(botAI);

    if (isMuruPhase)
    {
        for (Creature* creature : darkFiends)
        {
            if (creature && creature->IsAlive() &&
                creature->GetExactDist2d(muru) < massDispelRange)
            {
                darkFiendNearMuru = creature;
                break;
            }
        }
    }

    if (bot->getClass() == CLASS_PRIEST)
    {
        if (isMuruPhase)
        {
            if (darkFiendNearMuru && botAI->CanCastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), muru))
                return botAI->CastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), muru);

            for (Creature* creature : darkFiends)
            {
                if (creature && botAI->CanCastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), creature) &&
                    botAI->CastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), creature))
                {
                    return true;
                }
            }
        }

        for (Creature* creature : darkFiends)
        {
            if (creature && botAI->CanCastSpell(Id(SwpSpells::SPELL_DISPEL_MAGIC_RANK_1), creature))
                return botAI->CastSpell(Id(SwpSpells::SPELL_DISPEL_MAGIC_RANK_1), creature);
        }
    }
    else
    {
        for (Creature* creature : darkFiends)
        {
            if (creature && botAI->CanCastSpell(Id(SwpSpells::SPELL_PURGE_RANK_1), creature))
                return botAI->CastSpell(Id(SwpSpells::SPELL_PURGE_RANK_1), creature);
        }
    }

    return false;
}

bool MuruTanksMoveSentinelToSafePositionAction::Execute(Event /*event*/)
{
    Unit* voidSentinel = AI_VALUE2(Unit*, "find target", "void sentinel");
    if (!voidSentinel)
    {
        constexpr float arrivalDistance = 3.0f;
        float moveX;
        float moveY;
        bool backwards;
        if (!GetStepToPosition(
                bot, MURU_STACK_POSITION, arrivalDistance, nullptr, moveX, moveY, backwards))
        {
            return false;
        }

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE(Unit*, "current target") != voidSentinel)
    {
        return Attack(voidSentinel);
    }

    if (voidSentinel->GetVictim() != bot || !bot->IsWithinMeleeRange(voidSentinel))
        return false;

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, GetAssignedVoidSentinelTankPosition(voidSentinel), arrivalDist, voidSentinel,
            moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool MuruSecondAssistTankGuardRangedAction::Execute(Event /*event*/)
{
    constexpr float arrivalDist = 1.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, MURU_ENTRANCE_POSITION, arrivalDist, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool MuruMeleeFleeTheDarknessAction::Execute(Event /*event*/)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return false;

    Position const& entrancePosition = MURU_ENTRANCE_POSITION;
    Position const& stackPosition = MURU_STACK_POSITION;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget && muru->GetExactDist2d(currentTarget) > MURU_DARKNESS_SAFE_DISTANCE)
    {
        Position const& refPosition = PlayerbotAI::IsAssistTankOfIndex(bot, 1, true) ?
            entrancePosition : stackPosition;
        if (currentTarget->GetExactDist2d(refPosition) <= MURU_HOLDING_POSITION_RADIUS)
            return false;
    }

    if (PlayerbotAI::IsTank(bot))
    {
        if (IsTankingMuruVoidSentinel(botAI))
            return false;

        if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
            TryGetMuruDarknessEarlyState(bot, muru))
        {
            Position const& holdingPosition = PlayerbotAI::IsAssistTankOfIndex(bot, 1, true) ?
                entrancePosition : stackPosition;
            constexpr float arrivalDistance = 2.5f;
            float moveX;
            float moveY;
            bool backwards;
            if (!GetStepToPosition(
                    bot, holdingPosition, arrivalDistance, nullptr, moveX, moveY, backwards))
            {
                return false;
            }

            return MoveTo(
                SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false);
        }

        constexpr uint32 minInterval = 0;
        if (bot->GetExactDist2d(muru) > MURU_DARKNESS_SAFE_DISTANCE)
            return false;

        return FleePosition(muru->GetPosition(), MURU_DARKNESS_SAFE_DISTANCE, minInterval);
    }

    constexpr float stackArrivalDistance = 4.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, stackPosition, stackArrivalDistance, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool MuruCastStunOnBerserkerAction::Execute(Event /*event*/)
{
    Unit* berserker = FindMuruBerserkerToStun(botAI);
    if (!berserker)
        return false;

    auto const castStun = [&](char const* spell)
    {
        return botAI->CanCastSpell(spell, berserker) && botAI->CastSpell(spell, berserker);
    };

    switch (bot->getClass())
    {
        case CLASS_DRUID:
            return castStun("bash") || castStun("maim");
        case CLASS_PALADIN:
            return castStun("hammer of justice");
        case CLASS_ROGUE:
            return castStun("kidney shot");
        case CLASS_WARLOCK:
            return castStun("shadowfury");
        case CLASS_WARRIOR:
            return castStun("concussion blow") || castStun("shockwave");
        default:
            return bot->getRace() == RACE_TAUREN && castStun("war stomp");
    }
}

bool MuruInterruptFelFireballAction::Execute(Event /*event*/)
{
    Unit* furyMage = FindMuruFuryMageToInterrupt(botAI);
    if (!furyMage)
        return false;

    auto const castInterrupt = [&](char const* spell)
    {
        return botAI->CanCastSpell(spell, furyMage) && botAI->CastSpell(spell, furyMage);
    };

    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            return castInterrupt("mind freeze") || castInterrupt("strangulate");
        case CLASS_HUNTER:
            return castInterrupt("silencing shot");
        case CLASS_MAGE:
            return castInterrupt("counterspell");
        case CLASS_PALADIN:
            return castInterrupt("avenger's shield");
        case CLASS_PRIEST:
            return castInterrupt("silence");
        case CLASS_ROGUE:
            return castInterrupt("kick");
        case CLASS_SHAMAN:
            return castInterrupt("wind shear");
        case CLASS_WARLOCK:
            return castInterrupt("spell lock");
        case CLASS_WARRIOR:
            return castInterrupt("pummel") || castInterrupt("shield bash");
        default:
            return false;
    }
}

bool MuruCastSpellStealOnSpellFuryAction::Execute(Event /*event*/)
{
    Unit* furyMage = FindMuruFuryMageToSpellsteal(botAI);
    return furyMage &&
        botAI->CanCastSpell(Id(SwpSpells::SPELL_SPELLSTEAL), furyMage) &&
        botAI->CastSpell(Id(SwpSpells::SPELL_SPELLSTEAL), furyMage);
}

bool MuruWarlockEnslaveVoidSpawnAction::Execute(Event /*event*/)
{
    Creature* voidSpawn = FindAvailableVoidSpawnForEnslave(botAI);
    return voidSpawn &&
        botAI->CanCastSpell("enslave demon", voidSpawn) &&
        botAI->CastSpell("enslave demon", voidSpawn);
}

bool MuruVoidSpawnCastShadowBoltVolleyAction::Execute(Event /*event*/)
{
    Unit* voidSpawn = GetControlledVoidSpawn();
    if (!voidSpawn)
        return false;

    Unit* target = GetVoidSpawnVolleyPriorityTarget(voidSpawn);
    if (!target)
        return false;

    bool const commandedAttack = CommandControlledCreatureToAttack(voidSpawn, target);

    if (voidSpawn->GetExactDist2d(target) >
        MURU_SHADOW_BOLT_VOLLEY_RADIUS + target->GetCombatReach())
    {
        return commandedAttack;
    }

    constexpr uint32 volleySpellId = Id(SwpSpells::SPELL_SHADOW_BOLT_VOLLEY);
    if (voidSpawn->HasSpellCooldown(volleySpellId))
        return commandedAttack;

    voidSpawn->CastSpell(target, volleySpellId, true);
    voidSpawn->AddSpellCooldown(volleySpellId, 0, GetManualCastCooldown(volleySpellId));
    return true;
}

Unit* MuruEnslavedVoidSpawnAttackAction::GetControlledVoidSpawn() const
{
    Unit* voidSpawn = bot->GetCharm();
    if (!voidSpawn || !voidSpawn->IsAlive() ||
        voidSpawn->GetEntry() != Id(SwpNpcs::NPC_VOID_SPAWN))
    {
        return nullptr;
    }

    return voidSpawn;
}

Unit* MuruEnslavedVoidSpawnAttackAction::GetVoidSpawnVolleyPriorityTarget(Unit* voidSpawn) const
{
    MuruEncounterTargets targets;
    GatherMuruEncounterTargets(botAI, targets);

    Position const& origin = voidSpawn->GetPosition();
    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    Unit* furyMage = SelectNearestMuruTargetByEntry(
        currentTarget, Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE), targets.furyMages, origin);
    Unit* berserker = SelectNearestMuruTargetByEntry(
        currentTarget, Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER), targets.berserkers, origin);
    Unit* voidSentinel = SelectNearestMuruTargetByEntry(
        currentTarget, Id(SwpNpcs::NPC_VOID_SENTINEL), targets.voidSentinels, origin);

    Unit* validMuru = targets.muru;
    if (!IsMuruPhaseActive(validMuru) || TryGetMuruDarknessActiveState(bot, validMuru))
        validMuru = nullptr;

    std::array<Unit*, 5> priorities = {
        furyMage, berserker, voidSentinel, validMuru, targets.entropius };

    for (Unit* target : priorities)
    {
        if (target)
            return target;
    }

    return nullptr;
}

bool MuruKeepDistanceFromDarkFiendsAction::Execute(Event /*event*/)
{
    if (Creature* voidZone = FindMuruVoidZoneToAvoid(botAI))
    {
        float const distFromVoidZone = bot->GetExactDist2d(voidZone);
        bot->CastStop();
        return MoveAway(voidZone, VOID_ZONE_SAFE_DISTANCE - distFromVoidZone);
    }

    Creature* darkFiend = GetNearestMuruDarkFiend(botAI, DARK_FIEND_AVOID_SEARCH_RADIUS);
    if (!darkFiend)
        return false;

    float const distFromFiend = bot->GetExactDist2d(darkFiend);
    if (distFromFiend > DARK_FIEND_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return MoveAway(darkFiend, DARK_FIEND_SAFE_DISTANCE - distFromFiend);
}

bool MuruEscapeTheSingularityAction::Execute(Event /*event*/)
{
    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (!entropius)
        return false;

    Creature* singularity = botAI->GetCreature(AI_VALUE(ObjectGuid, "muru singularity"));
    if (!singularity || !singularity->IsAlive())
        return false;

    float const safeDistance = entropius->GetVictim() == bot ?
        SINGULARITY_TANK_SAFE_DISTANCE : SINGULARITY_SAFE_DISTANCE;
    float const currentDistance = bot->GetExactDist2d(singularity);
    if (currentDistance >= safeDistance)
        return false;

    return FleePosition(singularity->GetPosition(), safeDistance);
}
