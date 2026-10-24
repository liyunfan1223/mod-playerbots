/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_XT002.h"

#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SpellMgr.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "Unit.h"

#include <algorithm>
#include <list>
#include <string>
#include <utility>
#include <vector>

using namespace EncounterHelpers;

// XT-002 anchors. The tank spot sits just behind XT's spawn; the ranged spot keeps a 30 yd caster in range.
const Position ULDUAR_XT002_MAINTANK_SPOT = Position(895.82f, -12.53954f, 409.68756f);
const Position ULDUAR_XT002_RANGED_SPOT = Position(866.0f, -12.5f, 409.8f);
// South-west so the 8 yd splash reaches neither the formation nor a bomb cell, and 38 yd from the melee stack,
// inside a 9 s run. North is closer but findSmoothPath answers NOPATH there from every melee position.
const Position ULDUAR_XT002_SEARING_LIGHT_SPOT = Position(846.0f, -22.0f, 409.597f);
// Separate origins so melee and ranged carriers never drop Void Zones on each other, each the grid corner
// nearest the raid. The northern one is the southern reflected about the formation centre.
const Position ULDUAR_XT002_GRAVITY_BOMB_ORIGIN_MELEE = Position(871.5199f, -42.04216f, 409.80377f);
const Position ULDUAR_XT002_GRAVITY_BOMB_ORIGIN_RANGED_SOUTH = Position(837.0746f, -41.01061f, 409.80362f);
const Position ULDUAR_XT002_GRAVITY_BOMB_ORIGIN_RANGED_NORTH = Position(837.0746f, 28.01061f, 409.811f);

// XT and the Heart go UNIT_FLAG_NOT_SELECTABLE for part of the fight and AttackersValue::IsPossibleTarget
// rejects the flag, so scan the raw nearby-npc list to keep them visible through the Heart phases.
static Unit* GetFirstAliveNpcByEntry(PlayerbotAI* botAI, GuidVector const& npcs, uint32 entry)
{
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == entry)
            return unit;
    }

    return nullptr;
}

static Unit* GetFirstAliveNpcByEntry(PlayerbotAI* botAI, uint32 entry)
{
    return GetFirstAliveNpcByEntry(botAI, botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get(),
                                   entry);
}

Unit* GetXT002(PlayerbotAI* botAI) { return GetFirstAliveNpcByEntry(botAI, NPC_XT002); }

Unit* GetXT002ExposedHeart(PlayerbotAI* botAI)
{
    Unit* heart = GetFirstAliveNpcByEntry(botAI, NPC_HEART_OF_DECONSTRUCTOR);
    if (!heart || heart->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return nullptr;

    // Only worth hitting while it channels Exposed Heart: that aura is what moves its damage to XT.
    return heart->HasAura(SPELL_XT002_EXPOSED_HEART) ? heart : nullptr;
}

bool IsXT002Submerged(Unit* xt002)
{
    if (!xt002)
        return false;

    return xt002->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) || xt002->HasAura(SPELL_XT002_SUBMERGE);
}

uint32 GetXT002SearingLightSpellId(Player* bot)
{
    return sSpellMgr->GetSpellIdForDifficulty(SPELL_XT002_SEARING_LIGHT, bot);
}

uint32 GetXT002HeartbreakSpellId(Player* bot)
{
    return sSpellMgr->GetSpellIdForDifficulty(SPELL_XT002_HEARTBREAK, bot);
}

uint32 GetXT002GravityBombSpellId(Player* bot)
{
    return sSpellMgr->GetSpellIdForDifficulty(SPELL_XT002_GRAVITY_BOMB, bot);
}

bool IsXT002PummellerTank(Player* bot)
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    // Whoever holds XT keeps holding him, so the add belongs to the first assist tank and only falls
    // to the main tank when there is no second tank left.
    if (Player* assistTank = GetGroupAssistTank(bot, 0))
        return assistTank == bot;

    if (Player* mainTank = GetGroupMainTank(bot))
        return mainTank == bot;

    // Neither resolves only when this bot is the last tank standing, so it owns the add by default.
    return true;
}

