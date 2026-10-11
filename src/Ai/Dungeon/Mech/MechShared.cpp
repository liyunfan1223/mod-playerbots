/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MechShared.h"
#include "Cell.h"
#include "CellImpl.h"
#include "Creature.h"
#include "DynamicObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "MechTriggers.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include <algorithm>
#include <cmath>
#include <list>
#include <utility>
#include <vector>

namespace
{
    struct V2
    {
        float x;
        float y;
    };
    constexpr V2 ROOM_POLY[] = {
        { MechanarFlames::ROOM_X_MIN, MechanarFlames::ROOM_Y_MIN },
        { MechanarFlames::ROOM_X_MAX, MechanarFlames::ROOM_Y_MIN },
        { MechanarFlames::ROOM_X_MAX, MechanarFlames::ROOM_Y_MAX },
        { MechanarFlames::ROOM_X_MIN, MechanarFlames::ROOM_Y_MAX } };
    constexpr int ROOM_POLY_N = 4;

    void ClosestOnSegment(float px, float py, float ax, float ay, float bx, float by, float& cx, float& cy)
    {
        float const dx = bx - ax, dy = by - ay;
        float const len2 = dx * dx + dy * dy;
        float t = (len2 < 1e-6f) ? 0.0f : ((px - ax) * dx + (py - ay) * dy) / len2;
        t = std::max(0.0f, std::min(1.0f, t));
        cx = ax + t * dx;
        cy = ay + t * dy;
    }
}

namespace MechanarFlames
{
    bool InRoom(float x, float y)
    {
        bool inside = false;
        int j = ROOM_POLY_N - 1;
        for (int i = 0; i < ROOM_POLY_N; ++i)
        {
            float const xi = ROOM_POLY[i].x, yi = ROOM_POLY[i].y;
            float const xj = ROOM_POLY[j].x, yj = ROOM_POLY[j].y;
            if (((yi > y) != (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi + 1e-6f) + xi))
                inside = !inside;
            j = i;
        }
        return inside;
    }

    bool SegmentInRoom(float ax, float ay, float bx, float by)
    {
        return InRoom(ax, ay) && InRoom(bx, by) && InRoom((ax + bx) * 0.5f, (ay + by) * 0.5f);
    }

    void ClampIntoRoom(float& x, float& y)
    {
        if (InRoom(x, y))
            return;

        float best = 1e18f, resX = x, resY = y;
        int j = ROOM_POLY_N - 1;
        for (int i = 0; i < ROOM_POLY_N; ++i)
        {
            float cx, cy;
            ClosestOnSegment(x, y, ROOM_POLY[j].x, ROOM_POLY[j].y, ROOM_POLY[i].x, ROOM_POLY[i].y, cx, cy);
            float const d = (cx - x) * (cx - x) + (cy - y) * (cy - y);
            if (d < best)
            {
                best = d;
                resX = cx;
                resY = cy;
            }
            j = i;
        }

        float tx = ROOM_ANCHOR_X - resX, ty = ROOM_ANCHOR_Y - resY;
        float const len = std::sqrt(tx * tx + ty * ty);
        if (len > 1e-3f)
        {
            resX += tx / len * 0.5f;
            resY += ty / len * 0.5f;
        }
        x = resX;
        y = resY;
    }

