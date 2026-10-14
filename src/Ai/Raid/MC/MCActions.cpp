/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MCActions.h"
#include "MCHelpers.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include <algorithm>

static constexpr float LIVING_BOMB_DISTANCE = 20.0f;
static constexpr float INFERNO_DISTANCE = 20.0f;

// dedicated tank positions; prevents assist tanks from positioning Core Ragers on steep walls on pull
static const Position GOLEMAGG_TANK_POSITION{795.7308, -994.8848, -207.18661};
static const Position CORE_RAGER_TANK_POSITION{846.6453, -1019.0639, -198.9819};

// Midpoint of the two tank camps (56y apart): healers standing here reach both
static const Position GOLEMAGG_HEALER_POSITION{821.2f, -1007.0f, -203.0f};
static constexpr float HEALER_POSITION_TOLERANCE = 8.0f;

static constexpr float GOLEMAGGS_TRUST_DISTANCE = 30.0f;
static constexpr float CORE_RAGER_STEP_DISTANCE = 5.0f;

using namespace MoltenCoreHelpers;

bool McMoveFromGroupAction::Execute(Event /*event*/)
{
    return MoveFromGroup(LIVING_BOMB_DISTANCE);
}

bool McMoveFromBaronGeddonAction::Execute(Event /*event*/)
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "baron geddon"))
    {
        float distToTravel = INFERNO_DISTANCE - bot->GetDistance2d(boss);
        if (distToTravel > 0)
        {
            // Stop channeling spell first
            bot->AttackStop();
            bot->CastStop();

            return MoveAway(boss, distToTravel);
        }
    }
    return false;
}

bool McShazzrahMoveAwayAction::Execute(Event /*event*/)
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "shazzrah"))
    {
        float distToTravel = ARCANE_EXPLOSION_DISTANCE - bot->GetDistance2d(boss);
        if (distToTravel > 0)
            return MoveAway(boss, distToTravel);
    }
    return false;
}

bool McMoveFromLavaAction::Execute(Event /*event*/)
{
    // Escape to the nearest raid member on dry ground: raids fight on the
    // rock shelves, so a dry teammate marks the closest safe floor.
    Unit* dryTarget = nullptr;
    float bestDist = 100.0f;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
                continue;

            if (member->GetLiquidData().Flags & MAP_LIQUID_TYPE_MAGMA)
                continue;

            float dist = bot->GetDistance(member);
            if (dist < bestDist)
            {
                bestDist = dist;
                dryTarget = member;
            }
        }
    }

    // No dry raid member in range: stay put and let the in-lava trigger fire
    // again — any other destination (the current target included) can path
    // straight back into the pool.
    if (!dryTarget)
        return false;

    bot->CastStop();

    if (!MoveTo(dryTarget->GetMapId(), dryTarget->GetPositionX(), dryTarget->GetPositionY(),
                dryTarget->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_FORCED))
        return false;

    // Hold the AI so rotation/facing actions can't cancel the escape spline
    // mid-lava. Capped at 2s on purpose: the in-lava trigger refires and
    // re-issues the escape while the bot is still swimming.
    constexpr uint32 MAX_ESCAPE_HOLD_MS = 2000;
    constexpr float MIN_ESCAPE_SPEED = 0.1f;

    float const speed = bot->GetSpeed(MOVE_RUN);
    if (speed > MIN_ESCAPE_SPEED)
        botAI->SetNextCheckDelay(
            std::min(MAX_ESCAPE_HOLD_MS, uint32(IN_MILLISECONDS * bot->GetDistance(dryTarget) / speed)));

    return true;
}

bool McGolemaggBackOffAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "golemagg the incinerator");
    if (!boss)
        return false;

    // Out of swing range the stack stops growing and expires 30s after
    // the last application; the multiplier blocks re-engaging until then.
    bot->CastStop();

    float distToTravel = MAGMA_SPLASH_BACK_OFF_DISTANCE - bot->GetDistance2d(boss);
    if (distToTravel <= 0.0f)
        return true;

    return MoveAway(boss, distToTravel);
}

