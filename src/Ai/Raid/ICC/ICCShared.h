/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ICCSHARED_H
#define PLAYERBOTS_ICCSHARED_H

#include "Playerbots.h"
#include "RtiTargetValue.h"
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <optional>
#include <vector>

class Action;

struct IccBucketMarker
{
    int8 icon;
    char const* rti;
};

inline constexpr IccBucketMarker ICC_BUCKET_MARKERS[3] = {
    {RtiTargetValue::skullIndex, "skull"},
    {RtiTargetValue::crossIndex, "cross"},
    {RtiTargetValue::starIndex, "star"},
};

inline char const* IccRtiNameForBucket(size_t bucket)
{
    return ICC_BUCKET_MARKERS[bucket < 3 ? bucket : 0].rti;
}

inline bool IccStepToward(Player* bot, Position const& dest, float stepSize, float& outX, float& outY)
{
    float const dx = dest.GetPositionX() - bot->GetPositionX();
    float const dy = dest.GetPositionY() - bot->GetPositionY();
    float const len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.001f)
        return false;

    outX = bot->GetPositionX() + (dx / len) * stepSize;
    outY = bot->GetPositionY() + (dy / len) * stepSize;
    return true;
}

inline std::vector<size_t> IccBalancedGroupSizes(size_t total, size_t buckets)
{
    std::vector<size_t> sizes(buckets, 0);
    if (buckets == 0)
        return sizes;

    size_t const base = total / buckets;
    size_t const remainder = total % buckets;
    for (size_t i = 0; i < buckets; ++i)
    {
        sizes[i] = base;
        if (i < remainder)
            ++sizes[i];
    }

    return sizes;
}

inline size_t IccAssignedBucketIndex(size_t memberIndex, std::vector<size_t> const& sizes)
{
    size_t cursor = 0;
    for (size_t i = 0; i < sizes.size(); ++i)
    {
        if (memberIndex < cursor + sizes[i])
            return i;
        cursor += sizes[i];
    }

    return 0;
}

struct IccGuidLess
{
    bool operator()(WorldObject const* a, WorldObject const* b) const { return a->GetGUID() < b->GetGUID(); }
};

bool IccCastClassTaunt(Player* bot, PlayerbotAI* botAI, Unit* target);
std::optional<bool> IccTryClassCC(Player* bot, PlayerbotAI* botAI, Unit* target);
void IccEnsureIconOn(Player* bot, PlayerbotAI* botAI, int8 icon, Unit* target);
void IccApplyHeroicBuffToMember(Player* member, bool applyPainSupp, bool applyNoThreat);
void IccStripExperienced(Player* bot);
bool IccBossCastingBlisteringCold(Unit* boss);
bool IccBossHasRemorselessWinter(Unit* boss);
bool IccBossCastingRemorselessWinter(Unit* boss);
bool IccAnyGroupMemberHasAura(Player* bot, uint32 spellId);
bool IccAnyBloodPrincePresent(PlayerbotAI* botAI);
std::optional<bool> IccValithriaShouldHealRaid(Player* bot);
std::vector<Creature*> IccGetCreaturesByEntries(WorldObject* searcher, std::initializer_list<uint32> entries,
                                                float range);

#endif
