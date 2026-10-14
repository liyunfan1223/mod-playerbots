/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MECHSHARED_H
#define PLAYERBOTS_MECHSHARED_H

#include "Define.h"
#include "ObjectGuid.h"
#include <utility>
#include <vector>

class Player;
class Unit;
class Creature;

namespace MechanarFlames
{
    constexpr float INFERNO_RADIUS = 10.0f;
    // Fixated kiter keeps a wide running berth (INFERNO_SAFE_DIST). A non-fixated bot only
    // needs to clear the flame's actual fire footprint: its self-cast area aura (spell 35281,
    // radius 0 = self) drops trail patches (spell 35278, ~5yd) under it, so it is fine to fight
    // the boss from the front and only sidestep the patches (handled by the trail avoidance) plus
    // a small buffer for the instant a fresh patch lands.
    constexpr float INFERNO_AVOID_RANGE = 10.0f;
    constexpr float INFERNO_AVOID_CLEAR = 19.0f;
    constexpr float INFERNO_SAFE_DIST = 14.0f;
    constexpr float MELEE_FLAME_CLEAR = 12.0f;
    constexpr float KITE_THRESHOLD = 16.0f;
    constexpr float KITE_STEP = 14.0f;

    constexpr float KITE_TURN_RUNWAY = 16.0f;
    constexpr float KITE_TURN_THRESHOLD = 24.0f;
    constexpr float KITE_TURN_PASS_BEHIND = 20.0f;
    constexpr float KITE_TURN_WALL_INSET = 4.0f;
    constexpr float KITE_TURN_EXIT_PAST = 3.0f;
    constexpr float KITE_TURN_CLEAR_WEIGHT = 8.0f;
    constexpr float KITE_TURN_ABORT_CLEAR = -2.0f;
    constexpr uint32 KITE_TURN_FAILSAFE_MS = 12000;
    constexpr float KITE_GROUP_CLEARANCE = 16.0f;
    constexpr float HEALER_FIRE_BAIL_PCT = 50.0f;

    constexpr float HEALER_KITE_LEASH = 34.0f;
    constexpr float HEALER_KITE_LEASH_MAX = 38.0f;
    constexpr float HEALER_KITE_LEASH_WEIGHT = 3.0f;
    constexpr float HEALER_KITE_FLAME_PATH_WEIGHT = 2.5f;

    constexpr uint32 TRAIL_PATCH_SPELL = 35278;
    constexpr float TRAIL_PATCH_RADIUS = 5.0f;
    constexpr float TRAIL_DANGER_MARGIN = 2.5f;
    constexpr float TRAIL_SCAN = 18.0f;
    constexpr float TRAIL_CLEAR_STEP = 10.0f;

    struct TrailPatch
    {
        float x;
        float y;
        float radius;
    };

    void CollectTrailPatches(Player* bot, float scanRadius, std::vector<TrailPatch>& out);

    bool InTrailDanger(Player* bot);

    bool InTrailDangerCached(Player* bot);

    bool IsFlameNearCached(Player* bot);

    constexpr float ROOM_ANCHOR_X = 300.0f;
    constexpr float ROOM_ANCHOR_Y = 6.0f;
    constexpr float ROOM_X_MIN = 285.0f;
    constexpr float ROOM_X_MAX = 308.0f;
    constexpr float ROOM_Y_MIN = -26.0f;
    constexpr float ROOM_Y_MAX = 54.0f;
    constexpr float KITE_LONG_AXIS_WEIGHT = 7.0f;

    bool InRoom(float x, float y);

    bool SegmentInRoom(float ax, float ay, float bx, float by);

    void ClampIntoRoom(float& x, float& y);

    bool HealerHoldsFire(Player* bot);

    bool TankMustGrabBoss(Player* bot);

    bool IsFlame(Unit* unit);

    Creature* GetNearestFlame(Player* bot, float radius);

    void CollectAvoidFlames(Player* bot, std::vector<std::pair<float, float>>& out);

    void CollectChasingFlames(Player* bot, std::vector<Creature*>& out);

    ObjectGuid FindFixatingFlameGuid(Player* bot);

    Unit* GetFixatingFlame(Player* bot);

    Unit* GetSepethrea(Player* bot);
}

#endif