bool IsXT002AddEngageable(Unit* xt002, Unit* unit)
{
    if (!unit)
        return false;

    if (!xt002)
        return true;

    return unit->GetExactDist2d(xt002) <= ULDUAR_XT002_ADD_LEASH_RADIUS;
}

Unit* GetXT002EngageableAdd(PlayerbotAI* botAI, Player* bot, uint32 entry, float botReach)
{
    Unit* nearest = nullptr;
    float nearestDistance = 0.0f;

    GuidVector const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();

    // Same unit GetXT002 would return, read off the list already in hand instead of a scan per add.
    Unit* xt002 = GetFirstAliveNpcByEntry(botAI, npcs, NPC_XT002);

    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != entry)
            continue;

        if (!IsXT002AddEngageable(xt002, unit))
            continue;

        float const distance = unit->GetExactDist2d(bot);
        if (distance > botReach)
            continue;

        if (!nearest || distance < nearestDistance)
        {
            nearest = unit;
            nearestDistance = distance;
        }
    }

    return nearest;
}

// Formation roster, in the order every bot derives. The dead keep their slots: indexing by the living
// re-seats everyone behind a corpse mid-fight.
static bool BuildXT002RingMembers(Player* bot, std::vector<Player*>& out)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    out.clear();

    // Heal flag read once per member, not on both sides of every compare in the sort.
    std::vector<std::pair<bool, Player*>> ranked;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != bot->GetMapId())
            continue;

        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        if (!memberAI || PlayerbotAI::IsTank(member))
            continue;

        bool const heal = PlayerbotAI::IsHeal(member);
        if (!heal && !PlayerbotAI::IsRangedDps(member))
            continue;

        ranked.emplace_back(heal, member);
    }

    if (ranked.empty())
        return false;

    // Healers ahead of ranged dps, then guid: every bot derives the same order, and healers land on the centre
    // and inner ring, nearest the tank spot.
    std::sort(ranked.begin(), ranked.end(),
              [](std::pair<bool, Player*> const& left, std::pair<bool, Player*> const& right)
              {
                  if (left.first != right.first)
                      return left.first;
                  return left.second->GetGUID() < right.second->GetGUID();
              });

    for (std::pair<bool, Player*> const& entry : ranked)
        out.push_back(entry.second);

    return true;
}

// Raw formation geometry for one slot, no ground or collision pass.
static Position XT002RingSlotPoint(size_t slot, size_t total)
{
    // Bearing between two fixed points, so the layout never rotates, and it runs towards the tank spot
    // so slot 1 is the one nearest both XT and the tank.
    float const baseAngle =
        std::atan2(ULDUAR_XT002_MAINTANK_SPOT.GetPositionY() - ULDUAR_XT002_RANGED_SPOT.GetPositionY(),
                   ULDUAR_XT002_MAINTANK_SPOT.GetPositionX() - ULDUAR_XT002_RANGED_SPOT.GetPositionX());

    size_t const inner = std::min<size_t>(ULDUAR_XT002_RANGED_RING_INNER_SLOTS, total > 0 ? total - 1 : 0);

    float radiusX = 0.0f;
    float radiusY = 0.0f;
    float angle = baseAngle;

    // Slot 0 is the centre, nearest both the tank spot and the melee stack, so it goes to a healer.
    if (slot && slot <= inner)
    {
        radiusX = ULDUAR_XT002_RANGED_RING_INNER_X;
        radiusY = ULDUAR_XT002_RANGED_RING_INNER_Y;
        angle = baseAngle + 2.0f * static_cast<float>(M_PI) * static_cast<float>(slot - 1) / static_cast<float>(inner);
    }
    else if (slot)
    {
        size_t const outerCount = total - inner - 1;
        size_t const outerSlot = slot - inner - 1;
        radiusX = ULDUAR_XT002_RANGED_RING_OUTER_X;
        radiusY = ULDUAR_XT002_RANGED_RING_OUTER_Y;
        angle = baseAngle +
                2.0f * static_cast<float>(M_PI) * static_cast<float>(outerSlot) / static_cast<float>(outerCount);
    }

    angle = Position::NormalizeOrientation(angle);

    return Position(ULDUAR_XT002_RANGED_SPOT.GetPositionX() + std::cos(angle) * radiusX,
                    ULDUAR_XT002_RANGED_SPOT.GetPositionY() + ULDUAR_XT002_RANGED_RING_OFFSET_Y +
                        std::sin(angle) * radiusY,
                    ULDUAR_XT002_RANGED_SPOT.GetPositionZ());
}

