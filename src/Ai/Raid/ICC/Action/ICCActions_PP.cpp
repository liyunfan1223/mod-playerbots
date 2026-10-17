/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericSpellActions.h"
#include "ICCActions.h"
#include "ICCScripts.h"
#include "ICCTriggers.h"
#include "Multiplier.h"
#include "NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiValue.h"
#include "Vehicle.h"
#include <ctime>
#include <fstream>
#include <limits>

// Professor Putricide
bool IccPutricideMutatedPlagueAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return false;

    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        bot->AttackStop();
        bot->CastStop();
        if (bot->GetTarget())
            bot->SetTarget(ObjectGuid::Empty);
        return false;
    }

    auto GetPlagueStacks = [&](Unit* unit) -> uint32
    {
        if (!unit)
            return 0;

        Aura* a = botAI->GetAura("mutated plague", unit, false, true);
        return a ? a->GetStackAmount() : 0;
    };

    uint32 const myStacks = GetPlagueStacks(bot);

    bool shouldTaunt = false;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || member == bot || !member->IsAlive() || !member->IsInWorld())
                continue;

            if (!PlayerbotAI::IsTank(member))
                continue;

            if (myStacks < GetPlagueStacks(member))
            {
                shouldTaunt = true;
                break;
            }
        }
    }

    if (shouldTaunt && boss->GetVictim() != bot)
        IccCastClassTaunt(bot, botAI, boss);

    return false;
}

bool IccPutricideGrowingOozePuddleAction::Execute(Event /*event*/)
{
    if (PlayerbotAI::IsMainTank(bot) &&
        bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        bot->AttackStop();
        bot->CastStop();
        if (bot->GetTarget())
            bot->SetTarget(ObjectGuid::Empty);
        if (Unit* master = botAI->GetMaster())
            Follow(master);
        return true;
    }

    // Phase 3: only MT avoids hazards. Non-MT bots stack on MT blindly so
    // they don't scatter when a puddle drops on the stack.
    if (!PlayerbotAI::IsMainTank(bot))
    {
        Unit* bossP3 = AI_VALUE2(Unit*, "find target", "professor putricide");
        if (bossP3 && bossP3->HealthBelowPct(35))
            return false;
    }

    // Main tank rotation kite: when one or more active Growing Ooze Puddles
    // sit close to the boss, walk the boss to a position safe from ALL of
    // them. Tank picks an angle around the boss whose forward arc clears
    // every nearby puddle. Boss turns to face the tank, rotating its frontal
    // cone away from the puddle field. Stack bots line up behind the boss.
    // Phase 3: skip kite (MT idles near boss). Flee logic below still runs so
    // MT steps out of puddles.
    if (PlayerbotAI::IsMainTank(bot))
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
        if (boss && boss->IsAlive() && boss->GetVictim() == bot && !boss->HealthBelowPct(35))
        {
            constexpr float puddleNearBossRange = 8.0f;
            constexpr float puddleSafeRadius = 10.0f;
            constexpr float bossReach = 4.0f;
            constexpr int kiteAngleSteps = 24;

            std::vector<Unit*> nearPuddles;
            GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
            for (auto const& g : npcs)
            {
                Unit* u = botAI->GetUnit(g);
                if (!u || !u->IsAlive() || u->GetEntry() != NPC_GROWING_OOZE_PUDDLE)
                    continue;
                if (u->GetExactDist2d(boss) <= puddleNearBossRange)
                    nearPuddles.push_back(u);
            }

            if (!nearPuddles.empty())
            {
                // Centroid of near puddles → seed direction is boss-from-centroid.
                float cx = 0.0f, cy = 0.0f;
                for (Unit* p : nearPuddles)
                {
                    cx += p->GetPositionX();
                    cy += p->GetPositionY();
                }
                cx /= static_cast<float>(nearPuddles.size());
                cy /= static_cast<float>(nearPuddles.size());

                float seedX = boss->GetPositionX() - cx;
                float seedY = boss->GetPositionY() - cy;
                float seedLen = std::sqrt(seedX * seedX + seedY * seedY);
                float seedAngle = (seedLen > 0.01f) ? std::atan2(seedY, seedX) : 0.0f;

                float bossX = boss->GetPositionX();
                float bossY = boss->GetPositionY();
                float botZ = bot->GetPositionZ();

                // Scan angles around the boss starting from the seed (away
                // from centroid) and spiraling outward. Pick the first angle
                // whose tank-stand point clears every near puddle.
                float bestGoalX = 0.0f, bestGoalY = 0.0f;
                float bestMinDist = -FLT_MAX;
                bool found = false;
                bool foundClean = false;

                for (int i = 0; i < kiteAngleSteps; ++i)
                {
                    // Alternate +/- around seedAngle to prefer minimal rotation.
                    int sign = (i % 2 == 0) ? 1 : -1;
                    int step = (i + 1) / 2;
                    float angle = seedAngle + sign * step *
                                  (2.0f * static_cast<float>(M_PI) / kiteAngleSteps);

                    float gx = bossX + std::cos(angle) * bossReach;
                    float gy = bossY + std::sin(angle) * bossReach;

                    float minDist = FLT_MAX;
                    for (Unit* p : nearPuddles)
                    {
                        float dd = p->GetDistance2d(gx, gy);
                        if (dd < minDist)
                            minDist = dd;
                    }

                    if (!bot->IsWithinLOS(gx, gy, botZ))
                        continue;

                    if (minDist >= puddleSafeRadius)
                    {
                        bestGoalX = gx;
                        bestGoalY = gy;
                        foundClean = true;
                        found = true;
                        break;
                    }

                    if (minDist > bestMinDist)
                    {
                        bestMinDist = minDist;
                        bestGoalX = gx;
                        bestGoalY = gy;
                        found = true;
                    }
                }

                if (found && bot->GetExactDist2d(bestGoalX, bestGoalY) > 1.0f)
                {
                    return MoveTo(bot->GetMapId(), bestGoalX, bestGoalY, botZ, false, false, false, true,
                                  MovementPriority::MOVEMENT_COMBAT);
                }

                // No clean angle and already at best position: fall through
                // to puddle-flee logic below so tank still avoids damage.
                (void)foundClean;
            }
        }
    }

    Unit* closestPuddle = FindClosestThreateningPuddle();
    if (!closestPuddle)
        return false;

    Position movePosition = CalculateSafeMovePosition(closestPuddle);
    return MoveTo(bot->GetMapId(), movePosition.GetPositionX(), movePosition.GetPositionY(),
                  movePosition.GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
}

