/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MechActions.h"
#include "AiFactory.h"
#include "Creature.h"
#include "Group.h"
#include "MechShared.h"
#include "Playerbots.h"
#include "Timer.h"
#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace
{
    bool Normalize(float& x, float& y)
    {
        float const len = std::sqrt(x * x + y * y);
        if (len < 0.01f)
            return false;
        x /= len;
        y /= len;
        return true;
    }

    float Dist2d(float ax, float ay, float bx, float by)
    {
        float const dx = ax - bx, dy = ay - by;
        return std::sqrt(dx * dx + dy * dy);
    }

    float MinSegDist(float ax, float ay, float bx, float by, float px, float py)
    {
        float const dx = bx - ax, dy = by - ay;
        float const len2 = dx * dx + dy * dy;
        float t = (len2 < 1e-6f) ? 0.0f : ((px - ax) * dx + (py - ay) * dy) / len2;
        t = std::max(0.0f, std::min(1.0f, t));
        return Dist2d(ax + t * dx, ay + t * dy, px, py);
    }

    bool PartyCentroid(Player* bot, float& cx, float& cy)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        float sx = 0.0f, sy = 0.0f;
        int n = 0;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive() ||
                member->GetMapId() != bot->GetMapId())
                continue;
            if (bot->GetExactDist2d(member) > 60.0f)
                continue;
            sx += member->GetPositionX();
            sy += member->GetPositionY();
            ++n;
        }
        if (n == 0)
            return false;
        cx = sx / n;
        cy = sy / n;
        return true;
    }

    bool TankPosition(Player* bot, float& tx, float& ty)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        float best = 1e18f;
        bool found = false;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive() ||
                member->GetMapId() != bot->GetMapId())
                continue;
            if (!PlayerbotAI::IsTank(member))
                continue;
            float const d = bot->GetExactDist2d(member);
            if (d < best)
            {
                best = d;
                tx = member->GetPositionX();
                ty = member->GetPositionY();
                found = true;
            }
        }
        return found;
    }

    constexpr float KITE_FAN_OFFSETS[] = {
        0.0f, 0.35f, -0.35f, 0.70f, -0.70f, 1.05f, -1.05f, 1.40f, -1.40f,
        1.75f, -1.75f, 2.10f, -2.10f, 2.45f, -2.45f, 2.80f, -2.80f, 3.14f };

    bool SpotSafe(std::vector<std::pair<float, float>> const& flames,
                  std::vector<MechanarFlames::TrailPatch> const& patches, float x, float y,
                  float flameClear = MechanarFlames::INFERNO_SAFE_DIST)
    {
        for (auto const& f : flames)
        {
            float const dx = x - f.first, dy = y - f.second;
            if (dx * dx + dy * dy < flameClear * flameClear)
                return false;
        }
        for (MechanarFlames::TrailPatch const& p : patches)
        {
            float const dx = x - p.x, dy = y - p.y;
            float const r = p.radius + MechanarFlames::TRAIL_DANGER_MARGIN;
            if (dx * dx + dy * dy < r * r)
                return false;
        }
        return true;
    }

    int SegSamples(float ax, float ay, float bx, float by)
    {
        return std::max(5, static_cast<int>(Dist2d(ax, ay, bx, by) / 3.0f) + 1);
    }

    bool SegSafe(std::vector<std::pair<float, float>> const& flames,
                 std::vector<MechanarFlames::TrailPatch> const& patches,
                 float ax, float ay, float bx, float by)
    {
        int const n = SegSamples(ax, ay, bx, by);
        for (int k = 1; k <= n; ++k)
        {
            float const t = static_cast<float>(k) / n;
            if (!SpotSafe(flames, patches, ax + (bx - ax) * t, ay + (by - ay) * t))
                return false;
        }
        return true;
    }

    float HazardClearance(std::vector<std::pair<float, float>> const& flames,
                          std::vector<MechanarFlames::TrailPatch> const& patches, float x, float y,
                          float flameClear = MechanarFlames::INFERNO_SAFE_DIST)
    {
        float clear = 1e9f;
        for (auto const& f : flames)
            clear = std::min(clear, Dist2d(x, y, f.first, f.second) - flameClear);
        for (MechanarFlames::TrailPatch const& p : patches)
            clear = std::min(clear, Dist2d(x, y, p.x, p.y) - (p.radius + MechanarFlames::TRAIL_DANGER_MARGIN));
        return clear;
    }

    float SegClearance(std::vector<std::pair<float, float>> const& flames,
                       std::vector<MechanarFlames::TrailPatch> const& patches,
                       float ax, float ay, float bx, float by)
    {
        int const n = SegSamples(ax, ay, bx, by);
        float clear = 1e9f;
        for (int k = 0; k <= n; ++k)
        {
            float const t = static_cast<float>(k) / n;
            clear = std::min(clear, HazardClearance(flames, patches, ax + (bx - ax) * t, ay + (by - ay) * t));
        }
        return clear;
    }
}