bool GetXT002RangedSlot(Player* bot, Unit* xt002, Position& out)
{
    std::vector<Player*> members;
    if (!BuildXT002RingMembers(bot, members))
        return false;

    size_t slot = members.size();
    for (size_t i = 0; i < members.size(); ++i)
        if (members[i] == bot)
            slot = i;

    if (slot >= members.size())
        return false;

    size_t const total = members.size();

    // Every ranged bot and healer asks for this every tick, so skip the grid scan until Heartbreak: nothing
    // drops a puddle before it.
    std::list<Creature*> voidZones;
    if (IsXT002HeartbreakActive(bot, xt002))
        bot->GetCreatureListWithEntryInGrid(voidZones, PB_NPC_XT002_VOID_ZONE, ULDUAR_XT002_VOID_ZONE_SEARCH_RADIUS);

    // Index into the slots clear of Consumption: every bot sees the same puddles, so they agree on the new
    // layout. A bot left without one keeps its own, and the hazard dodge, which outranks the anchor, moves it.
    std::vector<Position> clear;
    for (size_t i = 0; i < total; ++i)
    {
        Position const candidate = XT002RingSlotPoint(i, total);
        bool blocked = false;
        for (Creature* voidZone : voidZones)
        {
            if (voidZone->GetExactDist2d(candidate.GetPositionX(), candidate.GetPositionY()) <
                ULDUAR_XT002_VOID_ZONE_RADIUS)
            {
                blocked = true;
                break;
            }
        }

        if (!blocked)
            clear.push_back(candidate);
    }

    out = ValidateFloorPoint(bot, slot < clear.size() ? clear[slot] : XT002RingSlotPoint(slot, total));

    return true;
}

// Where the formation says bots belong, not where they are standing: a carrier picks its destination
// while the raid is still walking, so the slots are what it has to miss.
std::vector<Position> GetXT002OtherFormationSlots(Player* bot)
{
    std::vector<Position> slots;
    std::vector<Player*> members;
    if (!BuildXT002RingMembers(bot, members))
        return slots;

    for (size_t i = 0; i < members.size(); ++i)
        if (members[i] != bot)
            slots.push_back(XT002RingSlotPoint(i, members.size()));

    return slots;
}

// Melee get no northern lot: from their stack findSmoothPath refuses everything north of the boss line, and
// the route that does path is longer than a carrier can run in 9 s. Their lot grows east instead.
std::vector<XT002BombLot> GetXT002BombLots(Player* bot)
{
    if (PlayerbotAI::IsMelee(bot))
        return {{ULDUAR_XT002_GRAVITY_BOMB_ORIGIN_MELEE, -1.0f, ULDUAR_XT002_BOMB_GRID_X_CELLS_MELEE}};

    return {{ULDUAR_XT002_GRAVITY_BOMB_ORIGIN_RANGED_SOUTH, -1.0f},
            {ULDUAR_XT002_GRAVITY_BOMB_ORIGIN_RANGED_NORTH, 1.0f}};
}
