/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TravelMgr.h"

#include <iomanip>
#include <numeric>

#include "CellImpl.h"
#include "ChatHelper.h"
#include "MMapFactory.h"
#include "MapMgr.h"
#include "PathGenerator.h"
#include "Playerbots.h"
#include "StrategyContext.h"
#include "TransportMgr.h"
#include "VMapFactory.h"
#include "VMapMgr2.h"

WorldPosition::WorldPosition(std::string const str)
{
    std::stringstream out(str);
    out >> this->m_mapId;
    out >> this->m_positionX;
    out >> this->m_positionY;
    out >> this->m_positionZ;
    out >> this->m_orientation;
}

WorldPosition::WorldPosition(uint32 mapId, const Position& pos)
    : WorldLocation(mapId, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation())
{
}

WorldPosition::WorldPosition(WorldObject const* wo)
{
    if (wo)
    {
        set(WorldLocation(wo->GetMapId(), wo->GetPositionX(), wo->GetPositionY(), wo->GetPositionZ(),
                          wo->GetOrientation()));
    }
}

WorldPosition::WorldPosition(std::vector<WorldPosition*> list, WorldPositionConst conType)
{
    uint32 size = list.size();
    if (!size)
        return;

    if (size == 1)
        set(*list.front());
    else if (conType == WP_RANDOM)
        set(*list[urand(0, size - 1)]);
    else if (conType == WP_CENTROID)
    {
        set(std::accumulate(list.begin(), list.end(), WorldLocation(list[0]->getMapId(), 0, 0, 0, 0),
                            [size](WorldLocation i, WorldPosition* j)
                            {
                                i.m_positionX += j->getX() / size;
                                i.m_positionY += j->getY() / size;
                                i.m_positionZ += j->getZ() / size;
                                i.NormalizeOrientation(i.m_orientation += j->getO() / size);
                                return i;
                            }));
    }
    else if (conType == WP_MEAN_CENTROID)
    {
        WorldPosition pos = WorldPosition(list, WP_CENTROID);
        set(*pos.closestSq(list));
    }
}

WorldPosition::WorldPosition(std::vector<WorldPosition> list, WorldPositionConst conType)
{
    uint32 size = list.size();
    if (!size)
        return;

    if (size == 1)
        set(list.front());
    else if (conType == WP_RANDOM)
        set(list[urand(0, size - 1)]);
    else if (conType == WP_CENTROID)
    {
        set(std::accumulate(list.begin(), list.end(), WorldLocation(list[0].getMapId(), 0, 0, 0, 0),
                            [size](WorldLocation i, WorldPosition& j)
                            {
                                i.m_positionX += j.getX() / size;
                                i.m_positionY += j.getY() / size;
                                i.m_positionZ += j.getZ() / size;
                                i.NormalizeOrientation(i.m_orientation += j.getO() / size);
                                return i;
                            }));
    }
    else if (conType == WP_MEAN_CENTROID)
    {
        WorldPosition pos = WorldPosition(list, WP_CENTROID);
        set(pos.closestSq(list));
    }
}

WorldPosition::WorldPosition(uint32 mapid, GridCoord grid)
    : WorldLocation(mapid, (int32(grid.x_coord) - CENTER_GRID_ID - 0.5) * SIZE_OF_GRIDS + CENTER_GRID_OFFSET,
                    (int32(grid.y_coord) - CENTER_GRID_ID - 0.5) * SIZE_OF_GRIDS + CENTER_GRID_OFFSET, 0, 0)
{
}

WorldPosition::WorldPosition(uint32 mapid, CellCoord cell)
    : WorldLocation(
          mapid, (int32(cell.x_coord) - CENTER_GRID_CELL_ID - 0.5) * SIZE_OF_GRID_CELL + CENTER_GRID_CELL_OFFSET,
          (int32(cell.y_coord) - CENTER_GRID_CELL_ID - 0.5) * SIZE_OF_GRID_CELL + CENTER_GRID_CELL_OFFSET, 0, 0)
{
}

WorldPosition::WorldPosition(uint32 mapid, mGridCoord grid)
    : WorldLocation(mapid, (32 - grid.first) * SIZE_OF_GRIDS, (32 - grid.second) * SIZE_OF_GRIDS, 0, 0)
{
}

void WorldPosition::set(const WorldLocation& pos) { WorldRelocate(pos); }

void WorldPosition::set(const WorldPosition& pos)
{
    WorldRelocate(pos.m_mapId, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation());
}

void WorldPosition::set(const WorldObject* pos)
{
    WorldRelocate(pos->GetMapId(), pos->GetPositionX(), pos->GetPositionY(), pos->GetPositionZ(), pos->GetOrientation());
}

void WorldPosition::setMapId(uint32 id)
{
    m_mapId = id;
}

void WorldPosition::setX(float x) { m_positionX = x; }

void WorldPosition::setY(float y) { m_positionY = y; }

void WorldPosition::setZ(float z) { m_positionZ = z; }

void WorldPosition::setO(float o) { m_orientation = o; }

WorldPosition::operator bool() const
{
    return GetMapId() != 0 || GetPositionX() != 0 || GetPositionY() != 0 || GetPositionZ() != 0;
}

bool operator==(WorldPosition const& p1, const WorldPosition& p2)
{
    return p1.GetMapId() == p2.GetMapId() && p2.GetPositionX() == p1.GetPositionX() &&
           p2.GetPositionY() == p1.GetPositionY() && p2.GetPositionZ() == p1.GetPositionZ() &&
           p2.GetOrientation() == p1.GetOrientation();
}

bool operator!=(WorldPosition const& p1, const WorldPosition& p2) { return !(p1 == p2); }

WorldPosition& WorldPosition::operator+=(WorldPosition const& p1)
{
    m_positionX += p1.GetPositionX();
    m_positionY += p1.GetPositionY();
    m_positionZ += p1.GetPositionZ();
    return *this;
}

WorldPosition& WorldPosition::operator-=(WorldPosition const& p1)
{
    m_positionX -= p1.GetPositionX();
    m_positionY -= p1.GetPositionY();
    m_positionZ -= p1.GetPositionZ();
    return *this;
}

uint32 WorldPosition::getMapId() { return GetMapId(); }

float WorldPosition::getX() { return GetPositionX(); }

float WorldPosition::getY() { return GetPositionY(); }

float WorldPosition::getZ() { return GetPositionZ(); }

float WorldPosition::getO() { return GetOrientation(); }

bool WorldPosition::isOverworld()
{
    return GetMapId() == 0 || GetMapId() == 1 || GetMapId() == 530 || GetMapId() == 571;
}

bool WorldPosition::isInWater()
{
    return getMap() ? getMap()->IsInWater(PHASEMASK_NORMAL, GetPositionX(), GetPositionY(), GetPositionZ(),
                                          DEFAULT_COLLISION_HEIGHT)
                    : false;
};

bool WorldPosition::isUnderWater()
{
    return getMap() ? getMap()->IsUnderWater(PHASEMASK_NORMAL, GetPositionX(), GetPositionY(), GetPositionZ(),
                                             DEFAULT_COLLISION_HEIGHT)
                    : false;
};

WorldPosition WorldPosition::relPoint(WorldPosition* center)
{
    return WorldPosition(GetMapId(), GetPositionX() - center->GetPositionX(), GetPositionY() - center->GetPositionY(),
                         GetPositionZ() - center->GetPositionZ(), GetOrientation());
}

WorldPosition WorldPosition::offset(WorldPosition* center)
{
    return WorldPosition(GetMapId(), GetPositionX() + center->GetPositionX(), GetPositionY() + center->GetPositionY(),
                         GetPositionZ() + center->GetPositionZ(), GetOrientation());
}

float WorldPosition::size()
{
    return sqrt(pow(GetPositionX(), 2.0) + pow(GetPositionY(), 2.0) + pow(GetPositionZ(), 2.0));
}

float WorldPosition::distance(WorldPosition* center)
{
    if (GetMapId() == center->getMapId())
        return relPoint(center).size();

    // this -> mapTransfer | mapTransfer -> center
    return sTravelMgr->mapTransDistance(*this, *center);
};

float WorldPosition::fDist(WorldPosition* center)
{
    if (GetMapId() == center->getMapId())
        return sqrt(sqDistance2d(center));

    // this -> mapTransfer | mapTransfer -> center
    return sTravelMgr->fastMapTransDistance(*this, *center);
};

float mapTransfer::fDist(WorldPosition start, WorldPosition end)
{
    return start.fDist(pointFrom) + portalLength + pointTo.fDist(end);
}

// When moving from this along list return last point that falls within range.
// Distance is move distance along path.
WorldPosition WorldPosition::lastInRange(std::vector<WorldPosition> list, float minDist, float maxDist)
{
    WorldPosition rPoint;

    float startDist = 0.0f;

    // Enter the path at the closest point.
    for (auto& p : list)
    {
        float curDist = distance(p);
        if (startDist < curDist || p == list.front())
            startDist = curDist + 0.1f;
    }

    float totalDist = 0.0f;

    // Follow the path from the last nearest point
    // Return last point in range.
    for (auto& p : list)
    {
        float curDist = distance(p);

        if (totalDist > 0)  // We have started the path. Keep counting.
            totalDist += p.distance(std::prev(&p, 1));

        if (curDist == startDist)  // Start the path here.
            totalDist = startDist;

        if (minDist > 0 && totalDist < minDist)
            continue;

        if (maxDist > 0 && totalDist > maxDist)
            continue;  // We do not break here because the path may loop back and have a second startDist point.

        rPoint = p;
    }

    return rPoint;
};

// Todo: remove or adjust to above standard.
WorldPosition WorldPosition::firstOutRange(std::vector<WorldPosition> list, float minDist, float maxDist)
{
    WorldPosition rPoint;

    for (auto& p : list)
    {
        if (minDist > 0 && distance(p) < minDist)
            return p;

        if (maxDist > 0 && distance(p) > maxDist)
            return p;

        rPoint = p;
    }

    return rPoint;
}

