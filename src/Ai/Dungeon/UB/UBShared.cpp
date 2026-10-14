/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UBShared.h"
#include "Creature.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <cmath>
#include <list>

namespace
{
float PointSegmentDist2d(float px, float py, float ax, float ay, float bx, float by)
{
    float const dx = bx - ax;
    float const dy = by - ay;
    float const len2 = dx * dx + dy * dy;
    float t = (len2 < 1e-6f) ? 0.0f : ((px - ax) * dx + (py - ay) * dy) / len2;
    t = std::max(0.0f, std::min(1.0f, t));
    return std::hypot(px - (ax + t * dx), py - (ay + t * dy));
}

bool IsMushroomDangerous(Creature* mushroom)
{
    if (!mushroom || !mushroom->IsAlive())
        return false;

    if (mushroom->HasAura(UnderbogHungarfen::SPELL_SPORE_CLOUD))
        return true;

    Aura* grow = mushroom->GetAura(UnderbogHungarfen::SPELL_GROW);
    return grow && grow->GetStackAmount() >= UnderbogHungarfen::GROW_STACKS_DANGER;
}
}

namespace UnderbogHungarfen
{
float MaxEffectRadius(uint32 spellId, float fallback)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return fallback;

    float radius = 0.0f;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        radius = std::max(radius, spellInfo->Effects[i].CalcRadius());

    return radius > 0.0f ? radius : fallback;
}

Unit* HungarfenTarget(AiObjectContext* context)
{
    return AI_VALUE2(Unit*, "find target", "hungarfen");
}

ObjectGuid FindHungarfenGuid(Player* bot)
{
    Creature* hungarfen = bot->FindNearestCreature(NPC_HUNGARFEN, MUSHROOM_SCAN_RANGE, false);
    return hungarfen ? hungarfen->GetGUID() : ObjectGuid::Empty;
}

bool HungarfenGone(Player* bot, ObjectGuid guid)
{
    Creature* hungarfen = ObjectAccessor::GetCreature(*bot, guid);
    return !hungarfen || !hungarfen->IsAlive();
}

float MushroomDangerRange(Player* bot)
{
    return MaxEffectRadius(SPELL_SPORE_CLOUD, SPORE_CLOUD_RADIUS_FALLBACK) + bot->GetCombatReach() +
           SPORE_CLOUD_MARGIN;
}

GuidVector FindMushroomGuids(Player* bot)
{
    std::list<Creature*> mushrooms;
    bot->GetCreatureListWithEntryInGrid(mushrooms, NPC_UNDERBOG_MUSHROOM, MUSHROOM_SCAN_RANGE);

    GuidVector guids;
    for (Creature* mushroom : mushrooms)
    {
        if (mushroom->IsAlive())
            guids.push_back(mushroom->GetGUID());
    }

    return guids;
}

bool IsMushroom(Unit* unit)
{
    return unit && unit->GetEntry() == NPC_UNDERBOG_MUSHROOM;
}

bool AnyMushroomAlive(Player* bot, GuidVector const& mushrooms)
{
    for (ObjectGuid guid : mushrooms)
    {
        Creature* mushroom = ObjectAccessor::GetCreature(*bot, guid);
        if (mushroom && mushroom->IsAlive())
            return true;
    }

    return false;
}

Creature* GetNearestDangerousMushroom(Player* bot, GuidVector const& mushrooms, float range)
{
    Creature* best = nullptr;
    float bestDist = range;
    for (ObjectGuid guid : mushrooms)
    {
        Creature* mushroom = ObjectAccessor::GetCreature(*bot, guid);
        if (!IsMushroomDangerous(mushroom))
            continue;

        float const dist = bot->GetDistance2d(mushroom);
        if (dist <= bestDist)
        {
            bestDist = dist;
            best = mushroom;
        }
    }

    return best;
}

