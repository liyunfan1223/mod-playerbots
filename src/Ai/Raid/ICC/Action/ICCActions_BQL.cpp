/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericSpellActions.h"
#include "ICCActions.h"
#include "ICCTriggers.h"
#include "Multiplier.h"
#include "NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiValue.h"
#include "Vehicle.h"

bool IccBqlGroupPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss)
        return false;

    Aura* frenzyAura = botAI->GetAura("Frenzied Bloodthirst", bot);
    Aura* shadowAura = botAI->GetAura("Swarming Shadows", bot);
    bool isTank = PlayerbotAI::IsTank(bot);
    if (isTank && shadowAura)
    {
        bot->RemoveAurasDueToSpell(shadowAura->GetId());
        return true;
    }

    // Handle tank positioning
    if (isTank && HandleTankPosition(boss, frenzyAura, shadowAura))
        return true;

    // Handle swarming shadows movement
    if (shadowAura && HandleShadowsMovement())
        return true;

    // Handle group positioning
    if (!frenzyAura && !shadowAura && HandleGroupPosition(boss, frenzyAura, shadowAura))
        return true;

    return false;
}

bool IccBqlGroupPositionAction::HandleTankPosition(Unit* boss, Aura* frenzyAura, Aura* shadowAura)
{
    if (frenzyAura || shadowAura)
        return false;

    // Main tank positioning
    if (PlayerbotAI::IsMainTank(bot) && botAI->HasAggro(boss))
    {
        if (bot->GetExactDist2d(ICC_BQL_TANK_POSITION) > 3.0f)
        {
            MoveTo(bot->GetMapId(), ICC_BQL_TANK_POSITION.GetPositionX(), ICC_BQL_TANK_POSITION.GetPositionY(),
                   ICC_BQL_TANK_POSITION.GetPositionZ(), false, false, false, true,
                   MovementPriority::MOVEMENT_COMBAT);
        }
    }

    // Assist tank positioning
    if (PlayerbotAI::IsAssistTank(bot) && !botAI->GetAura("Blood Mirror", bot))
    {
        if (Unit* mainTank = AI_VALUE(Unit*, "main tank"))
        {
            MoveTo(bot->GetMapId(), mainTank->GetPositionX(), mainTank->GetPositionY(), mainTank->GetPositionZ(),
                   false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    if (PlayerbotAI::IsAssistTank(bot) && botAI->GetAura("Blood Mirror", bot) && boss && boss->HealthAbovePct(90))
        return true; // don't do anything to avoid taking bite

    return false;
}

bool IccBqlGroupPositionAction::HandleShadowsMovement()
{
    float const SAFE_SHADOW_DIST = 4.0f;
    float const ARC_STEP = 0.05f;
    float const LANE_STEP = 0.15f;
    int const MAX_CURVES = 3;
    float const maxClosestDist = PlayerbotAI::IsMelee(bot) ? 25.0f : 20.0f;
    Position const& center = ICC_BQL_CENTER_POSITION;
    float const OUTER_CURVE_PREFERENCE = 200.0f;   // Strong preference for outer curves
    float const CURVE_SWITCH_PENALTY = 50.0f;      // Penalty for switching curves
    float const DISTANCE_PENALTY_FACTOR = 100.0f;  // Penalty per yard moved from current position
    float const MAX_CURVE_JUMP_DIST = 12.0f;       // Maximum distance for jumping between curves

    // Track current curve to avoid unnecessary switching (keyed per-instance to avoid
    // cross-instance pollution when multiple ICCs run simultaneously)
    auto& botCurrentCurve = IcecrownHelpers::IccState(bot->GetInstanceId()).bqlBotCurrentCurve;
    ObjectGuid curveKey = bot->GetGUID();
    bool hasLane = botCurrentCurve.count(curveKey) != 0;
    int currentCurve = hasLane ? botCurrentCurve[curveKey] : 0;

    // Find closest wall path
    Position lwall[4] = {ICC_BQL_LWALL1_POSITION, AdjustControlPoint(ICC_BQL_LWALL2_POSITION, center, 1.30f),
                         AdjustControlPoint(ICC_BQL_LWALL3_POSITION, center, 1.30f), ICC_BQL_LRWALL4_POSITION};
    Position rwall[4] = {ICC_BQL_RWALL1_POSITION, AdjustControlPoint(ICC_BQL_RWALL2_POSITION, center, 1.30f),
                         AdjustControlPoint(ICC_BQL_RWALL3_POSITION, center, 1.30f), ICC_BQL_LRWALL4_POSITION};
    Position* basePath = (bot->GetExactDist2d(lwall[0]) < bot->GetExactDist2d(rwall[0])) ? lwall : rwall;

    // Find all swarming shadows
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    constexpr int MAX_SHADOW_NPCS = 100;
    Unit* shadows[MAX_SHADOW_NPCS]{};  // Reasonable max estimate
    int shadowCount = 0;
    for (uint32 i = 0; i < npcs.size() && shadowCount < MAX_SHADOW_NPCS; i++)
    {
        Unit* unit = botAI->GetUnit(npcs[i]);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_SWARMING_SHADOWS)
            shadows[shadowCount++] = unit;
    }

    // Helper lambda to check if a position is inside a shadow
    auto IsPositionInShadow = [&](Position const& pos) -> bool
    {
        for (int i = 0; i < shadowCount; ++i)
        {
            if (pos.GetExactDist2d(shadows[i]) < SAFE_SHADOW_DIST)
                return true;
        }
        return false;
    };

    CurveInfo bestCurve;
    bestCurve.foundSafe = false;
    bestCurve.score = FLT_MAX;
    bool foundCurve = false;

    // Keep track of information about all curves for possible fallback
    CurveInfo curveInfos[MAX_CURVES];
    for (int i = 0; i < MAX_CURVES; i++)
    {
        curveInfos[i].foundSafe = false;
        curveInfos[i].score = FLT_MAX;
    }

    // Evaluate all curves starting from outermost (lowest index)
    for (int curveIdx = 0; curveIdx < MAX_CURVES; curveIdx++)
    {
        float laneScale = 1.0f - float(curveIdx) * LANE_STEP;
        Position path[4] = {AdjustControlPoint(basePath[0], center, laneScale),
                            AdjustControlPoint(basePath[1], center, laneScale),
                            AdjustControlPoint(basePath[2], center, laneScale),
                            AdjustControlPoint(basePath[3], center, laneScale)};

        // Find closest point on curve
        float minDist = 9999.0f;
        float t_closest = 0.0f;
        Position closestPoint = path[0];

        for (float t = 0.0f; t <= 1.0f; t += ARC_STEP)
        {
            Position pt = CalculateBezierPoint(t, path);
            float dist = bot->GetExactDist2d(pt);
            if (dist < minDist)
            {
                minDist = dist;
                t_closest = t;
                closestPoint = pt;
            }
        }

        // Check if the closest point is safe
        bool closestIsSafe = !IsPositionInShadow(closestPoint);

        // Find closest safe point by searching in both directions from closest point
        Position safeMoveTarget = closestPoint;
        bool foundSafe = closestIsSafe;

        // Only search for safe spots if the closest point isn't already safe
        if (!closestIsSafe)
        {
            // Find the nearest safe point along the curve, not by direct distance
            // but by distance along the curve from the closest point

            // Search forward on curve from closest point
            float forwardT = -1.0f;
            Position forwardPt;
            for (float t = t_closest + ARC_STEP; t <= 1.0f; t += ARC_STEP)
            {
                Position pt = CalculateBezierPoint(t, path);
                if (!IsPositionInShadow(pt))
                {
                    forwardT = t;
                    forwardPt = pt;
                    break;
                }
            }

            // Search backward on curve from closest point
            float backwardT = -1.0f;
            Position backwardPt;
            for (float t = t_closest - ARC_STEP; t >= 0.0f; t -= ARC_STEP)
            {
                Position pt = CalculateBezierPoint(t, path);
                if (!IsPositionInShadow(pt))
                {
                    backwardT = t;
                    backwardPt = pt;
                    break;
                }
            }

            // Choose the closest safe point based on curve distance, not direct distance
            if (forwardT >= 0 && backwardT >= 0)
            {
                // Both directions have safe points, choose the closer one by curve distance
                if (std::abs(forwardT - t_closest) < std::abs(backwardT - t_closest))
                {
                    safeMoveTarget = forwardPt;
                    foundSafe = true;
                }
                else
                {
                    safeMoveTarget = backwardPt;
                    foundSafe = true;
                }
            }
            else if (forwardT >= 0)
            {
                safeMoveTarget = forwardPt;
                foundSafe = true;
            }
            else if (backwardT >= 0)
            {
                safeMoveTarget = backwardPt;
                foundSafe = true;
            }
        }

        // Score this curve
        float distancePenalty = 0.0f;
        float score = 0.0f;

        if (foundSafe)
        {
            // If we found a safe point, penalize based on travel distance along the curve to reach it
            float safeDist = bot->GetExactDist2d(safeMoveTarget);

            // Add distance penalty based on how far we need to move along the curve
            distancePenalty = safeDist * (1.0f / DISTANCE_PENALTY_FACTOR);
            score = safeDist + distancePenalty;
        }
        else
        {
            // No safe point found, assign a high score
            distancePenalty = minDist * (1.0f / DISTANCE_PENALTY_FACTOR);
            score = minDist + distancePenalty + 1000.0f;  // Penalty for unsafe position
        }

        // Apply strong penalty for curves that are too far
        if (minDist > maxClosestDist)
            score += 500.0f;

        // Apply penalty for unsafe curves
        if (!foundSafe)
            score += 1000.0f;

        // Apply curve index preference (strongly prefer outer curves)
        score += curveIdx * OUTER_CURVE_PREFERENCE;

        // Apply curve switching penalty
        if (hasLane && curveIdx != currentCurve)
            score += CURVE_SWITCH_PENALTY;

        // MORE IMPORTANT: Apply additional curve switching penalty if the bot is far away
        // from the target curve (prevent jumping between curves when far away)
        if (curveIdx != currentCurve && minDist > MAX_CURVE_JUMP_DIST)
            score += 2000.0f;  // Strong penalty to prevent jumping between curves

        // Store this curve's info
        curveInfos[curveIdx].moveTarget = foundSafe ? safeMoveTarget : closestPoint;
        curveInfos[curveIdx].foundSafe = foundSafe;
        curveInfos[curveIdx].minDist = minDist;
        curveInfos[curveIdx].curveIdx = curveIdx;
        curveInfos[curveIdx].score = score;
        curveInfos[curveIdx].closestPoint = closestPoint;
        curveInfos[curveIdx].t_closest = t_closest;

        // Only update if this curve is better than our current best
        if (!foundCurve || score < bestCurve.score)
        {
            bestCurve = curveInfos[curveIdx];
            foundCurve = true;
        }
    }

    // Fallback: If we're trying to switch to a far curve and we're not near any curve,
    // find and use the closest curve instead of making a direct beeline
    if (foundCurve && bestCurve.minDist > MAX_CURVE_JUMP_DIST && bestCurve.curveIdx != currentCurve)
    {
        // Look for the closest curve first
        float closestDist = FLT_MAX;
        int closestCurveIdx = -1;

        for (int i = 0; i < MAX_CURVES; i++)
        {
            if (curveInfos[i].minDist < closestDist)
            {
                closestDist = curveInfos[i].minDist;
                closestCurveIdx = i;
            }
        }

        // If we found a closer curve, use that instead
        if (closestCurveIdx >= 0 && closestCurveIdx != bestCurve.curveIdx)
        {
            bestCurve = curveInfos[closestCurveIdx];
        }
    }

    // Remember the selected curve for next time
    if (foundCurve)
    {
        botCurrentCurve[curveKey] = bestCurve.curveIdx;
    }

    // Every lane blocked and standing in a shadow - escape to room center
    if (foundCurve && !bestCurve.foundSafe && IsPositionInShadow(bot->GetPosition()))
    {
        MoveTo(bot->GetMapId(), center.GetPositionX(), center.GetPositionY(), center.GetPositionZ(),
               false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
        return false;
    }

    // Create a move plan to guide the bot along the curve if necessary
    if (foundCurve && bot->GetExactDist2d(bestCurve.moveTarget) > 1.0f)
    {
        // Final check: ensure we're not moving into a shadow
        if (!IsPositionInShadow(bestCurve.moveTarget))
        {
            float laneScale = 1.0f - float(bestCurve.curveIdx) * LANE_STEP;
            Position path[4] = {AdjustControlPoint(basePath[0], center, laneScale),
                                AdjustControlPoint(basePath[1], center, laneScale),
                                AdjustControlPoint(basePath[2], center, laneScale),
                                AdjustControlPoint(basePath[3], center, laneScale)};

            // CRITICAL CHANGE: First check if we need to move to the curve
            float distToClosestPoint = bot->GetExactDist2d(bestCurve.closestPoint);

            // If we're not on the curve yet, first move to the closest point on the curve
            if (distToClosestPoint > 2.0f)
            {
                botAI->Reset();
                return MoveTo(bot->GetMapId(), bestCurve.closestPoint.GetPositionX(),
                              bestCurve.closestPoint.GetPositionY(), bestCurve.closestPoint.GetPositionZ(), false,
                              false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
            }

            // Now we know we're on or very close to the curve, so we'll follow it properly

            // Find target point on curve (t_target parameter)
            float t_target = 0.0f;
            float targetMinDist = 9999.0f;

            for (float t = 0.0f; t <= 1.0f; t += ARC_STEP)
            {
                Position pt = CalculateBezierPoint(t, path);
                float dist = bestCurve.moveTarget.GetExactDist2d(pt);
                if (dist < targetMinDist)
                {
                    targetMinDist = dist;
                    t_target = t;
                }
            }

            // Find an intermediate point along the curve between closest and target
            float t_step = (t_target > bestCurve.t_closest) ? ARC_STEP : -ARC_STEP;
            float t_intermediate = bestCurve.t_closest + t_step;
            Position intermediateTarget;
            bool foundValidIntermediate = false;

            // Limit the distance we move along the curve in one step
            float const MAX_CURVE_MOVEMENT = 7.0f;  // Max yards to move along curve
            float curveDistanceMoved = 0.0f;
            Position lastPos = bestCurve.closestPoint;

            while ((t_step > 0 && t_intermediate <= t_target) || (t_step < 0 && t_intermediate >= t_target))
            {
                Position pt = CalculateBezierPoint(t_intermediate, path);

                // Check if this point is safe
                if (!IsPositionInShadow(pt))
                {
                    // Calculate distance moved along curve so far
                    curveDistanceMoved += lastPos.GetExactDist2d(pt);
                    lastPos = pt;

                    // If we've moved the maximum allowed distance, use this position
                    if (curveDistanceMoved >= MAX_CURVE_MOVEMENT)
                    {
                        intermediateTarget = pt;
                        foundValidIntermediate = true;
                        break;
                    }

                    // Otherwise, continue moving along the curve
                    intermediateTarget = pt;
                    foundValidIntermediate = true;
                }
                else
                {
                    // We've hit a shadow, stop here
                    break;
                }

                t_intermediate += t_step;
            }

            // If we found a valid intermediate point, use it
            if (foundValidIntermediate)
            {
                botAI->Reset();
                MoveTo(bot->GetMapId(), intermediateTarget.GetPositionX(), intermediateTarget.GetPositionY(),
                       intermediateTarget.GetPositionZ(), false, false, false, true,
                       MovementPriority::MOVEMENT_FORCED, true, false);
            }
            else
            {
                botAI->Reset();
                // Fallback to direct movement to the target point on the curve
                MoveTo(bot->GetMapId(), bestCurve.moveTarget.GetPositionX(), bestCurve.moveTarget.GetPositionY(),
                       bestCurve.moveTarget.GetPositionZ(), false, false, false, true,
                       MovementPriority::MOVEMENT_FORCED, true, false);
            }
        }
    }

    return false;
}

Position IccBqlGroupPositionAction::AdjustControlPoint(Position const& wall, Position const& center, float factor)
{
    float dx = wall.GetPositionX() - center.GetPositionX();
    float dy = wall.GetPositionY() - center.GetPositionY();
    float dz = wall.GetPositionZ() - center.GetPositionZ();
    return Position(center.GetPositionX() + dx * factor, center.GetPositionY() + dy * factor,
                    center.GetPositionZ() + dz * factor);
}

Position IccBqlGroupPositionAction::CalculateBezierPoint(float t, Position const path[4])
{
    float omt = 1 - t;
    float omt2 = omt * omt;
    float omt3 = omt2 * omt;
    float t2 = t * t;
    float t3 = t2 * t;

    float x = omt3 * path[0].GetPositionX() + 3 * omt2 * t * path[1].GetPositionX() +
              3 * omt * t2 * path[2].GetPositionX() + t3 * path[3].GetPositionX();

    float y = omt3 * path[0].GetPositionY() + 3 * omt2 * t * path[1].GetPositionY() +
              3 * omt * t2 * path[2].GetPositionY() + t3 * path[3].GetPositionY();

    float z = omt3 * path[0].GetPositionZ() + 3 * omt2 * t * path[1].GetPositionZ() +
              3 * omt * t2 * path[2].GetPositionZ() + t3 * path[3].GetPositionZ();

    return Position(x, y, z);
}

bool IccBqlGroupPositionAction::HandleGroupPosition(Unit* boss, Aura* frenzyAura, Aura* shadowAura)
{
    if (frenzyAura || shadowAura)
        return false;

    GuidVector members = AI_VALUE(GuidVector, "group members");
    bool isRanged = PlayerbotAI::IsRanged(bot);
    bool isTank = PlayerbotAI::IsTank(bot);
    bool isMeleeDps = PlayerbotAI::IsMelee(bot) && !isTank;

    // Air-phase latch: only arm once boss has been anchored at tank pos (ground phase
    // established). Prevents false-trigger at pull when boss comes near center.
    // Disarm when boss returns to tank pos (ground phase resumed after landing).
    // Keyed per-instance so concurrent ICC raids don't share the latch.
    // Armed when boss lands from air, disarmed when boss returns to tank pos.
    // While armed, bots skip the pre-air center stack so they don't bunch up at center
    // during the post-air walk-back and die to lingering AoE.
    uint32 instanceId = bot->GetInstanceId();
    IcecrownHelpers::IccInstanceState& bqlState = IcecrownHelpers::IccState(instanceId);
    bool& groundPhaseEstablished = bqlState.bqlGroundPhaseEstablished;
    bool& wasAirborne = bqlState.bqlBossWasAirborne;
    bool& postAirLanding = bqlState.bqlPostAirLanding;
    float bossFromTank = boss->GetExactDist2d(ICC_BQL_TANK_POSITION);
    float bossFromCenter = boss->GetExactDist2d(ICC_BQL_CENTER_POSITION);
    bool bossAirborne = (boss->GetPositionZ() - ICC_BQL_CENTER_POSITION.GetPositionZ()) > 5.0f;

    // Landing edge: arm post-air latch
    if (wasAirborne && !bossAirborne)
        postAirLanding = true;
    wasAirborne = bossAirborne;

    if (!bossAirborne && bossFromTank < 10.0f)
    {
        groundPhaseEstablished = true;
        postAirLanding = false;
    }

    bool bossMovingToCenter = groundPhaseEstablished && !bossAirborne && !postAirLanding &&
                              bossFromCenter < 20.0f && bossFromTank > 10.0f;
    bool isAirPhase = bossAirborne || bossMovingToCenter;

    // Pre-airborne: nitro boost + move all bots to center (not ring slots yet)
    if (bossMovingToCenter)
    {
        float cx = ICC_BQL_CENTER_POSITION.GetPositionX();
        float cy = ICC_BQL_CENTER_POSITION.GetPositionY();
        float cz = ICC_BQL_CENTER_POSITION.GetPositionZ();
        if (bot->GetExactDist2d(cx, cy) > 2.0f && bot->IsWithinLOS(cx, cy, cz))
        {
            MoveTo(bot->GetMapId(), cx, cy, cz, false, false, false, true,
                   MovementPriority::MOVEMENT_COMBAT, true, false);
            return true;  // block until at center
        }
        return false;  // at center, let combat rotation run
    }

    // Air phase: every bot spreads around room center on concentric rings, hunters outer
    if (bossAirborne)
    {
        // Bloodbolt Whirl is heavy raid-wide damage — pop personal defensive CD to survive
        if (boss->FindCurrentSpellBySpellId(SPELL_BLOODBOLT_WHIRL))
        {
            static char const* defensives[] = {
                "shield wall", "last stand", "icebound fortitude", "survival instincts",
                "barkskin", "dispersion", "ice block", "divine shield", "divine protection",
                "evasion", "cloak of shadows", "deterrence", "shamanistic rage"
            };
            for (char const* spell : defensives)
            {
                if (botAI->CanCastSpell(spell, bot))
                {
                    botAI->CastSpell(spell, bot);
                    break;
                }
            }
        }

        float const SHADOW_AVOID_DIST = 7.0f;

        std::vector<Player*> hunters;
        std::vector<Player*> nonHunters;
        std::vector<Player*> tanks;
        for (auto const& guid : members)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member || !member->IsAlive())
                continue;
            Player* player = member->ToPlayer();
            if (!player)
                continue;
            if (!sPlayerbotsMgr.GetPlayerbotAI(player))
                continue;
            // Frenzied biters roam freely to reach their bite target — exclude from slot pool
            // (and tank stack) so we don't reserve a slot for them and so other bots can use
            // any slot they left.
            if (botAI->GetAura("Frenzied Bloodthirst", player))
                continue;
            if (PlayerbotAI::IsTank(player))
            {
                tanks.push_back(player);
                continue;
            }
            if (player->getClass() == CLASS_HUNTER)
                hunters.push_back(player);
            else
                nonHunters.push_back(player);
        }

        auto guidSort = [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); };
        std::sort(hunters.begin(), hunters.end(), guidSort);
        std::sort(nonHunters.begin(), nonHunters.end(), guidSort);
        std::sort(tanks.begin(), tanks.end(), guidSort);

        // Tanks stack together at lowest-GUID tank. Anchor tank sits at fixed center position.
        if (isTank)
        {
            Player* anchorTank = tanks.empty() ? nullptr : tanks.front();
            float tx, ty, tz;
            if (anchorTank == bot || !anchorTank)
            {
                tx = ICC_BQL_CENTER_POSITION.GetPositionX();
                ty = ICC_BQL_CENTER_POSITION.GetPositionY();
                tz = ICC_BQL_CENTER_POSITION.GetPositionZ();
            }
            else
            {
                tx = anchorTank->GetPositionX();
                ty = anchorTank->GetPositionY();
                tz = anchorTank->GetPositionZ();
            }
            if (bot->GetExactDist2d(tx, ty) > 2.0f && bot->IsWithinLOS(tx, ty, tz))
            {
                MoveTo(bot->GetMapId(), tx, ty, tz, false, false, false, true,
                       MovementPriority::MOVEMENT_COMBAT, true, false);
                return true;
            }
            return false;
        }

        // Roster order: hunters first (they get outer slots), then others (inner slots)
        std::vector<Player*> roster;
        roster.insert(roster.end(), hunters.begin(), hunters.end());
        roster.insert(roster.end(), nonHunters.begin(), nonHunters.end());

        int myIndex = -1;
        for (int i = 0; i < (int)roster.size(); i++)
        {
            if (roster[i] == bot)
            {
                myIndex = i;
                break;
            }
        }
        if (myIndex < 0)
            return false;

        // 4 rings around boss center, 73 slots total, all pairs >=7f apart, ring gap 8f
        struct AirSlot { float radius; float angle; };
        float const D2R = float(M_PI) / 180.0f;
        static AirSlot const allSlots[] = {
            // Inner ring r=10f, 8 slots at 45° — chord 7.65f
            {10.0f,   0.0f * D2R}, {10.0f,  45.0f * D2R}, {10.0f,  90.0f * D2R}, {10.0f, 135.0f * D2R},
            {10.0f, 180.0f * D2R}, {10.0f, 225.0f * D2R}, {10.0f, 270.0f * D2R}, {10.0f, 315.0f * D2R},
            // Ring 2 r=18f, 15 slots at 24° — chord 7.5f
            {18.0f,   0.0f * D2R}, {18.0f,  24.0f * D2R}, {18.0f,  48.0f * D2R}, {18.0f,  72.0f * D2R},
            {18.0f,  96.0f * D2R}, {18.0f, 120.0f * D2R}, {18.0f, 144.0f * D2R}, {18.0f, 168.0f * D2R},
            {18.0f, 192.0f * D2R}, {18.0f, 216.0f * D2R}, {18.0f, 240.0f * D2R}, {18.0f, 264.0f * D2R},
            {18.0f, 288.0f * D2R}, {18.0f, 312.0f * D2R}, {18.0f, 336.0f * D2R},
            // Ring 3 r=26f, 20 slots at 18° — chord 8.1f
            {26.0f,   0.0f * D2R}, {26.0f,  18.0f * D2R}, {26.0f,  36.0f * D2R}, {26.0f,  54.0f * D2R},
            {26.0f,  72.0f * D2R}, {26.0f,  90.0f * D2R}, {26.0f, 108.0f * D2R}, {26.0f, 126.0f * D2R},
            {26.0f, 144.0f * D2R}, {26.0f, 162.0f * D2R}, {26.0f, 180.0f * D2R}, {26.0f, 198.0f * D2R},
            {26.0f, 216.0f * D2R}, {26.0f, 234.0f * D2R}, {26.0f, 252.0f * D2R}, {26.0f, 270.0f * D2R},
            {26.0f, 288.0f * D2R}, {26.0f, 306.0f * D2R}, {26.0f, 324.0f * D2R}, {26.0f, 342.0f * D2R},
            // Outer ring r=34f, 30 slots at 12° — chord 7.1f
            {34.0f,   0.0f * D2R}, {34.0f,  12.0f * D2R}, {34.0f,  24.0f * D2R}, {34.0f,  36.0f * D2R},
            {34.0f,  48.0f * D2R}, {34.0f,  60.0f * D2R}, {34.0f,  72.0f * D2R}, {34.0f,  84.0f * D2R},
            {34.0f,  96.0f * D2R}, {34.0f, 108.0f * D2R}, {34.0f, 120.0f * D2R}, {34.0f, 132.0f * D2R},
            {34.0f, 144.0f * D2R}, {34.0f, 156.0f * D2R}, {34.0f, 168.0f * D2R}, {34.0f, 180.0f * D2R},
            {34.0f, 192.0f * D2R}, {34.0f, 204.0f * D2R}, {34.0f, 216.0f * D2R}, {34.0f, 228.0f * D2R},
            {34.0f, 240.0f * D2R}, {34.0f, 252.0f * D2R}, {34.0f, 264.0f * D2R}, {34.0f, 276.0f * D2R},
            {34.0f, 288.0f * D2R}, {34.0f, 300.0f * D2R}, {34.0f, 312.0f * D2R}, {34.0f, 324.0f * D2R},
            {34.0f, 336.0f * D2R}, {34.0f, 348.0f * D2R},
        };
        int const totalSlots = sizeof(allSlots) / sizeof(allSlots[0]);

        float const cx = ICC_BQL_CENTER_POSITION.GetPositionX();
        float const cy = ICC_BQL_CENTER_POSITION.GetPositionY();
        float const cz = ICC_BQL_CENTER_POSITION.GetPositionZ();

        // Shadow safety (rare during air phase but possible at transition)
        std::list<Creature*> shadowList;
        bot->GetCreatureListWithEntryInGrid(shadowList, NPC_SWARMING_SHADOWS, 100.0f);
        auto IsInShadow = [&](float x, float y) -> bool
        {
            for (Creature* shadow : shadowList)
            {
                if (!shadow->IsAlive())
                    continue;
                float sdx = x - shadow->GetPositionX();
                float sdy = y - shadow->GetPositionY();
                if ((sdx * sdx + sdy * sdy) < SHADOW_AVOID_DIST * SHADOW_AVOID_DIST)
                    return true;
            }
            return false;
        };
        auto AirSlotPos = [&](int idx, float& x, float& y)
        {
            float r = allSlots[idx].radius;
            float a = allSlots[idx].angle;
            x = cx + r * std::cos(a);
            y = cy + r * std::sin(a);
        };
        auto IsAirSlotSafe = [&](int idx) -> bool
        {
            float sx, sy;
            AirSlotPos(idx, sx, sy);
            return !IsInShadow(sx, sy);
        };

        // Persistent memory separate from ground phase (different slot sets).
        // Keyed per-instance to avoid cross-instance pollution.
        uint32 const airInstanceId = bot->GetInstanceId();
        auto& airSlotMemory = IcecrownHelpers::IccState(airInstanceId).bqlAirSlotMemory;

        std::vector<int> reservedSlots;
        for (Player* rp : roster)
        {
            if (rp == bot)
                continue;
            auto it = airSlotMemory.find(rp->GetGUID());
            if (it != airSlotMemory.end() && it->second >= 0 && it->second < totalSlots)
                reservedSlots.push_back(it->second);
        }
        auto IsReserved = [&](int s) -> bool
        {
            return std::find(reservedSlots.begin(), reservedSlots.end(), s) != reservedSlots.end();
        };

        bool botInShadow = IsInShadow(bot->GetPositionX(), bot->GetPositionY());

        int myAssignedSlot = -1;
        ObjectGuid myAirKey = bot->GetGUID();

        auto myMemIt = airSlotMemory.find(myAirKey);
        if (myMemIt != airSlotMemory.end())
        {
            int prev = myMemIt->second;
            if (prev >= 0 && prev < totalSlots && !IsReserved(prev) && IsAirSlotSafe(prev))
                myAssignedSlot = prev;
        }

        // Pick a new slot — hunters start from outer ring going in, others from inner going out
        if (myAssignedSlot < 0)
        {
            bool isHunter = (bot->getClass() == CLASS_HUNTER);
            for (int attempt = 0; attempt < totalSlots; attempt++)
            {
                int s;
                if (isHunter)
                    s = (totalSlots - 1) - ((myIndex + attempt) % totalSlots);
                else
                    s = (myIndex + attempt) % totalSlots;
                if (IsReserved(s))
                    continue;
                if (IsAirSlotSafe(s))
                {
                    myAssignedSlot = s;
                    break;
                }
            }
        }

        if (myAssignedSlot < 0)
        {
            airSlotMemory.erase(myAirKey);

            // No safe slot available — if standing in shadow, flee away from nearest shadow
            if (botInShadow)
            {
                Creature* nearest = nullptr;
                float bestDist = 1e9f;
                for (Creature* shadow : shadowList)
                {
                    if (!shadow->IsAlive())
                        continue;
                    float d = bot->GetExactDist2d(shadow);
                    if (d < bestDist)
                    {
                        bestDist = d;
                        nearest = shadow;
                    }
                }
                if (nearest)
                {
                    float dx = bot->GetPositionX() - nearest->GetPositionX();
                    float dy = bot->GetPositionY() - nearest->GetPositionY();
                    float mag = std::sqrt(dx * dx + dy * dy);
                    if (mag > 0.001f)
                    {
                        float fleeX = bot->GetPositionX() + (dx / mag) * 10.0f;
                        float fleeY = bot->GetPositionY() + (dy / mag) * 10.0f;
                        float fleeZ = bot->GetPositionZ();
                        bot->UpdateAllowedPositionZ(fleeX, fleeY, fleeZ);
                        MoveTo(bot->GetMapId(), fleeX, fleeY, fleeZ, false, false, false, true,
                               MovementPriority::MOVEMENT_FORCED, true, false);
                    }
                }
            }
        }
        else
        {
            airSlotMemory[myAirKey] = myAssignedSlot;

            float candidateX, candidateY;
            AirSlotPos(myAssignedSlot, candidateX, candidateY);
            float candidateZ = cz;
            bot->UpdateAllowedPositionZ(candidateX, candidateY, candidateZ);

            MovementPriority prio = botInShadow ? MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_COMBAT;
            float moveGate = botInShadow ? 0.0f : 1.0f;

            if (bot->IsWithinLOS(candidateX, candidateY, candidateZ) &&
                bot->GetExactDist2d(candidateX, candidateY) > moveGate)
            {
                MoveTo(bot->GetMapId(), candidateX, candidateY, candidateZ, false, false, false, true,
                       prio, true, false);
                // Still moving to slot — block combat so we actually get there
                return true;
            }
        }
        // At slot (or fleeing shadow) — let combat rotation attack/heal
        return false;
    }

    if (isMeleeDps && !isAirPhase)
    {
        if (bot->GetDistance2d(boss) > 2.0f)
        {
            MoveTo(bot->GetMapId(), boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ(), false, false,
                   false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        return false;
    }

    // Ground phase ranged positioning — persistent slot assignment, only affected bots reassign
    if (isRanged && !isAirPhase)
    {
        float const SHADOW_AVOID_DIST = 7.0f;

        // Gather ranged bots + melee bots (skip real players)
        std::vector<Player*> hunters;
        std::vector<Player*> otherRanged;
        std::vector<Player*> meleeBots;
        for (auto const& guid : members)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member || !member->IsAlive())
                continue;
            Player* player = member->ToPlayer();
            if (!player)
                continue;
            if (!sPlayerbotsMgr.GetPlayerbotAI(player))
                continue;
            if (PlayerbotAI::IsRanged(player))
            {
                if (player->getClass() == CLASS_HUNTER)
                    hunters.push_back(player);
                else
                    otherRanged.push_back(player);
            }
            else if (PlayerbotAI::IsMelee(player))
            {
                meleeBots.push_back(player);
            }
        }

        auto guidSort = [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); };
        std::sort(hunters.begin(), hunters.end(), guidSort);
        std::sort(otherRanged.begin(), otherRanged.end(), guidSort);
        std::sort(meleeBots.begin(), meleeBots.end(), guidSort);

        std::vector<Player*> roster;
        roster.insert(roster.end(), hunters.begin(), hunters.end());
        roster.insert(roster.end(), otherRanged.begin(), otherRanged.end());

        int myIndex = -1;
        for (int i = 0; i < (int)roster.size(); i++)
        {
            if (roster[i] == bot)
            {
                myIndex = i;
                break;
            }
        }
        if (myIndex < 0)
            return false;

        // Fixed world-space anchor and direction
        float const anchorX = ICC_BQL_TANK_POSITION.GetPositionX();
        float const anchorY = ICC_BQL_TANK_POSITION.GetPositionY();
        float const anchorZ = ICC_BQL_TANK_POSITION.GetPositionZ();
        float baseDx = ICC_BQL_CENTER_POSITION.GetPositionX() - anchorX;
        float baseDy = ICC_BQL_CENTER_POSITION.GetPositionY() - anchorY;
        float const anchorToCenter = std::atan2(baseDy, baseDx);

        // 19 fixed slots: 6 inner (20f) + 7 middle (27f) + 6 outer (35f)
        struct Slot { float radius; float angleOffset; };
        float const D2R = float(M_PI) / 180.0f;
        static Slot const allSlots[] = {
            {20.0f, -60.0f * D2R}, {20.0f, -36.0f * D2R}, {20.0f, -12.0f * D2R},
            {20.0f,  12.0f * D2R}, {20.0f,  36.0f * D2R}, {20.0f,  60.0f * D2R},
            {27.0f, -45.0f * D2R}, {27.0f, -30.0f * D2R}, {27.0f, -15.0f * D2R},
            {27.0f,   0.0f},       {27.0f,  15.0f * D2R}, {27.0f,  30.0f * D2R},
            {27.0f,  45.0f * D2R},
            {35.0f, -60.0f * D2R}, {35.0f, -36.0f * D2R}, {35.0f, -12.0f * D2R},
            {35.0f,  12.0f * D2R}, {35.0f,  36.0f * D2R}, {35.0f,  60.0f * D2R},
        };
        int const totalSlots = sizeof(allSlots) / sizeof(allSlots[0]);

        // Shadow list + safety check
        std::list<Creature*> shadowList;
        bot->GetCreatureListWithEntryInGrid(shadowList, NPC_SWARMING_SHADOWS, 100.0f);
        auto IsInShadow = [&](float x, float y) -> bool
        {
            for (Creature* shadow : shadowList)
            {
                if (!shadow->IsAlive())
                    continue;
                float sdx = x - shadow->GetPositionX();
                float sdy = y - shadow->GetPositionY();
                if ((sdx * sdx + sdy * sdy) < SHADOW_AVOID_DIST * SHADOW_AVOID_DIST)
                    return true;
            }
            return false;
        };
        auto SlotPos = [&](int idx, float& x, float& y)
        {
            float angle = anchorToCenter + allSlots[idx].angleOffset;
            float r = allSlots[idx].radius;
            x = anchorX + r * std::cos(angle);
            y = anchorY + r * std::sin(angle);
        };
        auto IsSlotSafe = [&](int idx) -> bool
        {
            float sx, sy;
            SlotPos(idx, sx, sy);
            return !IsInShadow(sx, sy);
        };

        // Persistent per-bot slot memory shared across all bots.
        // Keyed per-instance to avoid cross-instance pollution.
        uint32 const groundInstanceId = bot->GetInstanceId();
        auto& botSlotMemory = IcecrownHelpers::IccState(groundInstanceId).bqlBotSlotMemory;

        // Collect every OTHER bot's remembered slot as "reserved" — each bot owns its own
        // memory and we must respect their claim, even if we can't see the same shadows.
        // This prevents cascading reassignments when one bot moves.
        std::vector<int> reservedSlots;
        for (Player* rp : roster)
        {
            if (rp == bot)
                continue;
            auto it = botSlotMemory.find(rp->GetGUID());
            if (it != botSlotMemory.end() && it->second >= 0 && it->second < totalSlots)
                reservedSlots.push_back(it->second);
        }

        auto IsReserved = [&](int s) -> bool
        {
            return std::find(reservedSlots.begin(), reservedSlots.end(), s) != reservedSlots.end();
        };

        int myAssignedSlot = -1;
        bool myFellBack = false;
        ObjectGuid myGroundKey = bot->GetGUID();

        // Step 1: keep my remembered slot if still safe and not reserved by someone else
        auto myMemIt = botSlotMemory.find(myGroundKey);
        if (myMemIt != botSlotMemory.end())
        {
            int prev = myMemIt->second;
            if (prev >= 0 && prev < totalSlots && !IsReserved(prev) && IsSlotSafe(prev))
                myAssignedSlot = prev;
        }

        // Step 2: need a new slot — pick first safe slot not reserved by others
        if (myAssignedSlot < 0)
        {
            // Prefer slots by roster seniority (hunters/lower-GUID first get inner slots)
            // Start from index myIndex to fall into "my natural zone", wrap around
            for (int attempt = 0; attempt < totalSlots; attempt++)
            {
                int s = (myIndex + attempt) % totalSlots;
                if (IsReserved(s))
                    continue;
                if (IsSlotSafe(s))
                {
                    myAssignedSlot = s;
                    break;
                }
            }
        }

        if (myAssignedSlot < 0)
        {
            // No safe unreserved slot — fall back to melee. Forget my slot so others can use it.
            botSlotMemory.erase(myGroundKey);
            myFellBack = true;
        }
        else
        {
            botSlotMemory[myGroundKey] = myAssignedSlot;
        }

        if (myAssignedSlot >= 0)
        {
            float candidateX, candidateY;
            SlotPos(myAssignedSlot, candidateX, candidateY);
            float candidateZ = anchorZ;
            bot->UpdateAllowedPositionZ(candidateX, candidateY, candidateZ);

            if (bot->IsWithinLOS(candidateX, candidateY, candidateZ) &&
                bot->GetExactDist2d(candidateX, candidateY) > 1.0f)
            {
                MoveTo(bot->GetMapId(), candidateX, candidateY, candidateZ, false, false, false, true,
                       MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
        else if (myFellBack && !meleeBots.empty())
        {
            // No safe slot — flee to lowest-GUID melee bot until a safe slot frees up
            Player* anchor = meleeBots.front();
            if (anchor != bot)
            {
                float ax = anchor->GetPositionX();
                float ay = anchor->GetPositionY();
                float az = anchor->GetPositionZ();
                if (bot->GetExactDist2d(ax, ay) > 3.0f && bot->IsWithinLOS(ax, ay, az))
                {
                    MoveTo(bot->GetMapId(), ax, ay, az, false, false, false, true,
                           MovementPriority::MOVEMENT_FORCED, true, false);
                }
            }
        }
    }

    return false;
}

bool IccBqlPactOfDarkfallenAction::Execute(Event /*event*/)
{
    // Check if bot has Pact of the Darkfallen
    if (!botAI->GetAura("Pact of the Darkfallen", bot))
        return false;
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Find other players with Pact of the Darkfallen
    Player* tankWithAura = nullptr;
    std::vector<Player*> playersWithAura;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || member == bot)
            continue;
        if (botAI->GetAura("Pact of the Darkfallen", member))
        {
            playersWithAura.push_back(member);
            if (PlayerbotAI::IsTank(member))
                tankWithAura = member;
        }
    }

    if (playersWithAura.empty())
        return false;

    // Determine target position
    Position targetPos;
    if (tankWithAura)
    {
        // If there's a tank with aura, everyone moves to the tank (including the tank itself for center positioning)
        if (PlayerbotAI::IsTank(bot))
        {
            // If current bot is the tank, stay put or move slightly for better positioning
            targetPos.Relocate(bot);
        }
        else
        {
            // Non-tank bots move to the tank
            targetPos.Relocate(tankWithAura);
        }
    }
    else if (playersWithAura.size() >= 2)
    {
        // Calculate center position of all players with aura (including bot)
        CalculateCenterPosition(targetPos, playersWithAura);
    }
    else if (playersWithAura.size() == 1)
    {
        // Move to the single other player with aura
        targetPos.Relocate(playersWithAura[0]);
    }
    else
    {
        // No valid movement case found
        return true;
    }

    // Move to target position if needed
    return MoveToTargetPosition(targetPos, playersWithAura.size() + 1);  // +1 to include the bot itself
}

bool IccBqlPactOfDarkfallenAction::CalculateCenterPosition(Position& targetPos, std::vector<Player*> const& playersWithAura)
{
    float sumX = bot->GetPositionX();
    float sumY = bot->GetPositionY();
    float sumZ = bot->GetPositionZ();

    // Add positions of all other players with aura
    for (Player* player : playersWithAura)
    {
        sumX += player->GetPositionX();
        sumY += player->GetPositionY();
        sumZ += player->GetPositionZ();
    }

    // Calculate average position (center)
    int totalPlayers = playersWithAura.size() + 1;  // +1 for the bot itself
    targetPos.Relocate(sumX / totalPlayers, sumY / totalPlayers, sumZ / totalPlayers);

    return false;
}

bool IccBqlPactOfDarkfallenAction::MoveToTargetPosition(Position const& targetPos, int auraCount)
{
    float const POSITION_TOLERANCE = 0.1f;
    float distance = bot->GetDistance(targetPos);
    if (distance <= POSITION_TOLERANCE)
        return true;

    // Calculate movement increment
    float dx = targetPos.GetPositionX() - bot->GetPositionX();
    float dy = targetPos.GetPositionY() - bot->GetPositionY();
    float dz = targetPos.GetPositionZ() - bot->GetPositionZ();
    float len = sqrt(dx * dx + dy * dy);

    float moveX, moveY, moveZ;
    if (len > 5.0f && auraCount <= 2)
    {
        dx /= len;
        dy /= len;
        moveX = bot->GetPositionX() + dx * 5.0f;
        moveY = bot->GetPositionY() + dy * 5.0f;
        moveZ = bot->GetPositionZ() + (dz / distance) * 5.0f;
    }
    else
    {
        moveX = targetPos.GetPositionX();
        moveY = targetPos.GetPositionY();
        moveZ = targetPos.GetPositionZ();
    }

    botAI->Reset();
    MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    return false;
}

bool IccBqlVampiricBiteAction::Execute(Event /*event*/)
{
    // Only act when bot has Frenzied Bloodthirst
    if (!botAI->GetAura("Frenzied Bloodthirst", bot))
        return false;

    // Bloodbolt Whirl defensive CDs (divine shield, ice block, cloak) make bot untargetable
    // and block the bite cast. Strip them now so the bite can land.
    botAI->RemoveAura("divine shield");
    botAI->RemoveAura("ice block");
    botAI->RemoveAura("cloak of shadows");
    botAI->RemoveAura("deterrence");

    float const BITE_RANGE = 4.0f;
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Find best target
    Player* target = FindBestBiteTarget(group);
    if (!target)
        return false;

    // Handle movement or casting
    if (bot->GetExactDist2d(target) > BITE_RANGE)
        return MoveTowardsTarget(target);

    return CastVampiricBite(target);
}

Player* IccBqlVampiricBiteAction::FindBestBiteTarget(Group* group)
{
    // Collect frenzied biters (sorted by GUID) and all candidate targets.
    // Lower-GUID biters claim their nearest valid target first; higher-GUID
    // biters skip claimed targets. Deterministic across all bots so two biters
    // never converge on the same victim.
    std::vector<Player*> biters;
    std::vector<Player*> dpsHealCandidates;
    std::vector<Player*> tankCandidates;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (botAI->GetAura("Frenzied Bloodthirst", member))
        {
            biters.push_back(member);
            continue;
        }

        // Skip already-bitten / shadowed / mind-controlled
        if (botAI->GetAura("Essence of the Blood Queen", member) ||
            botAI->GetAura("Uncontrollable Frenzy", member) ||
            botAI->GetAura("Swarming Shadows", member))
            continue;

        if (PlayerbotAI::IsTank(member))
            tankCandidates.push_back(member);
        else if (PlayerbotAI::IsDps(member) || PlayerbotAI::IsHeal(member))
            dpsHealCandidates.push_back(member);
    }

    std::sort(biters.begin(), biters.end(),
              [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    auto pickFromPool = [&](std::vector<Player*>& pool) -> Player*
    {
        std::set<ObjectGuid> claimed;
        Player* myPick = nullptr;
        for (Player* biter : biters)
        {
            float bestDist = FLT_MAX;
            Player* bestTarget = nullptr;
            for (Player* cand : pool)
            {
                if (claimed.count(cand->GetGUID()))
                    continue;
                float d = biter->GetDistance(cand);
                if (d < bestDist)
                {
                    bestDist = d;
                    bestTarget = cand;
                }
            }
            if (!bestTarget)
                continue;
            claimed.insert(bestTarget->GetGUID());
            if (biter == bot)
            {
                myPick = bestTarget;
                break;
            }
        }
        return myPick;
    };

    if (Player* pick = pickFromPool(dpsHealCandidates))
        return pick;

    // Fallback: tanks only when no DPS/heal target available
    return pickFromPool(tankCandidates);
}

bool IccBqlVampiricBiteAction::IsInvalidTarget(Player* player)
{
    return botAI->GetAura("Frenzied Bloodthirst", player) || botAI->GetAura("Essence of the Blood Queen", player) ||
           botAI->GetAura("Uncontrollable Frenzy", player) || botAI->GetAura("Swarming Shadows", player);
}

bool IccBqlVampiricBiteAction::MoveTowardsTarget(Player* target)
{
    if (IsInvalidTarget(target) || !target->IsAlive())
        return false;

    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();

    // Don't gate movement on LOS — if blocked, take a step toward target so LOS
    // can clear next tick. Multiplier zeroes all non-bite actions for biters,
    // so silently aborting here would idle the bot until the aura expires.

    float dx = x - bot->GetPositionX();
    float dy = y - bot->GetPositionY();
    float dz = z - bot->GetPositionZ();
    float len = sqrt(dx * dx + dy * dy);

    float moveX, moveY, moveZ;
    if (len > 5.0f)
    {
        dx /= len;
        dy /= len;
        moveX = bot->GetPositionX() + dx * 5.0f;
        moveY = bot->GetPositionY() + dy * 5.0f;
        moveZ = bot->GetPositionZ() + (dz / len) * 5.0f;
    }
    else
    {
        moveX = x;
        moveY = y;
        moveZ = z;
    }

    MoveTo(target->GetMapId(), moveX, moveY, moveZ, false, false, false, true,
           MovementPriority::MOVEMENT_FORCED);

    return false;
}

bool IccBqlVampiricBiteAction::CastVampiricBite(Player* target)
{
    if (IsInvalidTarget(target) || !target->IsAlive())
        return false;

    return botAI->CanCastSpell("Vampiric Bite", target) && botAI->CastSpell("Vampiric Bite", target);
}