Unit* IccPutricideGrowingOozePuddleAction::FindClosestThreateningPuddle()
{
    constexpr float baseRadius = 2.0f;
    constexpr float stackMultiplier = 0.8f;
    constexpr float mainTankSafeDistance = 10.0f;
    constexpr float minDistance = 0.1f;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    if (npcs.empty())
        return nullptr;

    // Phase 3: MT no longer kites, treat as regular bot for puddle avoidance.
    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    bool const isMainTank = PlayerbotAI::IsMainTank(bot) && !(boss && boss->HealthBelowPct(35));

    Unit* closestPuddle = nullptr;
    float closestDistance = FLT_MAX;

    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || unit->GetEntry() != NPC_GROWING_OOZE_PUDDLE)
            continue;

        float currentDistance = std::max(minDistance, bot->GetExactDist2d(unit));
        float safeDistance = isMainTank ? mainTankSafeDistance : baseRadius;

        if (!isMainTank)
        {
            if (Aura* grow = unit->GetAura(SPELL_GROW_AURA))
                safeDistance += (grow->GetStackAmount() * stackMultiplier);
        }

        if (currentDistance < safeDistance && currentDistance < closestDistance)
        {
            closestDistance = currentDistance;
            closestPuddle = unit;
        }
    }

    return closestPuddle;
}

Position IccPutricideGrowingOozePuddleAction::CalculateSafeMovePosition(Unit* closestPuddle)
{
    constexpr float baseRadius = 2.0f;
    constexpr float stackMultiplier = 0.8f;
    constexpr float mainTankSafeDistance = 10.0f;
    constexpr float bufferDistance = 2.0f;
    constexpr float minDistance = 0.1f;
    constexpr int numAnglesToTest = 8;
    constexpr float tankShoveDistance = 6.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    bool const isMainTank = PlayerbotAI::IsMainTank(bot) && !(boss && boss->HealthBelowPct(35));
    bool const isP3Tank = PlayerbotAI::IsMainTank(bot) && boss && boss->HealthBelowPct(35);

    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float botZ = bot->GetPositionZ();

    float gateDx = 0.0f, gateDy = 0.0f, gateLen = 0.0f;
    if (isP3Tank)
    {
        gateDx = ICC_PUTRICIDE_GATE_POSITION.GetPositionX() - botX;
        gateDy = ICC_PUTRICIDE_GATE_POSITION.GetPositionY() - botY;
        gateLen = std::sqrt(gateDx * gateDx + gateDy * gateDy);
        if (gateLen > 0.01f)
        {
            gateDx /= gateLen;
            gateDy /= gateLen;
        }
    }

    float currentDistance = std::max(minDistance, bot->GetExactDist2d(closestPuddle));
    bool const useTankSafeDistance = isMainTank || isP3Tank;
    float safeDistance = useTankSafeDistance ? mainTankSafeDistance : baseRadius;
    if (!useTankSafeDistance)
    {
        if (Aura* grow = closestPuddle->GetAura(SPELL_GROW_AURA))
            safeDistance += (grow->GetStackAmount() * stackMultiplier);
    }

    float dx = botX - closestPuddle->GetPositionX();
    float dy = botY - closestPuddle->GetPositionY();
    float dist = std::max(minDistance, std::sqrt(dx * dx + dy * dy));

    if (dist < minDistance * 2.0f)
    {
        float randomAngle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * static_cast<float>(M_PI);
        dx = std::cos(randomAngle);
        dy = std::sin(randomAngle);
    }
    else
    {
        dx /= dist;
        dy /= dist;
    }

    float moveDistance = safeDistance - currentDistance + bufferDistance;

    // If the bot is already inside the puddle, anchor candidates on the
    // puddle's safe-radius circle instead of rotating around the bot's
    // current (unsafe) position. Otherwise a rotated move can land deeper
    // into the puddle. Outward radial from puddle center is always safe.
    bool const insidePuddle = currentDistance < safeDistance;

    for (int i = 0; i < numAnglesToTest; ++i)
    {
        float angle = (2.0f * static_cast<float>(M_PI) * i) / numAnglesToTest;
        float rotatedDx = dx * std::cos(angle) - dy * std::sin(angle);
        float rotatedDy = dx * std::sin(angle) + dy * std::cos(angle);

        float testX, testY;
        if (insidePuddle)
        {
            // Stand on safe ring around puddle, rotated by `angle` from the
            // bot-relative radial. i=0 → straight outward from puddle center.
            float radius = safeDistance + bufferDistance;
            testX = closestPuddle->GetPositionX() + rotatedDx * radius;
            testY = closestPuddle->GetPositionY() + rotatedDy * radius;
        }
        else
        {
            testX = botX + rotatedDx * moveDistance;
            testY = botY + rotatedDy * moveDistance;
        }

        // Reject any candidate that still sits inside the closest puddle.
        float candDist = closestPuddle->GetDistance2d(testX, testY);
        if (candDist < safeDistance)
            continue;

        if (!IsPositionTooCloseToOtherPuddles(testX, testY, closestPuddle) && bot->IsWithinLOS(testX, testY, botZ))
        {
            if (isP3Tank && gateLen > 0.01f)
            {
                float moveDx = testX - botX;
                float moveDy = testY - botY;
                if (moveDx * gateDx + moveDy * gateDy <= 0.0f)
                    continue;
            }

            if (PathCrossesAnyPuddle(botX, botY, testX, testY, nullptr))
                continue;

            if (PlayerbotAI::IsTank(bot))
            {
                float awayDx = testX - closestPuddle->GetPositionX();
                float awayDy = testY - closestPuddle->GetPositionY();
                float awayDist = std::sqrt(awayDx * awayDx + awayDy * awayDy);
                if (awayDist > 0.001f)
                {
                    awayDx /= awayDist;
                    awayDy /= awayDist;
                    testX += awayDx * tankShoveDistance;
                    testY += awayDy * tankShoveDistance;
                }
            }
            return Position(testX, testY, botZ);
        }
    }

    // Fallback: straight outward from puddle center on its safe ring.
    float fallbackRadius = safeDistance + bufferDistance;
    float fallbackX = insidePuddle ? (closestPuddle->GetPositionX() + dx * fallbackRadius)
                                   : (botX + dx * moveDistance);
    float fallbackY = insidePuddle ? (closestPuddle->GetPositionY() + dy * fallbackRadius)
                                   : (botY + dy * moveDistance);
    if (isP3Tank && gateLen > 0.01f)
    {
        float fbDx = fallbackX - botX;
        float fbDy = fallbackY - botY;
        if (fbDx * gateDx + fbDy * gateDy <= 0.0f)
        {
            fallbackX = closestPuddle->GetPositionX() + gateDx * fallbackRadius;
            fallbackY = closestPuddle->GetPositionY() + gateDy * fallbackRadius;
        }
    }
    if (PlayerbotAI::IsTank(bot))
    {
        float awayDx = fallbackX - closestPuddle->GetPositionX();
        float awayDy = fallbackY - closestPuddle->GetPositionY();
        float awayDist = std::sqrt(awayDx * awayDx + awayDy * awayDy);
        if (awayDist > 0.001f)
        {
            awayDx /= awayDist;
            awayDy /= awayDist;
            fallbackX += awayDx * tankShoveDistance;
            fallbackY += awayDy * tankShoveDistance;
        }
    }
    return Position(fallbackX, fallbackY, botZ);
}

