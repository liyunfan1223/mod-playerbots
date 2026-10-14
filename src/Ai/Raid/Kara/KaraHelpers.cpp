/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraHelpers.h"
#include "Playerbots.h"
#include <algorithm>
#include <limits>
#include <list>

namespace KaraHelpers
{

// General

bool IsSafePosition(float x, float y, std::vector<Unit*> const& hazards, float hazardRadius)
{
    for (Unit* hazard : hazards)
    {
        float dist = hazard->GetDistance2d(x, y);
        if (dist < hazardRadius)
            return false;
    }

    return true;
}

// Attumen the Huntsman

std::unordered_map<uint32, time_t> attumenDpsWaitTimer;

Unit* GetAttumenMounted(Player* bot)
{
    constexpr float searchRadius = 50.0f;
    return bot->FindNearestCreature(Id(KaraNpcs::NPC_ATTUMEN_THE_HUNTSMAN), searchRadius, true);
}

// Shade of Aran

bool IsAranCastingArcaneExplosion(Unit* aran)
{
    return aran && aran->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_ARCANE_EXPLOSION));
}

bool IsFlameWreathActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    if (aran->FindCurrentSpellBySpellId(Id(KaraSpells::SPELL_FLAME_WREATH_CAST)))
        return true;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(Id(KaraSpells::SPELL_FLAME_WREATH_AURA)))
            return true;
    }

    return false;
}

// Netherspite

std::unordered_map<uint32, time_t> netherspiteDpsWaitTimer;
std::unordered_map<uint32, ObjectGuid> currentRedBlocker;
std::unordered_map<uint32, ObjectGuid> currentGreenBlocker;
std::unordered_map<uint32, ObjectGuid> currentBlueBlocker;

bool IsBanishPhase(Unit* netherspite)
{
    return netherspite && netherspite->HasAura(Id(KaraSpells::SPELL_NETHERSPITE_BANISHED));
}

// Red beam blockers: tank bots, no Nether Exhaustion Red
std::vector<Player*> GetRedBlockers(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    std::vector<Player*> redBlockers;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsTank(member) ||
            !GET_PLAYERBOT_AI(member))
        {
            continue;
        }

        if (!member->HasAura(Id(KaraSpells::SPELL_NETHER_EXHAUSTION_RED)))
            redBlockers.push_back(member);
    }

    return redBlockers;
}

// Blue beam blockers: Mana-using DPS bots (includes Balance Druid)
// no Nether Exhaustion Blue and <25 stacks of Blue Beam debuff
std::vector<Player*> GetBlueBlockers(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    std::vector<Player*> blueBlockers;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != KARA_MAP_ID)
            continue;

        if (!GET_PLAYERBOT_AI(member) || member->getPowerType() != POWER_MANA ||
            !PlayerbotAI::IsDps(member))
        {
            continue;
        }

        if (member->HasAura(Id(KaraSpells::SPELL_NETHER_EXHAUSTION_BLUE)))
            continue;

        Aura* blueBuff = member->GetAura(Id(KaraSpells::SPELL_BLUE_BEAM_DEBUFF));
        if (!blueBuff || blueBuff->GetStackAmount() < 25)
            blueBlockers.push_back(member);
    }

    return blueBlockers;
}

// Green beam blockers:
// (1) Prioritize Rogue, cat Druid, and dps Warrior and DK bots, no Nether Exhaustion Green
// (2) Then assign Healer bots, no Nether Exhaustion Green and <25 stacks of Green Beam debuff
std::vector<Player*> GetGreenBlockers(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    std::vector<Player*> greenBlockers;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != KARA_MAP_ID)
            continue;

        if (!GET_PLAYERBOT_AI(member) || member->getPowerType() == POWER_MANA ||
            !PlayerbotAI::IsDps(member))
        {
            continue;
        }

        if (!member->HasAura(Id(KaraSpells::SPELL_NETHER_EXHAUSTION_GREEN)))
            greenBlockers.push_back(member);
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != KARA_MAP_ID)
            continue;

        if (!GET_PLAYERBOT_AI(member) || !PlayerbotAI::IsHeal(member))
            continue;

        if (member->HasAura(Id(KaraSpells::SPELL_NETHER_EXHAUSTION_GREEN)))
            continue;

        Aura* greenBuff = member->GetAura(Id(KaraSpells::SPELL_GREEN_BEAM_DEBUFF));
        if (!greenBuff || greenBuff->GetStackAmount() < 25)
        {
            greenBlockers.push_back(member);
        }
    }

    return greenBlockers;
}

