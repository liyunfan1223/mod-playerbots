/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagHelpers.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "GameObject.h"
#include "Map.h"
#include "Playerbots.h"
#include <algorithm>
#include <list>

using namespace EncounterHelpers;

namespace MagHelpers
{

std::unordered_map<uint32, uint32> magDpsWaitTimer;
std::unordered_map<uint32, uint32> blastNovaTimer;
std::unordered_map<uint32, bool> lastBlastNovaState;
std::unordered_set<uint32> ceilingCollapseApplied;
std::unordered_map<uint32, std::unordered_map<ObjectGuid, CubeInfo>> botToCubeAssignments;

std::vector<uint32> const MANTICRON_CUBE_DB_GUIDS = { 43157, 43158, 43159, 43160, 43161 };

std::vector<CubeInfo> GetAllCubeInfosByDbGuids(Map* map, std::vector<uint32> const& cubeDbGuids)
{
    std::vector<CubeInfo> cubes;
    if (!map)
        return cubes;

    for (uint32 dbGuid : cubeDbGuids)
    {
        auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(dbGuid);
        if (bounds.first == bounds.second)
            continue;

        GameObject* go = bounds.first->second;
        if (!go)
            continue;

        CubeInfo info;
        info.guid = go->GetGUID();
        info.x = go->GetPositionX();
        info.y = go->GetPositionY();
        info.z = go->GetPositionZ();
        cubes.push_back(info);
    }

    return cubes;
}

// Identify channelers by their database GUIDs
Creature* GetChanneler(Player* bot, uint32 dbGuid)
{
    Map* map = bot->GetMap();
    if (!map)
        return nullptr;

    auto it = map->GetCreatureBySpawnIdStore().find(dbGuid);
    if (it == map->GetCreatureBySpawnIdStore().end())
        return nullptr;

    Creature* channeler = it->second;
    if (!channeler->IsAlive())
        return nullptr;

    return channeler;
}

// Sorted by GUID so every warlock indexes the same abyssal.
GuidVector FindBurningAbyssalGuids(Player* bot)
{
    constexpr float searchRadius = 100.0f;
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(MagNpcs::NPC_BURNING_ABYSSAL), searchRadius);

    GuidVector guids;
    guids.reserve(creatureList.size());
    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            guids.push_back(creature->GetGUID());
    }

    std::sort(guids.begin(), guids.end());
    return guids;
}

std::vector<Unit*> GetBurningAbyssals(PlayerbotAI* botAI)
{
    GuidVector const& guids =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("mag burning abyssals")->RefGet();

    std::vector<Unit*> abyssals;
    abyssals.reserve(guids.size());
    for (ObjectGuid const guid : guids)
    {
        Unit* abyssal = botAI->GetUnit(guid);
        if (abyssal && abyssal->IsAlive())
            abyssals.push_back(abyssal);
    }

    return abyssals;
}

bool IsMagtheridonActive(Unit* magtheridon)
{
    return magtheridon && !magtheridon->HasAura(Id(MagSpells::SPELL_SHADOW_CAGE));
}

bool IsCubeClicker(Player* bot)
{
    auto mapIt = botToCubeAssignments.find(bot->GetInstanceId());
    return mapIt != botToCubeAssignments.end() &&
        mapIt->second.find(bot->GetGUID()) != mapIt->second.end();
}

bool IsBlastNovaCasting(Unit* magtheridon)
{
    return magtheridon && magtheridon->FindCurrentSpellBySpellId(Id(MagSpells::SPELL_BLAST_NOVA));
}

bool IsCeilingCollapsed(Player* bot)
{
    return ceilingCollapseApplied.contains(bot->GetInstanceId());
}

std::vector<Position> FindDebrisPositions(Player* bot)
{
    constexpr float searchRadius = 150.0f;
    return GetDynamicObjectPositions(bot, searchRadius, Id(MagSpells::SPELL_DEBRIS_SPAWN));
}

bool GetActiveDebrisPosition(PlayerbotAI* botAI, Position& debris)
{
    std::vector<Position> const& debrisPositions = botAI->GetAiObjectContext()
        ->GetValue<std::vector<Position>>("mag debris positions")->RefGet();
    if (debrisPositions.empty())
        return false;

    debris = debrisPositions.front();
    return true;
}

bool IsPositionInActiveDebris(PlayerbotAI* botAI, float x, float y, float radius)
{
    Position debris;
    return GetActiveDebrisPosition(botAI, debris) && debris.GetExactDist2d(x, y) <= radius;
}

std::vector<GameObject*> GetActiveConflagrations(PlayerbotAI* botAI)
{
    std::vector<GameObject*> blazes;
    auto const& gameObjects =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects")->Get();
    for (auto const& goGuid : gameObjects)
    {
        GameObject* go = botAI->GetGameObject(goGuid);
        if (!go || !go->isSpawned() || go->GetEntry() != Id(MagObjects::GO_BLAZE))
            continue;

        blazes.push_back(go);
    }

    return blazes;
}

bool IsPositionInConflagration(std::vector<GameObject*> const& blazes, float x, float y)
{
    for (GameObject* blaze : blazes)
    {
        if (blaze->GetDistance2d(x, y) < CONFLAGRATION_HAZARD_RADIUS)
            return true;
    }

    return false;
}

bool IsPositionInActiveConflagration(PlayerbotAI* botAI, float x, float y)
{
    return IsPositionInConflagration(GetActiveConflagrations(botAI), x, y);
}

}