// Returns true if (on the x-y plane) the position is inside the three points.
bool WorldPosition::isInside(WorldPosition* p1, WorldPosition* p2, WorldPosition* p3)
{
    if (getMapId() != p1->getMapId() != p2->getMapId() != p3->getMapId())
        return false;

    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = mSign(p1, p2);
    d2 = mSign(p2, p3);
    d3 = mSign(p3, p1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

MapEntry const* WorldPosition::getMapEntry() { return sMapStore.LookupEntry(GetMapId()); };

uint32 WorldPosition::getInstanceId()
{
    if (Map* map = sMapMgr->FindBaseMap(getMapId()))
        return map->GetInstanceId();

    return 0;
}

Map* WorldPosition::getMap()
{
    return sMapMgr->FindMap(GetMapId(), getMapEntry()->Instanceable() ? getInstanceId() : 0);
}

float WorldPosition::getHeight()  // remove const - whipowill
{
    return getMap()->GetHeight(getX(), getY(), getZ());
}

G3D::Vector3 WorldPosition::getVector3() { return G3D::Vector3(GetPositionX(), GetPositionY(), GetPositionZ()); }

std::string const WorldPosition::print()
{
    std::ostringstream out;
    out << GetMapId() << std::fixed << std::setprecision(2);
    out << ';' << GetPositionX();
    out << ';' << GetPositionY();
    out << ';' << GetPositionZ();
    out << ';' << GetOrientation();

    return out.str();
}

std::string const WorldPosition::to_string()
{
    std::stringstream out;
    out << GetMapId();
    out << GetPositionX();
    out << GetPositionY();
    out << GetPositionZ();
    out << GetOrientation();
    return out.str();
};

void WorldPosition::printWKT(std::vector<WorldPosition> points, std::ostringstream& out, uint32 dim, bool loop)
{
    switch (dim)
    {
        case 0:
            if (points.size() == 1)
                out << "\"POINT(";
            else
                out << "\"MULTIPOINT(";
            break;
        case 1:
            out << "\"LINESTRING(";
            break;
        case 2:
            out << "\"POLYGON((";
    }

    for (auto& p : points)
        out << p.getDisplayX() << " " << p.getDisplayY() << (!loop && &p == &points.back() ? "" : ",");

    if (loop)
        out << points.front().getDisplayX() << " " << points.front().getDisplayY();

    out << (dim == 2 ? "))\"," : ")\",");
}

WorldPosition WorldPosition::getDisplayLocation()
{
    WorldPosition pos = sTravelNodeMap->getMapOffset(getMapId());
    return offset(const_cast<WorldPosition*>(&pos));
}

uint16 WorldPosition::getAreaId() { return sMapMgr->GetAreaId(PHASEMASK_NORMAL, getMapId(), getX(), getY(), getZ()); }

AreaTableEntry const* WorldPosition::getArea()
{
    uint16 areaId = getAreaId();
    if (!areaId)
        return nullptr;

    return sAreaTableStore.LookupEntry(areaId);
}

std::string const WorldPosition::getAreaName(bool fullName, bool zoneName)
{
    if (!isOverworld())
    {
        MapEntry const* map = sMapStore.LookupEntry(getMapId());
        if (map)
            return map->name[0];
    }

    AreaTableEntry const* area = getArea();
    if (!area)
        return "";

    std::string areaName = area->area_name[0];

    if (fullName)
    {
        uint16 zoneId = area->zone;

        while (zoneId > 0)
        {
            AreaTableEntry const* parentArea = sAreaTableStore.LookupEntry(zoneId);
            if (!parentArea)
                break;

            std::string const subAreaName = parentArea->area_name[0];

            if (zoneName)
                areaName = subAreaName;
            else
                areaName = subAreaName + " " + areaName;

            zoneId = parentArea->zone;
        }
    }

    return std::move(areaName);
}

std::set<Transport*> WorldPosition::getTransports(uint32 entry)
{
    /*
    if(!entry)
        return getMap()->m_transports;
    else
    {
    */
    std::set<Transport*> transports;
    /*
    for (auto transport : getMap()->m_transports)
        if(transport->GetEntry() == entry)
            transports.insert(transport);

    return transports;
}
*/
    return transports;
}

std::vector<GridCoord> WorldPosition::getGridCoord(WorldPosition secondPos)
{
    std::vector<GridCoord> retVec;

    int lx = std::min(getGridCoord().x_coord, secondPos.getGridCoord().x_coord);
    int ly = std::min(getGridCoord().y_coord, secondPos.getGridCoord().y_coord);
    int ux = std::max(getGridCoord().x_coord, secondPos.getGridCoord().x_coord);
    int uy = std::max(getGridCoord().y_coord, secondPos.getGridCoord().y_coord);

    int32 border = 1;

    lx = std::min(std::max(border, lx), MAX_NUMBER_OF_GRIDS - border);
    ly = std::min(std::max(border, ly), MAX_NUMBER_OF_GRIDS - border);
    ux = std::min(std::max(border, ux), MAX_NUMBER_OF_GRIDS - border);
    uy = std::min(std::max(border, uy), MAX_NUMBER_OF_GRIDS - border);

    for (int x = lx - border; x <= ux + border; x++)
    {
        for (int y = ly - border; y <= uy + border; y++)
        {
            retVec.push_back(GridCoord(x, y));
        }
    }

    return retVec;
}

std::vector<WorldPosition> WorldPosition::fromGridCoord(GridCoord gridCoord)
{
    std::vector<WorldPosition> retVec;
    GridCoord g;

    for (uint32 d = 0; d < 4; d++)
    {
        g = gridCoord;

        if (d == 1 || d == 2)
            g.inc_x(1);

        if (d == 2 || d == 3)
            g.inc_y(1);

        retVec.push_back(WorldPosition(getMapId(), g));
    }

    return retVec;
}

std::vector<WorldPosition> WorldPosition::fromCellCoord(CellCoord cellcoord)
{
    std::vector<WorldPosition> retVec;
    CellCoord p;

    for (uint32 d = 0; d < 4; d++)
    {
        p = cellcoord;

        if (d == 1 || d == 2)
            p.inc_x(1);

        if (d == 2 || d == 3)
            p.inc_y(1);

        retVec.push_back(WorldPosition(getMapId(), p));
    }
    return retVec;
}

std::vector<WorldPosition> WorldPosition::gridFromCellCoord(CellCoord cellCoord)
{
    Cell c(cellCoord);

    return fromGridCoord(GridCoord(c.GridX(), c.GridY()));
}

std::vector<std::pair<int32, int32>> WorldPosition::getmGridCoords(WorldPosition secondPos)
{
    std::vector<mGridCoord> retVec;

    int lx = std::min(getmGridCoord().first, secondPos.getmGridCoord().first);
    int ly = std::min(getmGridCoord().second, secondPos.getmGridCoord().second);
    int ux = std::max(getmGridCoord().first, secondPos.getmGridCoord().first);
    int uy = std::max(getmGridCoord().second, secondPos.getmGridCoord().second);
    int border = 1;

    // lx = std::min(std::max(border, lx), MAX_NUMBER_OF_GRIDS - border);
    // ly = std::min(std::max(border, ly), MAX_NUMBER_OF_GRIDS - border);
    // ux = std::min(std::max(border, ux), MAX_NUMBER_OF_GRIDS - border);
    // uy = std::min(std::max(border, uy), MAX_NUMBER_OF_GRIDS - border);

    for (int x = lx - border; x <= ux + border; x++)
    {
        for (int y = ly - border; y <= uy + border; y++)
        {
            retVec.push_back(std::make_pair(x, y));
        }
    }

    return retVec;
}

std::vector<WorldPosition> WorldPosition::frommGridCoord(mGridCoord GridCoord)
{
    std::vector<WorldPosition> retVec;
    mGridCoord g;

    for (uint32 d = 0; d < 4; d++)
    {
        g = GridCoord;

        if (d == 1 || d == 2)
            g.second++;
        if (d == 2 || d == 3)
            g.first++;

        retVec.push_back(WorldPosition(getMapId(), g));
    }

    return retVec;
}

void WorldPosition::loadMapAndVMap(uint32 mapId, uint8 x, uint8 y)
{
    std::string const fileName = "load_map_grid.csv";

    if (isOverworld() && false || false)
    {
        if (!MMAP::MMapFactory::createOrGetMMapMgr()->loadMap(mapId, x, y))
            if (sPlayerbotAIConfig->hasLog(fileName))
            {
                std::ostringstream out;
                out << sPlayerbotAIConfig->GetTimestampStr();
                out << "+00,\"mmap\", " << x << "," << y << "," << (sTravelMgr->isBadMmap(mapId, x, y) ? "0" : "1")
                    << ",";
                printWKT(fromGridCoord(GridCoord(x, y)), out, 1, true);
                sPlayerbotAIConfig->log(fileName, out.str().c_str());
            }
    }
    else
    {
        // This needs to be disabled or maps will not load.
        // Needs more testing to check for impact on movement.
        if (false)
            if (!sTravelMgr->isBadVmap(mapId, x, y))
            {
                // load VMAPs for current map/grid...
                const MapEntry* i_mapEntry = sMapStore.LookupEntry(mapId);
                const char* mapName = i_mapEntry ? i_mapEntry->name[sWorld->GetDefaultDbcLocale()] : "UNNAMEDMAP\x0";

                int vmapLoadResult = VMAP::VMapFactory::createOrGetVMapMgr()->loadMap(
                    (sWorld->GetDataPath() + "vmaps").c_str(), mapId, x, y);
                switch (vmapLoadResult)
                {
                    case VMAP::VMAP_LOAD_RESULT_OK:
                        // LOG_ERROR("playerbots", "VMAP loaded name:{}, id:{}, x:{}, y:{} (vmap rep.: x:{}, y:{})",
                        // mapName, mapId, x, y, x, y);
                        break;
                    case VMAP::VMAP_LOAD_RESULT_ERROR:
                        // LOG_ERROR("playerbots", "Could not load VMAP name:{}, id:{}, x:{}, y:{} (vmap rep.: x:{},
                        // y:{})", mapName, mapId, x, y, x, y);
                        sTravelMgr->addBadVmap(mapId, x, y);
                        break;
                    case VMAP::VMAP_LOAD_RESULT_IGNORED:
                        sTravelMgr->addBadVmap(mapId, x, y);
                        // LOG_INFO("playerbots", "Ignored VMAP name:{}, id:{}, x:{}, y:{} (vmap rep.: x:{}, y:{})",
                        // mapName, mapId, x, y, x, y);
                        break;
                }

                if (sPlayerbotAIConfig->hasLog(fileName))
                {
                    std::ostringstream out;
                    out << sPlayerbotAIConfig->GetTimestampStr();
                    out << "+00,\"vmap\", " << x << "," << y << ", " << (sTravelMgr->isBadVmap(mapId, x, y) ? "0" : "1")
                        << ",";
                    printWKT(frommGridCoord(mGridCoord(x, y)), out, 1, true);
                    sPlayerbotAIConfig->log(fileName, out.str().c_str());
                }
            }

        if (!sTravelMgr->isBadMmap(mapId, x, y))
        {
            // load navmesh
            if (!MMAP::MMapFactory::createOrGetMMapMgr()->loadMap(mapId, x, y))
                sTravelMgr->addBadMmap(mapId, x, y);

            if (sPlayerbotAIConfig->hasLog(fileName))
            {
                std::ostringstream out;
                out << sPlayerbotAIConfig->GetTimestampStr();
                out << "+00,\"mmap\", " << x << "," << y << "," << (sTravelMgr->isBadMmap(mapId, x, y) ? "0" : "1")
                    << ",";
                printWKT(fromGridCoord(GridCoord(x, y)), out, 1, true);
                sPlayerbotAIConfig->log(fileName, out.str().c_str());
            }
        }
    }
}

void WorldPosition::loadMapAndVMaps(WorldPosition secondPos)
{
    for (auto& grid : getmGridCoords(secondPos))
    {
        loadMapAndVMap(getMapId(), grid.first, grid.second);
    }
}

std::vector<WorldPosition> WorldPosition::fromPointsArray(std::vector<G3D::Vector3> path)
{
    std::vector<WorldPosition> retVec;
    for (auto p : path)
        retVec.push_back(WorldPosition(getMapId(), p.x, p.y, p.z, getO()));

    return retVec;
}

// A single pathfinding attempt from one position to another. Returns pathfinding status and path.
std::vector<WorldPosition> WorldPosition::getPathStepFrom(WorldPosition startPos, Unit* bot)
{
    if (!bot)
        return {};

    // Load mmaps and vmaps between the two points.
    loadMapAndVMaps(startPos);

    PathGenerator path(bot);
    path.CalculatePath(startPos.getX(), startPos.getY(), startPos.getZ());

    Movement::PointsArray points = path.GetPath();
    PathType type = path.GetPathType();

    if (sPlayerbotAIConfig->hasLog("pathfind_attempt_point.csv"))
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(1);
        printWKT({startPos, *this}, out);
        sPlayerbotAIConfig->log("pathfind_attempt_point.csv", out.str().c_str());
    }

    if (sPlayerbotAIConfig->hasLog("pathfind_attempt.csv") && (type == PATHFIND_INCOMPLETE || type == PATHFIND_NORMAL))
    {
        std::ostringstream out;
        out << sPlayerbotAIConfig->GetTimestampStr() << "+00,";
        out << std::fixed << std::setprecision(1) << type << ",";
        printWKT(fromPointsArray(points), out, 1);
        sPlayerbotAIConfig->log("pathfind_attempt.csv", out.str().c_str());
    }

    if (type == PATHFIND_INCOMPLETE || type == PATHFIND_NORMAL)
        return fromPointsArray(points);

    return {};
}

bool WorldPosition::cropPathTo(std::vector<WorldPosition>& path, float maxDistance)
{
    if (path.empty())
        return false;

    auto bestPos = std::min_element(path.begin(), path.end(),
                                    [this](WorldPosition i, WorldPosition j)
                                    { return this->sqDistance(i) < this->sqDistance(j); });

    bool insRange = this->sqDistance(*bestPos) <= maxDistance * maxDistance;

    if (bestPos == path.end())
        return insRange;

    path.erase(std::next(bestPos), path.end());

    return insRange;
}

// A sequential series of pathfinding attempts. Returns the complete path and if the patfinder eventually found a way to
// the destination.
std::vector<WorldPosition> WorldPosition::getPathFromPath(std::vector<WorldPosition> startPath, Unit* bot,
                                                          uint8 maxAttempt)
{
    // We start at the end of the last path.
    WorldPosition currentPos = startPath.back();

    // No pathfinding across maps.
    if (getMapId() != currentPos.getMapId())
        return {};

    std::vector<WorldPosition> subPath, fullPath = startPath;

    // Limit the pathfinding attempts
    for (uint32 i = 0; i < maxAttempt; i++)
    {
        // Try to pathfind to this position.
        subPath = getPathStepFrom(currentPos, bot);

        // If we could not find a path return what we have now.
        if (subPath.empty() || currentPos.distance(&subPath.back()) < sPlayerbotAIConfig->targetPosRecalcDistance)
            break;

        // Append the path excluding the start (this should be the same as the end of the startPath)
        fullPath.insert(fullPath.end(), std::next(subPath.begin(), 1), subPath.end());

        // Are we there yet?
        if (isPathTo(subPath))
            break;

        // Continue pathfinding.
        currentPos = subPath.back();
    }

    return fullPath;
}

bool WorldPosition::GetReachableRandomPointOnGround(Player* bot, float radius, bool randomRange)
{
    radius *= randomRange ? rand_norm() : 1.f;
    float angle = rand_norm() * static_cast<float>(2 * M_PI);
    m_positionX += radius * cosf(angle);
    m_positionY += radius * sinf(angle);

    return getMap()->CanReachPositionAndGetValidCoords(bot, m_positionX, m_positionY, m_positionZ);
}

uint32 WorldPosition::getUnitsAggro(GuidVector& units, Player* bot)
{
    units.erase(std::remove_if(units.begin(), units.end(),
                               [this, bot](ObjectGuid guid)
                               {
                                   Creature* creature = ObjectAccessor::GetCreature(*bot, guid);
                                   if (!creature)
                                       return true;

                                   return sqDistance(WorldPosition(creature)) >
                                          creature->GetAttackDistance(bot) * creature->GetAttackDistance(bot);
                               }),
                units.end());

    return units.size();
}

void FindPointCreatureData::operator()(CreatureData const& creatureData)
{
    if (!entry || creatureData.id1 == entry)
        if ((!point || creatureData.mapid == point.getMapId()) &&
            (!radius || point.sqDistance(WorldPosition(creatureData.mapid, creatureData.posX, creatureData.posY,
                                                       creatureData.posZ)) < radius * radius))
        {
            data.push_back(&creatureData);
        }
}

void FindPointGameObjectData::operator()(GameObjectData const& gameobjectData)
{
    if (!entry || gameobjectData.id == entry)
        if ((!point || gameobjectData.mapid == point.getMapId()) &&
            (!radius || point.sqDistance(WorldPosition(gameobjectData.mapid, gameobjectData.posX, gameobjectData.posY,
                                                       gameobjectData.posZ)) < radius * radius))
        {
            data.push_back(&gameobjectData);
        }
}

std::vector<CreatureData const*> WorldPosition::getCreaturesNear(float radius, uint32 entry)
{
    FindPointCreatureData worker(*this, radius, entry);
    for (auto const& itr : sObjectMgr->GetAllCreatureData())
        worker(itr.second);

    return worker.GetResult();
}

std::vector<GameObjectData const*> WorldPosition::getGameObjectsNear(float radius, uint32 entry)
{
    FindPointGameObjectData worker(*this, radius, entry);
    for (auto const& itr : sObjectMgr->GetAllGOData())
        worker(itr.second);

    return worker.GetResult();
}

Creature* GuidPosition::GetCreature()
{
    if (!*this)
        return nullptr;

    if (loadedFromDB)
    {
        auto creatureBounds = getMap()->GetCreatureBySpawnIdStore().equal_range(GetCounter());
        if (creatureBounds.first != creatureBounds.second)
            return creatureBounds.second->second;

        return nullptr;
    }

    return getMap()->GetCreature(*this);
}

Unit* GuidPosition::GetUnit()
{
    if (!*this)
        return nullptr;

    if (loadedFromDB)
    {
        auto creatureBounds = getMap()->GetCreatureBySpawnIdStore().equal_range(GetCounter());
        if (creatureBounds.first != creatureBounds.second)
            return creatureBounds.second->second;

        return nullptr;
    }

    if (IsPlayer())
        return ObjectAccessor::FindPlayer(*this);

    if (IsPet())
        return getMap()->GetPet(*this);

    return GetCreature();
}

GameObject* GuidPosition::GetGameObject()
{
    if (!*this)
        return nullptr;

    if (loadedFromDB)
    {
        auto gameobjectBounds = getMap()->GetGameObjectBySpawnIdStore().equal_range(GetCounter());
        if (gameobjectBounds.first != gameobjectBounds.second)
            return gameobjectBounds.second->second;

        return nullptr;
    }

    return getMap()->GetGameObject(*this);
}

Player* GuidPosition::GetPlayer()
{
    if (!*this)
        return nullptr;

    if (IsPlayer())
        return ObjectAccessor::FindPlayer(*this);

    return nullptr;
}

bool GuidPosition::isDead()
{
    if (!getMap())
        return false;

    if (!getMap()->IsGridLoaded(getX(), getY()))
        return false;

    if (IsUnit() && GetUnit() && GetUnit()->IsInWorld() && GetUnit()->IsAlive())
        return false;

    if (IsGameObject() && GetGameObject() && GetGameObject()->IsInWorld())
        return false;

    return true;
}

GuidPosition::GuidPosition(WorldObject* wo) : ObjectGuid(wo->GetGUID()), WorldPosition(wo), loadedFromDB(false) {}

GuidPosition::GuidPosition(CreatureData const& creData)
    : ObjectGuid(HighGuid::Unit, creData.id1, creData.spawnId),
      WorldPosition(creData.mapid, creData.posX, creData.posY, creData.posZ, creData.orientation)
{
    loadedFromDB = true;
}

GuidPosition::GuidPosition(GameObjectData const& goData)
    : ObjectGuid(HighGuid::GameObject, goData.id),
      WorldPosition(goData.mapid, goData.posX, goData.posY, goData.posZ, goData.orientation)
{
    loadedFromDB = true;
}

CreatureTemplate const* GuidPosition::GetCreatureTemplate()
{
    return IsCreature() ? sObjectMgr->GetCreatureTemplate(GetEntry()) : nullptr;
}

GameObjectTemplate const* GuidPosition::GetGameObjectTemplate()
{
    return IsGameObject() ? sObjectMgr->GetGameObjectTemplate(GetEntry()) : nullptr;
}

WorldObject* GuidPosition::GetWorldObject()
{
    if (!*this)
        return nullptr;

    switch (GetHigh())
    {
        case HighGuid::Player:
            return ObjectAccessor::FindPlayer(*this);
        case HighGuid::Transport:
        case HighGuid::Mo_Transport:
        case HighGuid::GameObject:
            return GetGameObject();
        case HighGuid::Vehicle:
        case HighGuid::Unit:
            return GetCreature();
        case HighGuid::Pet:
            return getMap()->GetPet(*this);
        case HighGuid::DynamicObject:
            return getMap()->GetDynamicObject(*this);
        case HighGuid::Corpse:
            return getMap()->GetCorpse(*this);
        default:
            return nullptr;
    }

    return nullptr;
}

bool GuidPosition::HasNpcFlag(NPCFlags flag) { return IsCreature() && GetCreatureTemplate()->npcflag & flag; }

std::vector<WorldPosition*> TravelDestination::getPoints(bool ignoreFull)
{
    if (ignoreFull)
        return points;

    uint32 max = maxVisitorsPerPoint;
    if (!max)
        return points;

    std::vector<WorldPosition*> retVec;
    std::copy_if(points.begin(), points.end(), std::back_inserter(retVec),
                 [max](WorldPosition* p) { return p->getVisitors() < max; });
    return retVec;
}

WorldPosition* TravelDestination::nearestPoint(WorldPosition* pos)
{
    return *std::min_element(points.begin(), points.end(),
                             [pos](WorldPosition* i, WorldPosition* j) { return i->distance(pos) < j->distance(pos); });
}

std::vector<WorldPosition*> TravelDestination::touchingPoints(WorldPosition* pos)
{
    std::vector<WorldPosition*> ret_points;
    for (auto& point : points)
    {
        float dist = pos->distance(point);
        if (!dist)
            continue;

        if (dist > radiusMax * 2)
            continue;

        ret_points.push_back(point);
    }

    return ret_points;
};

std::vector<WorldPosition*> TravelDestination::sortedPoints(WorldPosition* pos)
{
    std::vector<WorldPosition*> ret_points = points;
    std::sort(ret_points.begin(), ret_points.end(),
              [pos](WorldPosition* i, WorldPosition* j) { return i->distance(pos) < j->distance(pos); });
    return ret_points;
};

std::vector<WorldPosition*> TravelDestination::nextPoint(WorldPosition* pos, bool ignoreFull)
{
    return sTravelMgr->getNextPoint(pos, ignoreFull ? points : getPoints());
}

bool TravelDestination::isFull(bool ignoreFull)
{
    if (!ignoreFull && maxVisitors > 0 && visitors >= maxVisitors)
        return true;

    if (maxVisitorsPerPoint > 0)
        if (getPoints(ignoreFull).empty())
            return true;

    return false;
}

std::string const QuestTravelDestination::getTitle() { return ChatHelper::FormatQuest(questTemplate); }

bool QuestRelationTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    if (botAI && !botAI->HasStrategy("rpg quest", BOT_STATE_NON_COMBAT))
        return false;

    if (relation == 0)
    {
        if ((int32)questTemplate->GetQuestLevel() >= (int32)bot->GetLevel() + (int32)5)
            return false;

        // skip for now this quest
        if (getPoints().front()->GetMapId() != bot->GetMapId())
            return false;

        if (!bot->GetMap()->GetEntry()->IsWorldMap() || !bot->CanTakeQuest(questTemplate, false))
            return false;

        uint32 dialogStatus = sTravelMgr->getDialogStatus(bot, entry, questTemplate);

        if (AI_VALUE(bool, "can fight equal"))
        {
            if (AI_VALUE(uint8, "free quest log slots") < 5)
                return false;

            //None has yellow exclamation mark.
            if (!AI_VALUE2(bool, "group or", "following party,near leader,can accept quest npc::" + std::to_string(entry)))
                if (!AI_VALUE2(bool, "group or", "following party,near leader,can accept quest low level npc::" + std::to_string(entry) + "need quest objective::" + std::to_string(questId))) //Noone can do this quest for a usefull reward.
                    return false;
        }
        else
        {
            if (!AI_VALUE2(bool, "group or", "following party,near leader,can accept quest low level npc::" + std::to_string(entry))) //Noone can pick up this quest for money.
                return false;

            if (AI_VALUE(uint8, "free quest log slots") < 10)
                return false;
        }

        // Do not try to pick up dungeon/elite quests in instances without a group.
        if ((questTemplate->GetType() == QUEST_TYPE_ELITE || questTemplate->GetType() == QUEST_TYPE_DUNGEON) &&
            !AI_VALUE(bool, "can fight boss"))
            return false;
    }
    else
    {
        if (!AI_VALUE2(bool, "group or", "following party,near leader,can turn in quest npc::" + std::to_string(entry)))
            return false;

        //Do not try to hand-in dungeon/elite quests in instances without a group.
        if ((questTemplate->GetType() == QUEST_TYPE_ELITE || questTemplate->GetType() == QUEST_TYPE_DUNGEON) && !AI_VALUE(bool, "can fight boss"))
        {
            WorldPosition pos(bot);
            if (!this->nearestPoint(&pos)->isOverworld())
                return false;
        }
    }

    return true;
}