bool McGolemaggMarkBossAction::Execute(Event /*event*/)
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "golemagg the incinerator"))
    {
        if (Group* group = bot->GetGroup())
        {
            ObjectGuid currentSkullGuid = group->GetTargetIcon(RtiTargetValue::skullIndex);
            if (currentSkullGuid.IsEmpty() || currentSkullGuid != boss->GetGUID())
            {
                group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), boss->GetGUID());
                return true;
            }
        }
    }
    return false;
}

bool McGolemaggTankAction::MoveUnitToPosition(Unit* target, Position const& tankPosition, float maxDistance,
                                              float stepDistance)
{
    if (bot->GetVictim() != target)
        return Attack(target);
    if (target->GetVictim() == bot)
    {
        float distanceToTankPosition = bot->GetExactDist2d(tankPosition.GetPositionX(), tankPosition.GetPositionY());
        if (distanceToTankPosition > maxDistance)
        {
            float dX = tankPosition.GetPositionX() - bot->GetPositionX();
            float dY = tankPosition.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * stepDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * stepDistance;
            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true,
                          true);
        }
    }
    else if (botAI->DoSpecificAction("taunt spell", Event(), true))
        return true;
    return false;
}

bool McGolemaggTankAction::FindCoreRagers(Unit*& coreRager1, Unit*& coreRager2) const
{
    coreRager1 = coreRager2 = nullptr;
    for (auto const& target : AI_VALUE(GuidVector, "possible targets no los"))
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_CORE_RAGER)
        {
            if (coreRager1 == nullptr)
                coreRager1 = unit;
            else if (coreRager2 == nullptr)
            {
                coreRager2 = unit;
                break; // There should be no third Core Rager.
            }
        }
    }
    return coreRager1 != nullptr && coreRager2 != nullptr;
}

bool McGolemaggMainTankAttackGolemaggAction::Execute(Event /*event*/)
{
    // At this point, we know we are not the last living tank in the group.
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "golemagg the incinerator"))
    {
        Unit* coreRager1;
        Unit* coreRager2;
        if (!FindCoreRagers(coreRager1, coreRager2))
            return false; // safety check

        // We only need to move if the Core Ragers still have Golemagg's Trust
        if (coreRager1->HasAura(SPELL_GOLEMAGGS_TRUST) || coreRager2->HasAura(SPELL_GOLEMAGGS_TRUST))
            return MoveUnitToPosition(boss, GOLEMAGG_TANK_POSITION, boss->GetCombatReach());
    }
    return false;
}

