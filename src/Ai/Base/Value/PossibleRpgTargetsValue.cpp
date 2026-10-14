/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PossibleRpgTargetsValue.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "NearestGameObjects.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include <unordered_set>

const std::vector<uint32> PossibleRpgTargetsValue::allowedNpcFlags = {
    UNIT_NPC_FLAG_INNKEEPER,
    UNIT_NPC_FLAG_GOSSIP,
    UNIT_NPC_FLAG_QUESTGIVER,
    UNIT_NPC_FLAG_FLIGHTMASTER,
    UNIT_NPC_FLAG_BANKER,
    UNIT_NPC_FLAG_GUILD_BANKER,
    UNIT_NPC_FLAG_TRAINER_CLASS,
    UNIT_NPC_FLAG_TRAINER_PROFESSION,
    UNIT_NPC_FLAG_VENDOR_AMMO,
    UNIT_NPC_FLAG_VENDOR_FOOD,
    UNIT_NPC_FLAG_VENDOR_POISON,
    UNIT_NPC_FLAG_VENDOR_REAGENT,
    UNIT_NPC_FLAG_AUCTIONEER,
    UNIT_NPC_FLAG_STABLEMASTER,
    UNIT_NPC_FLAG_PETITIONER,
    UNIT_NPC_FLAG_TABARDDESIGNER,
    UNIT_NPC_FLAG_BATTLEMASTER,

    UNIT_NPC_FLAG_TRAINER,
    UNIT_NPC_FLAG_VENDOR,
    UNIT_NPC_FLAG_REPAIR,
};

PossibleRpgTargetsValue::PossibleRpgTargetsValue(PlayerbotAI* botAI, float range)
    : NearestUnitsValue(botAI, "possible rpg targets", range, true)
{
}

void PossibleRpgTargetsValue::FindUnits(std::list<Unit*>& targets)
{
    Acore::AnyUnitInObjectRangeCheck u_check(bot, range);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, range);
}

bool PossibleRpgTargetsValue::AcceptUnit(Unit* unit)
{
    if (!unit || !unit->IsInWorld() || unit->IsDuringRemoveFromWorld())
        return false;

    if (unit->IsHostileTo(bot) || unit->IsPlayer())
        return false;

    if (ServerFacade::instance().GetDistance2d(bot, unit) <= sPlayerbotAIConfig.TooCloseDistance)
        return false;

    if (unit->HasNpcFlag(UNIT_NPC_FLAG_SPIRITHEALER))
        return false;

    for (uint32 npcFlag : allowedNpcFlags)
    {
        if (unit->HasNpcFlag(static_cast<NPCFlags>(npcFlag)))
            return true;
    }

    TravelTarget* travelTarget = context->GetValue<TravelTarget*>("travel target")->Get();
    if (travelTarget && travelTarget->getDestination() &&
        static_cast<uint32>(travelTarget->getDestination()->getEntry()) == unit->GetEntry())
        return true;

    if (urand(1, 100) < 25 && unit->IsFriendlyTo(bot))
        return true;

    if (urand(1, 100) < 5)
        return true;

    return false;
}

const std::vector<uint32> PossibleNewRpgTargetsValue::allowedNpcFlags = {
    UNIT_NPC_FLAG_INNKEEPER,
    UNIT_NPC_FLAG_GOSSIP,
    UNIT_NPC_FLAG_QUESTGIVER,
    UNIT_NPC_FLAG_FLIGHTMASTER,
    UNIT_NPC_FLAG_BANKER,
    UNIT_NPC_FLAG_GUILD_BANKER,
    UNIT_NPC_FLAG_TRAINER_CLASS,
    UNIT_NPC_FLAG_TRAINER_PROFESSION,
    UNIT_NPC_FLAG_VENDOR_AMMO,
    UNIT_NPC_FLAG_VENDOR_FOOD,
    UNIT_NPC_FLAG_VENDOR_POISON,
    UNIT_NPC_FLAG_VENDOR_REAGENT,
    UNIT_NPC_FLAG_AUCTIONEER,
    UNIT_NPC_FLAG_STABLEMASTER,
    UNIT_NPC_FLAG_PETITIONER,
    UNIT_NPC_FLAG_TABARDDESIGNER,
    UNIT_NPC_FLAG_BATTLEMASTER,

    UNIT_NPC_FLAG_TRAINER,
    UNIT_NPC_FLAG_VENDOR,
    UNIT_NPC_FLAG_REPAIR,
};

