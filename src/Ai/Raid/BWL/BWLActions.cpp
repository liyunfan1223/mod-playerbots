/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BWLActions.h"
#include "BWLHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

using namespace BlackwingLairHelpers;
using namespace EncounterHelpers;

static constexpr float INCREMENTAL_MOVE_STEP_DISTANCE = 3.0f;

static constexpr float RAZORGORE_CONE_RADIUS = 15.0f;
static constexpr float RAZORGORE_CONE_ARC = M_PI;
static constexpr float RAZORGORE_BACK_FUZZ_ARC = M_PI_4;
static constexpr float RAZORGORE_RANGE_DISTANCE = 15.0f;
static constexpr float RAZORGORE_MELEE_DISTANCE = 3.0f;

static constexpr float VAELASTRASZ_REPULSION_RANGE = 20.0f;
static constexpr float VAELASTRASZ_TAIL_SWEEP_RECOVERY_RANGE = 30.0f;
static constexpr float VAELASTRASZ_RANGED_BIAS_THRESHOLD = 25.0f;
static constexpr float VAELASTRASZ_RANGED_BIAS_FLEE_WEIGHT = 0.7f;
static constexpr float VAELASTRASZ_RANGED_BIAS_BOSS_WEIGHT = 1.0f - VAELASTRASZ_RANGED_BIAS_FLEE_WEIGHT;

// General

bool BwlOnyxiaScaleCloakAuraCheckAction::Execute(Event /*event*/)
{
    bot->AddAura(static_cast<uint32>(BlackwingLairSpells::SPELL_ONYXIA_SCALE_CLOAK), bot);
    return true;
}

bool BwlOnyxiaScaleCloakAuraCheckAction::isUseful()
{
    return !bot->HasAura(static_cast<uint32>(BlackwingLairSpells::SPELL_ONYXIA_SCALE_CLOAK));
}

bool BwlTurnOffSuppressionDeviceAction::Execute(Event /*event*/)
{
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    for (auto i = gos.begin(); i != gos.end(); ++i)
    {
        GameObject* go = botAI->GetGameObject(*i);
        if (IsActiveSuppressionDeviceInRange(go, bot))
        {
            go->SetGoState(GO_STATE_ACTIVE);
        }
    }
    return true;
}

// Razorgore the Untamed

bool BwlRazorgoreAvoidAoeAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "razorgore the untamed");
    if (!boss)
        return false;

    // The current target should not move.
    // Also prevents non-tanks from rotating the boss if they have aggro.
    if (boss->GetVictim() == bot)
        return false;

    float distance = bot->GetDistance2d(boss);

    // Bot is too close and standing in Razorgore's frontal cone
    if (distance <= RAZORGORE_CONE_RADIUS && boss->HasInArc(RAZORGORE_CONE_ARC, bot))
    {
        // Target position: directly behind the boss with a small random fuzz
        // to prevent all bots stacking on the same spot
        float moveAngle = boss->GetOrientation() + M_PI + frand(-RAZORGORE_BACK_FUZZ_ARC, RAZORGORE_BACK_FUZZ_ARC);
        float razorgoreRadius = PlayerbotAI::IsRanged(bot) ? RAZORGORE_RANGE_DISTANCE : RAZORGORE_MELEE_DISTANCE;

        float targetMoveX = boss->GetPositionX() + razorgoreRadius * cos(moveAngle);
        float targetMoveY = boss->GetPositionY() + razorgoreRadius * sin(moveAngle);

        // Move incrementally toward the target. Allows course correction.
        float dX = targetMoveX - bot->GetPositionX();
        float dY = targetMoveY - bot->GetPositionY();
        float dist = sqrt(dX * dX + dY * dY);
        if (dist == 0.0f)
            dist = 0.1f;

        float moveX = bot->GetPositionX() + (dX / dist) * INCREMENTAL_MOVE_STEP_DISTANCE;
        float moveY = bot->GetPositionY() + (dY / dist) * INCREMENTAL_MOVE_STEP_DISTANCE;

        return MoveTo(boss->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true);
    }

    // Ranged outside the cone but too close => back off to avoid War Stomp
    if (PlayerbotAI::IsRanged(bot))
    {
        float distToTravel = RAZORGORE_RANGE_DISTANCE - distance;
        if (distToTravel > 0)
            return MoveAway(boss, distToTravel);
    }

    return false;
}

bool BwlRazorgoreMarkBossAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "razorgore the untamed");
    if (!boss)
        return false;

    if (AreRazorgoreEggsAlive(botAI))
    {
        if (IsRazorgoreOffTank(bot))
        {
            if (MarkTargetWithMoon(bot, boss))
                return true;

            SetRtiTarget(botAI, "moon");

            if (AI_VALUE(Unit*, "current target") != boss)
                return Attack(boss);
        }
    }
    else
    {
        ClearTargetIcon(bot, RtiTargetValue::moonIndex);
    }

    return false;
}

bool BwlRazorgoreMarkBossAction::isUseful()
{
    return PlayerbotAI::IsTank(bot);
}

// Vaelastrasz the Corrupt

bool BwlVaelastraszMoveAwayAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "vaelastrasz the corrupt");

    // Current target with Burning Adrenaline stays put when Vaelastrasz is alive.
    if (boss && boss->IsAlive() && boss->GetVictim() == bot)
        return false;

    float fleeX, fleeY;
    if (!CalculateFleeDirection(boss, fleeX, fleeY))
        return false;

    return MoveAlongFleeDirection(boss, fleeX, fleeY);
}

bool BwlVaelastraszMoveAwayAction::CalculateFleeDirection(Unit const* boss, float& fleeX, float& fleeY) const
{
    fleeX = 0.0f;
    fleeY = 0.0f;
    bool bossDead = !boss || !boss->IsAlive();

    if (Group const* group = bot->GetGroup())
    {
        for (GroupReference const* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* p = gref->GetSource();
            if (!p || p == bot || !p->IsAlive() || p->GetMapId() != bot->GetMapId())
                continue;

            // Boss alive: ignore other Burning Adrenaline bots so they can group together
            if (!bossDead && p->HasAura(static_cast<uint32>(BlackwingLairSpells::SPELL_BURNING_ADRENALINE)))
                continue;

            // Compute a flee direction via weighted repulsion from other bots.
            // Weighted repulsion: closer = stronger push (linear falloff to 20y)
            float dist = bot->GetDistance2d(p);
            if (dist < VAELASTRASZ_REPULSION_RANGE && dist > 0.1f)
            {
                float weight = 1.0f - dist / VAELASTRASZ_REPULSION_RANGE;
                fleeX += (bot->GetPositionX() - p->GetPositionX()) / dist * weight;
                fleeY += (bot->GetPositionY() - p->GetPositionY()) / dist * weight;
            }
        }
    }

    float fleeLen = sqrt(fleeX * fleeX + fleeY * fleeY);

    // No repulsion
    if (fleeLen < 0.1f)
    {
        // Tail Sweep recovery: move back toward boss
        if (!bossDead && bot->GetDistance2d(boss) > VAELASTRASZ_TAIL_SWEEP_RECOVERY_RANGE)
        {
            float toBossX = boss->GetPositionX() - bot->GetPositionX();
            float toBossY = boss->GetPositionY() - bot->GetPositionY();
            float dist = sqrt(toBossX * toBossX + toBossY * toBossY);
            if (dist > 0.1f)
            {
                fleeX = toBossX / dist;
                fleeY = toBossY / dist;
                return true;
            }
        }
        return false;
    }

    fleeX /= fleeLen;
    fleeY /= fleeLen;

    // Ranged: blend flee direction toward boss when >25y to stay in cast range
    if (!bossDead && PlayerbotAI::IsRanged(bot))
    {
        float bossDist = bot->GetDistance2d(boss);
        if (bossDist > VAELASTRASZ_RANGED_BIAS_THRESHOLD)
        {
            float toBossX = (boss->GetPositionX() - bot->GetPositionX()) / bossDist;
            float toBossY = (boss->GetPositionY() - bot->GetPositionY()) / bossDist;
            if (fleeX * toBossX + fleeY * toBossY < 0.0f)
            {
                fleeX = fleeX * VAELASTRASZ_RANGED_BIAS_FLEE_WEIGHT + toBossX * VAELASTRASZ_RANGED_BIAS_BOSS_WEIGHT;
                fleeY = fleeY * VAELASTRASZ_RANGED_BIAS_FLEE_WEIGHT + toBossY * VAELASTRASZ_RANGED_BIAS_BOSS_WEIGHT;
                float len = sqrt(fleeX * fleeX + fleeY * fleeY);
                fleeX /= len;
                fleeY /= len;
            }
        }
    }

    return true;
}