bool McGolemaggHealerPositionAction::Execute(Event /*event*/)
{
    if (bot->GetExactDist2d(GOLEMAGG_HEALER_POSITION) <= HEALER_POSITION_TOLERANCE)
        return false;

    return MoveTo(bot->GetMapId(), GOLEMAGG_HEALER_POSITION.GetPositionX(), GOLEMAGG_HEALER_POSITION.GetPositionY(),
                  GOLEMAGG_HEALER_POSITION.GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool McGolemaggAssistTankAttackCoreRagerAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "golemagg the incinerator");
    if (!boss)
        return false;

    // Step 0: Filter additional assist tanks. We only need 2.
    bool isFirstAssistTank = PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
    bool isSecondAssistTank = PlayerbotAI::IsAssistTankOfIndex(bot, 1, true);
    if (!isFirstAssistTank && !isSecondAssistTank)
        return Attack(boss);

    // Step 1: Find both Core Ragers
    Unit* coreRager1;
    Unit* coreRager2;
    if (!FindCoreRagers(coreRager1, coreRager2))
        return false; // safety check

    // Step 2: Assign Core Rager to bot
    Unit* myCoreRager = nullptr;
    Unit* otherCoreRager = nullptr;
    if (isFirstAssistTank)
    {
        myCoreRager = coreRager1;
        otherCoreRager = coreRager2;
    }
    else // isSecondAssistTank is always true here
    {
        myCoreRager = coreRager2;
        otherCoreRager = coreRager1;
    }

    // Step 3: Select the right target
    if (myCoreRager->GetVictim() != bot)
    {
        // Step 3.1: My Core Rager isn't attacking me. Attack until it does.
        if (bot->GetVictim() != myCoreRager)
            return Attack(myCoreRager);
        return botAI->DoSpecificAction("taunt spell", event, true);
    }

    Unit* otherCoreRagerVictim = otherCoreRager->GetVictim();
    if (otherCoreRagerVictim) // Core Rager victim can be NULL
    {
        // Step 3.2: Check if the other Core Rager isn't attacking its assist tank.
        Player* otherCoreRagerPlayerVictim = otherCoreRagerVictim->ToPlayer();
        if (otherCoreRagerPlayerVictim &&
            !PlayerbotAI::IsAssistTankOfIndex(otherCoreRagerPlayerVictim, 0, true) &&
            !PlayerbotAI::IsAssistTankOfIndex(otherCoreRagerPlayerVictim, 1, true))
        {
            // Assume we are the only assist tank or the other assist tank is dead => pick up other Core Rager!
            if (bot->GetVictim() != otherCoreRager)
                return Attack(otherCoreRager);
            return botAI->DoSpecificAction("taunt spell", event, true);
        }
    }

    if (bot->GetVictim() != myCoreRager)
        return Attack(myCoreRager); // Step 3.3: Attack our Core Rager in case we previously switched in 3.2.

    // Step 4: Prevent Golemagg's Trust on Core Ragers
    if (myCoreRager->HasAura(SPELL_GOLEMAGGS_TRUST) ||
        (otherCoreRagerVictim == bot && otherCoreRager->HasAura(SPELL_GOLEMAGGS_TRUST)))
    {
        // Step 4.1: Move Core Ragers to dedicated tank position (only if Golemagg is far enough away from said position)
        float bossDistanceToCoreRagerTankPosition = boss->GetExactDist2d(
            CORE_RAGER_TANK_POSITION.GetPositionX(), CORE_RAGER_TANK_POSITION.GetPositionY());
        if (bossDistanceToCoreRagerTankPosition > GOLEMAGGS_TRUST_DISTANCE)
        {
            float distanceToTankPosition = bot->GetExactDist2d(CORE_RAGER_TANK_POSITION.GetPositionX(),
                                                               CORE_RAGER_TANK_POSITION.GetPositionY());
            if (distanceToTankPosition > CORE_RAGER_STEP_DISTANCE)
                return MoveUnitToPosition(myCoreRager, CORE_RAGER_TANK_POSITION, CORE_RAGER_STEP_DISTANCE);
        }

        // Step 4.2: if boss is too close to tank position, or we are already there, move away from Golemagg to try to out-range Golemagg's Trust
        return MoveAway(boss, CORE_RAGER_STEP_DISTANCE, true);
    }

    return false;
}

Unit* McCoreHoundMarkAction::GetTarget()
{
    Unit* highestHealthHound = nullptr;
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_CORE_HOUND)
        {
            if (!highestHealthHound || unit->GetHealth() > highestHealthHound->GetHealth())
                highestHealthHound = unit;
        }
    }

    if (!highestHealthHound)
        return nullptr;

    Group* group = bot->GetGroup();
    ObjectGuid currentSkullGuid = group ? group->GetTargetIcon(RtiTargetValue::skullIndex) : ObjectGuid::Empty;
    if (!currentSkullGuid.IsEmpty() && currentSkullGuid != highestHealthHound->GetGUID())
    {
        // Only switch skull if the new target has meaningfully more health (10% buffer) to prevent rapid re-marking
        if (Unit* currentSkullUnit = botAI->GetUnit(currentSkullGuid))
            if (currentSkullUnit->IsAlive() && highestHealthHound->GetHealth() <= currentSkullUnit->GetHealth() * 1.10f)
                return nullptr;
    }

    if (currentSkullGuid.IsEmpty() || currentSkullGuid != highestHealthHound->GetGUID())
        return highestHealthHound;

    return nullptr;
}

bool McCoreHoundMarkAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    bot->GetGroup()->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), target->GetGUID());
    return true;
}