bool IccPutricideGrowingOozePuddleAction::PathCrossesAnyPuddle(float fromX, float fromY, float toX, float toY, Unit* ignorePuddle)
{
    constexpr float baseRadius = 2.0f;
    constexpr float stackMultiplier = 0.8f;
    constexpr float mainTankSafeDistance = 10.0f;

    bool const useTankSafeDistance = PlayerbotAI::IsMainTank(bot);

    float segDx = toX - fromX;
    float segDy = toY - fromY;
    float segLenSq = segDx * segDx + segDy * segDy;
    if (segLenSq < 0.01f)
        return false;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || unit == ignorePuddle || unit->GetEntry() != NPC_GROWING_OOZE_PUDDLE)
            continue;

        float radius = useTankSafeDistance ? mainTankSafeDistance : baseRadius;
        if (!useTankSafeDistance)
        {
            if (Aura* grow = unit->GetAura(SPELL_GROW_AURA))
                radius += (grow->GetStackAmount() * stackMultiplier);
        }

        float px = unit->GetPositionX();
        float py = unit->GetPositionY();
        float t = ((px - fromX) * segDx + (py - fromY) * segDy) / segLenSq;
        if (t < 0.0f) t = 0.0f;
        else if (t > 1.0f) t = 1.0f;

        float closestX = fromX + segDx * t;
        float closestY = fromY + segDy * t;
        float ddx = closestX - px;
        float ddy = closestY - py;
        if (ddx * ddx + ddy * ddy < radius * radius)
            return true;
    }

    return false;
}

bool IccPutricideGrowingOozePuddleAction::IsPositionTooCloseToOtherPuddles(float x, float y, Unit* ignorePuddle)
{
    constexpr float baseRadius = 2.0f;
    constexpr float stackMultiplier = 0.8f;
    constexpr float mainTankSafeDistance = 10.0f;

    bool const isMainTank = PlayerbotAI::IsMainTank(bot);

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || unit == ignorePuddle || unit->GetEntry() != NPC_GROWING_OOZE_PUDDLE)
            continue;

        float safeDistance = isMainTank ? mainTankSafeDistance : baseRadius;
        if (!isMainTank)
        {
            if (Aura* grow = unit->GetAura(SPELL_GROW_AURA))
                safeDistance += (grow->GetStackAmount() * stackMultiplier);
        }

        float dist = unit->GetDistance2d(x, y);
        if (dist < safeDistance)
            return true;
    }

    return false;
}

bool IccPutricideVolatileOozeAction::Execute(Event /*event*/)
{
    constexpr float stackDistance = 7.0f;

    Unit* ooze = AI_VALUE2(Unit*, "find target", "volatile ooze");
    if (!ooze)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return false;

    if (PlayerbotAI::IsMainTank(bot) &&
        bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE &&
        bot->GetExactDist2d(ICC_PUTRICIDE_TANK_POSITION) > 20.0f &&
        !boss->HealthBelowPct(36) && boss->GetVictim() == bot)
        return MoveTo(bot->GetMapId(), ICC_PUTRICIDE_TANK_POSITION.GetPositionX(),
                      ICC_PUTRICIDE_TANK_POSITION.GetPositionY(), ICC_PUTRICIDE_TANK_POSITION.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);

    if (botAI->HasAura("Gaseous Bloat", bot) || botAI->HasAura("Unbound Plague", bot))
        return false;

    std::vector<Unit*> aliveOozes;
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == ooze->GetEntry())
            aliveOozes.push_back(unit);
    }

    if (aliveOozes.size() > 1)
    {
        for (size_t i = 0; i < aliveOozes.size() - 1; ++i)
            bot->Kill(bot, aliveOozes[i]);
    }

    MarkOozeWithSkull(ooze);

    if (PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsMainTank(bot))
    {
        if (bot->IsWithinMeleeRange(ooze))
        {
            Attack(ooze);
            return false;
        }

        // If we're closer to the ooze than the targeted stack player is,
        // running to the stack point just drags the ooze further. Go attack
        // the ooze directly — the stack target will end up here anyway.
        Unit* stackTarget = FindAuraTarget();
        if (stackTarget)
        {
            float botToOoze = bot->GetDistance2d(ooze);
            float stackToOoze = stackTarget->GetDistance2d(ooze);
            if (botToOoze < stackToOoze)
            {
                Attack(ooze);
                return false;
            }
        }
    }

    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsHeal(bot))
    {
        constexpr float nearbyStackRange = 20.0f;

        Unit* stackTarget = FindAuraTarget();
        float distToStack = stackTarget ? bot->GetDistance2d(stackTarget) : 0.0f;

        // Only run to the stack target if it is reasonably close. Otherwise
        // we'd chase across the room and barely attack. If it is far, check
        // whether any other group member is already stacking near it — if
        // yes, join them; if no, just stay put and attack from range.
        bool shouldMoveToStack = false;
        if (stackTarget && distToStack > stackDistance)
        {
            if (distToStack <= nearbyStackRange)
            {
                shouldMoveToStack = true;
            }
            else if (Group* group = bot->GetGroup())
            {
                for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                {
                    Player* member = itr->GetSource();
                    if (!member || !member->IsAlive() || member == bot || member == stackTarget)
                        continue;
                    if (member->GetDistance2d(stackTarget) <= stackDistance &&
                        bot->GetDistance2d(member) <= nearbyStackRange)
                    {
                        shouldMoveToStack = true;
                        break;
                    }
                }
            }
        }

        if (shouldMoveToStack)
        {
            return MoveTo(bot->GetMapId(), stackTarget->GetPositionX(), stackTarget->GetPositionY(),
                          stackTarget->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }

        if (ooze && !PlayerbotAI::IsHeal(bot))
        {
            bot->SetTarget(ooze->GetGUID());
            bot->SetFacingToObject(ooze);
            if (bot->IsWithinRange(ooze, 25.0f))
            {
                Attack(ooze);
                return false;
            }
        }
    }

    return false;
}

bool IccPutricideVolatileOozeAction::MarkOozeWithSkull(Unit* ooze)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    constexpr uint8 skullIconId = RtiTargetValue::skullIndex;
    ObjectGuid skullGuid = group->GetTargetIcon(skullIconId);
    Unit* markedUnit = botAI->GetUnit(skullGuid);

    if (markedUnit && (!markedUnit->IsAlive() || (ooze && markedUnit != ooze)))
        group->SetTargetIcon(skullIconId, bot->GetGUID(), ObjectGuid::Empty);

    if (ooze && ooze->IsAlive() && (!skullGuid || !markedUnit))
        group->SetTargetIcon(skullIconId, bot->GetGUID(), ooze->GetGUID());

    return false;
}

Unit* IccPutricideVolatileOozeAction::FindAuraTarget()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        if (botAI->HasAura("Volatile Ooze Adhesive", member))
            return member;
    }

    return nullptr;
}

