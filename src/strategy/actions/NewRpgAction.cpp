#include "NewRpgAction.h"

#include <cmath>

#include "NewRpgStrategy.h"
#include "ObjectGuid.h"
#include "PathGenerator.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Random.h"
#include "RandomPlayerbotMgr.h"
#include "Timer.h"
#include "TravelMgr.h"
#include "World.h"

bool TellRpgStatusAction::Execute(Event event)
{
    std::string out = botAI->rpgInfo.ToString();
    botAI->TellMasterNoFacing(out);
    return true;
}

bool NewRpgStatusUpdateAction::Execute(Event event)
{
    NewRpgInfo& info = botAI->rpgInfo;
    switch (info.status)
    {
        case NewRpgStatus::IDLE:
        {
            // // IDLE -> NEAR_NPC
            // if (!info.lastNearNpc || info.lastNearNpc + setNpcInterval < getMSTime() && urand(1, 100) <= 50)
            // {
            //     info.lastNearNpc = getMSTime();
            //     GuidVector possibleTargets = AI_VALUE(GuidVector, "possible rpg targets");
            //     if (possibleTargets.empty())
            //         break;
            //     info.status = NewRpgStatus::NEAR_NPC;
            // }
            // IDLE -> GO_GRIND
            if (!info.lastGrind || info.lastGrind + setGrindInterval < getMSTime())
            {
                info.lastGrind = getMSTime();
                WorldPosition pos = SelectRandomGrindPos();
                if (pos == WorldPosition())
                    break;
                info.status = NewRpgStatus::GO_GRIND;
                info.grindPos = pos;
                return true;
            }
            break;
        }
        case NewRpgStatus::GO_GRIND:
        {
            WorldPosition& originalPos = info.grindPos;
            assert(info.grindPos != WorldPosition());
            // GO_GRIND -> NEAR_RANDOM
            if (bot->GetExactDist(originalPos) < 10.0f)
            {
                info.status = NewRpgStatus::NEAR_RANDOM;
                info.grindPos = WorldPosition();
                return true;
            }
            // just choose another grindPos
            if (!info.lastGrind || info.lastGrind + setGrindInterval < getMSTime())
            {
                WorldPosition pos = SelectRandomGrindPos();
                if (pos == WorldPosition())
                    break;
                info.status = NewRpgStatus::GO_GRIND;
                info.lastGrind = getMSTime();
                info.grindPos = pos;
                return true;
            }
            break;
        }
        case NewRpgStatus::NEAR_RANDOM:
        {
            // NEAR_RANDOM -> GO_GRIND
            if (!info.lastGrind || info.lastGrind + setGrindInterval < getMSTime())
            {
                WorldPosition pos = SelectRandomGrindPos();
                if (pos == WorldPosition())
                    break;
                info.lastGrind = getMSTime();
                botAI->rpgInfo.status = NewRpgStatus::GO_GRIND;
                botAI->rpgInfo.grindPos = pos;
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

WorldPosition NewRpgStatusUpdateAction::SelectRandomGrindPos()
{
    const std::vector<WorldLocation>& locs = sRandomPlayerbotMgr->locsPerLevelCache[bot->GetLevel()];
    std::vector<WorldLocation> lo_prepared_locs, hi_prepared_locs;
    for (auto& loc : locs)
    {
        if (bot->GetMapId() != loc.GetMapId())
            continue;

        if (bot->GetExactDist(loc) < 500.0f)
        {
            hi_prepared_locs.push_back(loc);
        }

        if (bot->GetExactDist(loc) < 1500.0f)
        {
            lo_prepared_locs.push_back(loc);
        }
    }
    WorldPosition dest;
    if (urand(1, 100) <= 50 && !hi_prepared_locs.empty())
    {
        uint32 idx = urand(0, hi_prepared_locs.size() - 1);
        dest = hi_prepared_locs[idx];
    }
    else if (!lo_prepared_locs.empty())
    {
        uint32 idx = urand(0, lo_prepared_locs.size() - 1);
        dest = lo_prepared_locs[idx];
    }
    LOG_INFO("playerbots", "[New Rpg] Bot {} select random grind pos Map:{} X:{} Y:{} Z:{} ({}+{} available in {})",
             bot->GetName(), dest.GetMapId(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(),
             hi_prepared_locs.size(), lo_prepared_locs.size() - hi_prepared_locs.size(), locs.size());
    return dest;
}

bool NewRpgGoFarAwayPosAction::MoveFarTo(WorldPosition dest)
{
    float dis = bot->GetExactDist(dest);
    if (dis < pathFinderDis)
    {
        return MoveTo(dest.getMapId(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ());
    }

    int attempt = 10;
    float minDelta = MAXFLOAT;
    const float x = bot->GetPositionX();
    const float y = bot->GetPositionY();
    const float z = bot->GetPositionZ();
    float rx, ry, rz;
    bool found = false;
    while (--attempt)
    {
        float angle = bot->GetAngle(&dest);
        float delta = (rand_norm() - 0.5) * M_PI;
        angle += delta;
        float dis = rand_norm() * pathFinderDis;
        float dx = x + cos(angle) * dis;
        float dy = y + sin(angle) * dis;
        float dz = z;
        PathGenerator path(bot);
        path.CalculatePath(dx, dy, dz);
        // bool canReach = bot->GetMap()->CanReachPositionAndGetValidCoords(bot, x, y, z, dx, dy, dz, false);
        bool canReach = path.GetPathType() & (PATHFIND_NORMAL | PATHFIND_INCOMPLETE);

        if (canReach)
        {
            G3D::Vector3 endPos = path.GetPath().back();
            if (fabs(delta) < minDelta)
            {
                found = true;
                minDelta = fabs(delta);
                rx = endPos.x;
                ry = endPos.y;
                rz = endPos.z;
            }
        }
    }
    if (found)
    {
        return MoveTo(bot->GetMapId(), rx, ry, rz, false, false, false, true);
    }
    // fallback to direct move
    float angle = bot->GetAngle(&dest);
    return MoveTo(bot->GetMapId(), x + cos(angle) * pathFinderDis, y + sin(angle) * pathFinderDis, z);
}

bool NewRpgGoGrindAction::Execute(Event event) { return MoveFarTo(botAI->rpgInfo.grindPos); }

bool NewRpgMoveRandomAction::Execute(Event event)
{
    float distance = rand_norm() * moveStep;
    Map* map = bot->GetMap();
    const float x = bot->GetPositionX();
    const float y = bot->GetPositionY();
    const float z = bot->GetPositionZ();
    int attempts = 5;
    while (--attempts)
    {
        float angle = (float)rand_norm() * 2 * static_cast<float>(M_PI);
        float dx = x + distance * cos(angle);
        float dy = y + distance * sin(angle);
        float dz = z;
        if (!map->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                                                  dx, dy, dz))
            continue;

        if (map->IsInWater(bot->GetPhaseMask(), dx, dy, dz, bot->GetCollisionHeight()))
            continue;

        bool moved = MoveTo(bot->GetMapId(), dx, dy, dz, false, false, false, true);
        if (moved)
            return true;
    }

    return false;
}