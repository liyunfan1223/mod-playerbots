/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_STATSWEIGHTCALCULATOR_H
#define PLAYERBOTS_STATSWEIGHTCALCULATOR_H

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
    float CalculateItem(uint32 itemId, int32 randomPropertyId = 0, int32 slot = -1);
    float CalculateEnchant(uint32 enchantId);
    int32 PickBestRandomPropertyId(uint32 itemId);

    void SetOverflowPenalty(bool apply) { enable_overflow_penalty_ = apply; }
    void SetItemSetBonus(bool apply) { enable_item_set_bonus_ = apply; }
    void SetPvpSpec(bool isPvp) { pvpSpec_ = isPvp; }
    void SetExcludeResilience(bool exclude) { exclude_resilience_ = exclude; }

    private:
    void GenerateWeights(Player* player);
    void GenerateBasicWeights(Player* player);
    void GenerateAdditionalWeights(Player* player);

    void CalculateRandomProperty(int32 randomPropertyId, uint32 itemId);
    void CalculateItemSetMod(Player* player, ItemTemplate const* proto);
    void CalculateSocketBonus(Player* player, ItemTemplate const* proto);

    void CalculateItemTypePenalty(ItemTemplate const* proto);
    float ApplyPreferredSpecWeapons(ItemTemplate const* proto, int32 slot);

    bool NotBestArmorType(uint32 item_subclass_armor);

    void ApplyOverflowPenalty(Player* player);
    void ApplyWeightFinetune(Player* player);

private:
    Player* player_;
    CollectorType type_;
    CollectorType hitOverflowType_;
    std::unique_ptr<StatsCollector> collector_;
    uint8 cls;
    uint8 lvl;
    int tab;
    bool enable_overflow_penalty_;
    bool enable_item_set_bonus_;

    float weight_;
    float stats_weights_[STATS_TYPE_MAX];
    bool pvpSpec_ = false;
    bool exclude_resilience_ = false;
};

#endif
