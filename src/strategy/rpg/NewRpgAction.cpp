#include "NewRpgAction.h"

#include <cmath>
#include <cstdint>

#include "NewRpgStrategy.h"
#include "ObjectDefines.h"
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
    Player* owner = event.getOwner();
    if (!owner)
        return false;
    std::string out = botAI->rpgInfo.ToString();
    bot->Whisper(out.c_str(), LANG_UNIVERSAL, owner);
    return true;
}

bool NewRpgStatusUpdateAction::Execute(Event event)
{
    NewRpgInfo& info = botAI->rpgInfo;
    switch (info.status)
    {
        case NewRpgStatus::IDLE:
        {
            uint32 roll = urand(1, 100);
            // IDLE -> NEAR_NPC
            // if ((!info.lastNearNpc || info.lastNearNpc + setNpcInterval < getMSTime()) && roll <= 30)
            if (roll <= 30)
            {
                GuidVector possibleTargets = AI_VALUE(GuidVector, "possible rpg targets");
                if (!possibleTargets.empty())
                {
                    info.Reset();
                    info.lastNearNpc = getMSTime();
                    info.status = NewRpgStatus::NEAR_NPC;
                    return true;
                }
            }
            // IDLE -> GO_INNKEEPER
            else if (roll <= 45)
            {
                WorldPosition pos = SelectRandomInnKeeperPos();
                if (pos != WorldPosition() && bot->GetExactDist(pos) > 50.0f)
                {
                    info.Reset();
                    info.lastGoInnKeeper = getMSTime();
                    info.status = NewRpgStatus::GO_INNKEEPER;
                    info.innKeeperPos = pos;
                    return true;
                }
            }
            // IDLE -> GO_GRIND
            else if (roll <= 90)
            {
                WorldPosition pos = SelectRandomGrindPos();
                if (pos != WorldPosition())
                {
                    info.Reset();
                    info.lastGoGrind = getMSTime();
                    info.status = NewRpgStatus::GO_GRIND;
                    info.grindPos = pos;
                    return true;
                }
            }
            // IDLE -> REST
            info.Reset();
            info.status = NewRpgStatus::REST;
            info.lastRest = getMSTime();
            bot->SetStandState(UNIT_STAND_STATE_SIT);
            return true;
        }
        case NewRpgStatus::GO_GRIND:
        {
            WorldPosition& originalPos = info.grindPos;
            assert(info.grindPos != WorldPosition());
            // GO_GRIND -> NEAR_RANDOM
            if (bot->GetExactDist(originalPos) < 10.0f)
            {
                info.Reset();
                info.status = NewRpgStatus::NEAR_RANDOM;
                info.lastNearRandom = getMSTime();
                info.grindPos = WorldPosition();
                return true;
            }
            break;
        }
        case NewRpgStatus::GO_INNKEEPER:
        {
            WorldPosition& originalPos = info.innKeeperPos;
            assert(info.innKeeperPos != WorldPosition());
            // GO_INNKEEPER -> NEAR_NPC
            if (bot->GetExactDist(originalPos) < 10.0f)
            {
                info.Reset();
                info.lastNearNpc = getMSTime();
                info.status = NewRpgStatus::NEAR_NPC;
                info.innKeeperPos = WorldPosition();
                return true;
            }
            break;
        }
        case NewRpgStatus::NEAR_RANDOM:
        {
            // NEAR_RANDOM -> IDLE
            if (info.lastNearRandom + statusNearRandomDuration < getMSTime())
            {
                info.Reset();
                info.status = NewRpgStatus::IDLE;
                return true;
            }
            break;
        }
        case NewRpgStatus::NEAR_NPC:
        {
            if (info.lastNearNpc + statusNearNpcDuration < getMSTime())
            {
                info.Reset();
                info.status = NewRpgStatus::IDLE;
                return true;
            }
            break;
        }
        case NewRpgStatus::REST:
        {
            // REST -> IDLE
            if (info.lastRest + statusRestDuration < getMSTime())
            {
                info.Reset();
                info.status = NewRpgStatus::IDLE;
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

        if (bot->GetMap()->GetZoneId(bot->GetPhaseMask(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ()) !=
            bot->GetZoneId())
            continue;

        if (bot->GetExactDist(loc) < 500.0f)
        {
            hi_prepared_locs.push_back(loc);
        }

        if (bot->GetExactDist(loc) < 2500.0f)
        {
            lo_prepared_locs.push_back(loc);
        }
    }
    WorldPosition dest{};
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
    LOG_DEBUG("playerbots", "[New Rpg] Bot {} select random grind pos Map:{} X:{} Y:{} Z:{} ({}+{} available in {})",
              bot->GetName(), dest.GetMapId(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(),
              hi_prepared_locs.size(), lo_prepared_locs.size() - hi_prepared_locs.size(), locs.size());
    return dest;
}

WorldPosition NewRpgStatusUpdateAction::SelectRandomInnKeeperPos()
{
    const std::vector<WorldLocation>& locs = IsAlliance(bot->getRace())
                                                 ? sRandomPlayerbotMgr->allianceStarterPerLevelCache[bot->GetLevel()]
                                                 : sRandomPlayerbotMgr->hordeStarterPerLevelCache[bot->GetLevel()];
    std::vector<WorldLocation> prepared_locs;
    for (auto& loc : locs)
    {
        if (bot->GetMapId() != loc.GetMapId())
            continue;
        
        if (bot->GetMap()->GetZoneId(bot->GetPhaseMask(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ()) !=
            bot->GetZoneId())
            continue;
            
        float range = bot->GetLevel() <= 5 ? 500.0f : 2500.0f;
        if (bot->GetExactDist(loc) < range)
        {
            prepared_locs.push_back(loc);
        }
    }
    WorldPosition dest{};
    if (!prepared_locs.empty())
    {
        uint32 idx = urand(0, prepared_locs.size() - 1);
        dest = prepared_locs[idx];
    }
    LOG_DEBUG("playerbots", "[New Rpg] Bot {} select random inn keeper pos Map:{} X:{} Y:{} Z:{} ({} available in {})",
              bot->GetName(), dest.GetMapId(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(),
              prepared_locs.size(), locs.size());
    return dest;
}

bool NewRpgGoFarAwayPosAction::MoveFarTo(WorldPosition dest)
{
    if (dest == WorldPosition())
        return false;

    float dis = bot->GetExactDist(dest);
    if (dis < pathFinderDis)
    {
        return MoveTo(dest.getMapId(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), false, false,
                      false, true);
    }

    // performance optimization
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL))
    {
        return false;
    }

    // stuck check
    float disToDest = bot->GetDistance(dest);
    if (disToDest + 1.0f < botAI->rpgInfo.nearestMoveFarDis)
    {
        botAI->rpgInfo.nearestMoveFarDis = disToDest;
        botAI->rpgInfo.stuckTs = getMSTime();
        botAI->rpgInfo.stuckAttempts = 0;
    }
    else if (++botAI->rpgInfo.stuckAttempts >= 10 && botAI->rpgInfo.stuckTs + stuckTime < getMSTime())
    {
        // Unfortunately we've been stuck here for over 5 mins, fallback to teleporting directly to the destination
        botAI->rpgInfo.stuckTs = getMSTime();
        botAI->rpgInfo.stuckAttempts = 0;
        const AreaTableEntry* entry = sAreaTableStore.LookupEntry(bot->GetZoneId());
        std::string zone_name = PlayerbotAI::GetLocalizedAreaName(entry);
        LOG_DEBUG("playerbots", "[New Rpg] Teleport {} from ({},{},{},{}) to ({},{},{},{}) as it stuck when moving far - Zone: {} ({})", bot->GetName(),
            bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId(),
            dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), dest.getMapId(), bot->GetZoneId(), zone_name);
        return bot->TeleportTo(dest);
    }

    float minDelta = M_PI;
    const float x = bot->GetPositionX();
    const float y = bot->GetPositionY();
    const float z = bot->GetPositionZ();
    float rx, ry, rz;
    bool found = false;
    int attempt = 3;
    while (--attempt)
    {
        float angle = bot->GetAngle(&dest);
        float delta = urand(1, 100) <= 75 ? (rand_norm() - 0.5) * M_PI * 0.5 : (rand_norm() - 0.5) * M_PI * 2;
        angle += delta;
        float dis = rand_norm() * pathFinderDis;
        float dx = x + cos(angle) * dis;
        float dy = y + sin(angle) * dis;
        float dz = z + 0.5f;
        PathGenerator path(bot);
        path.CalculatePath(dx, dy, dz);
        PathType type = path.GetPathType();
        uint32 typeOk = PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_FARFROMPOLY;
        bool canReach = !(type & (~typeOk));

        if (canReach && fabs(delta) <= minDelta)
        {
            found = true;
            const G3D::Vector3& endPos = path.GetActualEndPosition();
            rx = endPos.x;
            ry = endPos.y;
            rz = endPos.z;
            minDelta = fabs(delta);
        }
    }
    if (found)
    {
        return MoveTo(bot->GetMapId(), rx, ry, rz, false, false, false, true);
    }
    return false;
}

bool NewRpgGoGrindAction::Execute(Event event) { return MoveFarTo(botAI->rpgInfo.grindPos); }

bool NewRpgGoInnKeeperAction::Execute(Event event) { return MoveFarTo(botAI->rpgInfo.innKeeperPos); }

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

bool NewRpgMoveNpcAction::Execute(Event event)
{
    NewRpgInfo& info = botAI->rpgInfo;
    if (!info.npcPos)
    {
        GuidVector possibleTargets = AI_VALUE(GuidVector, "possible rpg targets");
        if (possibleTargets.empty())
            return false;
        int idx = urand(0, possibleTargets.size() - 1);
        ObjectGuid guid = possibleTargets[idx];
        Unit* unit = botAI->GetUnit(guid);
        if (unit)
        {
            info.npcPos = GuidPosition(unit);
            info.lastReachNpc = 0;
        }
        else
            return false;
    }

    if (bot->GetDistance(info.npcPos) <= INTERACTION_DISTANCE)
    {
        if (!info.lastReachNpc)
        {
            info.lastReachNpc = getMSTime();
            return true;
        }

        if (info.lastReachNpc && info.lastReachNpc + stayTime > getMSTime())
            return false;

        info.npcPos = GuidPosition();
        info.lastReachNpc = 0;
    }
    else
    {
        assert(info.npcPos);
        Unit* unit = botAI->GetUnit(info.npcPos);
        if (!unit)
            return false;
        float x = unit->GetPositionX();
        float y = unit->GetPositionY();
        float z = unit->GetPositionZ();
        float mapId = unit->GetMapId();
        float angle = 0.f;
        if (bot->IsWithinLOS(x, y, z))
        {
            if (!unit->isMoving())
                angle = unit->GetAngle(bot) + (M_PI * irand(-25, 25) / 100.0);  // Closest 45 degrees towards the target
            else
                angle = unit->GetOrientation() +
                        (M_PI * irand(-25, 25) / 100.0);  // 45 degrees infront of target (leading it's movement)
        }
        else
            angle = 2 * M_PI * rand_norm();  // A circle around the target.
        float rnd = rand_norm();
        x += cos(angle) * INTERACTION_DISTANCE * rnd;
        y += sin(angle) * INTERACTION_DISTANCE * rnd;
        // bool exact = true;
        if (!unit->GetMap()->CheckCollisionAndGetValidCoords(unit, unit->GetPositionX(), unit->GetPositionY(),
                                                             unit->GetPositionZ(), x, y, z))
        {
            x = unit->GetPositionX();
            y = unit->GetPositionY();
            z = unit->GetPositionZ();
            // exact = false;
        }
        return MoveTo(mapId, x, y, z, false, false, false, true);
    }
    return true;
}