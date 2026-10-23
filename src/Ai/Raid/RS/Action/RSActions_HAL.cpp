/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ObjectAccessor.h"
#include "Pet.h"
#include "Playerbots.h"
#include "RSActions.h"
#include "RSScripts.h"
#include "ThreatManager.h"
#include "Timer.h"
#include "Vehicle.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <utility>
#include <vector>

bool RsHalionTankPositionAction::Execute(Event )
{
    if (RsHalionBossTank(botAI) != bot)
        return false;

    Unit* boss = RsHalionAnyPhysicalBoss(botAI);
    if (!boss || !boss->IsInCombat())
        return false;

    context->GetValue<std::string>("rti")->Set("skull");

    if (bot->GetTarget() != boss->GetGUID())
        bot->SetTarget(boss->GetGUID());

    std::vector<Unit*> adds;
    RsHalionCollectAdds(botAI, adds);

    if (botAI->HasCheat(BotCheatMask::raid))
    {
        for (Unit* add : adds)
            RsForceThreat(add, bot);
    }

    if (Group* group = bot->GetGroup())
    {
        Unit* skullTarget = nullptr;

        Unit* current = ObjectAccessor::GetUnit(*bot, group->GetTargetIcon(RtiTargetValue::skullIndex));
        bool const currentIsAliveAdd = current && current->IsAlive() &&
            std::find(adds.begin(), adds.end(), current) != adds.end();

        if (currentIsAliveAdd)
        {
            skullTarget = current;
        }
        else if (!adds.empty())
        {
            for (Unit* add : adds)
            {
                if (add->GetEntry() == NPC_LIVING_INFERNO)
                {
                    skullTarget = add;
                    break;
                }
            }
            if (!skullTarget)
            {
                float best = std::numeric_limits<float>::max();
                for (Unit* add : adds)
                {
                    float const d = bot->GetExactDist2d(add);
                    if (d < best)
                    {
                        best = d;
                        skullTarget = add;
                    }
                }
            }
        }
        else
        {
            skullTarget = boss;
        }

        if (skullTarget && group->GetTargetIcon(RtiTargetValue::skullIndex) != skullTarget->GetGUID())
            group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), skullTarget->GetGUID());

        if (!adds.empty())
        {
            if (group->GetTargetIcon(RtiTargetValue::starIndex) != boss->GetGUID())
                group->SetTargetIcon(RtiTargetValue::starIndex, bot->GetGUID(), boss->GetGUID());
        }
        else if (!group->GetTargetIcon(RtiTargetValue::starIndex).IsEmpty())
        {
            group->SetTargetIcon(RtiTargetValue::starIndex, bot->GetGUID(), ObjectGuid::Empty);
        }
    }

    if (botAI->HasCheat(BotCheatMask::raid) && boss->GetVictim() != bot)
    {
        ThreatManager& mgr = boss->GetThreatMgr();
        mgr.AddThreat(bot, 1000000.0f, nullptr, true, true);
    }

    bool const committed = RsHalionTankMeteorCommitted(botAI);
    Position const& home = committed ? RS_HALION_TANK_METEOR_SPOT : RS_HALION_TANK_POSITION;

    if (bot->GetExactDist2d(home) > 1.0f)
    {
        if (committed)
            bot->CastStop();

        if (committed && !bot->HasAura(RS_SPELL_NITRO_BOOSTS))
            bot->AddAura(RS_SPELL_NITRO_BOOSTS, bot);

        return MoveTo(bot->GetMapId(), home.GetPositionX(), home.GetPositionY(), home.GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    bot->SetFacingToObject(boss);

    return false;
}

bool RsHalionAvoidConesAction::Execute(Event )
{
    if (PlayerbotAI::IsMainTank(bot))
        return false;

    if (PlayerbotAI::IsTank(bot) && !RsHalionAssistTankAsMelee(botAI))
        return false;

    Unit* boss = RsHalionAnyPhysicalBoss(botAI);
    if (!boss)
        return false;

    bool const melee = PlayerbotAI::IsMelee(bot);
    bool const addsUp = RsHalionAnyAddAlive(botAI);

    if (!PlayerbotAI::IsTank(bot) && !addsUp && RsHalionRealmThrottled(botAI, bot) &&
        (RsHalionLeadingTooMuch(botAI, bot) || RsHalionInThrottledHalf(bot)))
    {
        if (bot->GetVictim())
            bot->AttackStop();
        if (Pet* pet = bot->GetPet())
            if (pet->GetVictim())
                pet->AttackStop();
    }

    if (melee && addsUp)
        context->GetValue<std::string>("rti")->Set("star");
    else
        context->GetValue<std::string>("rti")->Set("skull");

    if (melee && bot->GetTarget() != boss->GetGUID())
        bot->SetTarget(boss->GetGUID());

    float const bossX = boss->GetPositionX();
    float const bossY = boss->GetPositionY();

    float const dA = bot->GetExactDist2d(RS_HALION_METEOR_SPOT_A.GetPositionX(), RS_HALION_METEOR_SPOT_A.GetPositionY());
    float const dB = bot->GetExactDist2d(RS_HALION_METEOR_SPOT_B.GetPositionX(), RS_HALION_METEOR_SPOT_B.GetPositionY());

    auto& botUsesSpotA = RubySanctumHelpers::RsState(bot->GetInstanceId()).meteorSpotUsesA;
    ObjectGuid const botGuid = bot->GetGUID();
    auto memIt = botUsesSpotA.find(botGuid);
    bool usesA = memIt != botUsesSpotA.end() ? memIt->second : dA <= dB;
    if (usesA && dB + 8.0f < dA)
        usesA = false;
    else if (!usesA && dA + 8.0f < dB)
        usesA = true;
    botUsesSpotA[botGuid] = usesA;

    Position const& spot = usesA ? RS_HALION_METEOR_SPOT_A : RS_HALION_METEOR_SPOT_B;

    if (bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) <= RS_HALION_LINE_LEASH)
        return false;

    float const lineX = spot.GetPositionX() - bossX;
    float const lineY = spot.GetPositionY() - bossY;
    float const lineLen = std::sqrt(lineX * lineX + lineY * lineY);
    if (lineLen < 0.01f)
        return false;

    float const ux = lineX / lineLen;
    float const uy = lineY / lineLen;

    float const minDist = melee ? RS_HALION_LINE_MELEE_MIN : RS_HALION_LINE_RANGED_MIN;
    float const maxDist = melee ? RS_HALION_LINE_MELEE_MAX : RS_HALION_LINE_RANGED_MAX;

    float const distToBoss = bot->GetExactDist2d(bossX, bossY);

    float const botDX = bot->GetPositionX() - bossX;
    float const botDY = bot->GetPositionY() - bossY;
    float const along = botDX * ux + botDY * uy;
    float const offset = std::fabs(botDX * uy - botDY * ux);

    bool const onLine = along > 0.0f && offset <= RS_HALION_LINE_OFFSET_OK;
    bool const inBand = distToBoss >= minDist && distToBoss <= maxDist;

    if (onLine && inBand)
        return false;

    float desired = distToBoss;
    if (desired < minDist)
        desired = minDist;
    else if (desired > maxDist)
        desired = maxDist;

    float const moveX = bossX + ux * desired;
    float const moveY = bossY + uy * desired;

    if (!bot->IsWithinLOS(moveX, moveY, bot->GetPositionZ()))
        return false;

    return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true);
}