bool SepethreaKiteFlameAction::Execute(Event)
{
    std::vector<Creature*> chasers;
    MechanarFlames::CollectChasingFlames(bot, chasers);
    if (chasers.empty())
        return false;

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();

    Creature* flame = chasers.front();
    float dist = bot->GetDistance2d(flame);
    for (size_t i = 1; i < chasers.size(); ++i)
    {
        float const d = bot->GetDistance2d(chasers[i]);
        if (d < dist)
        {
            dist = d;
            flame = chasers[i];
        }
    }

    std::vector<std::pair<float, float>> chaserPos;
    chaserPos.reserve(chasers.size());
    for (Creature* chaser : chasers)
        chaserPos.emplace_back(chaser->GetPositionX(), chaser->GetPositionY());

    float fx = bx - flame->GetPositionX();
    float fy = by - flame->GetPositionY();
    if (!Normalize(fx, fy))
    {
        fx = bx - MechanarFlames::ROOM_ANCHOR_X;
        fy = by - MechanarFlames::ROOM_ANCHOR_Y;
        if (!Normalize(fx, fy))
        {
            fx = 1.0f;
            fy = 0.0f;
        }
    }

    std::vector<std::pair<float, float>> avoidFlames;
    MechanarFlames::CollectAvoidFlames(bot, avoidFlames);
    std::vector<MechanarFlames::TrailPatch> patches;
    MechanarFlames::CollectTrailPatches(bot, 60.0f, patches);

    float tankX = 0.0f, tankY = 0.0f;
    bool const haveTank = TankPosition(bot, tankX, tankY);
    bool const leashed = PlayerbotAI::IsHeal(bot) && haveTank;
    float const tankDist = leashed ? bot->GetExactDist2d(tankX, tankY) : 0.0f;

    float px = tankX, py = tankY;
    bool const haveParty = haveTank || PartyCentroid(bot, px, py);
    float const groupDist = haveParty ? bot->GetExactDist2d(px, py) : 1000.0f;

    uint32 const nowMs = getMSTime();
    bool turning = _turnUntilMs != 0 && nowMs < _turnUntilMs;
    if (turning && (leashed || flame->GetGUID() != _turnFlame))
    {
        _turnUntilMs = 0;
        turning = false;
    }
    if (turning && !_turnLateral && dist > MechanarFlames::KITE_THRESHOLD &&
        (by - flame->GetPositionY()) * -_turnEnd > MechanarFlames::KITE_TURN_EXIT_PAST)
    {
        _turnUntilMs = 0;
        turning = false;
    }
    if (!turning && !leashed && std::fabs(fy) >= 0.5f && dist <= MechanarFlames::KITE_TURN_THRESHOLD)
    {
        float const endSign = fy > 0.0f ? 1.0f : -1.0f;
        float const endDist = fy > 0.0f ? MechanarFlames::ROOM_Y_MAX - by
                                        : by - MechanarFlames::ROOM_Y_MIN;
        if (endDist < MechanarFlames::KITE_TURN_RUNWAY)
        {
            float bestKey = -1e18f, bestLaneClear = -1e18f;
            for (float side : { 1.0f, -1.0f })
            {
                float const sideX = side > 0.0f
                                        ? MechanarFlames::ROOM_X_MAX - MechanarFlames::KITE_TURN_WALL_INSET
                                        : MechanarFlames::ROOM_X_MIN + MechanarFlames::KITE_TURN_WALL_INSET;
                float laneEndY = flame->GetPositionY() - endSign * MechanarFlames::KITE_TURN_PASS_BEHIND;
                laneEndY = std::min(std::max(laneEndY, MechanarFlames::ROOM_Y_MIN + 3.0f),
                                    MechanarFlames::ROOM_Y_MAX - 3.0f);
                float const laneClear =
                    std::min(SegClearance(avoidFlames, patches, bx, by, sideX, by),
                             SegClearance(avoidFlames, patches, sideX, by, sideX, laneEndY));
                float const key = laneClear * MechanarFlames::KITE_TURN_CLEAR_WEIGHT +
                                  std::fabs(sideX - flame->GetPositionX());
                if (key > bestKey)
                {
                    bestKey = key;
                    bestLaneClear = laneClear;
                    _turnSide = side;
                }
            }
            if (bestLaneClear > 0.0f)
            {
                _turnEnd = endSign;
                _turnFlame = flame->GetGUID();
                _turnLateral = true;
                _turnUntilMs = nowMs + MechanarFlames::KITE_TURN_FAILSAFE_MS;
                turning = true;
            }
        }
    }
    if (turning)
    {
        float const sideX = _turnSide > 0.0f
                                ? MechanarFlames::ROOM_X_MAX - MechanarFlames::KITE_TURN_WALL_INSET
                                : MechanarFlames::ROOM_X_MIN + MechanarFlames::KITE_TURN_WALL_INSET;
        if (_turnLateral && std::fabs(bx - sideX) <= 2.0f)
            _turnLateral = false;

        float laneEndY = flame->GetPositionY() - _turnEnd * MechanarFlames::KITE_TURN_PASS_BEHIND;
        laneEndY = std::min(std::max(laneEndY, MechanarFlames::ROOM_Y_MIN + 3.0f),
                            MechanarFlames::ROOM_Y_MAX - 3.0f);

        float laneClear;
        if (_turnLateral)
            laneClear = std::min(SegClearance(avoidFlames, patches, bx, by, sideX, by),
                                 SegClearance(avoidFlames, patches, sideX, by, sideX, laneEndY));
        else
            laneClear = SegClearance(avoidFlames, patches, bx, by, sideX, laneEndY);

        if (laneClear < MechanarFlames::KITE_TURN_ABORT_CLEAR)
        {
            _turnUntilMs = 0;
            turning = false;
        }
        else
            return MoveTo(bot->GetMapId(), sideX, _turnLateral ? by : laneEndY, bot->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (dist >= MechanarFlames::KITE_THRESHOLD && groupDist >= MechanarFlames::KITE_GROUP_CLEARANCE &&
        (!leashed || tankDist <= MechanarFlames::HEALER_KITE_LEASH) &&
        SpotSafe(avoidFlames, patches, bx, by))
        return false;

    if (haveParty)
    {
        float ax = bot->GetPositionX() - px;
        float ay = bot->GetPositionY() - py;
        if (Normalize(ax, ay))
        {
            float const clr = MechanarFlames::KITE_GROUP_CLEARANCE;
            float bias = (clr - groupDist) / clr;
            if (bias < 0.0f)
                bias = 0.0f;
            bias *= 1.5f;
            fx += bias * ax;
            fy += bias * ay;
            Normalize(fx, fy);
        }
    }

    float const step = MechanarFlames::KITE_STEP;

    if (!MechanarFlames::InRoom(bx, by))
    {
        float inX = bx, inY = by;
        MechanarFlames::ClampIntoRoom(inX, inY);
        float dx = inX - bx, dy = inY - by;
        if (Normalize(dx, dy))
        {
            float rX = bx + dx * step, rY = by + dy * step;
            MechanarFlames::ClampIntoRoom(rX, rY);
            return MoveTo(bot->GetMapId(), rX, rY, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    float const stepLens[] = { step, 10.0f, 7.0f };
    float const baseAng = std::atan2(fy, fx);
    float bestX = 0.0f, bestY = 0.0f, bestScore = -1e18f;
    bool found = false;
    float safeX = 0.0f, safeY = 0.0f, bestClear = -1e18f;
    bool haveFallback = false;
    for (float off : KITE_FAN_OFFSETS)
    {
        float const ang = baseAng + off;
        for (float len : stepLens)
        {
            float const destX = bx + std::cos(ang) * len;
            float const destY = by + std::sin(ang) * len;
            if (!MechanarFlames::SegmentInRoom(bx, by, destX, destY))
                continue;

            float const clear = HazardClearance(avoidFlames, patches, destX, destY);
            if (clear > bestClear)
            {
                bestClear = clear;
                safeX = destX;
                safeY = destY;
                haveFallback = true;
            }

            if (!SegSafe(avoidFlames, patches, bx, by, destX, destY))
                continue;

            float const destTankDist = leashed ? Dist2d(destX, destY, tankX, tankY) : 0.0f;
            if (leashed && destTankDist > MechanarFlames::HEALER_KITE_LEASH_MAX &&
                destTankDist >= tankDist)
                continue;

            float fd = 1e9f;
            for (auto const& chaser : chaserPos)
                fd = std::min(fd, Dist2d(destX, destY, chaser.first, chaser.second));
            float const longAxis = leashed ? 0.0f : MechanarFlames::KITE_LONG_AXIS_WEIGHT;
            float score = fd + longAxis * std::fabs(std::sin(ang)) + len * 0.1f;
            if (leashed)
            {
                if (destTankDist > MechanarFlames::HEALER_KITE_LEASH)
                    score -= MechanarFlames::HEALER_KITE_LEASH_WEIGHT *
                             (destTankDist - MechanarFlames::HEALER_KITE_LEASH);
                float passDist = 1e9f;
                for (auto const& chaser : chaserPos)
                    passDist = std::min(passDist, MinSegDist(bx, by, destX, destY,
                                                             chaser.first, chaser.second));
                if (passDist < MechanarFlames::INFERNO_RADIUS)
                    score -= MechanarFlames::HEALER_KITE_FLAME_PATH_WEIGHT *
                             (MechanarFlames::INFERNO_RADIUS - passDist);
            }
            if (score > bestScore)
            {
                bestScore = score;
                bestX = destX;
                bestY = destY;
                found = true;
            }
        }
    }

    if (!found)
    {
        if (!haveFallback)
            return false;
        bestX = safeX;
        bestY = safeY;
    }

    return MoveTo(bot->GetMapId(), bestX, bestY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool SepethreaAvoidFlameAction::Execute(Event)
{
    std::vector<std::pair<float, float>> avoidFlames;
    MechanarFlames::CollectAvoidFlames(bot, avoidFlames);
    std::vector<MechanarFlames::TrailPatch> patches;
    MechanarFlames::CollectTrailPatches(bot, MechanarFlames::TRAIL_SCAN + MechanarFlames::INFERNO_AVOID_CLEAR, patches);

    float const clearDist = MechanarFlames::MELEE_FLAME_CLEAR;
    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    if (SpotSafe(avoidFlames, patches, bx, by, clearDist))
        return false;

    constexpr int DIRS = 16;
    float bestX = bx, bestY = by, bestClear = HazardClearance(avoidFlames, patches, bx, by, clearDist);
    bool found = false;
    for (float radius = 4.0f; radius <= MechanarFlames::INFERNO_AVOID_CLEAR + 8.0f && !found; radius += 3.0f)
    {
        for (int i = 0; i < DIRS; ++i)
        {
            float const ang = (i * 2.0f * static_cast<float>(M_PI)) / DIRS;
            float const x = bx + std::cos(ang) * radius;
            float const y = by + std::sin(ang) * radius;
            if (!MechanarFlames::InRoom(x, y))
                continue;
            if (SpotSafe(avoidFlames, patches, x, y, clearDist))
            {
                bestX = x;
                bestY = y;
                found = true;
                break;
            }
            float const clear = HazardClearance(avoidFlames, patches, x, y, clearDist);
            if (clear > bestClear)
            {
                bestClear = clear;
                bestX = x;
                bestY = y;
            }
        }
    }

    if (bestX == bx && bestY == by)
        return false;

    MovementPriority const priority =
        PlayerbotAI::IsHeal(bot) ? MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_FORCED;
    return MoveTo(bot->GetMapId(), bestX, bestY, bot->GetPositionZ(), false, false, false, false,
                  priority, true, false);
}

bool SepethreaAvoidTrailAction::Execute(Event)
{
    std::vector<MechanarFlames::TrailPatch> patches;
    MechanarFlames::CollectTrailPatches(bot, MechanarFlames::TRAIL_SCAN, patches);
    if (patches.empty())
        return false;

    float ax = 0.0f, ay = 0.0f;
    bool anyDanger = false;
    for (MechanarFlames::TrailPatch const& p : patches)
    {
        float const dx = bot->GetPositionX() - p.x;
        float const dy = bot->GetPositionY() - p.y;
        float const d = std::sqrt(dx * dx + dy * dy);
        float const danger = p.radius + MechanarFlames::TRAIL_DANGER_MARGIN;
        if (d >= danger)
            continue;
        anyDanger = true;
        if (d < 0.1f)
            continue;
        float const w = (danger - d) / danger;
        ax += dx / d * w;
        ay += dy / d * w;
    }

    if (!anyDanger)
        return false;

    if (!Normalize(ax, ay))
    {
        if (Unit* flame = MechanarFlames::GetNearestFlame(bot, 100.0f))
        {
            ax = bot->GetPositionX() - flame->GetPositionX();
            ay = bot->GetPositionY() - flame->GetPositionY();
        }
        if (!Normalize(ax, ay))
        {
            ax = bot->GetPositionX() - MechanarFlames::ROOM_ANCHOR_X;
            ay = bot->GetPositionY() - MechanarFlames::ROOM_ANCHOR_Y;
            if (!Normalize(ax, ay))
            {
                ax = 1.0f;
                ay = 0.0f;
            }
        }
    }

    float destX = bot->GetPositionX() + ax * MechanarFlames::TRAIL_CLEAR_STEP;
    float destY = bot->GetPositionY() + ay * MechanarFlames::TRAIL_CLEAR_STEP;
    if (!MechanarFlames::SegmentInRoom(bot->GetPositionX(), bot->GetPositionY(), destX, destY))
        MechanarFlames::ClampIntoRoom(destX, destY);

    MovementPriority const priority =
        PlayerbotAI::IsHeal(bot) ? MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_FORCED;

    return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(), false, false, false, false,
                  priority, true, false);
}

bool SepethreaFocusBossAction::Execute(Event)
{
    Unit* boss = MechanarFlames::GetSepethrea(bot);
    if (!boss)
        return false;

    if (AI_VALUE(Unit*, "current target") != boss)
        return Attack(boss);

    return false;
}
