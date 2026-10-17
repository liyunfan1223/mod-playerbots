/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOT_GATHERNODEMGR_H
#define PLAYERBOT_GATHERNODEMGR_H

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ObjectGuid.h"
#include "TravelMgr.h"

class GameObject;
class Map;
class Player;
class PlayerbotAI;

struct GatherNodeSpawn
{
    ObjectGuid::LowType spawnId{0};
    WorldPosition pos{};
    uint32 skillId{0};  // SKILL_HERBALISM or SKILL_MINING
    uint32 reqSkillValue{0};
};

/// Index of gatherable gameobject spawns (herbs/veins) on continent maps,
/// built once from sObjectMgr spawn data at world init. Immutable afterwards.
class GatherNodeMgr
{
public:
    static GatherNodeMgr& instance()
    {
        static GatherNodeMgr instance;
        return instance;
    }

    void Load();

    // Cheap check for CheckRpgStatusAvailable: any node in the bot's current
    // zone harvestable with its current skills?
    bool HasUsableNodes(Player* bot);

    // Random pick among the 4 nearest unvisited usable nodes in the bot's
    // current zone (jitter keeps co-located bots from herding on one route).
    GatherNodeSpawn const* GetNextNode(Player* bot, std::unordered_set<ObjectGuid::LowType> const& visited);

    // Live (spawned + GO_STATE_READY) object for a spawn point, nullptr if
    // none. Only meaningful when the grid at the spawn position is loaded.
    static GameObject* FindLiveNode(Map* map, ObjectGuid::LowType spawnId);

    // True when the spawn point is verifiably empty right now: grid loaded
    // and no live object. An unloaded grid yields false ("unknown"). Never
    // record this verdict anywhere - nodes respawn, so liveness must be
    // re-evaluated at every decision.
    static bool IsVerifiablyDown(Map* map, WorldPosition const& pos, ObjectGuid::LowType spawnId);

    // Nearest unvisited usable node within `radius` that is verifiably up
    // right now, or nullptr. Deliberately not zone-filtered: a live node
    // just across a zone border still counts as "passing by".
    GatherNodeSpawn const* GetNearestLiveNode(Player* bot, std::unordered_set<ObjectGuid::LowType> const& visited,
                                              float radius);

private:
    bool IsUsable(PlayerbotAI* botAI, Player* bot, GatherNodeSpawn const& node);

    std::vector<GatherNodeSpawn> const* GetZoneNodes(uint32 mapId, uint32 zoneId) const;

    // Bucketed per zone so the zone-scoped queries (HasUsableNodes,
    // GetNextNode) touch only the bot's zone instead of the whole continent.
    std::unordered_map<uint32 /*mapId*/, std::unordered_map<uint32 /*zoneId*/, std::vector<GatherNodeSpawn>>> _nodes;
};

#define sGatherNodeMgr GatherNodeMgr::instance()

#endif
