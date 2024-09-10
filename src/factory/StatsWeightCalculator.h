/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GEARSCORECALCULATOR_H
#define _PLAYERBOT_GEARSCORECALCULATOR_H

#include "Player.h"
#include "StatsCollector.h"

#define ITEM_SUBCLASS_MASK_SINGLE_HAND                                                                        \
    ((1 << ITEM_SUBCLASS_WEAPON_AXE) | (1 << ITEM_SUBCLASS_WEAPON_MACE) | (1 << ITEM_SUBCLASS_WEAPON_SWORD) | \
     (1 << ITEM_SUBCLASS_WEAPON_DAGGER) | (1 << ITEM_SUBCLASS_WEAPON_FIST))

enum StatsOverflowThreshold
{
    SPELL_HIT_OVERFLOW = 14,
    MELEE_HIT_OVERFLOW = 8,
    RANGED_HIT_OVERFLOW = 8,
    EXPERTISE_OVERFLOW = 26,
    DEFENSE_OVERFLOW = 140,
    ARMOR_PENETRATION_OVERFLOW = 100
};

class StatsWeightCalculator
{
public:
    StatsWeightCalculator(Player* player);
    void Reset();
    float CalculateItem(uint32 itemId);
    float CalculateEnchant(uint32 enchantId);

    void SetOverflowPenalty(bool apply) { enable_overflow_penalty_ = apply; }
    void SetItemSetBonus(bool apply) { enable_item_set_bonus_ = apply; }
    void SetQualityBlend(bool apply) { enable_quality_blend_ = apply; }

private:
    void GenerateWeights(Player* player);
    void GenerateBasicWeights(Player* player);
    void GenerateAdditionalWeights(Player* player);

    void CalculateItemSetBonus(Player* player, ItemTemplate const* proto);
    void CalculateSocketBonus(Player* player, ItemTemplate const* proto);

    void CalculateItemTypePenalty(ItemTemplate const* proto);

    bool NotBestArmorType(uint32 item_subclass_armor);

    void ApplyOverflowPenalty(Player* player);
    void ApplyWeightFinetune(Player* player);

private:
    Player* player_;
    CollectorType type_;
    CollectorType hitOverflowType_;
    std::unique_ptr<StatsCollector> collector_;
    uint8 cls;
    int tab;
    bool enable_overflow_penalty_;
    bool enable_item_set_bonus_;
    bool enable_quality_blend_;

    float weight_;
    float stats_weights_[STATS_TYPE_MAX];
};

#endif