std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers(Player* bot)
{
    uint32 const instanceId = bot->GetMap()->GetInstanceId();

    Player* redBlocker = nullptr;
    auto redBlockers = GetRedBlockers(bot);
    if (!redBlockers.empty())
    {
        ObjectGuid& redGuid = currentRedBlocker[instanceId];
        auto const it = std::find_if(redBlockers.begin(), redBlockers.end(), [&redGuid](Player* player)
        {
            return player && player->GetGUID() == redGuid;
        });

        if (it != redBlockers.end())
            redBlocker = *it;
        else
            redBlocker = redBlockers.front();

        redGuid = redBlocker ? redBlocker->GetGUID() : ObjectGuid::Empty;
    }
    else
    {
        redBlocker = nullptr;
    }

    Player* greenBlocker = nullptr;
    auto greenBlockers = GetGreenBlockers(bot);
    if (!greenBlockers.empty())
    {
        ObjectGuid& greenGuid = currentGreenBlocker[instanceId];
        auto const it = std::find_if(greenBlockers.begin(), greenBlockers.end(), [&greenGuid](Player* player)
        {
            return player && player->GetGUID() == greenGuid;
        });

        if (it != greenBlockers.end())
            greenBlocker = *it;
        else
            greenBlocker = greenBlockers.front();

        greenGuid = greenBlocker ? greenBlocker->GetGUID() : ObjectGuid::Empty;
    }
    else
    {
        greenBlocker = nullptr;
    }

    Player* blueBlocker = nullptr;
    auto blueBlockers = GetBlueBlockers(bot);
    if (!blueBlockers.empty())
    {
        ObjectGuid& blueGuid = currentBlueBlocker[instanceId];
        auto const it = std::find_if(blueBlockers.begin(), blueBlockers.end(), [&blueGuid](Player* player)
        {
            return player && player->GetGUID() == blueGuid;
        });

        if (it != blueBlockers.end())
            blueBlocker = *it;
        else
            blueBlocker = blueBlockers.front();

        blueGuid = blueBlocker ? blueBlocker->GetGUID() : ObjectGuid::Empty;
    }
    else
    {
        blueBlocker = nullptr;
    }

    return std::make_tuple(redBlocker, greenBlocker, blueBlocker);
}

std::vector<Unit*> GetAllVoidZones(Player* bot)
{
    std::vector<Unit*> voidZones;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 30.0f;

    bot->GetCreatureListWithEntryInGrid(creatureList, Id(KaraNpcs::NPC_VOID_ZONE), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            voidZones.push_back(creature);
    }

    return voidZones;
}

bool FindBeamPosition(
    Unit* netherspite, Unit* portal, std::vector<Unit*> const& voidZones,
    float idealDistance, Position& outPos)
{
    float bossX = netherspite->GetPositionX();
    float bossY = netherspite->GetPositionY();
    float portalX = portal->GetPositionX();
    float portalY = portal->GetPositionY();

    float dx = portalX - bossX;
    float dy = portalY - bossY;
    float length = netherspite->GetExactDist2d(portalX, portalY);
    if (length == 0.0f)
        return false;

    constexpr float voidZoneRadius = 4.0f;
    constexpr float searchMinDist = 18.0f;
    constexpr float searchStep = 0.5f;
    constexpr uint8 numSteps = 24;

    dx /= length;
    dy /= length;

    float bestDist = std::numeric_limits<float>::max();
    bool found = false;

    for (uint8 i = 0; i <= numSteps; ++i)
    {
        float const dist = searchMinDist + i * searchStep;
        float candidateX = bossX + dx * dist;
        float candidateY = bossY + dy * dist;
        float candidateZ = netherspite->GetPositionZ();
        if (!IsSafePosition(candidateX, candidateY, voidZones, voidZoneRadius))
            continue;

        float distToIdeal = fabs(dist - idealDistance);
        if (!found || distToIdeal < bestDist)
        {
            bestDist = distToIdeal;
            outPos = Position(candidateX, candidateY, candidateZ);
            found = true;
        }
    }

    return found;
}