    bool HealerHoldsFire(Player* bot)
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!PlayerbotAI::IsHeal(bot))
            return false;
        if (bot->GetHealthPct() <= HEALER_FIRE_BAIL_PCT)
            return false;
        return botAI->GetAiObjectContext()->GetValue<Unit*>("party member to heal")->Get() != nullptr;
    }

    bool TankMustGrabBoss(Player* bot)
    {
        if (!PlayerbotAI::IsTank(bot))
            return false;

        Unit* boss = GetSepethrea(bot);
        if (!boss)
            return false;

        Unit* victim = boss->GetVictim();
        if (!victim)
            return true;
        if (victim == bot)
            return true;
        Player* victimPlayer = victim->ToPlayer();
        return !victimPlayer || !PlayerbotAI::IsTank(victimPlayer);
    }

    bool IsFlame(Unit* unit)
    {
        if (!unit)
            return false;
        uint32 const entry = unit->GetEntry();
        return entry == static_cast<uint32>(MechanarIDs::NPC_RAGING_FLAMES) ||
               entry == static_cast<uint32>(MechanarIDs::NPC_RAGING_FLAMES_HEROIC);
    }

    static void CollectFlames(Player* bot, float radius, std::list<Creature*>& out)
    {
        bot->GetCreatureListWithEntryInGrid(
            out, static_cast<uint32>(MechanarIDs::NPC_RAGING_FLAMES), radius);
        bot->GetCreatureListWithEntryInGrid(
            out, static_cast<uint32>(MechanarIDs::NPC_RAGING_FLAMES_HEROIC), radius);
    }

    void CollectAvoidFlames(Player* bot, std::vector<std::pair<float, float>>& out)
    {
        std::list<Creature*> flames;
        CollectFlames(bot, 100.0f, flames);
        for (Creature* flame : flames)
        {
            if (!flame || !flame->IsAlive())
                continue;
            if (flame->GetVictim() == bot)
                continue;
            out.emplace_back(flame->GetPositionX(), flame->GetPositionY());
        }
    }

    void CollectChasingFlames(Player* bot, std::vector<Creature*>& out)
    {
        std::list<Creature*> flames;
        CollectFlames(bot, 100.0f, flames);
        for (Creature* flame : flames)
            if (flame && flame->IsAlive() && flame->GetVictim() == bot)
                out.push_back(flame);
    }

    Creature* GetNearestFlame(Player* bot, float radius)
    {
        std::list<Creature*> flames;
        CollectFlames(bot, radius, flames);

        Creature* best = nullptr;
        float bestDist = radius;
        for (Creature* flame : flames)
        {
            if (!flame || !flame->IsAlive())
                continue;
            float const d = bot->GetDistance2d(flame);
            if (d <= bestDist)
            {
                bestDist = d;
                best = flame;
            }
        }
        return best;
    }

    ObjectGuid FindFixatingFlameGuid(Player* bot)
    {
        std::list<Creature*> flames;
        CollectFlames(bot, 100.0f, flames);
        for (Creature* flame : flames)
            if (flame && flame->IsAlive() && flame->GetVictim() == bot)
                return flame->GetGUID();
        return ObjectGuid::Empty;
    }

    Unit* GetFixatingFlame(Player* bot)
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        ObjectGuid guid =
            botAI->GetAiObjectContext()->GetValue<ObjectGuid>("sepethrea fixating flame")->Get();
        if (guid.IsEmpty())
            return nullptr;

        Unit* flame = ObjectAccessor::GetUnit(*bot, guid);
        if (!flame || !flame->IsAlive() || flame->GetVictim() != bot)
            return nullptr;
        return flame;
    }

    void CollectTrailPatches(Player* bot, float scanRadius, std::vector<TrailPatch>& out)
    {
        std::vector<WorldObject*> objs;
        Acore::AllWorldObjectsInRange check(bot, scanRadius);
        Acore::WorldObjectListSearcher<Acore::AllWorldObjectsInRange> searcher(
            bot, objs, check, GRID_MAP_TYPE_MASK_DYNAMICOBJECT);
        Cell::VisitObjects(bot, searcher, scanRadius);

        for (WorldObject* o : objs)
        {
            DynamicObject* dyn = o->ToDynObject();
            if (!dyn || dyn->GetSpellId() != TRAIL_PATCH_SPELL)
                continue;
            float r = dyn->GetRadius();
            if (r <= 0.0f)
                r = TRAIL_PATCH_RADIUS;
            out.push_back({ dyn->GetPositionX(), dyn->GetPositionY(), r });
        }
    }

    bool InTrailDanger(Player* bot)
    {
        std::vector<TrailPatch> patches;
        CollectTrailPatches(bot, TRAIL_SCAN, patches);
        for (TrailPatch const& p : patches)
        {
            float const dx = bot->GetPositionX() - p.x;
            float const dy = bot->GetPositionY() - p.y;
            if (std::sqrt(dx * dx + dy * dy) < p.radius + TRAIL_DANGER_MARGIN)
                return true;
        }
        return false;
    }

    bool InTrailDangerCached(Player* bot)
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        return botAI->GetAiObjectContext()->GetValue<bool>("sepethrea trail danger")->Get();
    }

    bool IsFlameNearCached(Player* bot)
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        return botAI->GetAiObjectContext()->GetValue<bool>("sepethrea flame near")->Get();
    }

    Unit* GetSepethrea(Player* bot)
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        return botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "nethermancer sepethrea")->Get();
    }
}