Creature* GetNearestUnderbatInLashRange(Player* bot, GuidVector const& attackers)
{
    Creature* best = nullptr;
    float bestDist = 0.0f;
    for (ObjectGuid guid : attackers)
    {
        Unit* unit = ObjectAccessor::GetUnit(*bot, guid);
        Creature* bat = unit ? unit->ToCreature() : nullptr;
        if (!bat || !bat->IsAlive() || bat->GetEntry() != NPC_UNDERBAT)
            continue;

        if (!bat->IsWithinCombatRange(bot, UNDERBAT_LASH_RANGE + UNDERBAT_LASH_MARGIN))
            continue;

        float const dist = bot->GetDistance2d(bat);
        if (!best || dist < bestDist)
        {
            bestDist = dist;
            best = bat;
        }
    }

    return best;
}

bool AnyUnderbatInLashRange(Player* bot, GuidVector const& attackers)
{
    return GetNearestUnderbatInLashRange(bot, attackers) != nullptr;
}

Creature* GetLashingUnderbat(Player* bot, GuidVector const& attackers)
{
    Creature* best = nullptr;
    float bestDist = 0.0f;
    for (ObjectGuid guid : attackers)
    {
        Unit* unit = ObjectAccessor::GetUnit(*bot, guid);
        Creature* bat = unit ? unit->ToCreature() : nullptr;
        if (!bat || !bat->IsAlive() || bat->GetEntry() != NPC_UNDERBAT)
            continue;

        if (!bat->IsWithinCombatRange(bot, UNDERBAT_LASH_RANGE + UNDERBAT_LASH_MARGIN))
            continue;

        if (bat->HasInArc(UNDERBAT_SAFE_ARC, bot))
            continue;

        float const dist = bot->GetDistance2d(bat);
        if (!best || dist < bestDist)
        {
            bestDist = dist;
            best = bat;
        }
    }

    return best;
}

Unit* UnderbatRallyUnit(Player* bot, GuidVector const& attackers)
{
    Creature* bat = GetNearestUnderbatInLashRange(bot, attackers);
    if (!bat)
        return nullptr;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive() ||
                member->GetMapId() != bot->GetMapId())
                continue;

            if (!PlayerbotAI::IsTank(member))
                continue;

            if (bot->GetExactDist(member) > UNDERBAT_RALLY_MAX_RANGE)
                continue;

            return member;
        }
    }

    Unit* victim = bat->GetVictim();
    if (victim && victim != bot && bot->GetExactDist(victim) <= UNDERBAT_RALLY_MAX_RANGE)
        return victim;

    return nullptr;
}

bool SpotClearOfUnderbats(Player* bot, GuidVector const& attackers, float x, float y, float z)
{
    Position const spot(x, y, z);
    for (ObjectGuid guid : attackers)
    {
        Unit* unit = ObjectAccessor::GetUnit(*bot, guid);
        Creature* bat = unit ? unit->ToCreature() : nullptr;
        if (!bat || !bat->IsAlive() || bat->GetEntry() != NPC_UNDERBAT)
            continue;

        float const window =
            UNDERBAT_LASH_RANGE + UNDERBAT_LASH_MARGIN + bat->GetCombatReach() + bot->GetCombatReach();
        if (bat->GetExactDist(x, y, z) > window)
            continue;

        if (!bat->HasInArc(UNDERBAT_SAFE_ARC, &spot))
            return false;
    }

    return true;
}

bool RetreatPathUnsafe(Player* bot, GuidVector const& mushrooms, float destX, float destY)
{
    float const dangerRange = MushroomDangerRange(bot);
    float const startX = bot->GetPositionX();
    float const startY = bot->GetPositionY();

    for (ObjectGuid guid : mushrooms)
    {
        Creature* mushroom = ObjectAccessor::GetCreature(*bot, guid);
        if (!mushroom || !mushroom->IsAlive())
            continue;

        if (mushroom->GetExactDist2d(destX, destY) < dangerRange)
            return true;

        if (IsMushroomDangerous(mushroom) &&
            PointSegmentDist2d(mushroom->GetPositionX(), mushroom->GetPositionY(), startX, startY, destX, destY) <
                dangerRange)
            return true;
    }

    return false;
}
}