bool RsHalionCombustionAction::Execute(Event )
{
    if (RsHalionHasCombustion(bot) && RsHalionCombustionAtSpot(bot))
    {
        bot->RemoveAurasDueToSpell(SPELL_FIERY_COMBUSTION, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
        bot->RemoveAurasDueToSpell(SPELL_MARK_OF_COMBUSTION, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
        return false;
    }

    if (RsHalionBossTank(botAI) == bot)
        return false;

    if (!RsHalionHasCombustion(bot))
    {
        if (RsHalionCombustionReturning(bot))
        {
            Position const& rally = RsHalionMeteorSpot(bot->GetInstanceId());
            return MoveTo(bot->GetMapId(), rally.GetPositionX(), rally.GetPositionY(), rally.GetPositionZ(),
                          false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
        return false;
    }

    if (!bot->HasAura(RS_SPELL_NITRO_BOOSTS))
        bot->AddAura(RS_SPELL_NITRO_BOOSTS, bot);

    Position const& spot = RsHalionCombustionSpot(bot);

    if (bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) <= RS_HALION_COMBUSTION_REACH)
        return false;

    return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(),
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool RsHalionMeteorAction::Execute(Event )
{
    if (PlayerbotAI::IsTank(bot) && !RsHalionAssistTankAsMelee(botAI))
        return false;

    if (!RsHalionAnyPhysicalBoss(botAI))
        return false;

    if (!RsHalionMeteorShouldRally(bot))
        return false;

    bot->CastStop();
    botAI->Reset();

    Position const& target = RsHalionMeteorSpot(bot->GetInstanceId());
    Position const& mid = RS_HALION_METEOR_MID;

    float const distToTarget = bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY());
    float const midToTarget = mid.GetExactDist2d(target.GetPositionX(), target.GetPositionY());
    float const distToMid = bot->GetExactDist2d(mid.GetPositionX(), mid.GetPositionY());

    Position const& leg = (distToMid > RS_HALION_METEOR_REACH && distToTarget > midToTarget) ? mid : target;

    return MoveTo(bot->GetMapId(), leg.GetPositionX(), leg.GetPositionY(), leg.GetPositionZ(),
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED, true);
}

bool RsHalionStartPositionAction::Execute(Event )
{
    if (RsHalionBossTank(botAI) == bot)
        return false;

    Unit* boss = RsHalionPhase1Boss(botAI);
    if (!boss || !boss->HealthAbovePct(99))
        return false;

    if (bot->GetExactDist2d(RS_HALION_METEOR_SPOT_A.GetPositionX(), RS_HALION_METEOR_SPOT_A.GetPositionY()) <=
        RS_HALION_METEOR_REACH)
        return false;

    return MoveTo(bot->GetMapId(), RS_HALION_METEOR_SPOT_A.GetPositionX(), RS_HALION_METEOR_SPOT_A.GetPositionY(),
                  RS_HALION_METEOR_SPOT_A.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool RsHalionAddsAction::Execute(Event )
{
    return false;
}

bool RsHalionAddTankAction::Execute(Event )
{
    return false;
}

bool RsHalionEnterPortalAction::Execute(Event )
{
    if (PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* physBoss = RsHalionAnyPhysicalBoss(botAI);
    bool const phase3 = physBoss != nullptr && !physBoss->HealthAbovePct(50);

    bool p3TwilightExit = false;
    if (RsHalionInTwilight(bot))
    {
        Unit* twilightBoss = RsHalionTwilightBoss(botAI);
        bool const tBossLow = twilightBoss != nullptr && !twilightBoss->HealthAbovePct(50);
        if (!tBossLow || RsHalionP3TwilightAssigned(bot))
            return false;

        if (RsHalionHasConsumption(bot))
            return false;

        if (!RsHalionPortalCommit(botAI, bot))
            return false;

        p3TwilightExit = true;
    }
    else if (RsHalionPortalHeldForAdds(botAI))
    {
        if (RsHalionFirstCrosser(botAI) == bot)
        {
            uint32 const remaining = RsHalionPortalAddHoldRemainingMs(botAI);
            uint32 const shown = (remaining + 999) / 1000 + 2;

            uint32& last = RubySanctumHelpers::RsState(bot->GetInstanceId()).portalCountdownLastShown;
            if (last != shown)
            {
                last = shown;
                botAI->Yell(std::to_string(shown));
            }
        }
        return false;
    }

    auto& botPortalTarget = RubySanctumHelpers::RsState(bot->GetInstanceId()).botPortalTarget;
    ObjectGuid const botGuid = bot->GetGUID();

    GameObject* portal = nullptr;
    auto cachedIt = botPortalTarget.find(botGuid);
    if (cachedIt != botPortalTarget.end())
    {
        portal = ObjectAccessor::GetGameObject(*bot, cachedIt->second);
        if (!portal)
            botPortalTarget.erase(cachedIt);
    }

    if (!portal)
    {
        portal = RsHalionFindPortal(botAI);
        if (!portal)
            return false;
        botPortalTarget[botGuid] = portal->GetGUID();
    }

    bool const isFirstCrosser = (p3TwilightExit || phase3) ? false : (RsHalionFirstCrosser(botAI) == bot);
    if (!isFirstCrosser && !p3TwilightExit && !phase3)
    {
        Player* firstCrosser = RsHalionFirstCrosser(botAI);
        if (!firstCrosser || !RsHalionInTwilight(firstCrosser))
            return false;

        uint32& seen = RubySanctumHelpers::RsState(bot->GetInstanceId()).portalSeen[bot->GetGUID()];
        if (seen == 0)
            seen = getMSTime();

        if (GetMSTimeDiffToNow(seen) < 1000)
            return false;
    }

    float const distToPortal = bot->GetExactDist2d(portal->GetPositionX(), portal->GetPositionY());

    if (distToPortal > INTERACTION_DISTANCE - 0.5f)
    {
        float goalX = portal->GetPositionX();
        float goalY = portal->GetPositionY();

        std::vector<Unit*> pools;
        RsHalionCollectHazardPools(bot, pools);
        if (!pools.empty())
        {
            float const dx = goalX - bot->GetPositionX();
            float const dy = goalY - bot->GetPositionY();
            float const len = std::sqrt(dx * dx + dy * dy);
            if (len > 0.01f)
            {
                float const ux = dx / len;
                float const uy = dy / len;
                float const probe = std::min(len, RS_HALION_CONSUMPTION_STEP);
                float const aheadX = bot->GetPositionX() + ux * probe;
                float const aheadY = bot->GetPositionY() + uy * probe;

                if (!RsHalionSpotClearOfPools(pools, aheadX, aheadY))
                {
                    bool routed = false;
                    for (float const deg : {35.0f, -35.0f, 60.0f, -60.0f, 90.0f, -90.0f})
                    {
                        float const a = std::atan2(uy, ux) + deg * static_cast<float>(M_PI) / 180.0f;
                        float const sideX = bot->GetPositionX() + std::cos(a) * probe;
                        float const sideY = bot->GetPositionY() + std::sin(a) * probe;
                        if (RsHalionSpotClearOfPools(pools, sideX, sideY) &&
                            bot->IsWithinLOS(sideX, sideY, bot->GetPositionZ()))
                        {
                            goalX = sideX;
                            goalY = sideY;
                            routed = true;
                            break;
                        }
                    }
                    if (!routed)
                        return false;
                }
            }
        }

        if (p3TwilightExit && RsHalionCutterShouldMove(bot->GetInstanceId()) &&
            !bot->HasAura(RS_SPELL_MAGIC_BARRIER))
        {
            bot->AddAura(RS_SPELL_MAGIC_BARRIER, bot);
            ObjectGuid const barrierGuid = bot->GetGUID();
            botAI->AddTimedEvent([barrierGuid]()
            {
                if (Player* p = ObjectAccessor::FindPlayer(barrierGuid))
                    p->RemoveAura(RS_SPELL_MAGIC_BARRIER);
            }, 5000);
        }

        return MoveTo(bot->GetMapId(), goalX, goalY, portal->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED);
    }

    botPortalTarget.erase(botGuid);
    botAI->RemoveShapeshift();
    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    bot->SetFacingToObject(portal);

    portal->Use(bot);

    if (p3TwilightExit)
        bot->RemoveAura(SPELL_TWILIGHT_REALM);

    if (!p3TwilightExit && !phase3)
    {
        bot->AddAura(RS_SPELL_MAGIC_BARRIER, bot);

        ObjectGuid const guid = bot->GetGUID();
        botAI->AddTimedEvent([guid]()
        {
            if (Player* p = ObjectAccessor::FindPlayer(guid))
                p->RemoveAura(RS_SPELL_MAGIC_BARRIER);
        }, 5000);
    }

    return true;
}

bool RsHalionP2TankPositionAction::Execute(Event )
{
    if (RsHalionTwilightTank(botAI) != bot || !RsHalionInTwilight(bot))
        return false;

    Unit* boss = RsHalionTwilightBoss(botAI);
    if (!boss)
        return false;

    if (bot->GetTarget() != boss->GetGUID())
        bot->SetTarget(boss->GetGUID());

    if (Group* group = bot->GetGroup())
    {
        if (group->GetTargetIcon(RtiTargetValue::crossIndex) != boss->GetGUID())
            group->SetTargetIcon(RtiTargetValue::crossIndex, bot->GetGUID(), boss->GetGUID());
    }

    if (botAI->HasCheat(BotCheatMask::raid))
    {
        ThreatManager& mgr = boss->GetThreatMgr();
        if (boss->GetVictim() != bot)
            mgr.AddThreat(bot, 1000000.0f, nullptr, true, true);
        mgr.FixateTarget(bot);
    }

    context->GetValue<std::string>("rti")->Set("cross");

    float const distToBoss = bot->GetExactDist2d(boss->GetPositionX(), boss->GetPositionY());
    if (std::fabs(distToBoss - RS_HALION_CUTTER_FLANK_DIST) > 2.0f)
    {
        float const tankAngle = std::atan2(bot->GetPositionY() - boss->GetPositionY(),
                                           bot->GetPositionX() - boss->GetPositionX());
        float const rx = boss->GetPositionX() + std::cos(tankAngle) * RS_HALION_CUTTER_FLANK_DIST;
        float const ry = boss->GetPositionY() + std::sin(tankAngle) * RS_HALION_CUTTER_FLANK_DIST;
        if (bot->IsWithinLOS(rx, ry, bot->GetPositionZ()))
            return MoveTo(bot->GetMapId(), rx, ry, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool RsHalionP2AvoidConesAction::Execute(Event )
{
    if (RsHalionTwilightTank(botAI) == bot)
        return false;

    Unit* boss = RsHalionTwilightBoss(botAI);
    if (!boss)
        return false;

    if (RsHalionPortalCommit(botAI, bot))
        return false;

    if (RsHalionCutterShouldMove(bot->GetInstanceId()))
    {
        auto& botPortalTarget = RubySanctumHelpers::RsState(bot->GetInstanceId()).botPortalTarget;
        auto portalIt = botPortalTarget.find(bot->GetGUID());
        if (portalIt != botPortalTarget.end())
        {
            botPortalTarget.erase(portalIt);
            if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
            {
                bot->GetMotionMaster()->Clear();
                bot->StopMoving();
            }
        }
    }

    if (!PlayerbotAI::IsTank(bot) && RsHalionRealmThrottled(botAI, bot) &&
        (RsHalionLeadingTooMuch(botAI, bot) || RsHalionInThrottledHalf(bot)))
    {
        if (bot->GetVictim())
            bot->AttackStop();
        if (Pet* pet = bot->GetPet())
            if (pet->GetVictim())
                pet->AttackStop();
    }

    if (PlayerbotAI::IsTank(bot))
    {
        Player* twilightTank = RsHalionTwilightTank(botAI);
        if (!twilightTank)
            return false;
        if (bot->GetExactDist2d(twilightTank) <= 2.0f)
            return false;
        return MoveTo(bot->GetMapId(), twilightTank->GetPositionX(), twilightTank->GetPositionY(),
                      twilightTank->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    context->GetValue<std::string>("rti")->Set("cross");

    Player* tank = RsHalionTwilightTank(botAI);
    if (!tank)
        return false;

    std::vector<std::pair<Unit*, Unit*>> pairs;
    bool const singleCutter = RsHalionCollectOrbPairs(boss, pairs) && pairs.size() == 1;
    bool const cutterFiring = RsHalionCutterActive(bot->GetInstanceId());

    if (RsHalionCutterBeamDanger(botAI, bot) && singleCutter && cutterFiring)
    {
        bot->CastStop();

        Unit* na = pairs[0].first;
        Unit* nb = pairs[0].second;
        float const nearestSigned = RsHalionBeamSignedDist(bot->GetPositionX(), bot->GetPositionY(), na, nb);
        float const nearest = std::fabs(nearestSigned);
        float const ex = nb->GetPositionX() - na->GetPositionX();
        float const ey = nb->GetPositionY() - na->GetPositionY();
        float const elen = std::sqrt(ex * ex + ey * ey);
        if (elen > 0.01f)
        {
            float const side = nearestSigned >= 0.0f ? 1.0f : -1.0f;
            float const nx = side * ey / elen;
            float const ny = side * -ex / elen;
            float const clear = RS_HALION_CUTTER_DANGER + RS_HALION_CUTTER_MARGIN;
            float const stepLen = std::max(clear - nearest, RS_HALION_P2_STEP);
            float const moveX = bot->GetPositionX() + nx * stepLen;
            float const moveY = bot->GetPositionY() + ny * stepLen;
            if (bot->IsWithinLOS(moveX, moveY, bot->GetPositionZ()))
                return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false,
                              true, MovementPriority::MOVEMENT_FORCED, true);
        }
    }

    bool const lockedClass = bot->getClass() == CLASS_ROGUE || bot->getClass() == CLASS_WARRIOR;
    bool const loose = RsHalionCutterShouldMove(bot->GetInstanceId()) &&
        !lockedClass &&
        !RsHalionCutterBeamDanger(botAI, bot);

    if (loose && bot->IsNonMeleeSpellCast(false, false, true))
        return false;

    if (RsHalionCutterBeamDanger(botAI, bot) && cutterFiring)
        bot->CastStop();

    float const bossX = boss->GetPositionX();
    float const bossY = boss->GetPositionY();
    float const bossToTank = std::atan2(tank->GetPositionY() - bossY, tank->GetPositionX() - bossX);
    float const radius = PlayerbotAI::IsMelee(bot) ? RS_HALION_P2_MELEE_DIST : RS_HALION_P2_RANGED_DIST;
    float slotAngle = bossToTank + static_cast<float>(M_PI) * 95.0f / 180.0f;

    if (singleCutter && cutterFiring)
    {
        float const clear = RS_HALION_CUTTER_DANGER + RS_HALION_CUTTER_MARGIN;
        auto slotClear = [&](float ang) -> bool
        {
            float const sx = bossX + std::cos(ang) * radius;
            float const sy = bossY + std::sin(ang) * radius;
            return std::fabs(RsHalionBeamSignedDist(sx, sy, pairs[0].first, pairs[0].second)) > clear;
        };

        if (!slotClear(slotAngle))
        {
            float const stepArc = static_cast<float>(M_PI) / 18.0f;
            for (int i = 1; i <= 18; ++i)
            {
                if (slotClear(slotAngle + stepArc * static_cast<float>(i)))
                {
                    slotAngle += stepArc * static_cast<float>(i);
                    break;
                }
                if (slotClear(slotAngle - stepArc * static_cast<float>(i)))
                {
                    slotAngle -= stepArc * static_cast<float>(i);
                    break;
                }
            }
        }
    }

    float const targetX = bossX + std::cos(slotAngle) * radius;
    float const targetY = bossY + std::sin(slotAngle) * radius;

    float const dx = targetX - bot->GetPositionX();
    float const dy = targetY - bot->GetPositionY();
    float const dist = std::sqrt(dx * dx + dy * dy);
    if (dist <= (loose ? RS_HALION_P2_STEP : 1.0f))
        return false;

    float const botZ = bot->GetPositionZ();
    auto losOk = [&](float x, float y) -> bool
    {
        return bot->IsWithinLOS(x, y, botZ) || bot->IsWithinLOS(x, y, botZ + 2.0f) ||
               bot->IsWithinLOS(x, y, botZ - 2.0f);
    };

    float const stepLen = std::min(dist, RS_HALION_P2_STEP);
    float const moveX = bot->GetPositionX() + (dx / dist) * stepLen;
    float const moveY = bot->GetPositionY() + (dy / dist) * stepLen;

    if (losOk(moveX, moveY))
        return MoveTo(bot->GetMapId(), moveX, moveY, botZ, false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true);

    float const toBossX = bossX - bot->GetPositionX();
    float const toBossY = bossY - bot->GetPositionY();
    float const toBossLen = std::sqrt(toBossX * toBossX + toBossY * toBossY);
    if (toBossLen < 0.01f)
        return false;

    float const radialStep = std::min(toBossLen, RS_HALION_P2_STEP);
    float const radialX = bot->GetPositionX() + (toBossX / toBossLen) * radialStep;
    float const radialY = bot->GetPositionY() + (toBossY / toBossLen) * radialStep;
    if (!losOk(radialX, radialY))
        return false;

    return MoveTo(bot->GetMapId(), radialX, radialY, botZ, false, false, false, false,
                  MovementPriority::MOVEMENT_FORCED, true);
}

bool RsHalionConsumptionAction::Execute(Event )
{
    auto& clearedForConsumption = RubySanctumHelpers::RsState(bot->GetInstanceId()).clearedForConsumption;
    ObjectGuid const botGuid = bot->GetGUID();

    if (!bot->HasAura(SPELL_MARK_OF_CONSUMPTION) && !bot->HasAura(SPELL_SOUL_CONSUMPTION))
    {
        clearedForConsumption.erase(botGuid);
        return false;
    }

    if (RsHalionIsPhase3(botAI) && bot->HealthBelowPct(RS_HALION_CONSUMPTION_HOLD_PCT))
    {
        if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
        {
            bot->GetMotionMaster()->Clear();
            bot->StopMoving();
        }
        return false;
    }

    Unit* boss = RsHalionTwilightBoss(botAI);
    if (!boss)
        return false;

    if (!clearedForConsumption.count(botGuid))
    {
        if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
        {
            bot->GetMotionMaster()->Clear();
            bot->StopMoving();
        }
        clearedForConsumption.insert(botGuid);
    }

    if (!bot->HasAura(RS_SPELL_NITRO_BOOSTS))
        bot->AddAura(RS_SPELL_NITRO_BOOSTS, bot);

    float const bossX = boss->GetPositionX();
    float const bossY = boss->GetPositionY();
    float const distToBoss = bot->GetExactDist2d(bossX, bossY);

    if (distToBoss >= RS_HALION_CONSUMPTION_OUT_DIST - RS_HALION_CONSUMPTION_OUT_REACH)
    {
        bot->RemoveAurasDueToSpell(SPELL_SOUL_CONSUMPTION, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
        bot->RemoveAurasDueToSpell(SPELL_MARK_OF_CONSUMPTION, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
        return false;
    }

    std::vector<std::pair<Unit*, Unit*>> pairs;
    RsHalionCollectOrbPairs(boss, pairs);

    std::vector<Unit*> pools;
    RsHalionCollectHazardPools(bot, pools);

    auto beamClear = [&](float px, float py) -> bool
    {
        for (auto const& pair : pairs)
            if (std::fabs(RsHalionBeamSignedDist(px, py, pair.first, pair.second)) <= RS_HALION_CUTTER_BOT_CLEAR)
                return false;
        return true;
    };

    bool const startInPool = !RsHalionSpotClearOfPools(pools, bot->GetPositionX(), bot->GetPositionY());

    float const radAngle = distToBoss > 0.01f
        ? std::atan2(bot->GetPositionY() - bossY, bot->GetPositionX() - bossX)
        : 0.0f;
    float const stepArc = static_cast<float>(M_PI) / 36.0f;

    bool moveResult = false;
    auto tryReachOut = [&](bool needBeamClear, bool needPoolClear, bool needPathClear) -> bool
    {
        for (int i = 0; i <= 24; ++i)
        {
            for (float const sign : {1.0f, -1.0f})
            {
                float const a = radAngle + sign * stepArc * static_cast<float>(i);
                float const goalX = bossX + std::cos(a) * RS_HALION_CONSUMPTION_OUT_DIST;
                float const goalY = bossY + std::sin(a) * RS_HALION_CONSUMPTION_OUT_DIST;

                if (needBeamClear && !beamClear(goalX, goalY))
                    continue;
                if (needPoolClear && !RsHalionSpotClearOfPools(pools, goalX, goalY))
                    continue;

                float const dx = goalX - bot->GetPositionX();
                float const dy = goalY - bot->GetPositionY();
                float const dist = std::sqrt(dx * dx + dy * dy);
                float const stepLen = std::min(RS_HALION_CONSUMPTION_STEP, dist);
                float const moveX = dist < 0.01f ? goalX : bot->GetPositionX() + (dx / dist) * stepLen;
                float const moveY = dist < 0.01f ? goalY : bot->GetPositionY() + (dy / dist) * stepLen;

                if (needBeamClear && !beamClear(moveX, moveY))
                    continue;
                if (needPoolClear && !RsHalionSpotClearOfPools(pools, moveX, moveY))
                    continue;
                if (needPathClear && !startInPool &&
                    !RsHalionPathClearOfPools(pools, bot->GetPositionX(), bot->GetPositionY(), moveX, moveY))
                    continue;
                if (!bot->IsWithinLOS(moveX, moveY, bot->GetPositionZ()))
                    continue;

                moveResult = MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false,
                                    true, MovementPriority::MOVEMENT_FORCED);
                return true;
            }
        }
        return false;
    };

    if (tryReachOut(true, true, true) || tryReachOut(true, true, false) ||
        tryReachOut(true, false, false) || tryReachOut(false, false, false))
        return moveResult;

    return false;
}

bool RsHalionHealConsumptionAction::Execute(Event )
{
    Player* target = RsHalionConsumptionHealTarget(botAI);
    if (!target)
        return false;

    if (bot->GetExactDist2d(target) > 36.0f)
        return MoveTo(bot->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT);

    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            return botAI->CastSpell("flash heal", target) || botAI->CastSpell("greater heal", target) ||
                   botAI->CastSpell("renew", target);
        case CLASS_DRUID:
            return botAI->CastSpell("regrowth", target) || botAI->CastSpell("nourish", target) ||
                   botAI->CastSpell("rejuvenation", target);
        case CLASS_SHAMAN:
            return botAI->CastSpell("lesser healing wave", target) || botAI->CastSpell("healing wave", target) ||
                   botAI->CastSpell("riptide", target);
        case CLASS_PALADIN:
            return botAI->CastSpell("flash of light", target) || botAI->CastSpell("holy light", target);
        default:
            break;
    }

    return false;
}

bool RsHalionCutterAction::Execute(Event )
{
    if (bot->HasAura(SPELL_MARK_OF_CONSUMPTION) || bot->HasAura(SPELL_SOUL_CONSUMPTION))
        return false;

    Unit* boss = RsHalionTwilightBoss(botAI);
    if (!boss)
        return false;

    std::vector<std::pair<Unit*, Unit*>> pairs;
    if (!RsHalionCollectOrbPairs(boss, pairs))
        return false;

    if (!RsHalionCutterShouldMove(bot->GetInstanceId()))
        return false;

    if (RsHalionPortalCommit(botAI, bot))
        return false;

    float const bossX = boss->GetPositionX();
    float const bossY = boss->GetPositionY();

    auto nearestBeamDist = [&](float px, float py) -> float
    {
        float best = std::numeric_limits<float>::max();
        for (auto const& pair : pairs)
            best = std::min(best, std::fabs(RsHalionBeamSignedDist(px, py, pair.first, pair.second)));
        return best;
    };

    Player* twilightTank = RsHalionTwilightTank(botAI);

    RubySanctumHelpers::CutterTiming& timing = RubySanctumHelpers::RsState(bot->GetInstanceId()).cutterTiming;
    float const spinSign = timing.spinSign != 0.0f ? timing.spinSign : 1.0f;

    if (twilightTank == bot)
    {
        if (boss->GetVictim() != bot)
            return false;

        float const radius = RS_HALION_CUTTER_FLANK_DIST;
        float const distToBoss = bot->GetExactDist2d(bossX, bossY);
        float const tankAngle = std::atan2(bot->GetPositionY() - bossY, bot->GetPositionX() - bossX);

        if (std::fabs(distToBoss - radius) > 2.0f)
        {
            float const rx = bossX + std::cos(tankAngle) * radius;
            float const ry = bossY + std::sin(tankAngle) * radius;
            if (bot->IsWithinLOS(rx, ry, bot->GetPositionZ()))
                return MoveTo(bot->GetMapId(), rx, ry, bot->GetPositionZ(), false, false, false, true,
                              MovementPriority::MOVEMENT_FORCED, true, false);
        }

        float const trailAngle = RS_HALION_CUTTER_TANK_TRAIL / radius;
        float const tolAngle = RS_HALION_CUTTER_TANK_TRAIL_TOL / radius;

        float bestDist = std::numeric_limits<float>::max();
        float follow = 0.0f;
        for (auto const& pair : pairs)
        {
            float const phi = std::atan2(pair.second->GetPositionY() - pair.first->GetPositionY(),
                                         pair.second->GetPositionX() - pair.first->GetPositionX());
            for (float const c : {phi, phi + static_cast<float>(M_PI)})
            {
                float const targetAngle = c - spinSign * trailAngle;
                float f = spinSign * (targetAngle - tankAngle);
                while (f > static_cast<float>(M_PI))
                    f -= 2.0f * static_cast<float>(M_PI);
                while (f < -static_cast<float>(M_PI))
                    f += 2.0f * static_cast<float>(M_PI);
                if (std::fabs(f) < bestDist)
                {
                    bestDist = std::fabs(f);
                    follow = f;
                }
            }
        }

        if (std::fabs(follow) <= tolAngle)
            return false;

        float const stepLen = 3.0f;
        float const stepAngle = std::min(std::fabs(follow), stepLen / radius);
        float const moveSign = follow >= 0.0f ? spinSign : -spinSign;
        float const moveAngle = tankAngle + moveSign * stepAngle;

        float const moveX = bossX + std::cos(moveAngle) * radius;
        float const moveY = bossY + std::sin(moveAngle) * radius;

        if (nearestBeamDist(moveX, moveY) <= RS_HALION_CUTTER_HALF_WIDTH)
            return false;
        if (!bot->IsWithinLOS(moveX, moveY, bot->GetPositionZ()))
            return false;

        return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true);
    }

    return false;
}