bool IccPutricideGasCloudAction::Execute(Event /*event*/)
{
    Unit* gasCloud = AI_VALUE2(Unit*, "find target", "gas cloud");
    if (!gasCloud)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return false;

    if (PlayerbotAI::IsTank(bot) &&
        bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE &&
        bot->GetExactDist2d(ICC_PUTRICIDE_TANK_POSITION) > 20.0f && !boss->HealthBelowPct(36) &&
        boss->GetVictim() == bot)
        return MoveTo(bot->GetMapId(), ICC_PUTRICIDE_TANK_POSITION.GetPositionX(),
                      ICC_PUTRICIDE_TANK_POSITION.GetPositionY(), ICC_PUTRICIDE_TANK_POSITION.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    bool hasGaseousBloat = botAI->HasAura("Gaseous Bloat", bot);
    Unit* volatileOoze = AI_VALUE2(Unit*, "find target", "volatile ooze");

    std::vector<Unit*> aliveGasCloud;
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == gasCloud->GetEntry())
            aliveGasCloud.push_back(unit);
    }

    if (aliveGasCloud.size() > 1)
    {
        for (size_t i = 0; i < aliveGasCloud.size() - 1; ++i)
            bot->Kill(bot, aliveGasCloud[i]);
    }

    if (!hasGaseousBloat && volatileOoze)
        return false;

    if (hasGaseousBloat)
        return HandleGaseousBloatMovement(gasCloud);

    return HandleGroupAuraSituation(gasCloud);
}

