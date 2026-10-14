/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZAHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>

using namespace EncounterHelpers;

namespace
{

bool IsInsideSafeZone(std::vector<Position> const& corners, float x, float y)
{
    // The safe zone's shape is pentagonal, sort of like a rectangle with one of the corners cut
    // out (to exclude a fenced-off broken corner of the stage).
    size_t const count = corners.size();
    int8 insideSign = 0;
    for (size_t i = 0; i < count; ++i)
    {
        Position const& edgeStart = corners[i];
        Position const& edgeEnd = corners[(i + 1) % count];

        float cross =
            (edgeEnd.GetPositionX() - edgeStart.GetPositionX()) * (y - edgeStart.GetPositionY()) -
            (edgeEnd.GetPositionY() - edgeStart.GetPositionY()) * (x - edgeStart.GetPositionX());

        if (cross == 0.0f)
            continue;

        int8 const sign = cross > 0.0f ? 1 : -1;
        if (insideSign == 0)
            insideSign = sign;
        else if (insideSign != sign)
            return false;
    }

    return true;
}

bool IsPositionSafeFromHazards(
    float x, float y, std::vector<Unit*> const& hazards, float hazardRadius)
{
    for (Unit* hazard : hazards)
    {
        if (hazard->GetExactDist2d(x, y) < hazardRadius)
            return false;
    }

    return true;
}

}  // end anonymous namespace

namespace ZaHelpers
{

// Akil'zon <Eagle Avatar>

std::unordered_map<uint32, uint32> akilzonStormTimer;

bool IsInStormWindow(uint32 startMs)
{
    uint32 const elapsed = GetMSTimeDiffToNow(startMs);
    if (elapsed < AKILZON_STORM_PERIOD_MS - AKILZON_STORM_LEAD_MS)
        return false;

    uint32 const phase = (elapsed + AKILZON_STORM_LEAD_MS) % AKILZON_STORM_PERIOD_MS;
    return phase < AKILZON_STORM_LEAD_MS + AKILZON_STORM_DURATION_MS;
}

Player* GetElectricalStormTarget(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(Id(ZaSpells::SPELL_ELECTRICAL_STORM)))
            return member;
    }

    return nullptr;
}

// Nalorakk <Bear Avatar>

bool IsNalorakkInBearForm(Unit* nalorakk)
{
    return nalorakk && nalorakk->HasAura(Id(ZaSpells::SPELL_BEARFORM));
}

// Jan'alai <Dragonhawk Avatar>

std::pair<Unit*, Unit*> GetAmanishiHatcherPair(PlayerbotAI* botAI)
{
    Unit* lowest = nullptr;
    Unit* highest = nullptr;

    AiObjectContext* context = botAI->GetAiObjectContext();
    for (auto const& targetGuid : AI_VALUE(GuidVector, "possible targets no los"))
    {
        Unit* unit = botAI->GetUnit(targetGuid);
        if (unit && unit->GetEntry() == Id(ZaNpcs::NPC_AMANISHI_HATCHER))
        {
            if (!lowest || unit->GetGUID().GetCounter() < lowest->GetGUID().GetCounter())
                lowest = unit;

            if (!highest || unit->GetGUID().GetCounter() > highest->GetGUID().GetCounter())
                highest = unit;
        }
    }

    return {lowest, highest};
}

uint32 CountJanalaiHatchlingsByEntry(PlayerbotAI* botAI)
{
    uint32 count = 0;

    AiObjectContext* context = botAI->GetAiObjectContext();
    for (auto const& targetGuid : AI_VALUE(GuidVector, "attackers"))
    {
        Unit* unit = botAI->GetUnit(targetGuid);
        if (unit && unit->IsAlive() &&
            unit->GetEntry() == Id(ZaNpcs::NPC_AMANI_DRAGONHAWK_HATCHLING))
        {
            ++count;
        }
    }

    return count;
}

bool IsJanalaiBombing(Unit* janalai)
{
    return janalai && janalai->HasAura(Id(ZaSpells::SPELL_FIRE_BOMB_CHANNEL));
}