std::string const QuestRelationTravelDestination::getTitle()
{
    std::ostringstream out;

    if (relation == 0)
        out << "questgiver";
    else
        out << "questtaker";

    out << " " << ChatHelper::FormatWorldEntry(entry);
    return out.str();
}

bool QuestObjectiveTravelDestination::isActive(Player* bot)
{
    if (questTemplate->GetQuestLevel() > bot->GetLevel() + 1)
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();
    if (questTemplate->GetQuestLevel() + 5 > bot->GetLevel() && !AI_VALUE(bool, "can fight equal"))
        return false;

    // Check mob level
    if (getEntry() > 0)
    {
        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(getEntry());
        if (cInfo && (int)cInfo->maxlevel - (int)bot->GetLevel() > 4)
            return false;

        // Do not try to hand-in dungeon/elite quests in instances without a group.
        if (cInfo->rank > CREATURE_ELITE_NORMAL)
        {
            WorldPosition pos(bot);
            if (!this->nearestPoint(const_cast<WorldPosition*>(&pos))->isOverworld() &&
                !AI_VALUE(bool, "can fight boss"))
                return false;

            if (!AI_VALUE(bool, "can fight elite"))
                return false;
        }
    }

    if (questTemplate->GetType() == QUEST_TYPE_ELITE && !AI_VALUE(bool, "can fight elite"))
        return false;

    if (!sTravelMgr->getObjectiveStatus(bot, questTemplate, objective))
        return false;

    WorldPosition botPos(bot);

    if (getEntry() > 0 && !isOut(&botPos))
    {
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");

        for (auto& target : targets)
            if (target.GetEntry() == getEntry() && target.IsCreature() && botAI->GetCreature(target) &&
                botAI->GetCreature(target)->IsAlive())
                return true;

        return false;
    }

    return true;
}

std::string const QuestObjectiveTravelDestination::getTitle()
{
    std::ostringstream out;

    out << "objective " << objective;

    if (itemId)
        out << " loot " << ChatHelper::FormatItem(sObjectMgr->GetItemTemplate(itemId), 0, 0) << " from";
    else
        out << " to kill";

    out << " " << ChatHelper::FormatWorldEntry(entry);
    return out.str();
}

bool RpgTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if (!cInfo)
        return false;

    bool isUsefull = false;

    if (cInfo->npcflag & UNIT_NPC_FLAG_VENDOR)
        if (AI_VALUE2_LAZY(bool, "group or", "should sell,can sell,following party,near leader"))
            isUsefull = true;

    if (cInfo->npcflag & UNIT_NPC_FLAG_REPAIR)
        if (AI_VALUE2_LAZY(bool, "group or", "should repair,can repair,following party,near leader"))
            isUsefull = true;

    if (!isUsefull)
        return false;

    // Once the target rpged with it is added to the ignore list. We can now move on.
    GuidSet& ignoreList = GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<GuidSet&>("ignore rpg target")->Get();

    for (ObjectGuid const guid : ignoreList)
    {
        if (guid.GetEntry() == getEntry())
        {
            return false;
        }
    }

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
    ReputationRank reaction = bot->GetReputationRank(factionEntry->faction);

    return reaction > REP_NEUTRAL;
}

CreatureTemplate const* RpgTravelDestination::GetCreatureTemplate() { return sObjectMgr->GetCreatureTemplate(entry); }

std::string const RpgTravelDestination::getTitle()
{
    std::ostringstream out;

    if(entry > 0)
        out << "rpg npc ";

    out << " " << ChatHelper::FormatWorldEntry(entry);

    return out.str();
}

bool ExploreTravelDestination::isActive(Player* bot)
{
    AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaId);

    if (area->area_level && (uint32)area->area_level > bot->GetLevel() && bot->GetLevel() < DEFAULT_MAX_LEVEL)
        return false;

    if (area->exploreFlag == 0xffff)
        return false;

    int offset = area->exploreFlag / 32;

    uint32 val = (uint32)(1 << (area->exploreFlag % 32));
    uint32 currFields = bot->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);

    return !(currFields & val);
}

// std::string const ExploreTravelDestination::getTitle()
//{
//     return points[0]->getAreaName();
// };

bool GrindTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    if (!AI_VALUE(bool, "should get money"))
        return false;

    if (AI_VALUE(bool, "should sell"))
        return false;

    CreatureTemplate const* cInfo = GetCreatureTemplate();

    int32 botLevel = bot->GetLevel();

    uint8 botPowerLevel = AI_VALUE(uint8, "durability");
    float levelMod = botPowerLevel / 500.0f;   //(0-0.2f)
    float levelBoost = botPowerLevel / 50.0f;  //(0-2.0f)

    int32 maxLevel = std::max(botLevel * (0.5f + levelMod), botLevel - 5.0f + levelBoost);

    if ((int32)cInfo->maxlevel > maxLevel)  //@lvl5 max = 3, @lvl60 max = 57
        return false;

    int32 minLevel = std::max(botLevel * (0.4f + levelMod), botLevel - 12.0f + levelBoost);

    if ((int32)cInfo->maxlevel < minLevel)  //@lvl5 min = 3, @lvl60 max = 50
        return false;

    if (!cInfo->mingold)
        return false;

    if (cInfo->rank > CREATURE_ELITE_NORMAL && !AI_VALUE(bool, "can fight elite"))
        return false;

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
    ReputationRank reaction = bot->GetReputationRank(factionEntry->faction);

    return reaction < REP_NEUTRAL;
}

CreatureTemplate const* GrindTravelDestination::GetCreatureTemplate() { return sObjectMgr->GetCreatureTemplate(entry); }

std::string const GrindTravelDestination::getTitle()
{
    std::ostringstream out;

    out << "grind mob ";

    out << " " << ChatHelper::FormatWorldEntry(entry);

    return out.str();
}

bool BossTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    if (!AI_VALUE(bool, "can fight boss"))
        return false;

    CreatureTemplate const* cInfo = getCreatureTemplate();

    /*
    int32 botLevel = bot->GetLevel();

    uint8 botPowerLevel = AI_VALUE(uint8, "durability");
    float levelMod = botPowerLevel / 500.0f; //(0-0.2f)
    float levelBoost = botPowerLevel / 50.0f; //(0-2.0f)

    int32 maxLevel = botLevel + 3.0;

    if ((int32)cInfo->MaxLevel > maxLevel) //@lvl5 max = 3, @lvl60 max = 57
        return false;

    int32 minLevel = botLevel - 10;

    if ((int32)cInfo->MaxLevel < minLevel) //@lvl5 min = 3, @lvl60 max = 50
        return false;
    */

    if ((int32)cInfo->maxlevel > bot->GetLevel() + 3)
        return false;

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
    ReputationRank reaction = Unit::GetFactionReactionTo(bot->GetFactionTemplateEntry(), factionEntry);

    if (reaction >= REP_NEUTRAL)
        return false;

    WorldPosition botPos(bot);
    if (!isOut(&botPos))
    {
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");

        for (auto& target : targets)
            if (target.GetEntry() == getEntry() && target.IsCreature() && botAI->GetCreature(target) &&
                botAI->GetCreature(target)->IsAlive())
                return true;

        return false;
    }

    if (!AI_VALUE2(bool, "has upgrade", getEntry()))
        return false;

    return true;
}

CreatureTemplate const* BossTravelDestination::getCreatureTemplate() { return sObjectMgr->GetCreatureTemplate(entry); }

std::string const BossTravelDestination::getTitle()
{
    std::ostringstream out;
    out << "boss mob ";
    out << " " << ChatHelper::FormatWorldEntry(entry);

    return out.str();
}

TravelTarget::~TravelTarget()
{
    if (!tDestination)
        return;

    releaseVisitors();
    // sTravelMgr->botTargets.erase(std::remove(sTravelMgr->botTargets.begin(), sTravelMgr->botTargets.end(), this),
    // sTravelMgr->botTargets.end());
}

void TravelTarget::setTarget(TravelDestination* tDestination1, WorldPosition* wPosition1, bool groupCopy1)
{
    releaseVisitors();

    wPosition = wPosition1;
    tDestination = tDestination1;
    groupCopy = groupCopy1;
    forced = false;
    radius = 0;

    addVisitors();

    setStatus(TRAVEL_STATUS_TRAVEL);
}

void TravelTarget::copyTarget(TravelTarget* target)
{
    setTarget(target->tDestination, target->wPosition);
    groupCopy = target->isGroupCopy();
    forced = target->forced;
    extendRetryCount = target->extendRetryCount;
}

void TravelTarget::addVisitors()
{
    if (!visitor)
    {
        wPosition->addVisitor();
        tDestination->addVisitor();
    }

    visitor = true;
}

void TravelTarget::releaseVisitors()
{
    if (visitor)
    {
        if (tDestination)
            tDestination->remVisitor();
        if (wPosition)
            wPosition->remVisitor();
    }

    visitor = false;
}

float TravelTarget::distance(Player* bot)
{
    WorldPosition pos(bot);
    return wPosition->distance(&pos);
}

WorldPosition* TravelTarget::getPosition() { return wPosition; }

TravelDestination* TravelTarget::getDestination() { return tDestination; }

void TravelTarget::setStatus(TravelStatus status)
{
    m_status = status;
    startTime = getMSTime();

    switch (m_status)
    {
        case TRAVEL_STATUS_NONE:
        case TRAVEL_STATUS_PREPARE:
        case TRAVEL_STATUS_EXPIRED:
            statusTime = 1;
            break;
        case TRAVEL_STATUS_TRAVEL:
            statusTime = getMaxTravelTime() * 2 + sPlayerbotAIConfig->maxWaitForMove;
            break;
        case TRAVEL_STATUS_WORK:
            statusTime = tDestination->getExpireDelay();
            break;
        case TRAVEL_STATUS_COOLDOWN:
            statusTime = tDestination->getCooldownDelay();
        default:
            break;
    }
}

bool TravelTarget::isActive()
{
    if (m_status == TRAVEL_STATUS_NONE || m_status == TRAVEL_STATUS_EXPIRED || m_status == TRAVEL_STATUS_PREPARE)
        return false;

    if (forced && isTraveling())
        return true;

    if ((statusTime > 0 && startTime + statusTime < getMSTime()))
    {
        setStatus(TRAVEL_STATUS_EXPIRED);
        return false;
    }

    if (m_status == TRAVEL_STATUS_COOLDOWN)
        return true;

    if (isTraveling())
        return true;

    if (isWorking())
        return true;

    if (!tDestination->isActive(bot))  // Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return true;
    }

    return true;
};

uint32 TravelTarget::getMaxTravelTime() { return (1000.0 * distance(bot)) / bot->GetSpeed(MOVE_RUN); }

bool TravelTarget::isTraveling()
{
    if (m_status != TRAVEL_STATUS_TRAVEL)
        return false;

    if (!tDestination->isActive(bot) && !forced)  // Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return false;
    }

    WorldPosition pos(bot);

    bool HasArrived = tDestination->isIn(&pos, radius);

    if (HasArrived)
    {
        setStatus(TRAVEL_STATUS_WORK);
        return false;
    }

    if (!botAI->HasStrategy("travel", BOT_STATE_NON_COMBAT))
    {
        setTarget(sTravelMgr->nullTravelDestination, sTravelMgr->nullWorldPosition, true);
        return false;
    }

    return true;
}

bool TravelTarget::isWorking()
{
    if (m_status != TRAVEL_STATUS_WORK)
        return false;

    if (!tDestination->isActive(bot))  // Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return false;
    }

    WorldPosition pos(bot);

    /*
    bool HasLeft = tDestination->isOut(&pos);

    if (HasLeft)
    {
        setStatus(TRAVEL_STATUS_TRAVEL);
        return false;
    }
    */

    if (!botAI->HasStrategy("travel", BOT_STATE_NON_COMBAT))
    {
        setTarget(sTravelMgr->nullTravelDestination, sTravelMgr->nullWorldPosition, true);
        return false;
    }

    return true;
}

bool TravelTarget::isPreparing()
{
    if (m_status != TRAVEL_STATUS_PREPARE)
        return false;

    return true;
}

TravelState TravelTarget::getTravelState()
{
    if (!tDestination || tDestination->getName() == "NullTravelDestination")
        return TRAVEL_STATE_IDLE;

    if (tDestination->getName() == "QuestRelationTravelDestination")
    {
        if (((QuestRelationTravelDestination*)tDestination)->getRelation() == 0)
        {
            if (isTraveling() || isPreparing())
                return TRAVEL_STATE_TRAVEL_PICK_UP_QUEST;

            if (isWorking())
                return TRAVEL_STATE_WORK_PICK_UP_QUEST;
        }
        else
        {
            if (isTraveling() || isPreparing())
                return TRAVEL_STATE_TRAVEL_HAND_IN_QUEST;

            if (isWorking())
                return TRAVEL_STATE_WORK_HAND_IN_QUEST;
        }
    }
    else if (tDestination->getName() == "QuestObjectiveTravelDestination")
    {
        if (isTraveling() || isPreparing())
            return TRAVEL_STATE_TRAVEL_DO_QUEST;

        if (isWorking())
            return TRAVEL_STATE_WORK_DO_QUEST;
    }
    else if (tDestination->getName() == "RpgTravelDestination")
    {
        return TRAVEL_STATE_TRAVEL_RPG;
    }
    else if (tDestination->getName() == "ExploreTravelDestination")
    {
        return TRAVEL_STATE_TRAVEL_EXPLORE;
    }

    return TRAVEL_STATE_IDLE;
}