// Prince Malchezaar

std::vector<Unit*> GetSpawnedInfernals(Player* bot)
{
    std::vector<Unit*> infernals;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 100.0f;

    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(KaraNpcs::NPC_NETHERSPITE_INFERNAL), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            infernals.push_back(creature);
    }

    return infernals;
}

bool IsStraightPathSafe(
    float sx, float sy, float tx, float ty, std::vector<Unit*> const& hazards, float hazardRadius)
{
    float const totalDistX = tx - sx;
    float const totalDistY = ty - sy;
    float const totalDist = sqrt(totalDistX * totalDistX + totalDistY * totalDistY);
    if (totalDist == 0.0f)
        return true;

    constexpr float stepSize = 0.5f;
    int const numSteps = static_cast<int>(totalDist / stepSize);
    for (int i = 0; i <= numSteps; ++i)
    {
        float const checkDist = i * stepSize;
        float const t = (totalDist > 0.0f) ? checkDist / totalDist : 0.0f;
        float checkX = sx + totalDistX * t;
        float checkY = sy + totalDistY * t;
        for (Unit* hazard : hazards)
        {
            float const hx = checkX - hazard->GetPositionX();
            float const hy = checkY - hazard->GetPositionY();
            if ((hx*hx + hy*hy) < hazardRadius * hazardRadius)
                return false;
        }
    }

    return true;
}

bool TryFindSafePositionWithSafePath(
    Player* bot, float originX, float originY, float centerX, float centerY,
    std::vector<Unit*> const& hazards, float safeDistance, float maxSampleDist,
    float& outX, float& outY)
{
    constexpr uint8 numAngles = 64;
    constexpr float stepSize = 0.5f;

    // Attempt to find a safe path, but take any path to a safe position if no safe path is found
    for (bool requireSafePath : { true, false })
    {
        float bestMoveDistSq = std::numeric_limits<float>::max();
        bool found = false;

        for (int i = 0; i < numAngles; ++i)
        {
            float angle = (2.0f * M_PI * i) / numAngles;
            float dx = cos(angle);
            float dy = sin(angle);

            int const numSteps = static_cast<int>(maxSampleDist / stepSize);
            for (int j = 1; j <= numSteps; ++j)
            {
                float const dist = j * stepSize;
                float destX = centerX + dx * dist;
                float destY = centerY + dy * dist;
                float destZ = bot->GetPositionZ();
                if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                        bot, centerX, centerY, destZ, destX, destY, destZ, true))
                {
                    continue;
                }

                if (!IsSafePosition(destX, destY, hazards, safeDistance))
                    continue;

                if (requireSafePath)
                {
                    if (!IsStraightPathSafe(originX, originY, destX, destY, hazards, safeDistance))
                        continue;
                }

                float const toDestX = destX - originX;
                float const toDestY = destY - originY;
                float const moveDistSq = toDestX*toDestX + toDestY*toDestY;
                if (moveDistSq < bestMoveDistSq)
                {
                    bestMoveDistSq = moveDistSq;
                    outX = destX;
                    outY = destY;
                    found = true;
                }
            }
        }

        if (found)
            return true;
    }

    return false;
}

// Nightbane

std::unordered_map<uint32, time_t> nightbaneDpsWaitTimer;
std::unordered_map<uint32, time_t> nightbaneFlightPhaseStartTimer;

}
