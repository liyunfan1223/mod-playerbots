/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RSActions.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <utility>
#include <vector>

namespace
{
    constexpr float RS_BALTHARUS_FRONT_RANGE = 16.0f;

    std::vector<Unit*> RsBaltharusCasters(PlayerbotAI* botAI)
    {
        std::vector<Unit*> casters;

        if (Unit* boss = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "baltharus the warborn")->Get())
            casters.push_back(boss);

        RsCollectTargets(botAI, casters,
                         [](Unit* unit) { return unit->GetEntry() == NPC_BALTHARUS_THE_WARBORN_CLONE; });

        return casters;
    }

    bool RsBaltharusSpotInFrontOfAny(std::vector<Unit*> const& casters, float x, float y)
    {
        for (Unit* caster : casters)
            if (RsSpotInFrontArc(caster, x, y, RS_BALTHARUS_FRONT_RANGE))
                return true;
        return false;
    }

    struct MarkInfo
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        bool hasDest = false;
        bool reached = false;
    };

    std::map<std::pair<uint32, ObjectGuid>, MarkInfo> gMarkedAnchors;

    MarkInfo* GetMarkInfo(uint32 instanceId, ObjectGuid guid)
    {
        auto it = gMarkedAnchors.find(std::make_pair(instanceId, guid));
        return it != gMarkedAnchors.end() ? &it->second : nullptr;
    }

    void EnsureMarkedAnchor(uint32 instanceId, ObjectGuid guid)
    {
        gMarkedAnchors.emplace(std::make_pair(instanceId, guid), MarkInfo{});
    }
}