bool BwlVaelastraszMoveAwayAction::MoveAlongFleeDirection(Unit const* boss, float fleeX, float fleeY)
{
    bool bossDead = !boss || !boss->IsAlive();
    float baseAngle = atan2(fleeY, fleeX);

    // Pass 1: ranged try LOS positions first
    // Pass 2: everyone, no LOS req
    for (bool requireLos : {true, false})
    {
        // Ranged bots try LOS positions first, then fall back to no-LOS.
        if (requireLos && (!PlayerbotAI::IsRanged(bot) || bossDead))
            continue;

        // Try 5 angles (0°, ±45°, ±90°) around the flee direction to slide along walls.
        static constexpr float angleOffsets[] = {0.0f, M_PI_4, -M_PI_4, M_PI_2, -M_PI_2};
        for (float offset : angleOffsets)
        {
            float angle = baseAngle + offset;
            float targetX = bot->GetPositionX() + INCREMENTAL_MOVE_STEP_DISTANCE * cos(angle);
            float targetY = bot->GetPositionY() + INCREMENTAL_MOVE_STEP_DISTANCE * sin(angle);

            // Reject if LOS to boss would break (only checked in Pass 1 for ranged)
            if (requireLos && boss && !boss->IsWithinLOS(targetX, targetY, bot->GetPositionZ()))
                continue;

            if (MoveTo(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(), false, false,
                       false, false, MovementPriority::MOVEMENT_FORCED, true))
                return true;
        }
    }

    return false;
}

// Broodlord Lashlayer

bool BwlBroodlordRangedMoveAwayAction::Execute(Event /*event*/)
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "broodlord lashlayer"))
    {
        float distToTravel = BROODLORD_SAFE_DISTANCE - bot->GetDistance2d(boss);
        if (distToTravel > 0.0f)
            return MoveAway(boss, distToTravel);
    }
    return false;
}

// Firemaw / Ebonroc / Flamegor

bool BwlBlackDrakeAvoidBreathAction::isUseful()
{
    Unit const* boss = GetTarget();
    if (!boss)
        return false;

    // Only reposition while standing in the frontal breath cone.
    // No tail-swipe check: unlike RearFlankAction there is no rear danger zone here.
    return boss->HasInArc(2.0f * minAngle, bot);
}

// Chromaggus

bool BwlUseHourglassSandAction::Execute(Event /*event*/)
{
    return botAI->CastSpell(static_cast<uint32>(BlackwingLairSpells::SPELL_HOURGLASS_SAND), bot);
}

// Trash

static constexpr float WYRMGUARD_SAFE_DISTANCE = 16.0f;

Unit* BwlDeathTalonWyrmguardTankMoveAwayAction::GetTarget()
{
    // Find a nearby wyrmguard within 16 yards being tanked by someone else
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (!unit || !unit->IsAlive() || unit->GetEntry() != static_cast<uint32>(BlackwingLairNPCs::NPC_DEATH_TALON_WYRMGUARD))
            continue;
        if (bot->GetDistance2d(unit) > WYRMGUARD_SAFE_DISTANCE)
            continue;
        Unit* victim = unit->GetVictim();
        if (victim && victim != bot && victim->IsPlayer() && PlayerbotAI::IsTank(victim->ToPlayer()))
            return unit;
    }
    return nullptr;
}

bool BwlDeathTalonWyrmguardTankMoveAwayAction::isUseful()
{
    if (!GetTarget())
        return false;

    // Must be actively tanking a wyrmguard before moving away from another tank's
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (unit && unit->IsAlive() && unit->GetEntry() == static_cast<uint32>(BlackwingLairNPCs::NPC_DEATH_TALON_WYRMGUARD) &&
            unit->GetVictim() == bot)
            return true;
    }
    return false;
}

bool BwlDeathTalonWyrmguardTankMoveAwayAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    float distToTravel = WYRMGUARD_SAFE_DISTANCE - bot->GetDistance2d(target);
    if (distToTravel <= 0.0f)
        return false;

    return MoveAway(target, distToTravel);
}

Unit* BwlDeathTalonWyrmguardRangedMoveAwayAction::GetTarget()
{
    // Find the closest wyrmguard within 16 yards
    Unit* closest = nullptr;
    float closestDist = WYRMGUARD_SAFE_DISTANCE;
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (!unit || !unit->IsAlive() || unit->GetEntry() != static_cast<uint32>(BlackwingLairNPCs::NPC_DEATH_TALON_WYRMGUARD))
            continue;
        float dist = bot->GetDistance2d(unit);
        if (dist < closestDist)
        {
            closestDist = dist;
            closest = unit;
        }
    }
    return closest;
}

bool BwlDeathTalonWyrmguardRangedMoveAwayAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target || target->GetVictim() == bot)
        return false;

    float distToTravel = WYRMGUARD_SAFE_DISTANCE - bot->GetDistance2d(target);
    if (distToTravel <= 0.0f)
        return false;

    return MoveAway(target, distToTravel);
}
