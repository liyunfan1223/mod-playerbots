/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PossibleRpgTargetsValue.h"

#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "NearestGameObjects.h"

std::vector<uint32> PossibleRpgTargetsValue::allowedNpcFlags;

PossibleRpgTargetsValue::PossibleRpgTargetsValue(PlayerbotAI* botAI, float range)
    : NearestUnitsValue(botAI, "possible rpg targets", range, true)
{
    if (allowedNpcFlags.empty())
    {
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_INNKEEPER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_GOSSIP);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_QUESTGIVER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_FLIGHTMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_BANKER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_GUILD_BANKER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER_CLASS);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER_PROFESSION);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR_AMMO);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR_FOOD);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR_POISON);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR_REAGENT);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_AUCTIONEER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_STABLEMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_PETITIONER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TABARDDESIGNER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_BATTLEMASTER);

        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_REPAIR);
    }
}

void PossibleRpgTargetsValue::FindUnits(std::list<Unit*>& targets)
{
    Acore::AnyUnitInObjectRangeCheck u_check(bot, range);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, range);
}

bool PossibleRpgTargetsValue::AcceptUnit(Unit* unit)
{
    if (unit->IsHostileTo(bot) || unit->GetTypeId() == TYPEID_PLAYER)
        return false;

    if (sServerFacade->GetDistance2d(bot, unit) <= sPlayerbotAIConfig->tooCloseDistance)
        return false;

    if (unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER))
        return false;

    for (uint32 npcFlag : allowedNpcFlags)
    {
        if (unit->HasFlag(UNIT_NPC_FLAGS, npcFlag))
            return true;
    }

    TravelTarget* travelTarget = context->GetValue<TravelTarget*>("travel target")->Get();
    if (travelTarget->getDestination() && travelTarget->getDestination()->getEntry() == unit->GetEntry())
        return true;

    if (urand(1, 100) < 25 && unit->IsFriendlyTo(bot))
        return true;

    if (urand(1, 100) < 5)
        return true;

    return false;
}


std::vector<uint32> PossibleNewRpgTargetsValue::allowedNpcFlags;

PossibleNewRpgTargetsValue::PossibleNewRpgTargetsValue(PlayerbotAI* botAI, float range)
    : NearestUnitsValue(botAI, "possible new rpg targets", range, true)
{
    if (allowedNpcFlags.empty())
    {
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_INNKEEPER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_GOSSIP);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_QUESTGIVER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_FLIGHTMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_BANKER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_GUILD_BANKER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER_CLASS);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER_PROFESSION);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR_AMMO);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR_FOOD);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR_POISON);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR_REAGENT);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_AUCTIONEER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_STABLEMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_PETITIONER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TABARDDESIGNER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_BATTLEMASTER);

        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_REPAIR);
    }
}

GuidVector PossibleNewRpgTargetsValue::Calculate()
{
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
    std::sort(guidDistancePairs.begin(), guidDistancePairs.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    for (const auto& pair : guidDistancePairs) {
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
    if (unit->IsHostileTo(bot) || unit->GetTypeId() == TYPEID_PLAYER)
        return false;

    if (unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER))
        return false;

    for (uint32 npcFlag : allowedNpcFlags)
    {
        if (unit->HasFlag(UNIT_NPC_FLAGS, npcFlag))
            return true;
    }

    return false;
}

std::vector<GameobjectTypes> PossibleNewRpgGameObjectsValue::allowedGOFlags;

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
    std::sort(guidDistancePairs.begin(), guidDistancePairs.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    for (const auto& pair : guidDistancePairs) {
        results.push_back(pair.first);
    }
    return results;
}