bool RsBaltharusBrandAction::Execute(Event )
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    uint32 const instanceId = bot->GetInstanceId();

    for (auto it = gMarkedAnchors.begin(); it != gMarkedAnchors.end();)
    {
        if (it->first.first == instanceId)
        {
            Player* owner = ObjectAccessor::FindPlayer(it->first.second);
            if (!owner || !owner->HasAura(SPELL_ENERVATING_BRAND))
            {
                it = gMarkedAnchors.erase(it);
                continue;
            }
        }
        ++it;
    }

    if (bot->HasAura(SPELL_ENERVATING_BRAND))
        EnsureMarkedAnchor(instanceId, bot->GetGUID());

    Group* const group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> others;
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (member && member != bot && member->IsAlive())
            others.push_back(member);
    }

    if (others.empty())
        return false;

    MarkInfo* myMark = GetMarkInfo(instanceId, bot->GetGUID());

    if (myMark && myMark->hasDest)
    {

        constexpr float settleDist = 1.5f;

        float const distToDest = bot->GetExactDist2d(myMark->x, myMark->y);
        if (distToDest <= settleDist)
        {
            myMark->reached = true;

            return false;
        }

        return MoveTo(bot->GetMapId(), myMark->x, myMark->y, myMark->z, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true);
    }

    constexpr float MAX_NON_MARKED_RANGED_DISTANCE = 35.0f;
    bool const isNonMarkedRanged = (myMark == nullptr) && (!PlayerbotAI::IsMelee(bot));

    auto minDistFrom = [&](float x, float y) -> float
    {
        float best = std::numeric_limits<float>::max();
        for (Player* member : others)
        {
            MarkInfo* memberMark = GetMarkInfo(instanceId, member->GetGUID());

            if (myMark)
            {
                if (memberMark)
                    continue;
            }
            else
            {

                if (memberMark && !memberMark->reached)
                    continue;
            }

            best = std::min(best, member->GetExactDist2d(x, y));
        }
        return best;
    };

    bool const hereClear = minDistFrom(bot->GetPositionX(), bot->GetPositionY()) >= RS_BALTHARUS_BRAND_RANGE;
    std::vector<Unit*> const casters = RsBaltharusCasters(botAI);
    bool const hereInFront = RsBaltharusSpotInFrontOfAny(casters, bot->GetPositionX(), bot->GetPositionY());

    float const tankX = RS_BALTHARUS_TANK_POSITION.GetPositionX();
    float const tankY = RS_BALTHARUS_TANK_POSITION.GetPositionY();
    float const distToTank = bot->GetExactDist2d(tankX, tankY);
    bool effectiveHereClear = hereClear;
    if (isNonMarkedRanged && distToTank > MAX_NON_MARKED_RANGED_DISTANCE)
        effectiveHereClear = false;

    if (effectiveHereClear && !hereInFront)
        return false;

    int const directions = 8;
    float const increment = 3.0f;
    float const searchRadius = 30.0f;
    float bestX = bot->GetPositionX();
    float bestY = bot->GetPositionY();
    float bestZ = bot->GetPositionZ();
    float bestScore = -1.0f;

    for (int i = 0; i < directions; ++i)
    {
        float const angle = (i * 2.0f * static_cast<float>(M_PI)) / directions;
        for (float distance = increment; distance <= searchRadius; distance += increment)
        {
            float const moveX = bot->GetPositionX() + distance * std::cos(angle);
            float const moveY = bot->GetPositionY() + distance * std::sin(angle);
            float const moveZ = bot->GetPositionZ();

            if (isNonMarkedRanged)
            {
                float const dxTank = moveX - tankX;
                float const dyTank = moveY - tankY;
                float const dTank = std::sqrt(dxTank * dxTank + dyTank * dyTank);
                if (dTank > MAX_NON_MARKED_RANGED_DISTANCE)
                    continue;
            }

            float const clearance = minDistFrom(moveX, moveY);
            if (clearance < RS_BALTHARUS_BRAND_RANGE)
                continue;

            if (RsBaltharusSpotInFrontOfAny(casters, moveX, moveY))
                continue;

            if (!bot->IsWithinLOS(moveX, moveY, moveZ))
                continue;

            if (myMark)
            {
                bool tooCloseToOtherMarked = false;
                for (auto const& entry : gMarkedAnchors)
                {

                    if (entry.first.first != instanceId || entry.first.second == bot->GetGUID())
                        continue;
                    MarkInfo const& other = entry.second;
                    if (!other.hasDest)
                        continue;
                    float const dx = moveX - other.x;
                    float const dy = moveY - other.y;
                    float const dd = std::sqrt(dx * dx + dy * dy);
                    if (dd < RS_BALTHARUS_BRAND_RANGE)
                    {
                        tooCloseToOtherMarked = true;
                        break;
                    }
                }
                if (tooCloseToOtherMarked)
                    continue;
            }

            if (clearance > bestScore)
            {
                bestScore = clearance;
                bestX = moveX;
                bestY = moveY;
                bestZ = moveZ;
            }
        }
    }

    if (bestScore > 0.0f)
    {

        if (myMark)
        {
            myMark->x = bestX;
            myMark->y = bestY;
            myMark->z = bestZ;
            myMark->hasDest = true;
            myMark->reached = false;
        }

        return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true);
    }

    if (myMark)
    {
        Player* nearest = nullptr;
        float nearestDist = std::numeric_limits<float>::max();
        for (Player* member : others)
        {
            float const d = member->GetExactDist2d(bot->GetPositionX(), bot->GetPositionY());
            if (d < nearestDist)
            {
                nearestDist = d;
                nearest = member;
            }
        }

        if (nearest)
        {
            float dx = bot->GetPositionX() - nearest->GetPositionX();
            float dy = bot->GetPositionY() - nearest->GetPositionY();
            float const len = std::sqrt(dx * dx + dy * dy);
            if (len < 0.01f)
            {
                dx = 1.0f;
                dy = 0.0f;
            }
            else
            {
                dx /= len;
                dy /= len;
            }

            float const stepOut = (RS_BALTHARUS_BRAND_RANGE - nearestDist) + 2.0f;
            float fbX = bot->GetPositionX() + dx * stepOut;
            float fbY = bot->GetPositionY() + dy * stepOut;
            float fbZ = bot->GetPositionZ();

            if (!RsBaltharusSpotInFrontOfAny(casters, fbX, fbY) && bot->IsWithinLOS(fbX, fbY, fbZ))
            {
                myMark->x = fbX;
                myMark->y = fbY;
                myMark->z = fbZ;
                myMark->hasDest = true;
                myMark->reached = false;

                return MoveTo(bot->GetMapId(), fbX, fbY, fbZ, false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true);
            }
        }
    }

    if (isNonMarkedRanged && distToTank > MAX_NON_MARKED_RANGED_DISTANCE)
    {
        float const dx = bot->GetPositionX() - tankX;
        float const dy = bot->GetPositionY() - tankY;
        float len = std::sqrt(dx * dx + dy * dy);
        float tx, ty, tz = bot->GetPositionZ();
        if (len > 0.0001f)
        {
            tx = tankX + (dx / len) * MAX_NON_MARKED_RANGED_DISTANCE;
            ty = tankY + (dy / len) * MAX_NON_MARKED_RANGED_DISTANCE;
        }
        else
        {

            tx = tankX + MAX_NON_MARKED_RANGED_DISTANCE;
            ty = tankY;
        }

        if (!RsBaltharusSpotInFrontOfAny(casters, tx, ty) && bot->IsWithinLOS(tx, ty, tz))
            return MoveTo(bot->GetMapId(), tx, ty, tz, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true);
    }

    return false;
}

