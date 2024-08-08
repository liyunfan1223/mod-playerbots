/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_STATSCOLLECTOR_H
#define _PLAYERBOT_STATSCOLLECTOR_H

#include "ItemTemplate.h"
#include "SpellInfo.h"

enum StatsType : uint8
{
    // Basic stats
    STATS_TYPE_AGILITY = 0,
    STATS_TYPE_STRENGTH,
    STATS_TYPE_INTELLECT,
    STATS_TYPE_SPIRIT,
    STATS_TYPE_STAMINA,
    STATS_TYPE_HIT,
    STATS_TYPE_CRIT,
    STATS_TYPE_HASTE,
    // Stats for tank
    STATS_TYPE_ARMOR,
    STATS_TYPE_DEFENSE,
    STATS_TYPE_DODGE,
    STATS_TYPE_PARRY,
    STATS_TYPE_BLOCK_VALUE,
    STATS_TYPE_BLOCK_RATING,
    STATS_TYPE_RESILIENCE,
    STATS_TYPE_HEALTH_REGENERATION,
    // Stats for spell damage and heal
    STATS_TYPE_SPELL_POWER,
    STATS_TYPE_SPELL_PENETRATION,
    STATS_TYPE_MANA_REGENERATION,
    // Stats for physical damage and melee
    STATS_TYPE_ATTACK_POWER,
    STATS_TYPE_ARMOR_PENETRATION,
    STATS_TYPE_EXPERTISE,
    // Stats for weapon dps
    STATS_TYPE_MELEE_DPS,
    STATS_TYPE_RANGED_DPS,
    STATS_TYPE_MAX = 24
};

enum class CollectorType
{
    MELEE,
    RANGED,
    SPELL
};

class StatsCollector
{
public:
    StatsCollector(CollectorType type);
    StatsCollector(StatsCollector& stats) = default;
    void Reset();
    void CollectItemStats(ItemTemplate const* proto);
    void CollectSpellStats(uint32 spellId, uint32 trigger = ITEM_SPELLTRIGGER_ON_EQUIP);
    void CollectSpellEffectStats(const SpellEffectInfo& effectInfo, float multiplier = 1.0f);
    void CollectEnchantStats(SpellItemEnchantmentEntry const* enchant);

public:
    uint32 stats[STATS_TYPE_MAX];

private:
    void CollectByItemStatType(uint32 itemStatType, int32 val);
    bool CollectSpecialCaseSpellStats(uint32 spellId);
    bool CollectSpecialEnchantSpellStats(uint32 enchantSpellId);

private:
    CollectorType type_;
};

#endif