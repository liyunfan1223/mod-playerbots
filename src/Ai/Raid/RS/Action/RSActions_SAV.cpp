/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Playerbots.h"
#include "RSActions.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace
{
    constexpr int RS_SAVIANA_BEACON_SLOTS = 8;
    constexpr float RS_SAVIANA_BEACON_RING = 18.0f;
    constexpr float RS_SAVIANA_BEACON_REACH = 3.0f;

    std::map<std::pair<uint32, ObjectGuid>, int> g_savianaBeaconSlot;

    Position RsSavianaBeaconSlotPos(int slot)
    {
        float const angle = slot * (2.0f * static_cast<float>(M_PI) / RS_SAVIANA_BEACON_SLOTS);
        return Position(RS_SAVIANA_TANK_POSITION.GetPositionX() + std::cos(angle) * RS_SAVIANA_BEACON_RING,
                        RS_SAVIANA_TANK_POSITION.GetPositionY() + std::sin(angle) * RS_SAVIANA_BEACON_RING,
                        RS_SAVIANA_TANK_POSITION.GetPositionZ());
    }

    constexpr int RS_SAVIANA_MELEE_INNER = 8;
    constexpr int RS_SAVIANA_MELEE_OUTER = 12;
    constexpr int RS_SAVIANA_MELEE_SLOTS = RS_SAVIANA_MELEE_INNER + RS_SAVIANA_MELEE_OUTER;
    constexpr float RS_SAVIANA_MELEE_INNER_RING = 12.0f;
    constexpr float RS_SAVIANA_MELEE_OUTER_RING = 20.0f;
    constexpr float RS_SAVIANA_MELEE_REACH = 3.0f;

    constexpr float RS_SAVIANA_BEACON_DANGER = 12.0f;

    std::map<std::pair<uint32, ObjectGuid>, int> g_savianaMeleeSlot;

    constexpr float RS_SAVIANA_FRONT_RANGE = 30.0f;

    Position RsSavianaMeleeSlotPos(int slot)
    {
        bool const inner = slot < RS_SAVIANA_MELEE_INNER;
        int const count = inner ? RS_SAVIANA_MELEE_INNER : RS_SAVIANA_MELEE_OUTER;
        int const index = inner ? slot : slot - RS_SAVIANA_MELEE_INNER;
        float const ring = inner ? RS_SAVIANA_MELEE_INNER_RING : RS_SAVIANA_MELEE_OUTER_RING;

        float const offset = inner ? 0.0f : (static_cast<float>(M_PI) / count);
        float const angle = index * (2.0f * static_cast<float>(M_PI) / count) + offset;

        return Position(RS_SAVIANA_TANK_POSITION.GetPositionX() + std::cos(angle) * ring,
                        RS_SAVIANA_TANK_POSITION.GetPositionY() + std::sin(angle) * ring,
                        RS_SAVIANA_TANK_POSITION.GetPositionZ());
    }
}

