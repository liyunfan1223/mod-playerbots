/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulHelpers.h"
#include "AiFactory.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include <algorithm>
#include <limits>
#include <list>

namespace GruulHelpers
{

// High King Maulgar

bool IsMaulgarTank(Player* bot)
{
    // Note: IsMainTank() is not necessarily a tank (by either strategy or spec). It can be anybody
    // with the main tank flag. Raid strategies will have problems with non-tank main tanks so this
    // assumes you are using a real tank for your main tank.
    return PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot);
}

bool IsOlmTank(Player* bot)
{
    // Although passing true for indexLivingOnly means a death will swap the Blindeye tank to Olm,
    // this is intended since Blindeye dies first and Olm is more important to control anyway.
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool IsBlindeyeTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 1, true);
}

ObjectGuid FindKroshMageTankGuid(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return ObjectGuid::Empty;

    // If an assistant Mage (player or bot) is found, return immediately.
    // Otherwise, return the bot Mage with the highest HP as fallback.
    Player* highestHpBotMage = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != GRUUL_MAP_ID ||
            member->getClass() != CLASS_MAGE)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member->GetGUID();

        if (!GET_PLAYERBOT_AI(member))
            continue;

        uint32 const hp = member->GetMaxHealth();
        if (!highestHpBotMage || hp > highestHp)
        {
            highestHpBotMage = member;
            highestHp = hp;
        }
    }

    return highestHpBotMage ? highestHpBotMage->GetGUID() : ObjectGuid::Empty;
}

Player* GetKroshMageTank(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    ObjectGuid const guid = AI_VALUE(ObjectGuid, "high king maulgar krosh mage tank");

    return guid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(guid);
}

bool IsKroshMageTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return bot->getClass() == CLASS_MAGE && GetKroshMageTank(botAI) == bot;
}

ObjectGuid FindKigglerMoonkinTankGuid(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return ObjectGuid::Empty;

    // If an assistant Balance Druid (player or bot) is found, return immediately.
    // Otherwise, return the bot Balance Druid with the highest HP as fallback.
    Player* highestHpBotMoonkin = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != GRUUL_MAP_ID ||
            member->getClass() != CLASS_DRUID ||
            AiFactory::GetPlayerSpecTab(member) != DRUID_TAB_BALANCE)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member->GetGUID();

        if (!GET_PLAYERBOT_AI(member))
            continue;

        uint32 const hp = member->GetMaxHealth();
        if (!highestHpBotMoonkin || hp > highestHp)
        {
            highestHpBotMoonkin = member;
            highestHp = hp;
        }
    }

    return highestHpBotMoonkin ? highestHpBotMoonkin->GetGUID() : ObjectGuid::Empty;
}

Player* GetKigglerMoonkinTank(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    ObjectGuid const guid = AI_VALUE(ObjectGuid, "high king maulgar kiggler moonkin tank");

    return guid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(guid);
}

bool IsKigglerMoonkinTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return bot->getClass() == CLASS_DRUID && GetKigglerMoonkinTank(botAI) == bot;
}

GuidVector FindNearbyWildFelStalkerGuids(Player* bot)
{
    if (bot->GetMapId() != GRUUL_MAP_ID)
        return {};

    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(GruulNpcs::NPC_WILD_FEL_STALKER), WILD_FEL_STALKER_SEARCH_RADIUS);

    GuidVector guids;
    guids.reserve(creatureList.size());
    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            guids.push_back(creature->GetGUID());
    }

    std::sort(guids.begin(), guids.end(), [](ObjectGuid const& lhs, ObjectGuid const& rhs)
    {
        return lhs.GetCounter() < rhs.GetCounter();
    });

    return guids;
}

std::vector<Unit*> GetNearbyWildFelStalkers(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& guids = AI_VALUE_REF(GuidVector, "high king maulgar wild fel stalkers");

    std::vector<Unit*> felStalkers;
    felStalkers.reserve(guids.size());
    for (ObjectGuid const& guid : guids)
    {
        Unit* felStalker = botAI->GetUnit(guid);
        if (felStalker && felStalker->IsAlive())
            felStalkers.push_back(felStalker);
    }

    return felStalkers;
}

// Gruul the Dragonkiller

namespace
{
std::vector<Position> const& GetCaveInPositions(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()
        ->GetValue<std::vector<Position>>("gruul the dragonkiller cave in")->RefGet();
}
}

bool GetNearestCaveInPosition(PlayerbotAI* botAI, Position& pool)
{
    Player* bot = botAI->GetBot();
    bool found = false;
    float nearestDistance = std::numeric_limits<float>::max();
    for (Position const& position : GetCaveInPositions(botAI))
    {
        float const distance = bot->GetExactDist2d(position);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            pool = position;
            found = true;
        }
    }

    return found;
}

bool IsNearCaveIn(PlayerbotAI* botAI, float radius)
{
    Player* bot = botAI->GetBot();
    for (Position const& position : GetCaveInPositions(botAI))
    {
        if (bot->GetExactDist2d(position) < radius)
            return true;
    }

    return false;
}

bool IsInCaveIn(PlayerbotAI* botAI)
{
    return IsNearCaveIn(botAI, CAVE_IN_RADIUS);
}

bool HasGroundSlam(Player* bot)
{
    return bot->HasAura(Id(GruulSpells::SPELL_GROUND_SLAM_1)) ||
        bot->HasAura(Id(GruulSpells::SPELL_GROUND_SLAM_2));
}

}