GuidVector FindNearbyFireBombGuids(Player* bot)
{
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(ZaNpcs::NPC_FIRE_BOMB), JANALAI_FIRE_BOMB_SEARCH_RADIUS);

    GuidVector guids;
    guids.reserve(creatureList.size());
    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            guids.push_back(creature->GetGUID());
    }

    return guids;
}

std::vector<Unit*> GetNearbyFireBombs(PlayerbotAI* botAI)
{
    GuidVector const& guids =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("jan'alai fire bombs")->RefGet();

    std::vector<Unit*> bombs;
    bombs.reserve(guids.size());
    for (ObjectGuid const& guid : guids)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive())
            bombs.push_back(unit);
    }

    return bombs;
}

// Shortest move to a spot that is safe from Fire Bombs, within the measured area
bool FindSafeStepInJanalaiZone(
    Player* bot, std::vector<Unit*> const& hazards, std::vector<Position> const& safeZone,
    float maxSearchDistance, float hazardRadius, float moveDist,
    float& stepX, float& stepY, float& stepZ)
{
    constexpr uint8 angleCount = 16;
    constexpr float angleStep = 2.0f * M_PI / angleCount;
    constexpr float distanceStep = 1.0f;

    uint32 const ringCount = static_cast<uint32>(maxSearchDistance / distanceStep);

    for (uint32 ring = 1; ring <= ringCount; ++ring)
    {
        float const distance = ring * distanceStep;
        for (uint8 i = 0; i < angleCount; ++i)
        {
            float const angle = i * angleStep;
            float const x = bot->GetPositionX() + distance * std::cos(angle);
            float const y = bot->GetPositionY() + distance * std::sin(angle);

            if (!IsInsideSafeZone(safeZone, x, y))
                continue;

            if (!IsPositionSafeFromHazards(x, y, hazards, hazardRadius))
                continue;

            if (!CanTakeStepTowards(bot, x, y, moveDist, stepX, stepY, stepZ))
                continue;

            return true;
        }
    }

    return false;
}

// Halazzi <Lynx Avatar>
// N/A

// Hex Lord Malacrass

ObjectGuid FindNearbyFreezingTrapGuid(Player* bot)
{
    if (bot->GetMapId() != ZA_MAP_ID)
        return ObjectGuid::Empty;

    GameObject* trap = bot->FindNearestGameObject(
        Id(ZaObjects::GO_FREEZING_TRAP), ZA_FREEZING_TRAP_SEARCH_RADIUS, true);

    return trap ? trap->GetGUID() : ObjectGuid::Empty;
}

GameObject* GetNearbyFreezingTrap(PlayerbotAI* botAI)
{
    ObjectGuid const guid = botAI->GetAiObjectContext()
        ->GetValue<ObjectGuid>("hex lord malacrass freezing trap")->Get();

    return guid.IsEmpty() ? nullptr : botAI->GetGameObject(guid);
}

// Zul'jin

bool GetZuljinSpreadSlotIndex(Player* bot, size_t slotCount, size_t& slotIndex)
{
    if (slotCount == 0)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != ZA_MAP_ID || !PlayerbotAI::IsRanged(member))
            continue;

        if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    auto const healerIt = std::find(healers.begin(), healers.end(), bot);
    if (healerIt != healers.end())
    {
        slotIndex = static_cast<size_t>(std::distance(healers.begin(), healerIt)) % slotCount;
        return true;
    }

    auto const dpsIt = std::find(rangedDps.begin(), rangedDps.end(), bot);
    if (dpsIt == rangedDps.end())
        return false;

    size_t const ordinal =
        healers.size() + static_cast<size_t>(std::distance(rangedDps.begin(), dpsIt));
    slotIndex = ordinal % slotCount;
    return true;
}

Player* GetZuljinCreepingParalysisDispelTarget(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* closestTarget = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->HasAura(Id(ZaSpells::SPELL_CREEPING_PARALYSIS)))
            continue;

        float distance = bot->GetExactDist(member);
        if (distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

}