bool RsSavianaConflagrationAction::Execute(Event )
{
    uint32 const instanceId = bot->GetInstanceId();
    auto const myKey = std::make_pair(instanceId, bot->GetGUID());

    if (!bot->HasAura(SPELL_FLAME_BEACON))
    {
        g_savianaBeaconSlot.erase(myKey);
        return false;
    }

    std::set<int> reserved;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || member == bot || !member->IsAlive())
                continue;
            if (!member->HasAura(SPELL_FLAME_BEACON))
                continue;

            auto it = g_savianaBeaconSlot.find(std::make_pair(instanceId, member->GetGUID()));
            if (it != g_savianaBeaconSlot.end())
                reserved.insert(it->second);
        }
    }

    int mySlot = -1;

    auto mem = g_savianaBeaconSlot.find(myKey);
    if (mem != g_savianaBeaconSlot.end() && !reserved.count(mem->second))
        mySlot = mem->second;

    if (mySlot < 0)
    {
        float bestDist = std::numeric_limits<float>::max();
        for (int i = 0; i < RS_SAVIANA_BEACON_SLOTS; ++i)
        {
            if (reserved.count(i))
                continue;
            Position const slotPos = RsSavianaBeaconSlotPos(i);
            float const d = bot->GetExactDist2d(slotPos.GetPositionX(), slotPos.GetPositionY());
            if (d < bestDist)
            {
                bestDist = d;
                mySlot = i;
            }
        }
    }

    if (mySlot < 0)
        return false;

    g_savianaBeaconSlot[myKey] = mySlot;

    Position const target = RsSavianaBeaconSlotPos(mySlot);
    if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) <= RS_SAVIANA_BEACON_REACH)
        return false;

    float destX = target.GetPositionX();
    float destY = target.GetPositionY();
    float destZ = target.GetPositionZ();

    bool exact = true;
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                        bot->GetPositionZ(), destX, destY, destZ))
        exact = false;

    return MoveTo(bot->GetMapId(), destX, destY, destZ, false, false, true, exact,
                  MovementPriority::MOVEMENT_FORCED);
}

bool RsSavianaTankPositionAction::Execute(Event )
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "saviana ragefire");
    if (!boss)
        return false;

    float const safeThreshold = 3.0f;
    float const unsafeLeash = 20.0f;
    float const leash = boss->IsLevitating() ? unsafeLeash : safeThreshold;

    float const distance = bot->GetExactDist2d(RS_SAVIANA_TANK_POSITION.GetPositionX(),
                                               RS_SAVIANA_TANK_POSITION.GetPositionY());

    if (!boss->IsLevitating() && distance <= leash && !bot->IsWithinMeleeRange(boss))
        return MoveTo(bot->GetMapId(), boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT);

    if (distance > leash)
        return MoveTo(bot->GetMapId(), RS_SAVIANA_TANK_POSITION.GetPositionX(),
                      RS_SAVIANA_TANK_POSITION.GetPositionY(), RS_SAVIANA_TANK_POSITION.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT);

    return false;
}

bool RsSavianaAvoidFrontAction::Execute(Event )
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "saviana ragefire");
    if (!boss || boss->IsLevitating())
        return false;

    if (!RsSpotInFrontArc(boss, bot->GetPositionX(), bot->GetPositionY(), RS_SAVIANA_FRONT_RANGE))
        return false;

    float const step = 5.0f;

    if (!PlayerbotAI::IsMelee(bot))
    {
        float const bossX = boss->GetPositionX();
        float const bossY = boss->GetPositionY();
        float const radius = bot->GetExactDist2d(bossX, bossY);

        float const toBot = std::atan2(bot->GetPositionY() - bossY, bot->GetPositionX() - bossX);
        float const leftFlank = boss->GetOrientation() + static_cast<float>(M_PI) / 2.0f;
        float const rightFlank = boss->GetOrientation() - static_cast<float>(M_PI) / 2.0f;

        float const flank = std::fabs(RsAngleDiff(toBot, leftFlank)) <= std::fabs(RsAngleDiff(toBot, rightFlank))
            ? leftFlank : rightFlank;
        float const flankX = bossX + std::cos(flank) * radius;
        float const flankY = bossY + std::sin(flank) * radius;

        float const dx = flankX - bot->GetPositionX();
        float const dy = flankY - bot->GetPositionY();
        float const dist = std::sqrt(dx * dx + dy * dy);
        if (dist < 0.01f)
            return false;

        float const hop = std::min(step, dist);
        float const moveX = bot->GetPositionX() + (dx / dist) * hop;
        float const moveY = bot->GetPositionY() + (dy / dist) * hop;

        if (!bot->IsWithinLOS(moveX, moveY, bot->GetPositionZ()))
            return false;

        return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true);
    }

    int const directions = 12;

    for (int i = 0; i < directions; ++i)
    {
        float const angle = (i * 2.0f * static_cast<float>(M_PI)) / directions;
        float const moveX = bot->GetPositionX() + step * std::cos(angle);
        float const moveY = bot->GetPositionY() + step * std::sin(angle);
        float const moveZ = bot->GetPositionZ();

        if (RsSpotInFrontArc(boss, moveX, moveY, RS_SAVIANA_FRONT_RANGE))
            continue;

        if (!bot->IsWithinLOS(moveX, moveY, moveZ))
            continue;

        return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true);
    }

    return false;
}