// Sparse starting zones where the default scan range is insufficient for WANDER_NPC (requires >= 3 NPCs)
static const std::unordered_set<uint32> rpgRangeOverrideAreaIds = { 3526 /* Ammen Vale */, 2117 /* Deathknell */ };

PossibleNewRpgTargetsValue::PossibleNewRpgTargetsValue(PlayerbotAI* botAI, float range)
    : NearestUnitsValue(botAI, "possible new rpg targets", range, true), defaultRange(range)
{
}

GuidVector PossibleNewRpgTargetsValue::Calculate()
{
    if (rpgRangeOverrideAreaIds.count(bot->GetAreaId()) && defaultRange < 200.0f)
        range = 200.0f;
    else
        range = defaultRange;

    std::list<Unit*> targets;
    FindUnits(targets);

    GuidVector results;
    std::vector<std::pair<ObjectGuid, float>> guidDistancePairs;
    for (Unit* unit : targets)
    {
        if (AcceptUnit(unit) && (ignoreLos || bot->IsWithinLOSInMap(unit)))
            guidDistancePairs.push_back({unit->GetGUID(), bot->GetExactDist(unit)});
    }
    // Override to sort by distance
    std::sort(guidDistancePairs.begin(), guidDistancePairs.end(), [](auto const& a, auto const& b) {
        return a.second < b.second;
    });

    for (auto const& pair : guidDistancePairs) {
        results.push_back(pair.first);
    }
    return results;
}

void PossibleNewRpgTargetsValue::FindUnits(std::list<Unit*>& targets)
{
    Acore::AnyUnitInObjectRangeCheck u_check(bot, range);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, range);
}

bool PossibleNewRpgTargetsValue::AcceptUnit(Unit* unit)
{
    if (!unit || !unit->IsInWorld() || unit->IsDuringRemoveFromWorld())
        return false;

    if (unit->IsHostileTo(bot) || unit->IsPlayer())
        return false;

    if (unit->HasNpcFlag(UNIT_NPC_FLAG_SPIRITHEALER))
        return false;

    for (uint32 npcFlag : allowedNpcFlags)
    {
        if (unit->HasNpcFlag(static_cast<NPCFlags>(npcFlag)))
            return true;
    }

    return false;
}

const std::vector<GameobjectTypes> PossibleNewRpgGameObjectsValue::allowedGOFlags = {
    GAMEOBJECT_TYPE_QUESTGIVER,
};

GuidVector PossibleNewRpgGameObjectsValue::Calculate()
{
    std::list<GameObject*> targets;
    AnyGameObjectInObjectRangeCheck u_check(bot, range);
    Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, range);

    std::vector<std::pair<ObjectGuid, float>> guidDistancePairs;
    for (GameObject* go : targets)
    {
        bool flagCheck = false;
        for (uint32 goFlag : allowedGOFlags)
        {
            if (go->GetGoType() == goFlag)
            {
                flagCheck = true;
                break;
            }
        }
        if (!flagCheck)
            continue;

        if (!ignoreLos && !bot->IsWithinLOSInMap(go))
            continue;

        guidDistancePairs.push_back({go->GetGUID(), bot->GetExactDist(go)});
    }
    GuidVector results;

    // Sort by distance
    std::sort(guidDistancePairs.begin(), guidDistancePairs.end(), [](auto const& a, auto const& b) {
        return a.second < b.second;
    });

    for (auto const& pair : guidDistancePairs) {
        results.push_back(pair.first);
    }
    return results;
}