void TravelMgr::Clear()
{
    std::shared_lock<std::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& m = ObjectAccessor::GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
        TravelMgr::setNullTravelTarget(itr->second);

    for (auto& quest : quests)
    {
        for (auto& dest : quest.second->questGivers)
        {
            delete dest;
        }

        for (auto& dest : quest.second->questTakers)
        {
            delete dest;
        }

        for (auto& dest : quest.second->questObjectives)
        {
            delete dest;
        }
    }

    questGivers.clear();
    quests.clear();
}

void TravelMgr::logQuestError(uint32 errorNr, Quest* quest, uint32 objective, uint32 unitId, uint32 itemId)
{
    bool logQuestErrors = false;  // For debugging.

    if (!logQuestErrors)
        return;

    std::string unitName = "<unknown>";
    CreatureTemplate const* cInfo = nullptr;
    GameObjectTemplate const* gInfo = nullptr;

    if (unitId > 0)
        cInfo = sObjectMgr->GetCreatureTemplate(unitId);
    else
        gInfo = sObjectMgr->GetGameObjectTemplate(unitId * -1);

    if (cInfo)
        unitName = cInfo->Name;
    else if (gInfo)
        unitName = gInfo->name;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);

    if (errorNr == 1)
    {
        LOG_ERROR("playerbots", "Quest {} [{}] has {} {} [{}] but none is found in the world.",
                  quest->GetTitle().c_str(), quest->GetQuestId(), objective == 0 ? "quest giver" : "quest taker",
                  unitName.c_str(), unitId);
    }
    else if (errorNr == 2)
    {
        LOG_ERROR("playerbots", "Quest {} [{}] needs {} [{}] for objective {} but none is found in the world.",
                  quest->GetTitle().c_str(), quest->GetQuestId(), unitName.c_str(), unitId, objective);
    }
    else if (errorNr == 3)
    {
        LOG_ERROR("playerbots", "Quest {} [{}] needs itemId {} but no such item exists.", quest->GetTitle().c_str(),
                  quest->GetQuestId(), itemId);
    }
    else if (errorNr == 4)
    {
        LOG_ERROR(
            "playerbots",
            "Quest {} [{}] needs {} [{}] for loot of item {} [{}] for objective {} but none is found in the world.",
            quest->GetTitle().c_str(), quest->GetQuestId(), unitName.c_str(), unitId, proto->Name1.c_str(), itemId,
            objective);
    }
    else if (errorNr == 5)
    {
        LOG_ERROR("playerbots", "Quest {} [{}] needs item {} [{}] for objective {} but none is found in the world.",
                  quest->GetTitle().c_str(), quest->GetQuestId(), proto->Name1.c_str(), itemId, objective);
    }
    else if (errorNr == 6)
    {
        LOG_ERROR("playerbots", "Quest {} [{}] has no quest giver.", quest->GetTitle().c_str(), quest->GetQuestId());
    }
    else if (errorNr == 7)
    {
        LOG_ERROR("playerbots", "Quest {} [{}] has no quest taker.", quest->GetTitle().c_str(), quest->GetQuestId());
    }
    else if (errorNr == 8)
    {
        LOG_ERROR("playerbots", "Quest {} [{}] has no quest viable quest objective.", quest->GetTitle().c_str(),
                  quest->GetQuestId());
    }
}