bool RsSavianaMeleeSpreadAction::Execute(Event )
{
    uint32 const instanceId = bot->GetInstanceId();
    auto const myKey = std::make_pair(instanceId, bot->GetGUID());

    Unit* boss = AI_VALUE2(Unit*, "find target", "saviana ragefire");
    if (!boss || !boss->IsLevitating())
    {
        g_savianaMeleeSlot.erase(myKey);
        return false;
    }

    auto moveChecked = [&](float destX, float destY, float destZ)
    {
        bool const exact = bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(),
                                                                          bot->GetPositionY(), bot->GetPositionZ(),
                                                                          destX, destY, destZ);
        return MoveTo(bot->GetMapId(), destX, destY, destZ, false, false, true, exact,
                      MovementPriority::MOVEMENT_FORCED);
    };

    std::set<int> reserved;
    std::vector<Position> beacons;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || member == bot || !member->IsAlive())
                continue;

            if (member->HasAura(SPELL_FLAME_BEACON))
            {
                beacons.push_back(member->GetPosition());
                continue;
            }

            auto it = g_savianaMeleeSlot.find(std::make_pair(instanceId, member->GetGUID()));
            if (it != g_savianaMeleeSlot.end())
                reserved.insert(it->second);
        }
    }

    auto slotSafe = [&](int slot) -> bool
    {
        if (reserved.count(slot))
            return false;
        Position const slotPos = RsSavianaMeleeSlotPos(slot);
        for (Position const& beacon : beacons)
            if (beacon.GetExactDist2d(slotPos.GetPositionX(), slotPos.GetPositionY()) < RS_SAVIANA_BEACON_DANGER)
                return false;
        return true;
    };

    int mySlot = -1;

    auto mem = g_savianaMeleeSlot.find(myKey);
    if (mem != g_savianaMeleeSlot.end() && slotSafe(mem->second))
        mySlot = mem->second;

    if (mySlot < 0)
    {
        float bestDist = std::numeric_limits<float>::max();
        for (int i = 0; i < RS_SAVIANA_MELEE_SLOTS; ++i)
        {
            if (!slotSafe(i))
                continue;
            Position const slotPos = RsSavianaMeleeSlotPos(i);
            float const d = bot->GetExactDist2d(slotPos.GetPositionX(), slotPos.GetPositionY());
            if (d < bestDist)
            {
                bestDist = d;
                mySlot = i;
            }
        }
    }

    if (mySlot < 0)
    {
        g_savianaMeleeSlot.erase(myKey);

        Position const* nearest = nullptr;
        float nearestDist = std::numeric_limits<float>::max();
        for (Position const& beacon : beacons)
        {
            float const d = bot->GetExactDist2d(beacon.GetPositionX(), beacon.GetPositionY());
            if (d < nearestDist)
            {
                nearestDist = d;
                nearest = &beacon;
            }
        }

        if (!nearest || nearestDist >= RS_SAVIANA_BEACON_DANGER)
            return false;

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

        float const stepOut = RS_SAVIANA_BEACON_DANGER - nearestDist + 2.0f;
        return moveChecked(bot->GetPositionX() + dx * stepOut, bot->GetPositionY() + dy * stepOut,
                           bot->GetPositionZ());
    }

    g_savianaMeleeSlot[myKey] = mySlot;

    Position const target = RsSavianaMeleeSlotPos(mySlot);
    if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) <= RS_SAVIANA_MELEE_REACH)
        return false;

    return moveChecked(target.GetPositionX(), target.GetPositionY(), target.GetPositionZ());
}