bool RsBaltharusHealerPositionAction::Execute(Event )
{
    if (!PlayerbotAI::IsHeal(bot) || PlayerbotAI::IsTank(bot))
        return false;

    if (bot->HasAura(SPELL_ENERVATING_BRAND))
        return false;

    float const distToHealSpot = bot->GetExactDist2d(RS_BALTHARUS_HEALER_POSITION.GetPositionX(),
                                                     RS_BALTHARUS_HEALER_POSITION.GetPositionY());
    if (distToHealSpot <= RS_BALTHARUS_HEALER_REACH)
        return false;

    return MoveTo(bot->GetMapId(), RS_BALTHARUS_HEALER_POSITION.GetPositionX(),
                  RS_BALTHARUS_HEALER_POSITION.GetPositionY(),
                  RS_BALTHARUS_HEALER_POSITION.GetPositionZ(),
                  false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool RsBaltharusAvoidFrontAction::Execute(Event )
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    std::vector<Unit*> const casters = RsBaltharusCasters(botAI);
    if (casters.empty())
        return false;

    Unit* anchor = nullptr;
    if (Unit* victim = bot->GetVictim())
        for (Unit* caster : casters)
            if (caster == victim)
                anchor = caster;
    if (!anchor)
    {
        float nearestDist = std::numeric_limits<float>::max();
        for (Unit* caster : casters)
        {
            float const d = bot->GetExactDist2d(caster);
            if (d < nearestDist)
            {
                nearestDist = d;
                anchor = caster;
            }
        }
    }
    if (!anchor)
        return false;

    float nearestCaster = std::numeric_limits<float>::max();
    for (Unit* caster : casters)
        nearestCaster = std::min(nearestCaster, bot->GetExactDist2d(caster));
    if (nearestCaster > 20.0f)
        return false;

    if (PlayerbotAI::IsMelee(bot))
    {
        float const flankDist = 4.0f;
        float const settleDist = 2.0f;

        static float const flankOffsets[] = {
            static_cast<float>(M_PI),
            static_cast<float>(M_PI) * 0.75f,
            static_cast<float>(M_PI) * 1.25f,
            static_cast<float>(M_PI) * 0.5f,
            static_cast<float>(M_PI) * 1.5f
        };

        for (float const offset : flankOffsets)
        {
            float const angle = anchor->GetOrientation() + offset;
            float const targetX = anchor->GetPositionX() + flankDist * std::cos(angle);
            float const targetY = anchor->GetPositionY() + flankDist * std::sin(angle);
            float const targetZ = anchor->GetPositionZ();

            if (RsBaltharusSpotInFrontOfAny(casters, targetX, targetY))
                continue;

            if (!bot->IsWithinLOS(targetX, targetY, targetZ))
                continue;

            if (bot->GetExactDist2d(targetX, targetY) < settleDist)
                return false;

            return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true);
        }
    }

    if (!RsBaltharusSpotInFrontOfAny(casters, bot->GetPositionX(), bot->GetPositionY()))
        return false;

    float const step = 7.0f;
    int const directions = 12;

    for (int i = 0; i < directions; ++i)
    {
        float const angle = (i * 2.0f * static_cast<float>(M_PI)) / directions;
        float const moveX = bot->GetPositionX() + step * std::cos(angle);
        float const moveY = bot->GetPositionY() + step * std::sin(angle);
        float const moveZ = bot->GetPositionZ();

        if (RsBaltharusSpotInFrontOfAny(casters, moveX, moveY))
            continue;

        if (!bot->IsWithinLOS(moveX, moveY, moveZ))
            continue;

        return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true);
    }

    return false;
}

bool RsBaltharusTankPositionAction::HoldAt(Position const& spot, Unit* faceAwayFrom)
{
    bool const onSpot = bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) <= 3.0f;
    bool const inMelee = faceAwayFrom && bot->IsWithinMeleeRange(faceAwayFrom);

    if (faceAwayFrom && !inMelee && !onSpot)
        return MoveTo(bot->GetMapId(), faceAwayFrom->GetPositionX(), faceAwayFrom->GetPositionY(),
                      faceAwayFrom->GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT);

    if (!onSpot)
        return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_COMBAT);

    if (faceAwayFrom && std::fabs(bot->GetOrientation() - RS_BALTHARUS_TANK_FACING) > 0.1f)
        bot->SetFacingTo(RS_BALTHARUS_TANK_FACING);

    return false;
}