void TravelMgr::LoadQuestTravelTable()
{
    if (!sTravelMgr->quests.empty())
        return;

    // Clearing store (for reloading case)
    Clear();

    /* remove this
    questGuidMap cQuestMap = GAI_VALUE(questGuidMap,"quest objects");

    for (auto cQuest : cQuestMap)
    {
        LOG_INFO("playerbots", "[Quest id: {}]", cQuest.first);

        for (auto cObj : cQuest.second)
        {
            LOG_INFO("playerbots", " [Objective type: {}]", cObj.first);

            for (auto cCre : cObj.second)
            {
                LOG_INFO("playerbots", " {} {}", cCre.GetTypeName().c_str(), cCre.GetEntry());
            }
        }
    }
    */

    struct unit
    {
        uint32 type;
        uint32 entry;
        uint32 map;
        float x;
        float y;
        float z;
        float o;
        uint32 c;
    } t_unit;
    std::vector<unit> units;

    /*struct relation
    {
        uint32 type;
        uint32 role;
        uint32 entry;
        uint32 questId;
    } t_rel;
    std::vector<relation> relations;

    struct loot
    {
        uint32 type;
        uint32 entry;
        uint32 item;
    } t_loot;
    std::vector<loot> loots;*/

    ObjectMgr::QuestMap const& questMap = sObjectMgr->GetQuestTemplates();
    std::vector<uint32> questIds;
    std::unordered_map<uint32, uint32> entryCount;

    for (auto& quest : questMap)
        questIds.push_back(quest.first);

    sort(questIds.begin(), questIds.end());

    LOG_INFO("playerbots", "Loading units locations.");
    for (auto& creatureData : WorldPosition().getCreaturesNear())
    {
        t_unit.type = 0;
        t_unit.entry = creatureData->id1;
        t_unit.map = creatureData->mapid;
        t_unit.x = creatureData->posX;
        t_unit.y = creatureData->posY;
        t_unit.z = creatureData->posZ;
        t_unit.o = creatureData->orientation;

        entryCount[creatureData->id1]++;

        units.push_back(t_unit);
    }

    for (auto& unit : units)
    {
        unit.c = entryCount[unit.entry];
    }

    LOG_INFO("playerbots", "Loading game object locations.");
    for (auto& gameobjectData : WorldPosition().getGameObjectsNear())
    {
        t_unit.type = 1;
        t_unit.entry = gameobjectData->id;
        t_unit.map = gameobjectData->mapid;
        t_unit.x = gameobjectData->posX;
        t_unit.y = gameobjectData->posY;
        t_unit.z = gameobjectData->posZ;
        t_unit.o = gameobjectData->orientation;
        t_unit.c = 1;

        units.push_back(t_unit);
    }

    /*
    //                          0    1  2   3          4          5          6           7     8
    std::string const query = "SELECT 0,guid,id,map,position_x,position_y,position_z,orientation, (SELECT COUNT(*) FROM
    creature k WHERE c.id1 = k.id1) FROM creature c UNION ALL SELECT
    1,guid,id,map,position_x,position_y,position_z,orientation, (SELECT COUNT(*) FROM gameobject h WHERE h.id = g.id)
    FROM gameobject g";

    QueryResult result = WorldDatabase.Query(query.c_str());
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            t_unit.type = fields[0].Get<uint32>();
            t_unit.guid = fields[1].Get<uint32>();
            t_unit.entry = fields[2].Get<uint32>();
            t_unit.map = fields[3].Get<uint32>();
            t_unit.x = fields[4].Get<float>();
            t_unit.y = fields[5].Get<float>();
            t_unit.z = fields[6].Get<float>();
            t_unit.o = fields[7].Get<float>();
            t_unit.c = uint32(fields[8].Get<uint64>());

            units.push_back(t_unit);

        } while (result->NextRow());

        LOG_INFO("playerbots", ">> Loaded {} units locations.", units.size());
    }
    else
    {
        LOG_ERROR("playerbots", ">> Error loading units locations.");
    }

    query = "SELECT 0, 0, id, quest FROM creature_queststarter UNION ALL SELECT 0, 1, id, quest FROM creature_questender
    UNION ALL SELECT 1, 0, id, quest FROM gameobject_queststarter UNION ALL SELECT 1, 1, id, quest FROM
    gameobject_questender"; result = WorldDatabase.Query(query.c_str());

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            t_rel.type = fields[0].Get<uint32>();
            t_rel.role = fields[1].Get<uint32>();
            t_rel.entry = fields[2].Get<uint32>();
            t_rel.questId = fields[3].Get<uint32>();

            relations.push_back(t_rel);

        } while (result->NextRow());

        LOG_INFO("playerbots", ">> Loaded {} relations.", relations.size());
    }
    else
    {
        LOG_ERROR("playerbots", ">> Error loading relations.");
    }

    query = "SELECT 0, ct.entry, item FROM creature_template ct JOIN creature_loot_template clt ON (ct.lootid =
    clt.entry) UNION ALL SELECT 0, entry, item FROM npc_vendor UNION ALL SELECT 1, gt.entry, item FROM
    gameobject_template gt JOIN gameobject_loot_template glt ON (gt.TYPE = 3 AND gt.DATA1 = glt.entry)"; result =
    WorldDatabase.Query(query.c_str());

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            t_loot.type = fields[0].Get<uint32>();
            t_loot.entry = fields[1].Get<uint32>();
            t_loot.item = fields[2].Get<uint32>();

            loots.push_back(t_loot);

        } while (result->NextRow());

        LOG_INFO("playerbots", ">> Loaded {} loot lists.", loots.size());
    }
    else
    {
        LOG_ERROR("playerbots", ">> Error loading loot lists.");
    }
    */

    LOG_INFO("playerbots", "Loading quest data.");

    bool loadQuestData = true;
    if (loadQuestData)
    {
        questGuidpMap questMap = GAI_VALUE(questGuidpMap, "quest guidp map");

        for (auto& q : questMap)
        {
            uint32 questId = q.first;

            QuestContainer* container = new QuestContainer;

            for (auto& r : q.second)
            {
                uint32 flag = r.first;

                for (auto& e : r.second)
                {
                    int32 entry = e.first;

                    QuestTravelDestination* loc;
                    std::vector<QuestTravelDestination*> locs;

                    if (flag & (uint32)QuestRelationFlag::questGiver)
                    {
                        loc = new QuestRelationTravelDestination(
                            questId, entry, 0, sPlayerbotAIConfig->tooCloseDistance, sPlayerbotAIConfig->sightDistance);
                        loc->setExpireDelay(5 * 60 * 1000);
                        loc->setMaxVisitors(15, 0);
                        container->questGivers.push_back(loc);
                        locs.push_back(loc);
                    }
                    if (flag & (uint32)QuestRelationFlag::questTaker)
                    {
                        loc = new QuestRelationTravelDestination(
                            questId, entry, 1, sPlayerbotAIConfig->tooCloseDistance, sPlayerbotAIConfig->sightDistance);
                        loc->setExpireDelay(5 * 60 * 1000);
                        loc->setMaxVisitors(15, 0);
                        container->questTakers.push_back(loc);
                        locs.push_back(loc);
                    }
                    else
                    {
                        uint32 objective = 0;
                        if (flag & (uint32)QuestRelationFlag::objective1)
                            objective = 0;
                        else if (flag & (uint32)QuestRelationFlag::objective2)
                            objective = 1;
                        else if (flag & (uint32)QuestRelationFlag::objective3)
                            objective = 2;
                        else if (flag & (uint32)QuestRelationFlag::objective4)
                            objective = 3;

                        loc = new QuestObjectiveTravelDestination(questId, entry, objective,
                                                                  sPlayerbotAIConfig->tooCloseDistance,
                                                                  sPlayerbotAIConfig->sightDistance);
                        loc->setExpireDelay(1 * 60 * 1000);
                        loc->setMaxVisitors(100, 1);
                        container->questObjectives.push_back(loc);
                        locs.push_back(loc);
                    }

                    for (auto& guidP : e.second)
                    {
                        WorldPosition point = guidP;
                        for (auto tLoc : locs)
                        {
                            tLoc->addPoint(&point);
                        }
                    }
                }
            }

            if (!container->questTakers.empty())
            {
                quests.insert(std::make_pair(questId, container));

                for (auto loc : container->questGivers)
                    questGivers.push_back(loc);
            }
        }
    }

    /*
    if (loadQuestData && false)
    {
        for (auto& questId : questIds)
        {
            Quest* quest = questMap.find(questId)->second;

            QuestContainer* container = new QuestContainer;
            QuestTravelDestination* loc = nullptr;
            WorldPosition point;

            bool hasError = false;

            //Relations
            for (auto& r : relations)
            {
                if (questId != r.questId)
                    continue;

                int32 entry = r.type == 0 ? r.entry : r.entry * -1;

                loc = new QuestRelationTravelDestination(r.questId, entry, r.role, sPlayerbotAIConfig->tooCloseDistance,
    sPlayerbotAIConfig->sightDistance); loc->setExpireDelay(5 * 60 * 1000); loc->setMaxVisitors(15, 0);

                for (auto& u : units)
                {
                    if (r.type != u.type || r.entry != u.entry)
                        continue;

                    int32 guid = u.type == 0 ? u.guid : u.guid * -1;

                    point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
                    loc->addPoint(&point);
                }

                if (loc->getPoints(0).empty())
                {
                    logQuestError(1, quest, r.role, entry);
                    delete loc;
                    continue;
                }


                if (r.role == 0)
                {
                    container->questGivers.push_back(loc);
                }
                else
                    container->questTakers.push_back(loc);

            }

            //Mobs
            for (uint32 i = 0; i < 4; i++)
            {
                if (quest->RequiredNpcOrGoCount[i] == 0)
                    continue;

                uint32 reqEntry = quest->RequiredNpcOrGo[i];

                loc = new QuestObjectiveTravelDestination(questId, reqEntry, i, sPlayerbotAIConfig->tooCloseDistance,
    sPlayerbotAIConfig->sightDistance); loc->setExpireDelay(1 * 60 * 1000); loc->setMaxVisitors(100, 1);

                for (auto& u : units)
                {
                    int32 entry = u.type == 0 ? u.entry : u.entry * -1;

                    if (entry != reqEntry)
                        continue;

                    int32 guid = u.type == 0 ? u.guid : u.guid * -1;

                    point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
                    loc->addPoint(&point);
                }

                if (loc->getPoints(0).empty())
                {
                    logQuestError(2, quest, i, reqEntry);

                    delete loc;
                    hasError = true;
                    continue;
                }

                container->questObjectives.push_back(loc);
            }

            //Loot
            for (uint32 i = 0; i < 4; i++)
            {
                if (quest->RequiredItemCount[i] == 0)
                    continue;

                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(quest->RequiredItemId[i]);
                if (!proto)
                {
                    logQuestError(3, quest, i, 0, quest->RequiredItemId[i]);
                    hasError = true;
                    continue;
                }

                uint32 foundLoot = 0;

                for (auto& l : loots)
                {
                    if (l.item != quest->RequiredItemId[i])
                        continue;

                    int32 entry = l.type == 0 ? l.entry : l.entry * -1;

                    loc = new QuestObjectiveTravelDestination(questId, entry, i, sPlayerbotAIConfig->tooCloseDistance,
    sPlayerbotAIConfig->sightDistance, l.item); loc->setExpireDelay(1 * 60 * 1000); loc->setMaxVisitors(100, 1);

                    for (auto& u : units)
                    {
                        if (l.type != u.type || l.entry != u.entry)
                            continue;

                        int32 guid = u.type == 0 ? u.guid : u.guid * -1;

                        point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
                        loc->addPoint(&point);
                    }

                    if (loc->getPoints(0).empty())
                    {
                        logQuestError(4, quest, i, entry, quest->RequiredItemId[i]);
                        delete loc;
                        continue;
                    }

                    container->questObjectives.push_back(loc);

                    foundLoot++;
                }

                if (foundLoot == 0)
                {
                    hasError = true;
                    logQuestError(5, quest, i, 0, quest->RequiredItemId[i]);
                }
            }

            if (container->questTakers.empty())
                logQuestError(7, quest);

            if (!container->questGivers.empty() || !container->questTakers.empty() || hasError)
            {
                quests.insert(std::make_pair(questId, container));

                for (auto loc : container->questGivers)
                    questGivers.push_back(loc);
            }
        }

        LOG_INFO("playerbots", ">> Loaded {} quest details.", questIds.size());
    }
    */

    WorldPosition point;

    LOG_INFO("playerbots", "Loading Rpg, Grind and Boss locations.");

    // Rpg locations
    for (auto& u : units)
    {
        RpgTravelDestination* rLoc;
        GrindTravelDestination* gLoc;
        BossTravelDestination* bLoc;

        if (u.type != 0)
            continue;

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(u.entry);
        if (!cInfo)
            continue;

        std::vector<uint32> allowedNpcFlags;

        allowedNpcFlags.push_back(UNIT_NPC_FLAG_INNKEEPER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_GOSSIP);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_QUESTGIVER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_FLIGHTMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_BANKER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_AUCTIONEER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_STABLEMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_PETITIONER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TABARDDESIGNER);

        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_REPAIR);

        point = WorldPosition(u.map, u.x, u.y, u.z, u.o);

        for (std::vector<uint32>::iterator i = allowedNpcFlags.begin(); i != allowedNpcFlags.end(); ++i)
        {
            if ((cInfo->npcflag & *i) != 0)
            {
                rLoc = new RpgTravelDestination(u.entry, sPlayerbotAIConfig->tooCloseDistance,
                                                sPlayerbotAIConfig->sightDistance);
                rLoc->setExpireDelay(5 * 60 * 1000);
                rLoc->setMaxVisitors(15, 0);

                rLoc->addPoint(&point);
                rpgNpcs.push_back(rLoc);
                break;
            }
        }

        if (cInfo->mingold > 0)
        {
            gLoc = new GrindTravelDestination(u.entry, sPlayerbotAIConfig->tooCloseDistance,
                                              sPlayerbotAIConfig->sightDistance);
            gLoc->setExpireDelay(5 * 60 * 1000);
            gLoc->setMaxVisitors(100, 0);

            point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
            gLoc->addPoint(&point);
            grindMobs.push_back(gLoc);
        }

        if (cInfo->rank == 3 || (cInfo->rank == 1 && !point.isOverworld() && u.c == 1))
        {
            std::string const nodeName = cInfo->Name;

            bLoc = new BossTravelDestination(u.entry, sPlayerbotAIConfig->tooCloseDistance,
                                             sPlayerbotAIConfig->sightDistance);
            bLoc->setExpireDelay(5 * 60 * 1000);
            bLoc->setMaxVisitors(0, 0);

            bLoc->addPoint(&point);
            bossMobs.push_back(bLoc);
        }
    }

    LOG_INFO("playerbots", "Loading Explore locations.");

    // Explore points
    for (auto& u : units)
    {
        ExploreTravelDestination* loc;

        WorldPosition point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
        AreaTableEntry const* area = point.getArea();

        if (!area)
            continue;

        if (!area->exploreFlag)
            continue;

        if (u.type == 1)
            continue;

        auto iloc = exploreLocs.find(area->ID);

        if (iloc == exploreLocs.end())
        {
            loc = new ExploreTravelDestination(area->ID, sPlayerbotAIConfig->tooCloseDistance,
                                               sPlayerbotAIConfig->sightDistance);
            loc->setMaxVisitors(1000, 0);
            loc->setCooldownDelay(1000);
            loc->setExpireDelay(1000);
            loc->setTitle(area->area_name[0]);
            exploreLocs.insert_or_assign(area->ID, loc);
        }
        else
        {
            loc = iloc->second;
        }

        loc->addPoint(&point);
    }

    // Clear these logs files
    sPlayerbotAIConfig->openLog("zones.csv", "w");
    sPlayerbotAIConfig->openLog("creatures.csv", "w");
    sPlayerbotAIConfig->openLog("gos.csv", "w");
    sPlayerbotAIConfig->openLog("bot_movement.csv", "w");
    sPlayerbotAIConfig->openLog("bot_pathfinding.csv", "w");
    sPlayerbotAIConfig->openLog("pathfind_attempt.csv", "w");
    sPlayerbotAIConfig->openLog("pathfind_attempt_point.csv", "w");
    sPlayerbotAIConfig->openLog("pathfind_result.csv", "w");
    sPlayerbotAIConfig->openLog("load_map_grid.csv", "w");
    sPlayerbotAIConfig->openLog("strategy.csv", "w");

    sPlayerbotAIConfig->openLog("unload_grid.csv", "w");
    sPlayerbotAIConfig->openLog("unload_obj.csv", "w");

    sTravelNodeMap->loadNodeStore();

    sTravelNodeMap->generateAll();

    /*
    bool fullNavPointReload = false;
    bool storeNavPointReload = true;

   if (!fullNavPointReload && true)
        TravelNodeStore::loadNodes();

    //sTravelNodeMap->loadNodeStore();

    for (auto node : sTravelNodeMap->getNodes())
    {
        node->setLinked(true);
    }

    bool reloadNavigationPoints = false || fullNavPointReload || storeNavPointReload;

    if (reloadNavigationPoints)
    {
        LOG_INFO("playerbots", "Loading navigation points");

        //Npc nodes

        WorldPosition pos;

        for (auto& u : units)
        {
            if (u.type != 0)
                continue;

            CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(u.entry);
            if (!cInfo)
                continue;

            std::vector<uint32> allowedNpcFlags;

            allowedNpcFlags.push_back(UNIT_NPC_FLAG_INNKEEPER);
            allowedNpcFlags.push_back(UNIT_NPC_FLAG_FLIGHTMASTER);
            //allowedNpcFlags.push_back(UNIT_NPC_FLAG_QUESTGIVER);

            for (std::vector<uint32>::iterator i = allowedNpcFlags.begin(); i != allowedNpcFlags.end(); ++i)
            {
                if ((cInfo->npcflag & *i) != 0)
                {
                    pos = WorldPosition(u.map, u.x, u.y, u.z, u.o);

                    std::string const nodeName = pos.getAreaName(false);
                    if ((cInfo->npcflag & UNIT_NPC_FLAG_INNKEEPER) != 0)
                        nodeName += " innkeeper";
                    else
                        nodeName += " flightMaster";

                    sTravelNodeMap->addNode(&pos, nodeName, true, true);

                    break;
                }
            }
        }

        //Build flight paths
        for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
        {
            TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(i);

            if (!taxiPath)
                continue;

            TaxiNodesEntry const* startTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->from);
            if (!startTaxiNode)
                continue;

            TaxiNodesEntry const* endTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->to);
            if (!endTaxiNode)
                continue;

            TaxiPathNodeList const& nodes = sTaxiPathNodesByPath[taxiPath->ID];
            if (nodes.empty())
                continue;

            WorldPosition startPos(startTaxiNode->map_id, startTaxiNode->x, startTaxiNode->y, startTaxiNode->z);
            WorldPosition endPos(endTaxiNode->map_id, endTaxiNode->x, endTaxiNode->y, endTaxiNode->z);

            TravelNode* startNode = sTravelNodeMap->getNode(&startPos, nullptr, 15.0f);
            TravelNode* endNode = sTravelNodeMap->getNode(&endPos, nullptr, 15.0f);

            if (!startNode || !endNode)
                continue;

            std::vector<WorldPosition> ppath;

            for (auto& n : nodes)
                ppath.push_back(WorldPosition(n->mapid, n->x, n->y, n->z, 0.0));

            float totalTime = startPos.getPathLength(ppath) / (450 * 8.0f);

            TravelNodePath travelPath(0.1f, totalTime, (uint8) TravelNodePathType::flightPath, i, true);
            travelPath.setPath(ppath);

            startNode->setPathTo(endNode, travelPath);
        }

        //Unique bosses
        for (auto& u : units)
        {
            if (u.type != 0)
                continue;

            CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(u.entry);
            if (!cInfo)
                continue;

            pos = WorldPosition(u.map, u.x, u.y, u.z, u.o);

            if (cInfo->rank == 3 || (cInfo->rank == 1 && !pos.isOverworld() && u.c == 1))
            {
                std::string const nodeName = cInfo->Name;
                sTravelNodeMap->addNode(&pos, nodeName, true, true);
            }
        }

        std::map<uint8, std::string> startNames;
        startNames[RACE_HUMAN] = "Human";
        startNames[RACE_ORC] = "Orc and Troll";
        startNames[RACE_DWARF] = "Dwarf and Gnome";
        startNames[RACE_NIGHTELF] = "Night Elf";
        startNames[RACE_UNDEAD_PLAYER] = "Undead";
        startNames[RACE_TAUREN] = "Tauren";
        startNames[RACE_GNOME] = "Dwarf and Gnome";
        startNames[RACE_TROLL] = "Orc and Troll";
        startNames[RACE_DRAENEI] = "Draenei";
        startNames[RACE_BLOODELF] = "Blood Elf";

        for (uint32 i = 0; i < MAX_RACES; i++)
        {
            for (uint32 j = 0; j < MAX_CLASSES; j++)
            {
                PlayerInfo const* info = sObjectMgr->GetPlayerInfo(i, j);
                if (!info)
                    continue;

                pos = WorldPosition(info->mapId, info->positionX, info->positionY, info->positionZ, info->orientation);

                std::string const nodeName = startNames[i] + " start";
                sTravelNodeMap->addNode(&pos, nodeName, true, true);
            }
        }

        //Transports
        GameObjectTemplateContainer const* goTemplates = sObjectMgr->GetGameObjectTemplates();
        for (auto const& iter : *goTemplates)
        {
            GameObjectTemplate const* data = &iter.second;
            if (data && (data->type == GAMEOBJECT_TYPE_TRANSPORT || data->type == GAMEOBJECT_TYPE_MO_TRANSPORT))
            {
                TransportAnimation const* animation = sTransportMgr->GetTransportAnimInfo(iter.first);

                uint32 pathId = data->moTransport.taxiPathId;
                float moveSpeed = data->moTransport.moveSpeed;
                if (pathId >= sTaxiPathNodesByPath.size())
                    continue;

                TaxiPathNodeList const& path = sTaxiPathNodesByPath[pathId];

                std::vector<WorldPosition> ppath;
                TravelNode* prevNode = nullptr;

                //Elevators/Trams
                if (path.empty())
                {
                    if (animation)
                    {
                        TransportPathContainer aPath = animation->Path;
                        float timeStart;

                        for (auto& u : units)
                        {
                            if (u.type != 1)
                                continue;

                            if (u.entry != iter.first)
                                continue;

                            prevNode = nullptr;
                            WorldPosition lPos = WorldPosition(u.map, 0, 0, 0, 0);

                            for (auto& p : aPath)
                            {
                                float dx = cos(u.o) * p.second->X - sin(u.o) * p.second->Y;
                                float dy = sin(u.o) * p.second->X + cos(u.o) * p.second->Y;
                                WorldPosition pos = WorldPosition(u.map, u.x + dx, u.y + dy, u.z + p.second->Z, u.o);

                                if (prevNode)
                                {
                                    ppath.push_back(pos);
                                }

                                if (pos.distance(&lPos) == 0)
                                {
                                    TravelNode* node = sTravelNodeMap->addNode(&pos, data->name, true, true, true,
   iter.first);

                                    if (!prevNode)
                                    {
                                        ppath.push_back(pos);
                                        timeStart = p.second->TimeSeg;
                                    }
                                    else
                                    {
                                        float totalTime = (p.second->TimeSeg - timeStart) / 1000.0f;
                                        TravelNodePath travelPath(0.1f, totalTime, (uint8)
   TravelNodePathType::transport, entry, true); node->setPathTo(prevNode, travelPath); ppath.clear();
                                        ppath.push_back(pos);
                                        timeStart = p.second->TimeSeg;
                                    }

                                    prevNode = node;
                                }

                                lPos = pos;
                            }

                            if (prevNode)
                            {
                                for (auto& p : aPath)
                                {
                                    float dx = cos(u.o) * p.second->X - sin(u.o) * p.second->Y;
                                    float dy = sin(u.o) * p.second->X + cos(u.o) * p.second->Y;
                                    WorldPosition pos = WorldPosition(u.map, u.x + dx, u.y + dy, u.z + p.second->Z,
   u.o);

                                    ppath.push_back(pos);

                                    if (pos.distance(&lPos) == 0)
                                    {
                                        TravelNode* node = sTravelNodeMap->addNode(&pos, data->name, true, true, true,
   iter.first); if (node != prevNode)
                                        {
                                            float totalTime = (p.second->TimeSeg - timeStart) / 1000.0f;
                                            TravelNodePath travelPath(0.1f, totalTime, (uint8)
   TravelNodePathType::transport, entry, true); travelPath.setPath(ppath); node->setPathTo(prevNode, travelPath);
                                            ppath.clear();
                                            ppath.push_back(pos);
                                            timeStart = p.second->TimeSeg;
                                        }
                                    }

                                    lPos = pos;
                                }
                            }

                            ppath.clear();
                        }
                    }
                }
                else //Boats/Zepelins
                {
                    //Loop over the path and connect stop locations.
                    for (auto& p : path)
                    {
                        WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);

                        //if (data->displayId == 3015)
                        //    pos.setZ(pos.getZ() + 6.0f);
                        //else if(data->displayId == 3031)
                       //     pos.setZ(pos.getZ() - 17.0f);

                        if (prevNode)
                        {
                            ppath.push_back(pos);
                        }

                        if (p->delay > 0)
                        {
                            TravelNode* node = sTravelNodeMap->addNode(&pos, data->name, true, true, true, iter.first);

                            if (!prevNode)
                            {
                                ppath.push_back(pos);
                            }
                            else
                            {
                                TravelNodePath travelPath(0.1f, 0.0, (uint8) TravelNodePathType::transport, entry,
   true); travelPath.setPathAndCost(ppath, moveSpeed); node->setPathTo(prevNode, travelPath); ppath.clear();
                                ppath.push_back(pos);
                            }

                            prevNode = node;
                        }
                    }

                    if (prevNode)
                    {
                        //Continue from start until first stop and connect to end.
                        for (auto& p : path)
                        {
                            WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);

                            //if (data->displayId == 3015)
                            //    pos.setZ(pos.getZ() + 6.0f);
                            //else if (data->displayId == 3031)
                            //    pos.setZ(pos.getZ() - 17.0f);

                            ppath.push_back(pos);

                            if (p->delay > 0)
                            {
                                TravelNode* node = sTravelNodeMap->getNode(&pos, nullptr, 5.0f);
                                if (node != prevNode)
                                {
                                    TravelNodePath travelPath(0.1f, 0.0, (uint8) TravelNodePathType::transport, entry,
   true); travelPath.setPathAndCost(ppath, moveSpeed); node->setPathTo(prevNode, travelPath);
                                }
                            }
                        }
                    }

                    ppath.clear();
                }
            }
        }

        //Zone means
        for (auto& loc : exploreLocs)
        {
            std::vector<WorldPosition*> points;

            for (auto p : loc.second->getPoints(true))
                if (!p->isUnderWater())
                    points.push_back(p);

            if (points.empty())
                points = loc.second->getPoints(true);

            WorldPosition  pos = WorldPosition(points, WP_MEAN_CENTROID);

            TravelNode* node = sTravelNodeMap->addNode(&pos, pos.getAreaName(), true, true, false);
        }

        LOG_INFO("playerbots", ">> Loaded {} navigation points.", sTravelNodeMap->getNodes().size());
    }

    sTravelNodeMap->calcMapOffset();
    loadMapTransfers();
    */

    /*
    bool preloadNodePaths = false || fullNavPointReload || storeNavPointReload;             //Calculate paths using
    PathGenerator. bool preloadReLinkFullyLinked = false || fullNavPointReload || storeNavPointReload;      //Retry
    nodes that are fully linked. bool preloadUnlinkedPaths = false || fullNavPointReload;        //Try to connect points
    currently unlinked. bool preloadWorldPaths = true;            //Try to load paths in overworld. bool
    preloadInstancePaths = true;         //Try to load paths in instances. bool preloadSubPrint = false; //Print output
    every 2%.

    if (preloadNodePaths)
    {
        std::unordered_map<uint32, Map*> instances;

        //PathGenerator
        std::vector<WorldPosition> ppath;

        uint32 cur = 0, max = sTravelNodeMap->getNodes().size();

        for (auto& startNode : sTravelNodeMap->getNodes())
        {
            if (!preloadReLinkFullyLinked && startNode->isLinked())
                continue;

            for (auto& endNode : sTravelNodeMap->getNodes())
            {
                if (startNode == endNode)
                    continue;

                if (startNode->getPosition()->isOverworld() && !preloadWorldPaths)
                    continue;

                if (!startNode->getPosition()->isOverworld() && !preloadInstancePaths)
                    continue;

                if (startNode->hasCompletePathTo(endNode))
                    continue;

                if (!preloadUnlinkedPaths && !startNode->hasLinkTo(endNode))
                    continue;

                if (startNode->getMapId() != endNode->getMapId())
                    continue;

                //if (preloadUnlinkedPaths && !startNode->hasLinkTo(endNode) && startNode->isUselessLink(endNode))
                //    continue;

                startNode->buildPath(endNode, nullptr, false);

                //if (startNode->hasLinkTo(endNode) && !startNode->getPathTo(endNode)->getComplete())
                    //startNode->removeLinkTo(endNode);
            }

            startNode->setLinked(true);

            cur++;

            if (preloadSubPrint && (cur * 50) / max > ((cur - 1) * 50) / max)
            {
                sTravelNodeMap->printMap();
                sTravelNodeMap->printNodeStore();
            }
        }

        if (!preloadSubPrint)
        {
            sTravelNodeMap->printNodeStore();
            sTravelNodeMap->printMap();
        }

        LOG_INFO("playerbots", ">> Loaded paths for {} nodes.", sTravelNodeMap->getNodes().size());
    }

    bool removeLowLinkNodes = false || fullNavPointReload || storeNavPointReload;

    if (removeLowLinkNodes)
    {
        std::vector<TravelNode*> goodNodes;
        std::vector<TravelNode*> remNodes;
        for (auto& node : sTravelNodeMap->getNodes())
        {
            if (!node->getPosition()->isOverworld())
                continue;

            if (std::find(goodNodes.begin(), goodNodes.end(), node) != goodNodes.end())
                continue;

            if (std::find(remNodes.begin(), remNodes.end(), node) != remNodes.end())
                continue;

            std::vector<TravelNode*> nodes = node->getNodeMap(true);

            if (nodes.size() < 5)
                remNodes.insert(remNodes.end(), nodes.begin(), nodes.end());
            else
                goodNodes.insert(goodNodes.end(), nodes.begin(), nodes.end());
        }

        for (auto& node : remNodes)
            sTravelNodeMap->removeNode(node);

        LOG_INFO("playerbots", ">> Checked {} nodes.", sTravelNodeMap->getNodes().size());
    }

    bool cleanUpNodeLinks = false || fullNavPointReload || storeNavPointReload;
    bool cleanUpSubPrint = false;             //Print output every 2%.

    if (cleanUpNodeLinks)
    {
        //Routes
        uint32 cur = 0;
        uint32 max = sTravelNodeMap->getNodes().size();

        //Clean up node links
        for (auto& startNode : sTravelNodeMap->getNodes())
        {
             startNode->cropUselessLinks();

             cur++;
             if (cleanUpSubPrint && (cur * 10) / max > ((cur - 1) * 10) / max)
             {
                 sTravelNodeMap->printMap();
                 sTravelNodeMap->printNodeStore();
             }
        }

        LOG_INFO("playerbots", ">> Cleaned paths for {} nodes.", sTravelNodeMap->getNodes().size());
    }

    bool reCalculateCost = false || fullNavPointReload || storeNavPointReload;
    bool forceReCalculate = false;

    if (reCalculateCost)
    {
        for (auto& startNode : sTravelNodeMap->getNodes())
        {
            for (auto& path : *startNode->getLinks())
            {
                TravelNodePath* nodePath = path.second;

                if (path.second->getPathType() != TravelNodePathType::walk)
                    continue;

                if (nodePath->getCalculated() && !forceReCalculate)
                    continue;

                nodePath->calculateCost();
            }
        }

        LOG_INFO("playerbots", ">> Calculated pathcost for {} nodes.", sTravelNodeMap->getNodes().size());
    }

    bool mirrorMissingPaths = true || fullNavPointReload || storeNavPointReload;

    if (mirrorMissingPaths)
    {
        for (auto& startNode : sTravelNodeMap->getNodes())
        {
            for (auto& path : *startNode->getLinks())
            {
                TravelNode* endNode = path.first;

                if (endNode->hasLinkTo(startNode))
                    continue;

                if (path.second->getPathType() != TravelNodePathType::walk)
                    continue;

                TravelNodePath nodePath = *path.second;

                std::vector<WorldPosition> pPath = nodePath.getPath();
                std::reverse(pPath.begin(), pPath.end());

                nodePath.setPath(pPath);

                endNode->setPathTo(startNode, nodePath, true);
            }
        }

        LOG_INFO("playerbots", ">> Reversed missing paths for {} nodes.", sTravelNodeMap->getNodes().size());
    }
    */

    sTravelNodeMap->printMap();
    sTravelNodeMap->printNodeStore();
    sTravelNodeMap->saveNodeStore();

    // Creature/gos/zone export.
    if (sPlayerbotAIConfig->hasLog("creatures.csv"))
    {
        for (CreatureData const* cData : WorldPosition().getCreaturesNear())
        {
            CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(cData->id1);
            if (!cInfo)
                continue;

            WorldPosition point =
                WorldPosition(cData->mapid, cData->posX, cData->posY, cData->posZ, cData->orientation);

            std::string name = cInfo->Name;
            name.erase(remove(name.begin(), name.end(), ','), name.end());
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());

            std::ostringstream out;
            out << name << ",";
            point.printWKT(out);
            out << cInfo->maxlevel << ",";
            out << cInfo->rank << ",";
            out << cInfo->faction << ",";
            out << cInfo->npcflag << ",";
            out << point.getAreaName() << ",";
            out << std::fixed;

            sPlayerbotAIConfig->log("creatures.csv", out.str().c_str());
        }
    }

    if (sPlayerbotAIConfig->hasLog("vmangoslines.csv"))
    {
        uint32 mapId = 0;
        std::vector<WorldPosition> pos;

        static float const topNorthSouthLimit[] = {
            2032.048340f,  -6927.750000f, 1634.863403f,  -6157.505371f, 1109.519775f,  -5181.036133f, 1315.204712f,
            -4096.020508f, 1073.089233f,  -3372.571533f, 825.8331910f,  -3125.778809f, 657.3439940f,  -2314.813232f,
            424.7361450f,  -1888.283691f, 744.3958130f,  -1647.935425f, 1424.160645f,  -654.9481810f, 1447.065308f,
            -169.7513580f, 1208.715454f,  189.74870300f, 1596.240356f,  998.61669900f, 1577.923706f,  1293.4199220f,
            1458.520264f,  1727.3732910f, 1591.916138f,  3728.1394040f};

        pos.clear();

#define my_sizeof(type) ((char*)(&type + 1) - (char*)(&type))

        uint32 size = my_sizeof(topNorthSouthLimit) / my_sizeof(topNorthSouthLimit[0]);
        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (topNorthSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, topNorthSouthLimit[i], topNorthSouthLimit[i + 1], 0));
        }

        std::ostringstream out;
        out << "topNorthSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const ironforgeAreaSouthLimit[] = {
            -7491.33f, 3093.740f, -7472.04f, -391.880f, -6366.68f, -730.100f, -6063.96f, -1411.76f,
            -6087.62f, -2190.21f, -6349.54f, -2533.66f, -6308.63f, -3049.32f, -6107.82f, -3345.30f,
            -6008.49f, -3590.52f, -5989.37f, -4312.29f, -5806.26f, -5864.11f};

        pos.clear();

        size = my_sizeof(ironforgeAreaSouthLimit) / my_sizeof(ironforgeAreaSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (ironforgeAreaSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, ironforgeAreaSouthLimit[i], ironforgeAreaSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "ironforgeAreaSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const stormwindAreaNorthLimit[] = {
            -8004.250f, 3714.110f, -8075.000f, -179.000f, -8638.000f, 169.0000f, -9044.000f, 35.00000f,
            -9068.000f, -125.000f, -9094.000f, -147.000f, -9206.000f, -290.000f, -9097.000f, -510.000f,
            -8739.000f, -501.000f, -8725.500f, -1618.45f, -9810.400f, -1698.41f, -10049.60f, -1740.40f,
            -10670.61f, -1692.51f, -10908.48f, -1563.87f, -13006.40f, -1622.80f, -12863.23f, -4798.42f};

        pos.clear();

        size = my_sizeof(stormwindAreaNorthLimit) / my_sizeof(stormwindAreaNorthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (stormwindAreaNorthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, stormwindAreaNorthLimit[i], stormwindAreaNorthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "stormwindAreaNorthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const stormwindAreaSouthLimit[] = {
            -8725.3378910f, 3535.62402300f, -9525.6992190f, 910.13256800f, -9796.9531250f, 839.06958000f,
            -9946.3417970f, 743.10284400f,  -10287.361328f, 760.07647700f, -10083.828125f, 380.38989300f,
            -10148.072266f, 80.056450000f,  -10014.583984f, -161.6385190f, -9978.1464840f, -361.6380310f,
            -9877.4892580f, -563.3048710f,  -9980.9677730f, -1128.510498f, -9991.7177730f, -1428.793213f,
            -9887.5791020f, -1618.514038f,  -10169.600586f, -1801.582031f, -9966.2744140f, -2227.197754f,
            -9861.3095700f, -2989.841064f,  -9944.0263670f, -3205.886963f, -9610.2099610f, -3648.369385f,
            -7949.3295900f, -4081.389404f,  -7910.8593750f, -5855.578125f};

        pos.clear();

        size = my_sizeof(stormwindAreaSouthLimit) / my_sizeof(stormwindAreaSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (stormwindAreaSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, stormwindAreaSouthLimit[i], stormwindAreaSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "stormwindAreaSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        mapId = 1;

        static float const northMiddleLimit[] = {
            -2280.00f, 4054.000f, -2401.00f, 2365.000f, -2432.00f, 1338.000f, -2286.00f, 769.0000f, -2137.00f,
            662.0000f, -2044.54f, 489.8600f, -1808.52f, 436.3900f, -1754.85f, 504.5500f, -1094.55f, 651.7500f,
            -747.460f, 647.7300f, -685.550f, 408.4300f, -311.380f, 114.4300f, -358.400f, -587.420f, -377.920f,
            -748.700f, -512.570f, -919.490f, -280.650f, -1008.87f, -81.2900f, -930.890f, 284.3100f, -1105.39f,
            568.8600f, -892.280f, 1211.090f, -1135.55f, 879.6000f, -2110.18f, 788.9600f, -2276.02f, 899.6800f,
            -2625.56f, 1281.540f, -2689.42f, 1521.820f, -3047.85f, 1424.220f, -3365.69f, 1694.110f, -3615.20f,
            2373.780f, -4019.96f, 2388.130f, -5124.35f, 2193.790f, -5484.38f, 1703.570f, -5510.53f, 1497.590f,
            -6376.56f, 1368.000f, -8530.00f};

        pos.clear();

        size = my_sizeof(northMiddleLimit) / my_sizeof(northMiddleLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (northMiddleLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, northMiddleLimit[i], northMiddleLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "northMiddleLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const durotarSouthLimit[] = {
            2755.0f, -3766.f, 2225.0f, -3596.f, 1762.0f, -3746.f, 1564.0f, -3943.f, 1184.0f, -3915.f, 737.00f,
            -3782.f, -75.00f, -3742.f, -263.0f, -3836.f, -173.0f, -4064.f, -81.00f, -4091.f, -49.00f, -4089.f,
            -16.00f, -4187.f, -5.000f, -4192.f, -14.00f, -4551.f, -397.0f, -4601.f, -522.0f, -4583.f, -668.0f,
            -4539.f, -790.0f, -4502.f, -1176.f, -4213.f, -1387.f, -4674.f, -2243.f, -6046.f};

        pos.clear();

        size = my_sizeof(durotarSouthLimit) / my_sizeof(durotarSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (durotarSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, durotarSouthLimit[i], durotarSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "durotarSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const valleyoftrialsSouthLimit[] = {-324.f,  -3869.f, -774.f,  -3992.f, -965.f,  -4290.f, -932.f,
                                                         -4349.f, -828.f,  -4414.f, -661.f,  -4541.f, -521.f,  -4582.f};

        pos.clear();

        size = my_sizeof(valleyoftrialsSouthLimit) / my_sizeof(valleyoftrialsSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (valleyoftrialsSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, valleyoftrialsSouthLimit[i], valleyoftrialsSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "valleyoftrialsSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const middleToSouthLimit[] = {
            -2402.010000f, 4255.7000000f, -2475.933105f, 3199.5683590f,  // Desolace
            -2344.124023f, 1756.1643070f, -2826.438965f, 403.82473800f,  // Mulgore
            -3472.819580f, 182.52247600f,                                // Feralas
            -4365.006836f, -1602.575439f,                                // the Barrens
            -4515.219727f, -1681.356079f, -4543.093750f, -1882.869385f,  // Thousand Needles
            -4824.160000f, -2310.110000f, -5102.913574f, -2647.062744f, -5248.286621f,
            -3034.536377f, -5246.920898f, -3339.139893f, -5459.449707f, -4920.155273f,  // Tanaris
            -5437.000000f, -5863.000000f};

        pos.clear();

        size = my_sizeof(middleToSouthLimit) / my_sizeof(middleToSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (middleToSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, middleToSouthLimit[i], middleToSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "middleToSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const orgrimmarSouthLimit[] = {
            2132.5076f, -3912.2478f, 1944.4298f, -3855.2583f, 1735.6906f, -3834.2417f, 1654.3671f, -3380.9902f,
            1593.9861f, -3975.5413f, 1439.2548f, -4249.6923f, 1436.3106f, -4007.8950f, 1393.3199f, -4196.0625f,
            1445.2428f, -4373.9052f, 1407.2349f, -4429.4145f, 1464.7142f, -4545.2875f, 1584.1331f, -4596.8764f,
            1716.8065f, -4601.1323f, 1875.8312f, -4788.7187f, 1979.7647f, -4883.4585f, 2219.1562f, -4854.3330f};

        pos.clear();

        size = my_sizeof(orgrimmarSouthLimit) / my_sizeof(orgrimmarSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (orgrimmarSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, orgrimmarSouthLimit[i], orgrimmarSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "orgrimmarSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const feralasThousandNeedlesSouthLimit[] = {
            -6495.4995f, -4711.9810f, -6674.9995f, -4515.0019f, -6769.5717f, -4122.4272f, -6838.2651f, -3874.2792f,
            -6851.1314f, -3659.1179f, -6624.6845f, -3063.3843f, -6416.9067f, -2570.1301f, -5959.8466f, -2287.2634f,
            -5947.9135f, -1866.5028f, -5947.9135f, -820.48810f, -5876.7114f, -3.5138000f, -5876.7114f, 917.640700f,
            -6099.3603f, 1153.28840f, -6021.8989f, 1638.18090f, -6091.6176f, 2335.88920f, -6744.9946f, 2393.48550f,
            -6973.8608f, 3077.02810f, -7068.7241f, 4376.23040f, -7142.1211f, 4808.43310f};

        pos.clear();

        size = my_sizeof(feralasThousandNeedlesSouthLimit) / my_sizeof(feralasThousandNeedlesSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (feralasThousandNeedlesSouthLimit[i] == 0)
                break;

            pos.push_back(
                WorldPosition(mapId, feralasThousandNeedlesSouthLimit[i], feralasThousandNeedlesSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "feralasThousandNeedlesSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());
    }

    if (sPlayerbotAIConfig->hasLog("gos.csv"))
    {
        for (GameObjectData const* gData : WorldPosition().getGameObjectsNear())
        {
            GameObjectTemplate const* data = sObjectMgr->GetGameObjectTemplate(gData->id);
            if (!data)
                continue;

            WorldPosition point =
                WorldPosition(gData->mapid, gData->posX, gData->posY, gData->posZ, gData->orientation);

            std::string name = data->name;
            name.erase(remove(name.begin(), name.end(), ','), name.end());
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());

            std::ostringstream out;
            out << name << ",";
            point.printWKT(out);
            out << data->type << ",";
            out << point.getAreaName() << ",";
            out << std::fixed;

            sPlayerbotAIConfig->log("gos.csv", out.str().c_str());
        }
    }

    if (sPlayerbotAIConfig->hasLog("zones.csv"))
    {
        std::unordered_map<std::string, std::vector<WorldPosition>> zoneLocs;

        std::vector<WorldPosition> Locs;

        for (auto& u : units)
        {
            WorldPosition point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
            std::string const name = std::to_string(u.map) + point.getAreaName();

            if (zoneLocs.find(name) == zoneLocs.end())
                zoneLocs.insert_or_assign(name, Locs);

            zoneLocs.find(name)->second.push_back(point);
        }

        for (auto& loc : zoneLocs)
        {
            if (loc.second.empty())
                continue;

            if (!sTravelNodeMap->getMapOffset(loc.second.front().getMapId()) && loc.second.front().getMapId() != 0)
                continue;

            std::vector<WorldPosition> points = loc.second;
            ;

            std::ostringstream out;

            WorldPosition pos = WorldPosition(points, WP_MEAN_CENTROID);

            out << "\"center\""
                << ",";
            out << points.begin()->getMapId() << ",";
            out << points.begin()->getAreaName() << ",";
            out << points.begin()->getAreaName(true, true) << ",";

            pos.printWKT(out);

            out << "\n";

            out << "\"area\""
                << ",";
            out << points.begin()->getMapId() << ",";
            out << points.begin()->getAreaName() << ",";
            out << points.begin()->getAreaName(true, true) << ",";

            point.printWKT(points, out, 0);

            sPlayerbotAIConfig->log("zones.csv", out.str().c_str());
        }
    }

    bool printStrategyMap = false;

    if (printStrategyMap && sPlayerbotAIConfig->hasLog("strategy.csv"))
    {
        static std::map<uint8, std::string> classes;
        static std::map<uint8, std::map<uint8, std::string>> specs;
        classes[CLASS_DRUID] = "druid";
        specs[CLASS_DRUID][0] = "balance";
        specs[CLASS_DRUID][1] = "feral combat";
        specs[CLASS_DRUID][2] = "restoration";

        classes[CLASS_HUNTER] = "hunter";
        specs[CLASS_HUNTER][0] = "beast mastery";
        specs[CLASS_HUNTER][1] = "marksmanship";
        specs[CLASS_HUNTER][2] = "survival";

        classes[CLASS_MAGE] = "mage";
        specs[CLASS_MAGE][0] = "arcane";
        specs[CLASS_MAGE][1] = "fire";
        specs[CLASS_MAGE][2] = "frost";

        classes[CLASS_PALADIN] = "paladin";
        specs[CLASS_PALADIN][0] = "holy";
        specs[CLASS_PALADIN][1] = "protection";
        specs[CLASS_PALADIN][2] = "retribution";

        classes[CLASS_PRIEST] = "priest";
        specs[CLASS_PRIEST][0] = "discipline";
        specs[CLASS_PRIEST][1] = "holy";
        specs[CLASS_PRIEST][2] = "shadow";

        classes[CLASS_ROGUE] = "rogue";
        specs[CLASS_ROGUE][0] = "assasination";
        specs[CLASS_ROGUE][1] = "combat";
        specs[CLASS_ROGUE][2] = "subtlety";

        classes[CLASS_SHAMAN] = "shaman";
        specs[CLASS_SHAMAN][0] = "elemental";
        specs[CLASS_SHAMAN][1] = "enhancement";
        specs[CLASS_SHAMAN][2] = "restoration";

        classes[CLASS_WARLOCK] = "warlock";
        specs[CLASS_WARLOCK][0] = "affliction";
        specs[CLASS_WARLOCK][1] = "demonology";
        specs[CLASS_WARLOCK][2] = "destruction";

        classes[CLASS_WARRIOR] = "warrior";
        specs[CLASS_WARRIOR][0] = "arms";
        specs[CLASS_WARRIOR][1] = "fury";
        specs[CLASS_WARRIOR][2] = "protection";

        classes[CLASS_DEATH_KNIGHT] = "dk";
        specs[CLASS_DEATH_KNIGHT][0] = "blood";
        specs[CLASS_DEATH_KNIGHT][1] = "frost";
        specs[CLASS_DEATH_KNIGHT][2] = "unholy";

        // Use randombot 0.
        std::ostringstream cout;
        cout << sPlayerbotAIConfig->randomBotAccountPrefix << 0;
        std::string const accountName = cout.str();

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT_ID_BY_USERNAME);
        stmt->SetData(0, accountName);
        PreparedQueryResult result = LoginDatabase.Query(stmt);
        if (result)
        {
            Field* fields = result->Fetch();
            uint32 accountId = fields[0].Get<uint32>();

            WorldSession* session =
                new WorldSession(accountId, "", nullptr, SEC_PLAYER, EXPANSION_WRATH_OF_THE_LICH_KING, time_t(0),
                                 LOCALE_enUS, 0, false, false, 0, true);

            std::vector<std::pair<std::pair<uint32, uint32>, uint32>> classSpecLevel;

            std::unordered_map<std::string, std::vector<std::pair<std::pair<uint32, uint32>, uint32>>> actions;

            std::ostringstream out;

            for (uint8 race = RACE_HUMAN; race < MAX_RACES; race++)
            {
                for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
                {
                    if (cls != 10)
                    {
                        std::unique_ptr<CharacterCreateInfo> characterInfo =
                            std::make_unique<CharacterCreateInfo>("dummy", race, cls, 1, 1, 1, 1, 1, 1);
                        Player* player = new Player(session);
                        if (player->Create(sObjectMgr->GetGenerator<HighGuid::Player>().Generate(),
                                           characterInfo.get()))
                        {
                            for (uint8 tab = 0; tab < 3; tab++)
                            {
                                TalentSpec newSpec;
                                if (tab == 0)
                                    newSpec = TalentSpec(player, "1-0-0");
                                else if (tab == 1)
                                    newSpec = TalentSpec(player, "0-1-0");
                                else
                                    newSpec = TalentSpec(player, "0-0-1");

                                for (uint32 lvl = 1; lvl < MAX_LEVEL; lvl++)
                                {
                                    player->SetLevel(lvl);

                                    std::ostringstream tout;
                                    newSpec.ApplyTalents(player, &tout);

                                    PlayerbotAI* botAI = new PlayerbotAI(player);

                                    botAI->ResetStrategies(false);

                                    AiObjectContext* con = botAI->GetAiObjectContext();

                                    std::vector<std::string> tstrats;
                                    std::set<std::string> strategies;
                                    std::set<std::string> sstrats;

                                    tstrats = botAI->GetStrategies(BOT_STATE_COMBAT);
                                    sstrats = con->GetSupportedStrategies();
                                    if (!sstrats.empty())
                                        strategies.insert(tstrats.begin(), tstrats.end());

                                    tstrats = botAI->GetStrategies(BOT_STATE_NON_COMBAT);
                                    if (!tstrats.empty())
                                        strategies.insert(tstrats.begin(), tstrats.end());

                                    tstrats = botAI->GetStrategies(BOT_STATE_DEAD);
                                    if (!tstrats.empty())
                                        strategies.insert(tstrats.begin(), tstrats.end());

                                    sstrats = con->GetSupportedStrategies();
                                    if (!sstrats.empty())
                                        strategies.insert(sstrats.begin(), sstrats.end());

                                    for (auto& stratName : strategies)
                                    {
                                        Strategy* strat = con->GetStrategy(stratName);

                                        if (strat->getDefaultActions())
                                            for (uint32 i = 0; i < NextAction::size(strat->getDefaultActions()); i++)
                                            {
                                                NextAction* nextAction = strat->getDefaultActions()[i];

                                                std::ostringstream aout;
                                                aout << nextAction->getRelevance() << "," << nextAction->getName()
                                                     << ",,S:" << stratName;

                                                if (actions.find(aout.str().c_str()) != actions.end())
                                                    classSpecLevel = actions.find(aout.str().c_str())->second;
                                                else
                                                    classSpecLevel.clear();

                                                classSpecLevel.push_back(std::make_pair(std::make_pair(cls, tab), lvl));

                                                actions.insert_or_assign(aout.str().c_str(), classSpecLevel);
                                            }

                                        std::vector<TriggerNode*> triggers;
                                        strat->InitTriggers(triggers);
                                        for (auto& triggerNode : triggers)
                                        {
                                            // out << " TN:" << triggerNode->getName();

                                            if (Trigger* trigger = con->GetTrigger(triggerNode->getName()))
                                            {
                                                triggerNode->setTrigger(trigger);

                                                NextAction** nextActions = triggerNode->getHandlers();

                                                for (uint32 i = 0; i < NextAction::size(nextActions); i++)
                                                {
                                                    NextAction* nextAction = nextActions[i];
                                                    // out << " A:" << nextAction->getName() << "(" <<
                                                    // nextAction->getRelevance() << ")";

                                                    std::ostringstream aout;
                                                    aout << nextAction->getRelevance() << "," << nextAction->getName()
                                                         << "," << triggerNode->getName() << "," << stratName;

                                                    if (actions.find(aout.str().c_str()) != actions.end())
                                                        classSpecLevel = actions.find(aout.str().c_str())->second;
                                                    else
                                                        classSpecLevel.clear();

                                                    classSpecLevel.push_back(
                                                        std::make_pair(std::make_pair(cls, tab), lvl));

                                                    actions.insert_or_assign(aout.str().c_str(), classSpecLevel);
                                                }
                                            }
                                        }
                                    }

                                    delete botAI;
                                }
                            }
                        }

                        delete player;
                    }
                }
            }

            std::vector<std::string> actionKeys;

            for (auto& action : actions)
                actionKeys.push_back(action.first);

            std::sort(actionKeys.begin(), actionKeys.end(),
                      [](std::string const i, std::string const j)
                      {
                          std::stringstream is(i);
                          std::stringstream js(j);
                          float iref, jref;
                          std::string iact, jact, itrig, jtrig, istrat, jstrat;
                          is >> iref >> iact >> itrig >> istrat;
                          js >> jref >> jact >> jtrig >> jstrat;

                          if (iref > jref)
                              return true;

                          if (iref == jref && istrat < jstrat)
                              return true;

                          if (iref == jref && !(istrat > jstrat) && iact < jact)
                              return true;

                          if (iref == jref && !(istrat > jstrat) && !(iact > jact) && itrig < jtrig)
                              return true;

                          return false;
                      });

            sPlayerbotAIConfig->log("strategy.csv", "relevance, action, trigger, strategy, classes");

            for (auto& actionkey : actionKeys)
            {
                if (actions.find(actionkey)->second.size() != (MAX_LEVEL - 1) * (MAX_CLASSES - 1))
                {
                    classSpecLevel = actions.find(actionkey)->second;

                    std::vector<std::pair<std::pair<uint32, uint32>, std::pair<uint32, uint32>>> classs;

                    for (auto cl : classSpecLevel)
                    {
                        uint32 minLevel = MAX_LEVEL;
                        uint32 maxLevel = 0;

                        uint32 cls = cl.first.first;
                        uint32 tb = cl.first.second;

                        if (std::find_if(classs.begin(), classs.end(),
                                         [cls, tb](std::pair<std::pair<uint32, uint32>, std::pair<uint32, uint32>> i)
                                         { return i.first.first == cls && i.first.second == tb; }) == classs.end())
                        {
                            for (auto cll : classSpecLevel)
                            {
                                if (cll.first.first == cl.first.first && cll.first.second == cl.first.second)
                                {
                                    minLevel = std::min(minLevel, cll.second);
                                    maxLevel = std::max(maxLevel, cll.second);
                                }
                            }

                            classs.push_back(std::make_pair(cl.first, std::make_pair(minLevel, maxLevel)));
                        }
                    }

                    out << actionkey;

                    if (classs.size() != 9 * 3)
                    {
                        out << ",";

                        for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
                        {
                            bool a[3] = {false, false, false};
                            uint32 min[3] = {0, 0, 0};
                            uint32 max[3] = {0, 0, 0};

                            if (std::find_if(classs.begin(), classs.end(),
                                             [cls](std::pair<std::pair<uint32, uint32>, std::pair<uint32, uint32>> i)
                                             { return i.first.first == cls; }) == classs.end())
                                continue;

                            for (uint32 tb = 0; tb < 3; tb++)
                            {
                                auto tcl = std::find_if(
                                    classs.begin(), classs.end(),
                                    [cls, tb](std::pair<std::pair<uint32, uint32>, std::pair<uint32, uint32>> i)
                                    { return i.first.first == cls && i.first.second == tb; });

                                if (tcl == classs.end())
                                    continue;

                                a[tb] = true;
                                min[tb] = tcl->second.first;
                                max[tb] = tcl->second.second;
                            }

                            if (a[0] && a[1] && a[2] && min[0] == min[1] == min[2] && max[0] == max[1] == max[2])
                            {
                                if (min[0] != 1 || max[0] != MAX_LEVEL - 1)
                                    out << classes[cls] << "(" << min[0] << "-" << max[0] << ")";
                                else
                                    out << classes[cls];

                                if (cls != classs.back().first.first)
                                    out << ";";
                            }
                            else
                            {
                                for (uint32 tb = 0; tb < 3; tb++)
                                {
                                    if (!a[tb])
                                        continue;

                                    if (min[tb] != 1 || max[tb] != MAX_LEVEL - 1)
                                        out << specs[cls][tb] << " " << classes[cls] << "(" << min[tb] << "-" << max[tb]
                                            << ")";
                                    else
                                        out << specs[cls][tb] << " " << classes[cls];

                                    if (cls != classs.back().first.first || tb != classs.back().first.second)
                                        out << ";";
                                }
                            }
                        }
                    }
                    else
                        out << "all";

                    out << "\n";
                }
                else
                    out << actionkey << "\n";
            }

            sPlayerbotAIConfig->log("strategy.csv", out.str().c_str());
        }
    }

    /*

    sPlayerbotAIConfig->openLog(7, "w");

    //Zone area map REMOVE!
    uint32 k = 0;
    for (auto& node : sTravelNodeMap->getNodes())
    {
        WorldPosition* pos = node->getPosition();
        //map area
        for (uint32 x = 0; x < 2000; x++)
        {
            for (uint32 y = 0; y < 2000; y++)
            {
                if (!pos->getMap())
                    continue;

                float nx = pos->getX() + (x*5)-5000.0f;
                float ny = pos->getY() + (y*5)-5000.0f;
                float nz = pos->getZ() + 100.0f;

                //pos->getMap()->GetHitPosition(nx, ny, nz + 200.0f, nx, ny, nz, -0.5f);

                if (!pos->getMap()->GetHeightInRange(nx, ny, nz, 5000.0f)) // GetHeight can fail
                    continue;

                WorldPosition  npos = WorldPosition(pos->getMapId(), nx, ny, nz, 0.0);
                uint32 area = path.getArea(npos.getMapId(), npos.getX(), npos.getY(), npos.getZ());

                std::ostringstream out;
                out << std::fixed << area << "," << npos.getDisplayX() << "," << npos.getDisplayY();
                sPlayerbotAIConfig->log(7, out.str().c_str());
            }
        }
        k++;

        if (k > 0)
            break;
    }

    //Explore map output (REMOVE!)

    sPlayerbotAIConfig->openLog(5, "w");
    for (auto i : exploreLocs)
    {
        for (auto j : i.second->getPoints())
        {
            std::ostringstream out;
            std::string const name = i.second->getTitle();
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());
            out << std::fixed << std::setprecision(2) << name.c_str() << "," << i.first << "," << j->getDisplayX() <<
    "," << j->getDisplayY() << "," << j->getX() << "," << j->getY() << "," << j->getZ(); sPlayerbotAIConfig->log(5,
    out.str().c_str());
        }
    }

    */
}

uint32 TravelMgr::getDialogStatus(Player* pPlayer, int32 questgiver, Quest const* pQuest)
{
    uint32 dialogStatus = DIALOG_STATUS_NONE;

    QuestRelationBounds rbounds;   // QuestRelations (quest-giver)
    QuestRelationBounds irbounds;  // InvolvedRelations (quest-finisher)

    uint32 questId = pQuest->GetQuestId();

    if (questgiver > 0)
    {
        rbounds = sObjectMgr->GetCreatureQuestRelationBounds(questgiver);
        irbounds = sObjectMgr->GetCreatureQuestInvolvedRelationBounds(questgiver);
    }
    else
    {
        rbounds = sObjectMgr->GetGOQuestRelationBounds(questgiver * -1);
        irbounds = sObjectMgr->GetGOQuestInvolvedRelationBounds(questgiver * -1);
    }

    // Check markings for quest-finisher
    for (QuestRelations::const_iterator itr = irbounds.first; itr != irbounds.second; ++itr)
    {
        if (itr->second != questId)
            continue;

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        if (!pQuest)
        {
            continue;
        }

        QuestStatus status = pPlayer->GetQuestStatus(questId);

        if ((status == QUEST_STATUS_COMPLETE && !pPlayer->GetQuestRewardStatus(questId)) ||
            (pQuest->IsAutoComplete() && pPlayer->CanTakeQuest(pQuest, false)))
        {
            if (pQuest->IsAutoComplete() && pQuest->IsRepeatable())
            {
                dialogStatusNew = DIALOG_STATUS_REWARD_REP;
            }
            else
            {
                dialogStatusNew = DIALOG_STATUS_REWARD2;
            }
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
        {
            dialogStatusNew = DIALOG_STATUS_INCOMPLETE;
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    // check markings for quest-giver
    for (QuestRelations::const_iterator itr = rbounds.first; itr != rbounds.second; ++itr)
    {
        if (itr->second != questId)
            continue;

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        if (!pQuest)
        {
            continue;
        }

        QuestStatus status = pPlayer->GetQuestStatus(questId);

        if (status == QUEST_STATUS_NONE)  // For all other cases the mark is handled either at some place else, or with
                                          // involved-relations already
        {
            if (pPlayer->CanSeeStartQuest(pQuest))
            {
                if (pPlayer->SatisfyQuestLevel(pQuest, false))
                {
                    int32 lowLevelDiff = sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF);
                    if (pQuest->IsAutoComplete() || (pQuest->IsRepeatable() && pPlayer->IsQuestRewarded(questId)))
                    {
                        dialogStatusNew = DIALOG_STATUS_REWARD_REP;
                    }
                    else if (lowLevelDiff < 0 ||
                             pPlayer->GetLevel() <= pPlayer->GetQuestLevel(pQuest) + uint32(lowLevelDiff))
                    {
                        dialogStatusNew = DIALOG_STATUS_AVAILABLE;
                    }
                    else
                    {
                        dialogStatusNew = DIALOG_STATUS_LOW_LEVEL_AVAILABLE;
                    }
                }
                else
                {
                    dialogStatusNew = DIALOG_STATUS_UNAVAILABLE;
                }
            }
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    return dialogStatus;
}

// Selects a random WorldPosition from a list. Use a distance weighted distribution.
std::vector<WorldPosition*> TravelMgr::getNextPoint(WorldPosition* center, std::vector<WorldPosition*> points,
                                                    uint32 amount)
{
    std::vector<WorldPosition*> retVec;

    if (points.size() < 2)
    {
        if (points.size() == 1)
            retVec.push_back(points[0]);

        return retVec;
    }

    retVec = points;

    std::vector<uint32> weights;

    // List of weights based on distance (Gausian curve that starts at 100 and lower to 1 at 1000 distance)
    // std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights), [center](WorldPosition point) { return
    // 1 + 1000 * exp(-1 * pow(point.distance(center) / 400.0, 2)); });

    // List of weights based on distance (Twice the distance = half the weight). Caps out at 200.0000 range.
    std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights),
                   [center](WorldPosition* point)
                   { return static_cast<uint32>(200000.f / (1.f + point->distance(center))); });

    Acore::Containers::RandomShuffle(retVec);

    std::vector<float> dists;

    // Total sum of all those weights.
    /*
    uint32 sum = std::accumulate(weights.begin(), weights.end(), 0);

    //Pick a random point based on weights.
    for (uint32 nr = 0; nr < amount; nr++)
    {
        //Pick a random number in that range.
        uint32 rnd = urand(0, sum);

        for (unsigned i = 0; i < points.size(); ++i)
            if (rnd < weights[i] && (retVec.empty() || std::find(retVec.begin(), retVec.end(), points[i]) ==
    retVec.end()))
            {
                retVec.push_back(points[i]);
                break;
            }
            else
                rnd -= weights[i];
    }*/

    return retVec;
}

std::vector<WorldPosition> TravelMgr::getNextPoint(WorldPosition center, std::vector<WorldPosition> points,
                                                   uint32 amount)
{
    std::vector<WorldPosition> retVec;

    if (points.size() == 1)
    {
        retVec.push_back(points[0]);
        return retVec;
    }

    // List of weights based on distance (Gausian curve that starts at 100 and lower to 1 at 1000 distance)
    std::vector<uint32> weights;

    std::transform(points.begin(), points.end(), std::back_inserter(weights),
                   [center](WorldPosition point)
                   { return 1 + 1000 * static_cast<uint32>(exp(-1.f * pow(point.distance(center) / 400.f, 2.f))); });

    // Total sum of all those weights.
    uint32 sum = std::accumulate(weights.begin(), weights.end(), 0);

    // Pick a random number in that range.
    uint32 rnd = urand(0, sum);

    // Pick a random point based on weights.
    for (uint32 nr = 0; nr < amount; nr++)
    {
        for (unsigned i = 0; i < points.size(); ++i)
            if (rnd < weights[i] &&
                (retVec.empty() || std::find(retVec.begin(), retVec.end(), points[i]) == retVec.end()))
            {
                retVec.push_back(points[i]);
                break;
            }
            else
                rnd -= weights[i];
    }

    // Peiru: Crash failsafe - if the retVec is still empty but points exist, return first point
    if (retVec.empty() && points.size() > 0)
        retVec.push_back(points[0]);

    if (!retVec.empty())
        return retVec;

    assert(!"No valid point found.");

    return retVec;
}

QuestStatusData* TravelMgr::getQuestStatus(Player* bot, uint32 questId) { return &bot->getQuestStatusMap()[questId]; }

bool TravelMgr::getObjectiveStatus(Player* bot, Quest const* pQuest, uint32 objective)
{
    uint32 questId = pQuest->GetQuestId();
    if (!bot->IsActiveQuest(questId))
        return false;

    if (bot->GetQuestStatus(questId) != QUEST_STATUS_INCOMPLETE)
        return false;

    QuestStatusData* questStatus = sTravelMgr->getQuestStatus(bot, questId);

    uint32 reqCount = pQuest->RequiredItemCount[objective];
    uint32 hasCount = questStatus->ItemCount[objective];

    if (reqCount && hasCount < reqCount)
        return true;

    reqCount = pQuest->RequiredNpcOrGoCount[objective];
    hasCount = questStatus->CreatureOrGOCount[objective];

    if (reqCount && hasCount < reqCount)
        return true;

    return false;
}

std::vector<TravelDestination*> TravelMgr::getQuestTravelDestinations(Player* bot, int32 questId, bool ignoreFull,
                                                                      bool ignoreInactive, float maxDistance,
                                                                      bool ignoreObjectives)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    if (!questId)
    {
        for (auto& dest : questGivers)
        {
            if (!ignoreInactive && !dest->isActive(bot))
                continue;

            if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                continue;

            retTravelLocations.push_back(dest);
        }
        for (auto& quest : quests)
        {
            for (auto& dest : quest.second->questTakers)
            {
                if (!ignoreInactive && !dest->isActive(bot))
                    continue;

                if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                    continue;

                retTravelLocations.push_back(dest);
            }

            if (!ignoreObjectives)
                for (auto& dest : quest.second->questObjectives)
                {
                    if (!ignoreInactive && !dest->isActive(bot))
                        continue;

                    if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                        continue;

                    retTravelLocations.push_back(dest);
                }
        }
    }
    else if (questId == -1)
    {
        for (auto& dest : questGivers)
        {
            if (!ignoreInactive && !dest->isActive(bot))
                continue;

            if (dest->isFull(ignoreFull))
                continue;

            if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                continue;

            retTravelLocations.push_back(dest);
        }
    }
    else
    {
        auto i = quests.find(questId);

        if (i != quests.end())
        {
            for (auto& dest : i->second->questTakers)
            {
                if (!ignoreInactive && !dest->isActive(bot))
                    continue;

                if (dest->isFull(ignoreFull))
                    continue;

                if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                    continue;

                retTravelLocations.push_back(dest);
            }

            if (!ignoreObjectives)
                for (auto& dest : i->second->questObjectives)
                {
                    if (!ignoreInactive && !dest->isActive(bot))
                        continue;

                    if (dest->isFull(ignoreFull))
                        continue;

                    if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                        continue;

                    retTravelLocations.push_back(dest);
                }
        }
    }

    return retTravelLocations;
}

std::vector<TravelDestination*> TravelMgr::getRpgTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive,
                                                                    float maxDistance)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    for (auto& dest : rpgNpcs)
    {
        if (!ignoreInactive && !dest->isActive(bot))
            continue;

        if (dest->isFull(ignoreFull))
            continue;

        if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
            continue;

        retTravelLocations.push_back(dest);
    }

    return std::move(retTravelLocations);
}

std::vector<TravelDestination*> TravelMgr::getExploreTravelDestinations(Player* bot, bool ignoreFull,
                                                                        bool ignoreInactive)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    for (auto& dest : exploreLocs)
    {
        if (!ignoreInactive && !dest.second->isActive(bot))
            continue;

        if (dest.second->isFull(ignoreFull))
            continue;

        retTravelLocations.push_back(dest.second);
    }

    return retTravelLocations;
}

std::vector<TravelDestination*> TravelMgr::getGrindTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive,
                                                                      float maxDistance)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    for (auto& dest : grindMobs)
    {
        if (!ignoreInactive && !dest->isActive(bot))
            continue;

        if (dest->isFull(ignoreFull))
            continue;

        if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
            continue;

        retTravelLocations.push_back(dest);
    }

    return retTravelLocations;
}

void TravelMgr::setNullTravelTarget(Player* player)
{
    if (!player)
        return;

    PlayerbotAI* playerBotAI = GET_PLAYERBOT_AI(player);
    if (!playerBotAI)
        return;

    TravelTarget* target = playerBotAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();

    if (target)
        target->setTarget(sTravelMgr->nullTravelDestination, sTravelMgr->nullWorldPosition, true);
}

void TravelMgr::addMapTransfer(WorldPosition start, WorldPosition end, float portalDistance, bool makeShortcuts)
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return;

    // Calculate shortcuts.
    if (makeShortcuts)
    {
        for (auto& mapTransfers : mapTransfersMap)
        {
            uint32 sMapt = mapTransfers.first.first;
            uint32 eMapt = mapTransfers.first.second;

            for (auto& mapTransfer : mapTransfers.second)
            {
                if (eMapt == sMap && sMapt != eMap)  // [S1 >MT> E1 -> S2] >THIS> E2
                {
                    float newDistToEnd = mapTransDistance(*mapTransfer.getPointFrom(), start) + portalDistance;
                    if (mapTransDistance(*mapTransfer.getPointFrom(), end) > newDistToEnd)
                        addMapTransfer(*mapTransfer.getPointFrom(), end, newDistToEnd, false);
                }

                if (sMapt == eMap && eMapt != sMap)  // S1 >THIS> [E1 -> S2 >MT> E2]
                {
                    float newDistToEnd = portalDistance + mapTransDistance(end, *mapTransfer.getPointTo());
                    if (mapTransDistance(start, *mapTransfer.getPointTo()) > newDistToEnd)
                        addMapTransfer(start, *mapTransfer.getPointTo(), newDistToEnd, false);
                }
            }
        }
    }

    // Add actual transfer.
    auto mapTransfers = mapTransfersMap.find(std::make_pair(start.getMapId(), end.getMapId()));

    if (mapTransfers == mapTransfersMap.end())
        mapTransfersMap.insert({{sMap, eMap}, {mapTransfer(start, end, portalDistance)}});
    else
        mapTransfers->second.push_back(mapTransfer(start, end, portalDistance));
};

void TravelMgr::loadMapTransfers()
{
    for (auto& node : sTravelNodeMap->getNodes())
    {
        for (auto& link : *node->getLinks())
        {
            addMapTransfer(*node->getPosition(), *link.first->getPosition(), link.second->getDistance());
        }
    }
}

float TravelMgr::mapTransDistance(WorldPosition start, WorldPosition end)
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return start.distance(end);

    float minDist = 200000;

    auto mapTransfers = mapTransfersMap.find({sMap, eMap});
    if (mapTransfers == mapTransfersMap.end())
        return minDist;

    for (auto& mapTrans : mapTransfers->second)
    {
        float dist = mapTrans.distance(start, end);

        if (dist < minDist)
            minDist = dist;
    }

    return minDist;
}

float TravelMgr::fastMapTransDistance(WorldPosition start, WorldPosition end)
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return start.fDist(end);

    float minDist = 200000;

    auto mapTransfers = mapTransfersMap.find({sMap, eMap});

    if (mapTransfers == mapTransfersMap.end())
        return minDist;

    for (auto& mapTrans : mapTransfers->second)
    {
        float dist = mapTrans.fDist(start, end);

        if (dist < minDist)
            minDist = dist;
    }

    return minDist;
}

QuestTravelDestination::QuestTravelDestination(uint32 questId1, float radiusMin1, float radiusMax1)
    : TravelDestination(radiusMin1, radiusMax1)
{
    questId = questId1;
    questTemplate = sObjectMgr->GetQuestTemplate(questId);
}

bool QuestTravelDestination::isActive(Player* bot) { return bot->IsActiveQuest(questId); }

bool QuestObjectiveTravelDestination::isCreature() { return GetQuestTemplate()->RequiredNpcOrGo[objective] > 0; }

uint32 QuestObjectiveTravelDestination::ReqCreature()
{
    return isCreature() ? GetQuestTemplate()->RequiredNpcOrGo[objective] : 0;
}

uint32 QuestObjectiveTravelDestination::ReqGOId()
{
    return !isCreature() ? abs(GetQuestTemplate()->RequiredNpcOrGo[objective]) : 0;
}

uint32 QuestObjectiveTravelDestination::ReqCount() { return GetQuestTemplate()->RequiredNpcOrGoCount[objective]; }

void TravelMgr::printGrid(uint32 mapId, int x, int y, std::string const type)
{
    std::string const fileName = "unload_grid.csv";

    if (sPlayerbotAIConfig->hasLog(fileName))
    {
        WorldPosition p = WorldPosition(mapId, 0, 0, 0, 0);

        std::ostringstream out;
        out << sPlayerbotAIConfig->GetTimestampStr();
        out << "+00, " << 0 << 0 << x << "," << y << ", " << type << ",";
        p.printWKT(p.fromGridCoord(GridCoord(x, y)), out, 1, true);
        sPlayerbotAIConfig->log(fileName, out.str().c_str());
    }
}

void TravelMgr::printObj(WorldObject* obj, std::string const type)
{
    std::string fileName = "unload_grid.csv";

    if (sPlayerbotAIConfig->hasLog(fileName))
    {
        WorldPosition p = WorldPosition(obj);

        Cell cell(obj->GetPositionX(), obj->GetPositionY());

        std::vector<WorldPosition> vcell, vgrid;
        vcell = p.fromCellCoord(p.getCellCoord());
        vgrid = p.gridFromCellCoord(p.getCellCoord());

        {
            std::ostringstream out;
            out << sPlayerbotAIConfig->GetTimestampStr();
            out << "+00, " << obj->GetGUID().GetEntry() << "," << obj->GetGUID().GetCounter() << "," << cell.GridX()
                << "," << cell.GridY() << ", " << type << ",";

            p.printWKT(vcell, out, 1, true);
            sPlayerbotAIConfig->log(fileName, out.str().c_str());
        }

        {
            std::ostringstream out;
            out << sPlayerbotAIConfig->GetTimestampStr();
            out << "+00, " << obj->GetGUID().GetEntry() << "," << obj->GetGUID().GetCounter() << "," << cell.GridX()
                << "," << cell.GridY() << ", " << type << ",";

            p.printWKT(vgrid, out, 1, true);
            sPlayerbotAIConfig->log(fileName, out.str().c_str());
        }
    }

    fileName = "unload_obj.csv";

    if (sPlayerbotAIConfig->hasLog(fileName))
    {
        WorldPosition p = WorldPosition(obj);

        Cell cell(obj->GetPositionX(), obj->GetPositionY());
        {
            std::ostringstream out;
            out << sPlayerbotAIConfig->GetTimestampStr();
            out << "+00, " << obj->GetGUID().GetEntry() << "," << obj->GetGUID().GetCounter() << "," << cell.GridX()
                << "," << cell.GridY() << ", " << type << ",";

            p.printWKT({p}, out, 0);
            sPlayerbotAIConfig->log(fileName, out.str().c_str());
        }
    }
}