bool IccPutricideGasCloudAction::HandleGaseousBloatMovement(Unit* gasCloud)
{
    auto& g_bloatLastDir = IcecrownHelpers::IccState(bot->GetMap()->GetInstanceId()).ppBloatLastDir;

    if (!botAI->HasAura("Gaseous Bloat", bot))
    {
        g_bloatLastDir.erase(bot->GetGUID());
        return false;
    }

    // Lookup prior committed flee direction (if any) so we can reject
    // backtracking candidates this tick.
    ObjectGuid botKey = bot->GetGUID();
    auto lastDirIt = g_bloatLastDir.find(botKey);
    bool hasLastDir = lastDirIt != g_bloatLastDir.end();
    float lastDirX = hasLastDir ? lastDirIt->second.x : 0.0f;
    float lastDirY = hasLastDir ? lastDirIt->second.y : 0.0f;

    auto isBacktrack = [&](float candDx, float candDy) -> bool
    {
        if (!hasLastDir)
            return false;
        // Require forward progress: dot must be positive (>0). This still
        // allows up to ~89deg turns but blocks any move with a backward
        // component.
        return (candDx * lastDirX + candDy * lastDirY) <= 0.0f;
    };

    auto commitDir = [&](float fromX, float fromY, float toX, float toY)
    {
        float ddx = toX - fromX;
        float ddy = toY - fromY;
        float l = std::sqrt(ddx * ddx + ddy * ddy);
        if (l < 0.01f)
            return;
        g_bloatLastDir[botKey] = { ddx / l, ddy / l };
    };

    if (!bot->HasAura(SPELL_NITRO_BOOSTS))
        bot->AddAura(SPELL_NITRO_BOOSTS, bot);

    constexpr int numAngles = 32;
    constexpr float gasBombSafeDist = 6.0f;
    constexpr float movementIncrement = 5.0f;
    constexpr float maxTestDist = 30.0f;
    constexpr int checkDirs = 16;
    constexpr float checkDist = 8.0f;
    constexpr float minFreedomScore = 0.75f;

    Position botPos = bot->GetPosition();
    Position cloudPos = gasCloud->GetPosition();
    float cloudDist = gasCloud->GetExactDist2d(botPos);

    // Detect if bot is trapped near walls by testing 8 directions at 40yd.
    // Threshold for "corner-ish" is intentionally low so we trigger the
    // corner-escape branch before the bot fully parks against a wall.
    constexpr int cornerCheckDirs = 8;
    constexpr float cornerCheckDist = 40.0f;
    int blockedAtStart = 0;
    for (int i = 0; i < cornerCheckDirs; ++i)
    {
        float cA = (2.0f * static_cast<float>(M_PI) * i) / cornerCheckDirs;
        if (!bot->IsWithinLOS(botPos.GetPositionX() + std::cos(cA) * cornerCheckDist,
                              botPos.GetPositionY() + std::sin(cA) * cornerCheckDist, botPos.GetPositionZ()))
            blockedAtStart++;
    }

    // No distance early-exit: bloated bots must kite continuously because
    // the gas cloud actively chases them. Stopping even briefly lets it catch up.

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> gasBombs;
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_CHOKING_GAS_BOMB)
            gasBombs.push_back(unit);
    }

    float dx = botPos.GetPositionX() - cloudPos.GetPositionX();
    float dy = botPos.GetPositionY() - cloudPos.GetPositionY();
    float dist = std::max(0.1f, std::sqrt(dx * dx + dy * dy));
    dx /= dist;
    dy /= dist;

    Position bestPos;
    bool foundPath = false;
    float bestScore = 0.0f;

    // Corner escape: test all 32 angles at a fixed medium distance with strict requirements.
    // Trigger at >=3 blocked to catch edge-of-wall situations before parking.
    if (blockedAtStart >= 3)
    {
        constexpr float escRadius = 10.0f;
        for (int i = 0; i < 32; ++i)
        {
            float escAngle = (2.0f * static_cast<float>(M_PI) * i) / 32;
            float escX = botPos.GetPositionX() + std::cos(escAngle) * escRadius;
            float escY = botPos.GetPositionY() + std::sin(escAngle) * escRadius;
            float escZ = botPos.GetPositionZ();

            if (!bot->IsWithinLOS(escX, escY, escZ))
                continue;

            if (isBacktrack(escX - botPos.GetPositionX(), escY - botPos.GetPositionY()))
                continue;

            int freeDirs = 0;
            for (int j = 0; j < checkDirs; ++j)
            {
                float cA = (2.0f * static_cast<float>(M_PI) * j) / checkDirs;
                if (bot->IsWithinLOS(escX + std::cos(cA) * checkDist, escY + std::sin(cA) * checkDist, escZ))
                    freeDirs++;
            }

            // Count long-range openness (25f rays) — reject positions near walls.
            int farFreeDirs = 0;
            for (int k = 0; k < 8; ++k)
            {
                float cA = (2.0f * static_cast<float>(M_PI) * k) / 8;
                if (bot->IsWithinLOS(escX + std::cos(cA) * 25.0f, escY + std::sin(cA) * 25.0f, escZ))
                    farFreeDirs++;
            }

            float escCloudDist = cloudPos.GetExactDist2d(escX, escY);

            // Must be moving away, open locally, and open at range.
            if (escCloudDist > cloudDist && freeDirs >= 12 && farFreeDirs >= 6)
            {
                commitDir(botPos.GetPositionX(), botPos.GetPositionY(), escX, escY);
                botAI->Reset();
                return MoveTo(bot->GetMapId(), escX, escY, escZ, false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT);
            }
        }

        // Hard-corner relief: no strict candidate found. Re-scan and pick the
        // angle with the most local openness regardless of cloud direction —
        // even briefly stepping toward the cloud is preferable to standing
        // still in a wedge while it walks into us.
        int bestRelOpen = -1;
        float bestRelX = 0.0f, bestRelY = 0.0f, bestRelZ = botPos.GetPositionZ();
        for (int i = 0; i < 32; ++i)
        {
            float escAngle = (2.0f * static_cast<float>(M_PI) * i) / 32;
            float escX = botPos.GetPositionX() + std::cos(escAngle) * escRadius;
            float escY = botPos.GetPositionY() + std::sin(escAngle) * escRadius;
            float escZ = botPos.GetPositionZ();

            if (!bot->IsWithinLOS(escX, escY, escZ))
                continue;

            if (isBacktrack(escX - botPos.GetPositionX(), escY - botPos.GetPositionY()))
                continue;

            int openCount = 0;
            for (int j = 0; j < checkDirs; ++j)
            {
                float cA = (2.0f * static_cast<float>(M_PI) * j) / checkDirs;
                if (bot->IsWithinLOS(escX + std::cos(cA) * checkDist,
                                     escY + std::sin(cA) * checkDist, escZ))
                    openCount++;
            }
            if (openCount > bestRelOpen)
            {
                bestRelOpen = openCount;
                bestRelX = escX;
                bestRelY = escY;
                bestRelZ = escZ;
            }
        }
        if (bestRelOpen >= 0)
        {
            commitDir(botPos.GetPositionX(), botPos.GetPositionY(), bestRelX, bestRelY);
            botAI->Reset();
            return MoveTo(bot->GetMapId(), bestRelX, bestRelY, bestRelZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT);
        }
    }

    // Normal search: scan 32 angles × distance increments, score each valid position.
    for (int i = 0; i < numAngles; ++i)
    {
        float angle = (2.0f * static_cast<float>(M_PI) * i) / numAngles;
        float rotatedDx = dx * std::cos(angle) - dy * std::sin(angle);
        float rotatedDy = dx * std::sin(angle) + dy * std::cos(angle);

        for (float testDist = movementIncrement; testDist <= maxTestDist; testDist += movementIncrement)
        {
            float testX = botPos.GetPositionX() + rotatedDx * testDist;
            float testY = botPos.GetPositionY() + rotatedDy * testDist;
            float testZ = botPos.GetPositionZ();

            float newCloudDist = cloudPos.GetExactDist2d(testX, testY);

            // Reject positions on the far side of the cloud (through it).
            // The move direction must point generally away from the cloud.
            float toTestX = testX - botPos.GetPositionX();
            float toTestY = testY - botPos.GetPositionY();
            if (toTestX * dx + toTestY * dy <= 0.0f)
                continue;

            // Anti-backtrack: don't pick a candidate that walks against the
            // direction we already committed to this Bloat session.
            if (isBacktrack(toTestX, toTestY))
                continue;

            float minGasBombDist = FLT_MAX;
            for (Unit* bomb : gasBombs)
            {
                float bombDist = bomb->GetDistance2d(testX, testY);
                if (bombDist < minGasBombDist)
                    minGasBombDist = bombDist;
            }

            if (newCloudDist > cloudDist && minGasBombDist >= gasBombSafeDist &&
                bot->IsWithinLOS(testX, testY, testZ))
            {
                int freeDirections = 0;
                for (int j = 0; j < checkDirs; ++j)
                {
                    float checkAngle = (2.0f * static_cast<float>(M_PI) * j) / checkDirs;
                    float checkX = testX + std::cos(checkAngle) * checkDist;
                    float checkY = testY + std::sin(checkAngle) * checkDist;
                    if (bot->IsWithinLOS(checkX, checkY, testZ))
                        freeDirections++;
                }

                float freedomScore = static_cast<float>(freeDirections) / static_cast<float>(checkDirs);
                if (freedomScore < minFreedomScore)
                    continue;

                // Long-range corner check: 8 rays at 25f. If too many blocked, skip.
                int farFreeDirs = 0;
                for (int k = 0; k < 8; ++k)
                {
                    float cA = (2.0f * static_cast<float>(M_PI) * k) / 8;
                    if (bot->IsWithinLOS(testX + std::cos(cA) * 25.0f, testY + std::sin(cA) * 25.0f, testZ))
                        farFreeDirs++;
                }
                if (farFreeDirs < 5)
                    continue;

                // Score: cloud distance, freedom, gas bomb distance, continuity.
                bool canContinueMoving = false;
                {
                    float continueX = testX + rotatedDx * movementIncrement;
                    float continueY = testY + rotatedDy * movementIncrement;
                    if (bot->IsWithinLOS(continueX, continueY, testZ) &&
                        farFreeDirs >= 6)
                        canContinueMoving = true;
                }

                float continuity = canContinueMoving ? 5.0f : 0.0f;
                float combinedScore = newCloudDist + (freedomScore * 15.0f) + minGasBombDist +
                                      static_cast<float>(farFreeDirs) * 2.0f + continuity;

                if (!foundPath || combinedScore > bestScore)
                {
                    bestPos = Position(testX, testY, testZ);
                    bestScore = combinedScore;
                    foundPath = true;
                }
            }
        }
    }

    if (foundPath)
    {
        commitDir(botPos.GetPositionX(), botPos.GetPositionY(),
                  bestPos.GetPositionX(), bestPos.GetPositionY());
        botAI->Reset();
        return MoveTo(bot->GetMapId(), bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }

    // Fallback pass: no candidate met the strict freedom/openness thresholds.
    // Relax requirements and pick the most-open position that still moves us
    // away from the cloud. Better to make imperfect progress than to stand
    // still while the cloud walks into us.
    float bestFallbackScore = -FLT_MAX;
    Position bestFallbackPos;
    bool foundFallback = false;
    for (int i = 0; i < numAngles; ++i)
    {
        float angle = (2.0f * static_cast<float>(M_PI) * i) / numAngles;
        float rotatedDx = dx * std::cos(angle) - dy * std::sin(angle);
        float rotatedDy = dx * std::sin(angle) + dy * std::cos(angle);

        for (float testDist = movementIncrement; testDist <= maxTestDist; testDist += movementIncrement)
        {
            float testX = botPos.GetPositionX() + rotatedDx * testDist;
            float testY = botPos.GetPositionY() + rotatedDy * testDist;
            float testZ = botPos.GetPositionZ();

            // Must be moving away from the cloud.
            float toTestX = testX - botPos.GetPositionX();
            float toTestY = testY - botPos.GetPositionY();
            if (toTestX * dx + toTestY * dy <= 0.0f)
                continue;

            if (isBacktrack(toTestX, toTestY))
                continue;

            float newCloudDist = cloudPos.GetExactDist2d(testX, testY);
            if (newCloudDist <= cloudDist)
                continue;

            if (!bot->IsWithinLOS(testX, testY, testZ))
                continue;

            // Gas bomb hard-reject only.
            float minGasBombDist = FLT_MAX;
            for (Unit* bomb : gasBombs)
            {
                float bombDist = bomb->GetDistance2d(testX, testY);
                if (bombDist < minGasBombDist)
                    minGasBombDist = bombDist;
            }
            if (minGasBombDist < gasBombSafeDist)
                continue;

            // Score by cloud distance + local openness, no hard threshold.
            int freeDirections = 0;
            for (int j = 0; j < checkDirs; ++j)
            {
                float checkAngle = (2.0f * static_cast<float>(M_PI) * j) / checkDirs;
                if (bot->IsWithinLOS(testX + std::cos(checkAngle) * checkDist,
                                     testY + std::sin(checkAngle) * checkDist, testZ))
                    freeDirections++;
            }

            float score = newCloudDist + static_cast<float>(freeDirections) * 2.0f;
            if (score > bestFallbackScore)
            {
                bestFallbackScore = score;
                bestFallbackPos = Position(testX, testY, testZ);
                foundFallback = true;
            }
        }
    }

    if (foundFallback)
    {
        commitDir(botPos.GetPositionX(), botPos.GetPositionY(),
                  bestFallbackPos.GetPositionX(), bestFallbackPos.GetPositionY());
        botAI->Reset();
        return MoveTo(bot->GetMapId(), bestFallbackPos.GetPositionX(), bestFallbackPos.GetPositionY(),
                      bestFallbackPos.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    // Last resort: move directly away from the cloud ignoring everything.
    // Better to slide along a wall than to stand and eat it.
    float lastX = botPos.GetPositionX() + dx * movementIncrement;
    float lastY = botPos.GetPositionY() + dy * movementIncrement;
    commitDir(botPos.GetPositionX(), botPos.GetPositionY(), lastX, lastY);
    botAI->Reset();
    return MoveTo(bot->GetMapId(), lastX, lastY, botPos.GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool IccPutricideGasCloudAction::HandleGroupAuraSituation(Unit* gasCloud)
{
    Group* group = bot->GetGroup();
    if (!group || PlayerbotAI::IsHeal(bot))
        return false;

    constexpr float rangeMinSafeDistance = 15.0f;
    constexpr uint8 skullIconId = RtiTargetValue::skullIndex;

    Unit* volatileOoze = AI_VALUE2(Unit*, "find target", "volatile ooze");
    if ((!volatileOoze || !volatileOoze->IsAlive()) && gasCloud && gasCloud->IsAlive())
    {
        ObjectGuid currentSkull = group->GetTargetIcon(skullIconId);
        Unit* markedUnit = botAI->GetUnit(currentSkull);
        if (!markedUnit || !markedUnit->IsAlive() || markedUnit != gasCloud)
            group->SetTargetIcon(skullIconId, bot->GetGUID(), gasCloud->GetGUID());
    }

    float currentDist = gasCloud ? bot->GetDistance(gasCloud) : 0.0f;

    if (!GroupHasGaseousBloat(group) && gasCloud && currentDist < rangeMinSafeDistance)
    {
        float dx = bot->GetPositionX() - gasCloud->GetPositionX();
        float dy = bot->GetPositionY() - gasCloud->GetPositionY();
        float dist = std::max(0.1f, std::sqrt(dx * dx + dy * dy));
        dx /= dist;
        dy /= dist;

        float step = std::min(5.0f, rangeMinSafeDistance - currentDist);
        return MoveTo(bot->GetMapId(), bot->GetPositionX() + dx * step, bot->GetPositionY() + dy * step,
                      bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool IccPutricideGasCloudAction::GroupHasGaseousBloat(Group* group)
{
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && botAI->HasAura("Gaseous Bloat", member))
            return true;
    }
    return false;
}

bool IccPutricideAvoidMalleableGooAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return false;

    if (PlayerbotAI::IsTank(bot) &&
        bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        bot->AttackStop();
        bot->CastStop();
        if (bot->GetTarget())
            bot->SetTarget(ObjectGuid::Empty);
        if (Unit* master = botAI->GetMaster())
            Follow(master);
        return true;
    }

    // Reactive avoidance: find a position safe from ALL active Malleable Goo
    // impact points (boss casts up to 3 simultaneously and they may be near
    // each other - fleeing one can land in another). We sample a ring of
    // candidates around the bot and score by minimum distance to every active
    // hazard (goo impacts, ooze puddles, choking gas bombs). The best-scoring
    // candidate that clears the danger radius for all goos is chosen.
    // Only active in phase 1/2 (>35% HP) - in phase 3 bots must stack on the
    // boss for Mutated Plague healing.
    if (!boss->HealthBelowPct(35))
    {
        constexpr float gooDangerRadius = 8.0f;   // 5yd AoE + 3yd safety
        constexpr float puddleAvoidRadius = 6.0f;

        float botX = bot->GetPositionX();
        float botY = bot->GetPositionY();
        float botZ = bot->GetPositionZ();

        std::vector<Position> goos = IcecrownHelpers::ActiveGooPositions(bot->GetMap()->GetInstanceId(), 6000);
        bool botInDanger = false;
        for (Position const& goo : goos)
        {
            float dx = botX - goo.GetPositionX();
            float dy = botY - goo.GetPositionY();
            if (dx * dx + dy * dy < gooDangerRadius * gooDangerRadius)
                botInDanger = true;
        }

        if (botInDanger)
        {
            // Gather puddle / bomb obstacle positions to avoid in scoring
            std::vector<Position> obstacles;
            std::list<Creature*> nearbyUnits;
            bot->GetCreatureListWithEntryInGrid(nearbyUnits, NPC_GROWING_OOZE_PUDDLE, 40.0f);
            for (Creature* c : nearbyUnits)
                if (c && c->IsAlive())
                    obstacles.push_back(c->GetPosition());
            nearbyUnits.clear();
            bot->GetCreatureListWithEntryInGrid(nearbyUnits, NPC_CHOKING_GAS_BOMB, 40.0f);
            for (Creature* c : nearbyUnits)
                if (c && c->IsAlive())
                    obstacles.push_back(c->GetPosition());

            Position fromPos = bot->GetPosition();
            float bestScore = -1.0f;
            float bestX = botX, bestY = botY;
            bool found = false;

            // Sample ring: multiple radii × multiple angles. Prefer minimum
            // displacement (closer rings tried first via scoring bias).
            constexpr int angleSteps = 24;
            float const radii[] = {8.0f, 11.0f, 14.0f, 17.0f};
            for (float r : radii)
            {
                for (int i = 0; i < angleSteps; ++i)
                {
                    float a = (2.0f * float(M_PI) * i) / angleSteps;
                    float cx = botX + std::cos(a) * r;
                    float cy = botY + std::sin(a) * r;
                    Position toPos(cx, cy, botZ);

                    // Must clear every goo's danger radius
                    float minGooDistSq = std::numeric_limits<float>::max();
                    bool safe = true;
                    for (Position const& g : goos)
                    {
                        float gdx = cx - g.GetPositionX();
                        float gdy = cy - g.GetPositionY();
                        float d2 = gdx * gdx + gdy * gdy;
                        if (d2 < gooDangerRadius * gooDangerRadius)
                        {
                            safe = false;
                            break;
                        }
                        if (d2 < minGooDistSq)
                            minGooDistSq = d2;
                    }
                    if (!safe)
                        continue;

                    // Reachability checks
                    if (!bot->IsWithinLOS(cx, cy, botZ))
                        continue;
                    if (HasObstacleBetween(fromPos, toPos))
                        continue;

                    // Score = min distance to nearest hazard at the candidate
                    // (goo, puddle, bomb), penalize travel distance lightly.
                    float minHazard = std::sqrt(minGooDistSq);
                    for (Position const& o : obstacles)
                    {
                        float odx = cx - o.GetPositionX();
                        float ody = cy - o.GetPositionY();
                        float od = std::sqrt(odx * odx + ody * ody);
                        // Treat obstacles as having a minimum stand-off
                        float effective = od - (puddleAvoidRadius - 4.0f);
                        if (effective < minHazard)
                            minHazard = effective;
                    }

                    float travel = std::sqrt((cx - botX) * (cx - botX) +
                                             (cy - botY) * (cy - botY));
                    float score = minHazard - travel * 0.1f;

                    if (score > bestScore)
                    {
                        bestScore = score;
                        bestX = cx;
                        bestY = cy;
                        found = true;
                    }
                }
                // If we found a clean candidate at this radius, stop expanding
                if (found)
                    break;
            }

            if (found)
            {
                return MoveTo(bot->GetMapId(), bestX, bestY, botZ, false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }

    if (HandleTankPositioning(boss))
        return false;

    if (AI_VALUE2(Unit*, "find target", "volatile ooze") || AI_VALUE2(Unit*, "find target", "gas cloud"))
        return false;

    if (HandleUnboundPlague(boss))
        return false;

    // Only stack at boss in phase 3 (<=35% HP). Above 35% bots may stand
    // anywhere; the goo flee block above handles emergencies.
    if (!boss->HealthBelowPct(35))
        return false;

    return HandleBossPositioning(boss);
}

bool IccPutricideAvoidMalleableGooAction::HandleTankPositioning(Unit* boss)
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (boss && boss->IsAlive() &&
        !AI_VALUE2(Unit*, "find target", "volatile ooze") && !AI_VALUE2(Unit*, "find target", "gas cloud"))
    {
        if (Group* group = bot->GetGroup())
        {
            constexpr uint8 skullIconId = RtiTargetValue::skullIndex;
            ObjectGuid skullGuid = group->GetTargetIcon(skullIconId);
            Unit* markedUnit = botAI->GetUnit(skullGuid);
            if (!skullGuid || !markedUnit || !markedUnit->IsAlive())
                group->SetTargetIcon(skullIconId, bot->GetGUID(), boss->GetGUID());
        }
    }

    constexpr float bombSearchRange = 100.0f;
    constexpr float safeDistance = 15.0f;

    Unit* bomb = bot->FindNearestCreature(NPC_CHOKING_GAS_BOMB, bombSearchRange);
    if (!bomb)
        return false;

    float currentDistance = bot->GetDistance2d(bomb);

    if (currentDistance < safeDistance)
        return MoveAway(bomb, safeDistance - currentDistance);

    return false;
}

bool IccPutricideAvoidMalleableGooAction::HandleUnboundPlague(Unit* boss)
{
    if (boss && boss->HealthBelowPct(35))
        return false;

    if (!botAI->HasAura("Unbound Plague", bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    constexpr float unboundPlagueDistance = 20.0f;
    constexpr float unboundPlagueBuffer = 2.0f;
    float closestDistance = unboundPlagueDistance;
    Unit* closestPlayer = nullptr;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        float dist = bot->GetDistance2d(member);
        if (dist < closestDistance)
        {
            closestDistance = dist;
            closestPlayer = member;
        }
    }

    if (!closestPlayer || closestDistance >= unboundPlagueDistance)
    {
        bot->Kill(bot, bot);
        return true;
    }

    float dx = bot->GetPositionX() - closestPlayer->GetPositionX();
    float dy = bot->GetPositionY() - closestPlayer->GetPositionY();
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist <= 0.0f)
        return false;

    dx /= dist;
    dy /= dist;
    float moveDistance = unboundPlagueDistance - closestDistance + unboundPlagueBuffer;

    float moveX = bot->GetPositionX() + dx * moveDistance;
    float moveY = bot->GetPositionY() + dy * moveDistance;

    if (bot->IsWithinLOS(moveX, moveY, bot->GetPositionZ()))
    {
        return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool IccPutricideAvoidMalleableGooAction::HandleBossPositioning(Unit* boss)
{
    if (PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* mainTank = AI_VALUE(Unit*, "main tank");
    if (!mainTank || !mainTank->IsAlive())
        return false;

    constexpr float stackTolerance = 2.0f;
    constexpr float hunterMaxDistance = 12.0f;
    constexpr float frontalConeHalfAngle = float(M_PI) / 3.0f;  // 60 degrees -> 120 deg cone

    float distToTank = bot->GetExactDist2d(mainTank);

    if (bot->getClass() == CLASS_HUNTER)
    {
        // Hunter slack: stay up to 12f from main tank, but only if standing in
        // the boss's frontal cone. Outside the cone or beyond 12f -> move to MT.
        float bossOrient = boss->GetOrientation();
        float toBotX = bot->GetPositionX() - boss->GetPositionX();
        float toBotY = bot->GetPositionY() - boss->GetPositionY();
        float toBotLen = std::sqrt(toBotX * toBotX + toBotY * toBotY);
        bool inFrontCone = false;
        if (toBotLen > 0.01f)
        {
            float bearing = std::atan2(toBotY, toBotX);
            float delta = bearing - bossOrient;
            while (delta > float(M_PI)) delta -= 2.0f * float(M_PI);
            while (delta < -float(M_PI)) delta += 2.0f * float(M_PI);
            inFrontCone = std::fabs(delta) <= frontalConeHalfAngle;
        }

        if (distToTank <= hunterMaxDistance && inFrontCone)
            return false;
    }
    else if (distToTank <= stackTolerance)
    {
        return false;
    }

    bot->SetFacingToObject(boss);
    return MoveTo(bot->GetMapId(), mainTank->GetPositionX(), mainTank->GetPositionY(), mainTank->GetPositionZ(),
                  false, false, false, PlayerbotAI::IsRanged(bot), MovementPriority::MOVEMENT_COMBAT);
}

bool IccPutricideAvoidMalleableGooAction::HasObstacleBetween(Position const& from, Position const& to)
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_GROWING_OOZE_PUDDLE || unit->GetEntry() == NPC_CHOKING_GAS_BOMB)
        {
            if (IsOnPath(from, to, unit->GetPosition(), 3.0f))
                return true;
        }
    }
    return false;
}

bool IccPutricideAvoidMalleableGooAction::IsOnPath(Position const& from, Position const& to, Position const& point,
                                                   float threshold)
{
    float pathX = to.GetPositionX() - from.GetPositionX();
    float pathY = to.GetPositionY() - from.GetPositionY();
    float pathLen = std::sqrt(pathX * pathX + pathY * pathY);

    if (pathLen < 0.1f)
        return false;

    float normX = pathX / pathLen;
    float normY = pathY / pathLen;

    float toPointX = point.GetPositionX() - from.GetPositionX();
    float toPointY = point.GetPositionY() - from.GetPositionY();
    float proj = toPointX * normX + toPointY * normY;

    if (proj < 0.0f || proj > pathLen)
        return false;

    float closestX = from.GetPositionX() + normX * proj;
    float closestY = from.GetPositionY() + normY * proj;
    float distToPath = std::sqrt((point.GetPositionX() - closestX) * (point.GetPositionX() - closestX) +
                                 (point.GetPositionY() - closestY) * (point.GetPositionY() - closestY));

    return distToPath < threshold;
}

bool IccPutricideAbominationAction::IsSomeoneAlreadyPiloting()
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* m = itr->GetSource();
        if (!m || m == bot || !m->IsAlive())
            continue;
        if (Unit* vb = m->GetVehicleBase())
        {
            uint32 e = vb->GetEntry();
            if (e == NPC_MUTATED_ABOMINATION_10 || e == NPC_MUTATED_ABOMINATION_25)
                return true;
        }
    }
    return false;
}

Unit* IccPutricideAbominationAction::FindClosestPuddle(float maxRange)
{
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* best = nullptr;
    float bestDist = maxRange;
    for (auto const& g : npcs)
    {
        Unit* u = botAI->GetUnit(g);
        if (!u || !u->IsAlive() || u->GetEntry() != NPC_GROWING_OOZE_PUDDLE)
            continue;
        float d = bot->GetExactDist2d(u);
        if (d < bestDist)
        {
            bestDist = d;
            best = u;
        }
    }
    return best;
}

bool IccPutricideAbominationAction::BecomeAbomination()
{
    GameObject* go = bot->FindNearestGameObject(GO_PUTRICIDE_DRINK_ME, 100.0f);
    if (!go || !go->isSpawned())
        return false;

    float dist = bot->GetDistance(go);
    if (dist > INTERACTION_DISTANCE)
    {
        return MoveTo(bot->GetMapId(), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    bot->SetFacingToObject(go);
    bool hasAura = botAI->HasAura("Mutated Transformation", bot);
    if (!hasAura)
    {
        go->Use(bot);
        return true;
    }
    return false;
}

Unit* IccPutricideAbominationAction::PickSlashTarget(Unit* boss)
{
    Unit* volatileOoze = AI_VALUE2(Unit*, "find target", "volatile ooze");
    if (volatileOoze && volatileOoze->IsAlive())
        return volatileOoze;

    Unit* gasCloud = AI_VALUE2(Unit*, "find target", "gas cloud");
    if (gasCloud && gasCloud->IsAlive())
        return gasCloud;

    return boss;
}

bool IccPutricideAbominationAction::TryRegurgitate(Unit* abo, Unit* target)
{
    if (!abo || !target || !target->IsAlive())
        return false;

    if (abo->GetExactDist2d(target) > 50.0f)
        return false;

    if (botAI->HasAura("Regurgitated Ooze", target))
        return false;

    uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", "Regurgitated Ooze");
    if (!spellId)
        return false;

    if (abo->HasSpellCooldown(spellId))
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    if (!abo->HasInArc(CAST_ANGLE_IN_FRONT, target, 100.0f))
    {
        abo->SetFacingToObject(target);
        return false;
    }

    Spell* spell = new Spell(abo, spellInfo, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
    SpellCastTargets targets;
    targets.SetUnitTarget(target);
    spell->prepare(&targets);

    abo->AddSpellCooldown(spellId, 0, 1000);
    return true;
}

bool IccPutricideAbominationAction::TryEatOoze(Unit* abo, Unit* puddle)
{
    if (!abo || !puddle || !puddle->IsAlive())
        return false;

    constexpr float eatRange = 4.0f;
    if (abo->GetExactDist2d(puddle) > eatRange)
    {
        return MoveTo(bot->GetMapId(), puddle->GetPositionX(), puddle->GetPositionY(), puddle->GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }

    // In range of puddle — hold position until it dies, even on cooldown
    uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", "Eat Ooze");
    if (!spellId)
        return true;

    if (abo->HasSpellCooldown(spellId))
        return true;

    if (botAI->CanCastVehicleSpell(spellId, puddle) && botAI->CastVehicleSpell(spellId, puddle))
        abo->AddSpellCooldown(spellId, 0, 1000);

    return true;
}

bool IccPutricideAbominationAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsAssistTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return false;

    Unit* abo = bot->GetVehicleBase();
    bool piloting =
        abo && (abo->GetEntry() == NPC_MUTATED_ABOMINATION_10 || abo->GetEntry() == NPC_MUTATED_ABOMINATION_25);

    if (!piloting)
    {
        if (boss->HealthBelowPct(35))
            return false;
        if (IsSomeoneAlreadyPiloting())
            return false;
        Unit* nearestPuddle = FindClosestPuddle(50.0f);
        if (!nearestPuddle)
            return false;
        return BecomeAbomination();
    }

    Unit* volatileOoze = AI_VALUE2(Unit*, "find target", "volatile ooze");
    Unit* gasCloud = AI_VALUE2(Unit*, "find target", "gas cloud");
    Unit* puddle = FindClosestPuddle(100.0f);

    // Priority 1: Regurgitated Ooze on volatile ooze (aura-gated, energy bypassed)
    if (volatileOoze && volatileOoze->IsAlive() &&
        !botAI->HasAura("Regurgitated Ooze", volatileOoze))
    {
        if (TryRegurgitate(abo, volatileOoze))
            return true;
    }

    // Priority 2: Regurgitated Ooze on gas cloud (aura-gated, energy bypassed)
    if (gasCloud && gasCloud->IsAlive() &&
        !botAI->HasAura("Regurgitated Ooze", gasCloud))
    {
        if (TryRegurgitate(abo, gasCloud))
            return true;
    }

    // Priority 3: actively seek and eat puddle
    if (puddle)
    {
        if (TryEatOoze(abo, puddle))
            return true;
    }

    // Priority 4: slash target volatile > gas > boss
    Unit* slashTarget = PickSlashTarget(boss);
    if (slashTarget)
    {
        if (abo->GetExactDist2d(slashTarget) > 5.0f)
        {
            return MoveTo(bot->GetMapId(), slashTarget->GetPositionX(), slashTarget->GetPositionY(),
                          slashTarget->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }

        if (abo)
        {
            uint32 msSpellId = AI_VALUE2(uint32, "vehicle spell id", "Mutated Slash");
            if (msSpellId && !abo->HasSpellCooldown(msSpellId) && botAI->CanCastVehicleSpell(msSpellId, slashTarget) &&
                botAI->CastVehicleSpell(msSpellId, slashTarget))
            {
                abo->AddSpellCooldown(msSpellId, 0, 1000);
                return true;
            }
        }

        if (bot->GetTarget() != slashTarget->GetGUID())
            bot->SetTarget(slashTarget->GetGUID());
        Attack(slashTarget);
    }

    return true;
}