bool RsBaltharusTankPositionAction::Execute(Event )
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!(bot->HasAura(RS_SPELL_PAIN_SUPPRESION)))
        bot->AddAura(RS_SPELL_PAIN_SUPPRESION, bot);

    Unit* boss = RsFindTarget(botAI, [](Unit* unit) { return unit->GetEntry() == NPC_BALTHARUS_THE_WARBORN; });
    if (!boss)
        return false;

    bool const isMainTank = PlayerbotAI::IsMainTank(bot);

    Player* assistTankPlayer = nullptr;
    Player* mainTankPlayer = nullptr;
    bool assistTankAlive = false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (!assistTankPlayer && PlayerbotAI::IsAssistTank(member))
            {
                assistTankPlayer = member;
                assistTankAlive = true;
            }

            if (!mainTankPlayer && PlayerbotAI::IsMainTank(member))
            {
                mainTankPlayer = member;
            }

            if (assistTankPlayer && mainTankPlayer)
                break;
        }
    }

    auto IsPlayerMarked = [&](Player* player) -> bool
    {
        if (!player)
            return false;
        if (Group* grp = player->GetGroup())
        {
            for (uint8 icon = 0; icon < 8; ++icon)
            {
                if (grp->GetTargetIcon(icon) == player->GetGUID())
                    return true;
            }
        }
        return false;
    };

    std::vector<Unit*> clones;
    RsCollectTargets(botAI, clones, [](Unit* unit) { return unit->GetEntry() == NPC_BALTHARUS_THE_WARBORN_CLONE; });

    if (isMainTank)
    {
        bot->SetTarget(boss->GetGUID());

        if (Group* group = bot->GetGroup())
        {
            if (group->GetTargetIcon(RtiTargetValue::skullIndex) != boss->GetGUID())
                group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), boss->GetGUID());
        }

        if (boss->GetVictim() != bot)
            RsCastClassTaunt(botAI, bot, boss);

        if (!assistTankAlive)
        {
            for (Unit* clone : clones)
            {
                if (!clone)
                    continue;

                if (clone->GetVictim() != bot)
                    RsCastClassTaunt(botAI, bot, clone);
            }
        }

        boss->SetFacingTo(RS_BALTHARUS_TANK_FACING);
        for (Unit* clone : clones)
        {
            if (clone && clone->IsAlive())
                clone->SetFacingTo(RS_BALTHARUS_TANK_FACING);
        }

        if (IsPlayerMarked(bot))
            return false;

        return HoldAt(RS_BALTHARUS_TANK_POSITION, boss);
    }

    if (PlayerbotAI::IsAssistTank(bot))
    {
        if (clones.empty())
            return false;

        std::vector<Unit*> myClones;
        for (Unit* clone : clones)
        {
            if (!clone)
                continue;

            if (mainTankPlayer)
            {
                float const dAssist = bot->GetExactDist2d(clone);
                float const dMain = mainTankPlayer->GetExactDist2d(clone);
                if (dAssist < dMain)
                    myClones.push_back(clone);
            }
            else
            {

                myClones.push_back(clone);
            }
        }

        if (myClones.empty())
        {
            Unit* nearest = nullptr;
            float best = std::numeric_limits<float>::max();
            for (Unit* clone : clones)
            {
                if (!clone)
                    continue;
                float const d = bot->GetExactDist2d(clone);
                if (d < best)
                {
                    best = d;
                    nearest = clone;
                }
            }
            if (nearest)
                myClones.push_back(nearest);
        }

        for (Unit* clone : myClones)
        {
            if (!clone)
                continue;

            if (clone->GetVictim() != bot)
                RsCastClassTaunt(botAI, bot, clone);

            if (clone->IsAlive())
                clone->SetFacingTo(RS_BALTHARUS_TANK_FACING);
        }

        Unit* myClone = myClones.front();
        if (myClone)
            bot->SetTarget(myClone->GetGUID());

        boss->SetFacingTo(RS_BALTHARUS_TANK_FACING);

        if (IsPlayerMarked(bot))
            return false;

        float const assistX = RS_BALTHARUS_TANK_POSITION.GetPositionX() - 25.0f;
        float const assistY = RS_BALTHARUS_TANK_POSITION.GetPositionY();
        float const assistZ = RS_BALTHARUS_TANK_POSITION.GetPositionZ();

        bool const onAssistSpot = bot->GetExactDist2d(assistX, assistY) <= 3.0f;
        bool const cloneInMelee = myClone && bot->IsWithinMeleeRange(myClone);

        if (myClone && !cloneInMelee && !onAssistSpot)
            return MoveTo(bot->GetMapId(), myClone->GetPositionX(), myClone->GetPositionY(),
                          myClone->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);

        if (!onAssistSpot)
            return MoveTo(bot->GetMapId(), assistX, assistY, assistZ, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);

        if (std::fabs(bot->GetOrientation() - RS_BALTHARUS_TANK_FACING) > 0.1f)
            bot->SetFacingTo(RS_BALTHARUS_TANK_FACING);
        return false;
    }

    return false;
}
