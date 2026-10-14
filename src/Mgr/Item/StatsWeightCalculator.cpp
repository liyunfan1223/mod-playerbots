/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "StatsWeightCalculator.h"
#include "AiFactory.h"
#include "DBCStores.h"
#include "ItemEnchantmentMgr.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "PlayerbotAI.h"
#include "PlayerbotFactory.h"
#include "RandomItemMgr.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "SpellMgr.h"
#include "StatsCollector.h"
#include "Unit.h"
#include <memory>

namespace
{
constexpr uint32 SPELL_MOLTEN_ARMOR_RANKS[] = { 30482, 43045, 43046 };
constexpr uint32 SPELL_FEL_ARMOR_RANKS[] = { 28176, 28189, 47892, 47893 };
constexpr uint32 SPELL_CAREFUL_AIM = 34484;
constexpr uint32 SPELL_HUNTER_VS_WILD = 56341;
constexpr uint32 SPELL_ARMORED_TO_THE_TEETH = 61222;
constexpr uint32 SPELL_MENTAL_DEXTERITY = 51885;
constexpr uint32 SPELL_ROGUE_SWORD_SPECIALIZATION = 13964;
constexpr uint32 SPELL_POLEAXE_SPECIALIZATION = 12785;
constexpr uint32 SPELL_NERVES_OF_COLD_STEEL = 50138;
constexpr uint32 SPELL_SHADOW_FOCUS = 15835;
constexpr uint32 SPELL_ARCANE_FOCUS = 12840;
}

template <size_t Size>
bool HasAnySpell(Player* player, uint32 const (&spellIds)[Size])
{
    for (uint32 const spellId : spellIds)
    {
        if (player->HasSpell(spellId))
            return true;
    }

    return false;
}

StatsWeightCalculator::StatsWeightCalculator(Player* player) : player_(player)
{
    if (PlayerbotAI::IsHeal(player))
        type_ = CollectorType::SPELL_HEAL;
    else if (PlayerbotAI::IsCaster(player))
        type_ = CollectorType::SPELL_DMG;
    else if (PlayerbotAI::IsTank(player))
        type_ = CollectorType::MELEE_TANK;
    else if (PlayerbotAI::IsMelee(player))
        type_ = CollectorType::MELEE_DMG;
    else
        type_ = CollectorType::RANGED;
    cls = player->getClass();
    lvl = player->GetLevel();
    tab = AiFactory::GetPlayerSpecTab(player);
    collector_ = std::make_unique<StatsCollector>(type_, cls, lvl);

    if (cls == CLASS_DEATH_KNIGHT && tab == DEATH_KNIGHT_TAB_UNHOLY)
        hitOverflowType_ = CollectorType::SPELL;
    else if (cls == CLASS_SHAMAN && tab == SHAMAN_TAB_ENHANCEMENT)
        hitOverflowType_ = CollectorType::SPELL;
    else if (cls == CLASS_ROGUE)
        hitOverflowType_ = CollectorType::SPELL;
    else
        hitOverflowType_ = type_;

    enable_overflow_penalty_ = true;
    enable_item_set_bonus_ = true;
}

void StatsWeightCalculator::Reset()
{
    collector_->Reset();
    weight_ = 0;
    for (uint32 i = 0; i < STATS_TYPE_MAX; i++)
    {
        stats_weights_[i] = 0;
    }
}

float StatsWeightCalculator::CalculateItem(uint32 itemId, int32 randomPropertyIds, int32 slot)
{
    ItemTemplate const* proto = &sObjectMgr->GetItemTemplateStore()->at(itemId);

    if (!proto)
        return 0.0f;

    Reset();

    collector_->CollectItemStats(proto);

    if (randomPropertyIds != 0)
        CalculateRandomProperty(randomPropertyIds, itemId);

    if (enable_overflow_penalty_)
        ApplyOverflowPenalty(player_);

    GenerateWeights(player_);
    for (uint32 i = 0; i < STATS_TYPE_MAX; i++)
    {
        weight_ += stats_weights_[i] * collector_->stats[i];
    }

    CalculateItemTypePenalty(proto);

    if (enable_item_set_bonus_)
        CalculateItemSetMod(player_, proto);

    CalculateSocketBonus(player_, proto);

    // Apply weapon speed governance if slot is provided and this is a weapon
    if (sPlayerbotAIConfig.PreferredSpecWeapons && slot >= 0 && proto->Class == ITEM_CLASS_WEAPON)
        weight_ *= ApplyPreferredSpecWeapons(proto, slot);

    return weight_;
}

float StatsWeightCalculator::CalculateEnchant(uint32 enchantId)
{
    SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchantId);

    if (!enchant)
        return 0.0f;

    Reset();

    collector_->CollectEnchantStats(enchant);

    if (enable_overflow_penalty_)
        ApplyOverflowPenalty(player_);

    GenerateWeights(player_);
    for (uint32 i = 0; i < STATS_TYPE_MAX; i++)
    {
        weight_ += stats_weights_[i] * collector_->stats[i];
    }

    return weight_;
}

void StatsWeightCalculator::CalculateRandomProperty(int32 randomPropertyId, uint32 itemId)
{
    if (randomPropertyId > 0)
    {
        ItemRandomPropertiesEntry const* item_rand = sItemRandomPropertiesStore.LookupEntry(randomPropertyId);
        if (!item_rand)
        {
            return;
        }

        for (uint32 i = PROP_ENCHANTMENT_SLOT_0; i < MAX_ENCHANTMENT_SLOT; ++i)
        {
            uint32 enchantId = item_rand->Enchantment[i - PROP_ENCHANTMENT_SLOT_0];
            SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchantId);
            if (enchant)
                collector_->CollectEnchantStats(enchant);
        }
    }
    else
    {
        ItemRandomSuffixEntry const* item_rand = sItemRandomSuffixStore.LookupEntry(-randomPropertyId);
        if (!item_rand)
        {
            return;
        }

        for (uint32 i = PROP_ENCHANTMENT_SLOT_0; i < MAX_ENCHANTMENT_SLOT; ++i)
        {
            uint32 enchantId = item_rand->Enchantment[i - PROP_ENCHANTMENT_SLOT_0];
            SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchantId);
            uint32 enchant_amount = 0;

            for (int k = 0; k < MAX_ITEM_ENCHANTMENT_EFFECTS; ++k)
            {
                if (item_rand->Enchantment[k] == enchantId)
                {
                    enchant_amount = uint32((item_rand->AllocationPct[k] * GenerateEnchSuffixFactor(itemId)) / 10000);
                    break;
                }
            }

            if (enchant)
                collector_->CollectEnchantStats(enchant, enchant_amount);
        }
    }
}

int32 StatsWeightCalculator::PickBestRandomPropertyId(uint32 itemId)
{
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return 0;

    bool isSuffix = false;
    uint32 poolEntry = proto->RandomProperty;
    if (!poolEntry)
    {
        poolEntry = proto->RandomSuffix;
        isSuffix = true;
    }
    if (!poolEntry)
        return 0;

    std::vector<uint32> const& pool = sRandomItemMgr.GetEnchantmentPool(poolEntry);
    if (pool.empty())
        return 0;

    Reset();
    GenerateWeights(player_);

    int32 bestId = 0;
    float bestScore = 0.0f;
    for (uint32 enchId : pool)
    {
        int32 candidate = isSuffix ? -static_cast<int32>(enchId) : static_cast<int32>(enchId);

        collector_->Reset();
        CalculateRandomProperty(candidate, itemId);

        float score = 0.0f;
        for (uint32 i = 0; i < STATS_TYPE_MAX; ++i)
            score += stats_weights_[i] * collector_->stats[i];

        if (bestId == 0 || score > bestScore)
        {
            bestId = candidate;
            bestScore = score;
        }
    }

    collector_->Reset();
    return bestId;
}

void StatsWeightCalculator::GenerateWeights(Player* player)
{
    GenerateBasicWeights(player);
    GenerateAdditionalWeights(player);
    ApplyWeightFinetune(player);
}

void StatsWeightCalculator::GenerateBasicWeights(Player* player)
{
    // Basic weights
    stats_weights_[STATS_TYPE_STAMINA] += 0.1f;
    stats_weights_[STATS_TYPE_ARMOR] += 0.001f;
    stats_weights_[STATS_TYPE_BONUS] += 1.0f;
    stats_weights_[STATS_TYPE_MELEE_DPS] += 0.01f;
    stats_weights_[STATS_TYPE_RANGED_DPS] += 0.01f;

    if (cls == CLASS_HUNTER && (tab == HUNTER_TAB_BEAST_MASTERY || tab == HUNTER_TAB_SURVIVAL))
    {
        stats_weights_[STATS_TYPE_AGILITY] += 2.5f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 1.5f;
        stats_weights_[STATS_TYPE_HIT] += 1.7f;
        stats_weights_[STATS_TYPE_CRIT] += 1.4f;
        stats_weights_[STATS_TYPE_HASTE] += 1.6f;
        stats_weights_[STATS_TYPE_SPELL_POWER] -= 1.0f;
        stats_weights_[STATS_TYPE_RANGED_DPS] += 7.5f;
    }
    else if (cls == CLASS_HUNTER && tab == HUNTER_TAB_MARKSMANSHIP)
    {
        stats_weights_[STATS_TYPE_AGILITY] += 2.3f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 2.25f;
        stats_weights_[STATS_TYPE_HIT] += 2.1f;
        stats_weights_[STATS_TYPE_CRIT] += 2.0f;
        stats_weights_[STATS_TYPE_HASTE] += 1.8f;
        stats_weights_[STATS_TYPE_SPELL_POWER] -= 1.0f;
        stats_weights_[STATS_TYPE_RANGED_DPS] += 10.0f;
    }
    else if (cls == CLASS_ROGUE && tab == ROGUE_TAB_COMBAT)
    {
        stats_weights_[STATS_TYPE_AGILITY] += 1.9f;
        stats_weights_[STATS_TYPE_STRENGTH] += 1.1f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 1.8f;
        stats_weights_[STATS_TYPE_HIT] += 2.1f;
        stats_weights_[STATS_TYPE_CRIT] += 1.4f;
        stats_weights_[STATS_TYPE_HASTE] += 1.7f;
        stats_weights_[STATS_TYPE_SPELL_POWER] -= 1.0f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 2.0f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 7.0f;
    }
    else if (cls == CLASS_DRUID && tab == DRUID_TAB_FERAL && !PlayerbotAI::IsTank(player))
    {
        stats_weights_[STATS_TYPE_AGILITY] += 2.2f;
        stats_weights_[STATS_TYPE_STRENGTH] += 2.4f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 2.3f;
        stats_weights_[STATS_TYPE_HIT] += 1.9f;
        stats_weights_[STATS_TYPE_CRIT] += 1.5f;
        stats_weights_[STATS_TYPE_HASTE] += 2.1f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 2.1f;
        // Weapon DPS is taken into account as part of Attack Power for Cat Druids.
    }
    else if (cls == CLASS_ROGUE && (tab == ROGUE_TAB_ASSASSINATION || tab == ROGUE_TAB_SUBTLETY))
    {
        stats_weights_[STATS_TYPE_AGILITY] += 1.5f;
        stats_weights_[STATS_TYPE_STRENGTH] += 1.1f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 1.2f;
        stats_weights_[STATS_TYPE_HIT] += 2.1f;
        stats_weights_[STATS_TYPE_CRIT] += 1.1f;
        stats_weights_[STATS_TYPE_HASTE] += 1.8f;
        stats_weights_[STATS_TYPE_SPELL_POWER] -= 1.0f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 2.1f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 5.0f;
    }
    else if (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_FURY)
    {
        stats_weights_[STATS_TYPE_AGILITY] += 0.8f;
        stats_weights_[STATS_TYPE_STRENGTH] += 2.5f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 0.8f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 2.1f;
        stats_weights_[STATS_TYPE_HIT] += 2.3f;
        stats_weights_[STATS_TYPE_CRIT] += 2.2f;
        stats_weights_[STATS_TYPE_HASTE] += 0.8f;
        stats_weights_[STATS_TYPE_SPELL_POWER] -= 2.0f;
        stats_weights_[STATS_TYPE_DEFENSE] -= 1.0f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 2.5f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 7.0f;
    }
    else if (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_ARMS)
    {
        stats_weights_[STATS_TYPE_AGILITY] += 0.8f;
        stats_weights_[STATS_TYPE_STRENGTH] += 2.5f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 0.8f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 1.7f;
        stats_weights_[STATS_TYPE_HIT] += 2.0f;
        stats_weights_[STATS_TYPE_CRIT] += 1.9f;
        stats_weights_[STATS_TYPE_HASTE] += 0.8f;
        stats_weights_[STATS_TYPE_SPELL_POWER] -= 2.0f;
        stats_weights_[STATS_TYPE_DEFENSE] -= 1.0f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 1.4f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 7.0f;
    }
    else if (cls == CLASS_DEATH_KNIGHT && tab == DEATH_KNIGHT_TAB_FROST)
    {
        stats_weights_[STATS_TYPE_AGILITY] += 0.5f;
        stats_weights_[STATS_TYPE_STRENGTH] += 2.5f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 0.5f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 2.7f;
        stats_weights_[STATS_TYPE_HIT] += 2.3f;
        stats_weights_[STATS_TYPE_CRIT] += 2.2f;
        stats_weights_[STATS_TYPE_HASTE] += 2.1f;
        stats_weights_[STATS_TYPE_SPELL_POWER] -= 1.0f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 2.5f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 7.0f;
    }
    else if (cls == CLASS_DEATH_KNIGHT && tab == DEATH_KNIGHT_TAB_UNHOLY)
    {
        stats_weights_[STATS_TYPE_AGILITY] += 0.5f;
        stats_weights_[STATS_TYPE_STRENGTH] += 2.5f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 0.5f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 1.3f;
        stats_weights_[STATS_TYPE_HIT] += 2.2f;
        stats_weights_[STATS_TYPE_CRIT] += 1.7f;
        stats_weights_[STATS_TYPE_HASTE] += 1.8f;
        stats_weights_[STATS_TYPE_SPELL_POWER] -= 1.0f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 1.5f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 5.0f;
    }
    else if (cls == CLASS_PALADIN && tab == PALADIN_TAB_RETRIBUTION)
    {
        stats_weights_[STATS_TYPE_AGILITY] += 0.5f;
        stats_weights_[STATS_TYPE_STRENGTH] += 2.5f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 0.5f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 1.5f;
        stats_weights_[STATS_TYPE_HIT] += 1.9f;
        stats_weights_[STATS_TYPE_CRIT] += 1.7f;
        stats_weights_[STATS_TYPE_HASTE] += 1.6f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 2.0f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 9.0f;
    }
    else if ((cls == CLASS_SHAMAN && tab == SHAMAN_TAB_ENHANCEMENT))
    {
        stats_weights_[STATS_TYPE_AGILITY] += 1.4f;
        stats_weights_[STATS_TYPE_STRENGTH] += 1.1f;
        stats_weights_[STATS_TYPE_INTELLECT] += 0.3f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_SPELL_POWER] += 0.5f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 0.9f;
        stats_weights_[STATS_TYPE_HIT] += 2.1f;
        stats_weights_[STATS_TYPE_CRIT] += 1.5f;
        stats_weights_[STATS_TYPE_HASTE] += 1.8f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 2.0f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 8.5f;
    }
    else if (cls == CLASS_WARLOCK ||
             (cls == CLASS_MAGE && tab != MAGE_TAB_FIRE) ||
             (cls == CLASS_PRIEST && tab == PRIEST_TAB_SHADOW) ||
             (cls == CLASS_DRUID && tab == DRUID_TAB_BALANCE))
    {
        stats_weights_[STATS_TYPE_INTELLECT] += 0.3f;
        stats_weights_[STATS_TYPE_SPIRIT] += 0.6f;
        stats_weights_[STATS_TYPE_SPELL_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_HIT] += 1.1f;
        stats_weights_[STATS_TYPE_CRIT] += 0.8f;
        stats_weights_[STATS_TYPE_HASTE] += 1.0f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] -= 1.0f;
        stats_weights_[STATS_TYPE_RANGED_DPS] += 1.0f;
    }
    else if (cls == CLASS_MAGE && tab == MAGE_TAB_FIRE)
    {
        stats_weights_[STATS_TYPE_INTELLECT] += 0.3f;
        stats_weights_[STATS_TYPE_SPIRIT] += 0.7f;
        stats_weights_[STATS_TYPE_SPELL_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_HIT] += 1.2f;
        stats_weights_[STATS_TYPE_CRIT] += 1.1f;
        stats_weights_[STATS_TYPE_HASTE] += 0.8f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] -= 1.0f;
        stats_weights_[STATS_TYPE_RANGED_DPS] += 1.0f;
    }
    else if (cls == CLASS_SHAMAN && tab == SHAMAN_TAB_ELEMENTAL)
    {
        stats_weights_[STATS_TYPE_INTELLECT] += 0.5f;
        stats_weights_[STATS_TYPE_SPELL_POWER] += 1.2f;
        stats_weights_[STATS_TYPE_HIT] += 1.1f;
        stats_weights_[STATS_TYPE_CRIT] += 0.8f;
        stats_weights_[STATS_TYPE_HASTE] += 1.0f;
        stats_weights_[STATS_TYPE_MANA_REGENERATION] += 0.5f;
    }
    else if ((cls == CLASS_PALADIN && tab == PALADIN_TAB_HOLY) ||
             (cls == CLASS_SHAMAN && tab == SHAMAN_TAB_RESTORATION))
    {
        stats_weights_[STATS_TYPE_INTELLECT] += 0.9f;
        stats_weights_[STATS_TYPE_SPIRIT] += 0.15f;
        stats_weights_[STATS_TYPE_HEAL_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_MANA_REGENERATION] += 0.9f;
        stats_weights_[STATS_TYPE_CRIT] += 0.6f;
        stats_weights_[STATS_TYPE_HASTE] += 0.8f;
    }
    else if ((cls == CLASS_PRIEST && tab != PRIEST_TAB_SHADOW) ||
             (cls == CLASS_DRUID && tab == DRUID_TAB_RESTORATION))
    {
        stats_weights_[STATS_TYPE_INTELLECT] += 0.8f;
        stats_weights_[STATS_TYPE_SPIRIT] += 0.6f;
        stats_weights_[STATS_TYPE_HEAL_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_MANA_REGENERATION] += 0.9f;
        stats_weights_[STATS_TYPE_CRIT] += 0.6f;
        stats_weights_[STATS_TYPE_HASTE] += 0.8f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] -= 1.0f;
        stats_weights_[STATS_TYPE_RANGED_DPS] += 1.0f;
    }
    else if ((cls == CLASS_WARRIOR && tab == WARRIOR_TAB_PROTECTION) ||
             (cls == CLASS_PALADIN && tab == PALADIN_TAB_PROTECTION))
    {
        stats_weights_[STATS_TYPE_AGILITY] += 0.2f;
        stats_weights_[STATS_TYPE_STRENGTH] += 1.3f;
        stats_weights_[STATS_TYPE_STAMINA] += 3.0f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 0.2f;
        stats_weights_[STATS_TYPE_DEFENSE] += 2.5f;
        stats_weights_[STATS_TYPE_PARRY] += 2.0f;
        stats_weights_[STATS_TYPE_DODGE] += 2.0f;
        // stats_weights_[STATS_TYPE_RESILIENCE] += 2.0f;
        stats_weights_[STATS_TYPE_BLOCK_RATING] += 1.0f;
        stats_weights_[STATS_TYPE_BLOCK_VALUE] += 0.5f;
        stats_weights_[STATS_TYPE_ARMOR] += 0.15f;
        stats_weights_[STATS_TYPE_HIT] += 2.0f;
        stats_weights_[STATS_TYPE_SPELL_POWER] -= 2.0f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 3.0f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 2.0f;
    }
    else if (cls == CLASS_DEATH_KNIGHT && tab == DEATH_KNIGHT_TAB_BLOOD)
    {
        stats_weights_[STATS_TYPE_AGILITY] += 0.2f;
        stats_weights_[STATS_TYPE_STRENGTH] += 1.3f;
        stats_weights_[STATS_TYPE_STAMINA] += 3.0f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 0.2f;
        stats_weights_[STATS_TYPE_DEFENSE] += 2.5f;
        stats_weights_[STATS_TYPE_PARRY] += 2.0f;
        stats_weights_[STATS_TYPE_DODGE] += 2.0f;
        stats_weights_[STATS_TYPE_BLOCK_RATING] -= 2.0f;
        stats_weights_[STATS_TYPE_BLOCK_VALUE] -= 2.0f;
        // stats_weights_[STATS_TYPE_RESILIENCE] += 2.0f;
        stats_weights_[STATS_TYPE_ARMOR] += 0.15f;
        stats_weights_[STATS_TYPE_HIT] += 2.0f;
        stats_weights_[STATS_TYPE_SPELL_POWER] -= 1.0f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 3.0f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 2.0f;
    }
    else
    {
        // BEAR DRUID TANK
        stats_weights_[STATS_TYPE_AGILITY] += 2.2f;
        stats_weights_[STATS_TYPE_STRENGTH] += 2.4f;
        stats_weights_[STATS_TYPE_STAMINA] += 4.0f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_DEFENSE] += 0.3f;
        stats_weights_[STATS_TYPE_DODGE] += 0.7f;
        // stats_weights_[STATS_TYPE_RESILIENCE] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR] += 0.15f;
        stats_weights_[STATS_TYPE_HIT] += 3.0f;
        stats_weights_[STATS_TYPE_CRIT] += 1.3f;
        stats_weights_[STATS_TYPE_HASTE] += 2.3f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 3.7f;
        // Weapon DPS is taken into account as part of Attack Power for Bear Druids.
    }
}

void StatsWeightCalculator::GenerateAdditionalWeights(Player* player)
{
    uint8 cls = player->getClass();
    // int tab = AiFactory::GetPlayerSpecTab(player);
    if (cls == CLASS_HUNTER)
    {
        if (player->HasAura(SPELL_CAREFUL_AIM))
            stats_weights_[STATS_TYPE_INTELLECT] += 1.1f;
        if (player->HasAura(SPELL_HUNTER_VS_WILD))
            stats_weights_[STATS_TYPE_STAMINA] += 0.3f;
    }
    else if (cls == CLASS_WARRIOR)
    {
        if (player->HasAura(SPELL_ARMORED_TO_THE_TEETH))
            stats_weights_[STATS_TYPE_ARMOR] += 0.03f;
    }
    else if (cls == CLASS_SHAMAN)
    {
        if (player->HasAura(SPELL_MENTAL_DEXTERITY))
            stats_weights_[STATS_TYPE_INTELLECT] += 1.1f;
    }
    else if (cls == CLASS_MAGE)
    {
        if (!HasAnySpell(player, SPELL_MOLTEN_ARMOR_RANKS))
        {
            if (tab != MAGE_TAB_FIRE)
                stats_weights_[STATS_TYPE_SPIRIT] -= 0.6f;
            else
                stats_weights_[STATS_TYPE_SPIRIT] -= 0.7f;
        }
    }
    else if (cls == CLASS_WARLOCK)
    {
        if (!HasAnySpell(player, SPELL_FEL_ARMOR_RANKS))
            stats_weights_[STATS_TYPE_SPIRIT] -= 0.4f;
    }

    if (pvpSpec_ && !exclude_resilience_)
        stats_weights_[STATS_TYPE_RESILIENCE] += 7.0f;
}

void StatsWeightCalculator::CalculateItemSetMod(Player* player, ItemTemplate const* proto)
{
    uint32 itemSet = proto->ItemSet;
    if (!itemSet)
        return;

    float multiplier = 1.0f;
    size_t i = 0;
    for (i = 0; i < player->ItemSetEff.size(); i++)
    {
        if (player->ItemSetEff[i])
        {
            ItemSetEffect* eff = player->ItemSetEff[i];

            uint32 setId = eff->setid;
            if (itemSet != setId)
                continue;

            ItemSetEntry const* setEntry = sItemSetStore.LookupEntry(setId);
            if (!setEntry)
                continue;

            uint32 itemCount = eff->item_count;
            uint32 max_items = 0;
            for (size_t j = 0; j < MAX_ITEM_SET_SPELLS; j++)
                max_items = std::max(max_items, setEntry->items_to_triggerspell[j]);
            if (itemCount < max_items)
            {
                multiplier += 0.1f * itemCount;  // 10% bonus for each item already equipped
            }
            else
            {
                multiplier = 1.0f;  // All item set effect has been triggerred
            }
            break;
        }
    }

    if (i == player->ItemSetEff.size())
        multiplier = 1.05f;  // this is the first item in the item set

    weight_ *= multiplier;
}

void StatsWeightCalculator::CalculateSocketBonus(Player* /*player*/, ItemTemplate const* proto)
{
    uint32 socketNum = 0;
    for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT + MAX_GEM_SOCKETS;
         ++enchant_slot)
    {
        uint8 socketColor = proto->Socket[enchant_slot - SOCK_ENCHANTMENT_SLOT].Color;

        if (!socketColor)  // no socket slot
            continue;

        socketNum++;
    }

    float multiplier = 1.0f + socketNum * 0.03f;  // 3% bonus for socket

    weight_ *= multiplier;
}

void StatsWeightCalculator::CalculateItemTypePenalty(ItemTemplate const* proto)
{
    // // penalty for different type armor
    // if (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass >= ITEM_SUBCLASS_ARMOR_CLOTH &&
    //     proto->SubClass <= ITEM_SUBCLASS_ARMOR_PLATE && NotBestArmorType(proto->SubClass))
    // {
    //     weight_ *= 1.0;
    // }
    if (proto->Class == ITEM_CLASS_WEAPON)
    {
        // double hand
        bool isDoubleHand = proto->Class == ITEM_CLASS_WEAPON &&
                            !(ITEM_SUBCLASS_MASK_SINGLE_HAND & (1 << proto->SubClass)) &&
                            !(ITEM_SUBCLASS_MASK_WEAPON_RANGED & (1 << proto->SubClass));

        if (isDoubleHand)
        {
            weight_ *= 0.5;
            // spec without double hand
            // enhancement, rogue, ice dk, unholy dk, shield tank, fury warrior without titan's grip but with duel wield
            if (((cls == CLASS_SHAMAN && tab == SHAMAN_TAB_ENHANCEMENT && player_->CanDualWield()) ||
                 (cls == CLASS_ROGUE) || (cls == CLASS_DEATH_KNIGHT && tab == DEATH_KNIGHT_TAB_FROST) ||
                 (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_FURY && !player_->CanTitanGrip() &&
                  player_->CanDualWield()) ||
                 (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_PROTECTION) ||
                 (cls == CLASS_PALADIN && tab == PALADIN_TAB_PROTECTION) ||
                 (cls == CLASS_PALADIN && tab == PALADIN_TAB_HOLY)))
            {
                weight_ *= 0.1;
            }
        }
        // spec with double hand
        // fury without duel wield, arms, bear, retribution, blood dk
        if (!isDoubleHand)
        {
            if ((cls == CLASS_HUNTER && !player_->CanDualWield()) ||
                (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_FURY && !player_->CanDualWield()) ||
                (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_ARMS) || (cls == CLASS_DRUID && tab == DRUID_TAB_FERAL) ||
                (cls == CLASS_PALADIN && tab == PALADIN_TAB_RETRIBUTION) ||
                (cls == CLASS_DEATH_KNIGHT && tab == DEATH_KNIGHT_TAB_BLOOD) ||
                (cls == CLASS_SHAMAN && tab == SHAMAN_TAB_ENHANCEMENT && !player_->CanDualWield()))
            {
                weight_ *= 0.1;
            }
            // caster's main hand (cannot duel weapon but can equip two-hands stuff)
            if ((cls == CLASS_MAGE || cls == CLASS_PRIEST || cls == CLASS_WARLOCK || cls == CLASS_DRUID ||
                (cls == CLASS_SHAMAN && !player_->CanDualWield())) &&
                !(cls == CLASS_PALADIN && tab == PALADIN_TAB_HOLY))
            {
                weight_ *= 0.65;
            }
            if (cls == CLASS_PALADIN && tab == PALADIN_TAB_HOLY)
            {
                weight_ *= 0.8;
            }
        }
        // fury with titan's grip
        if ((!isDoubleHand || proto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM ||
             proto->SubClass == ITEM_SUBCLASS_WEAPON_STAFF) &&
            (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_FURY && player_->CanTitanGrip()))
        {
            weight_ *= 0.1;
        }

        if (cls == CLASS_HUNTER && proto->SubClass == ITEM_SUBCLASS_WEAPON_THROWN)
        {
            weight_ *= 0.1;
        }

        if (lvl >= 10 && cls == CLASS_ROGUE && (tab == ROGUE_TAB_ASSASSINATION || tab == ROGUE_TAB_SUBTLETY) &&
            proto->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER)
        {
            weight_ *= 1.5;
        }

        if (cls == CLASS_ROGUE && player_->HasAura(SPELL_ROGUE_SWORD_SPECIALIZATION) &&
            (proto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD || proto->SubClass == ITEM_SUBCLASS_WEAPON_AXE))
        {
            weight_ *= 1.1;
        }
        if (cls == CLASS_WARRIOR && player_->HasAura(SPELL_POLEAXE_SPECIALIZATION) &&
            (proto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM || proto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2))
        {
            weight_ *= 1.1;
        }
        if (cls == CLASS_DEATH_KNIGHT && player_->HasAura(SPELL_NERVES_OF_COLD_STEEL) && !isDoubleHand)
        {
            weight_ *= 1.3;
        }
        bool slowDelay = proto->Delay > 2500;
        if (cls == CLASS_SHAMAN && tab == SHAMAN_TAB_ENHANCEMENT && slowDelay)
            weight_ *= 1.1;
    }
}

bool StatsWeightCalculator::NotBestArmorType(uint32 item_subclass_armor)
{
    if (player_->HasSkill(SKILL_PLATE_MAIL))
    {
        return item_subclass_armor != ITEM_SUBCLASS_ARMOR_PLATE;
    }
    if (player_->HasSkill(SKILL_MAIL))
    {
        return item_subclass_armor != ITEM_SUBCLASS_ARMOR_MAIL;
    }
    if (player_->HasSkill(SKILL_LEATHER))
    {
        return item_subclass_armor != ITEM_SUBCLASS_ARMOR_LEATHER;
    }
    return false;
}

void StatsWeightCalculator::ApplyOverflowPenalty(Player* player)
{
    {
        float hit_current, hit_overflow;
        float validPoints;
        if (hitOverflowType_ & CollectorType::SPELL)
        {
            hit_current = player->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
            hit_current +=
                player->GetTotalAuraModifier(SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT);  // suppression (18176)
            hit_current += player->GetRatingBonusValue(CR_HIT_SPELL);

            if (cls == CLASS_PRIEST && tab == PRIEST_TAB_SHADOW && player->HasAura(SPELL_SHADOW_FOCUS))
                hit_current += 3;
            if (cls == CLASS_MAGE && tab == MAGE_TAB_ARCANE && player->HasAura(SPELL_ARCANE_FOCUS))
                hit_current += 3;

            hit_overflow = SPELL_HIT_OVERFLOW;
            if (hit_overflow > hit_current)
                validPoints = (hit_overflow - hit_current) / player->GetRatingMultiplier(CR_HIT_SPELL);
            else
                validPoints = 0;
        }
        else if (hitOverflowType_ & CollectorType::MELEE)
        {
            hit_current = player->GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);
            hit_current += player->GetRatingBonusValue(CR_HIT_MELEE);
            hit_overflow = MELEE_HIT_OVERFLOW;
            if (hit_overflow > hit_current)
                validPoints = (hit_overflow - hit_current) / player->GetRatingMultiplier(CR_HIT_MELEE);
            else
                validPoints = 0;
        }
        else
        {
            hit_current = player->GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);
            hit_current += player->GetRatingBonusValue(CR_HIT_RANGED);
            hit_overflow = RANGED_HIT_OVERFLOW;
            if (hit_overflow > hit_current)
                validPoints = (hit_overflow - hit_current) / player->GetRatingMultiplier(CR_HIT_RANGED);
            else
                validPoints = 0;
        }
        collector_->stats[STATS_TYPE_HIT] = std::min(collector_->stats[STATS_TYPE_HIT], validPoints);
    }

    {
        if (type_ & CollectorType::MELEE)
        {
            float expertise_current, expertise_overflow;
            expertise_current = player->GetUInt32Value(PLAYER_EXPERTISE);
            expertise_current += player->GetRatingBonusValue(CR_EXPERTISE);
            expertise_overflow = EXPERTISE_OVERFLOW;

            float validPoints;
            if (expertise_overflow > expertise_current)
                validPoints = (expertise_overflow - expertise_current) / player->GetRatingMultiplier(CR_EXPERTISE);
            else
                validPoints = 0;

            collector_->stats[STATS_TYPE_EXPERTISE] = std::min(collector_->stats[STATS_TYPE_EXPERTISE], validPoints);
        }
    }

    {
        if (type_ & CollectorType::MELEE)
        {
            float defense_current, defense_overflow;
            defense_current = player->GetRatingBonusValue(CR_DEFENSE_SKILL);
            defense_overflow = DEFENSE_OVERFLOW;

            float validPoints;
            if (defense_overflow > defense_current)
                validPoints = (defense_overflow - defense_current) / player->GetRatingMultiplier(CR_DEFENSE_SKILL);
            else
                validPoints = 0;

            collector_->stats[STATS_TYPE_DEFENSE] = std::min(collector_->stats[STATS_TYPE_DEFENSE], validPoints);
        }
    }

    {
        if (type_ & (CollectorType::MELEE | CollectorType::RANGED))
        {
            float armor_penetration_current, armor_penetration_overflow;
            armor_penetration_current = player->GetRatingBonusValue(CR_ARMOR_PENETRATION);
            armor_penetration_overflow = ARMOR_PENETRATION_OVERFLOW;

            float validPoints;
            if (armor_penetration_overflow > armor_penetration_current)
                validPoints = (armor_penetration_overflow - armor_penetration_current) /
                              player->GetRatingMultiplier(CR_ARMOR_PENETRATION);
            else
                validPoints = 0;

            collector_->stats[STATS_TYPE_ARMOR_PENETRATION] =
                std::min(collector_->stats[STATS_TYPE_ARMOR_PENETRATION], validPoints);
        }
    }
}

void StatsWeightCalculator::ApplyWeightFinetune(Player* player)
{
    {
        if (type_ & (CollectorType::MELEE | CollectorType::RANGED))
        {
            float armor_penetration_current /*, armor_penetration_overflow*/;  // not used, line marked for removal.
            armor_penetration_current = player->GetRatingBonusValue(CR_ARMOR_PENETRATION);
            if (armor_penetration_current > 50)
                stats_weights_[STATS_TYPE_ARMOR_PENETRATION] *= 1.2f;
        }
    }
}

float StatsWeightCalculator::ApplyPreferredSpecWeapons(ItemTemplate const* proto, int32 slot)
{
    // Multiply score by 3x when this weapon's delay matches the spec-ideal speed.
    float weight = 2.0f;

    // Applies to mainhand, offhand, and ranged slots only.
    if (slot != EQUIPMENT_SLOT_MAINHAND &&
        slot != EQUIPMENT_SLOT_OFFHAND  &&
        slot != EQUIPMENT_SLOT_RANGED)
        return 1.0f;

    uint32 delay = proto->Delay;  // milliseconds
    float boost = 1.0f + weight;  // applied on a match

    // Hunter: melee weapons are stat sticks — speed irrelevant.
    // Ranged weapons scale Aimed/Chimera/Explosive Shot from top-end damage,
    // so a slow ranged weapon (>=2600 ms) is strongly preferred.
    if (cls == CLASS_HUNTER)
    {
        if (slot == EQUIPMENT_SLOT_RANGED && delay >= 2600)
            return boost;
        return 1.0f;
    }

    // Feral Druid: forms normalise attack speed; raw weapon Delay is irrelevant.
    if (cls == CLASS_DRUID && tab == DRUID_TAB_FERAL)
        return 1.0f;

    switch (cls)
    {
        case CLASS_WARRIOR:
            if (tab == WARRIOR_TAB_ARMS)
            {
                // Arms: slow 2H axes or polearms in mainhand only (Axe Specialization: +5% crit).
                bool isAxeOrPolearm = (proto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2 ||
                                       proto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM);
                if (slot == EQUIPMENT_SLOT_MAINHAND && delay >= 3400 && isAxeOrPolearm)
                    return boost;
            }
            else if (tab == WARRIOR_TAB_FURY)
            {
                if (!player_->CanDualWield())
                {
                    // Pre-DW: treat like Arms — slow 2H in mainhand only.
                    if (slot == EQUIPMENT_SLOT_MAINHAND && delay >= 3400)
                        return boost;
                }
                else if (player_->CanTitanGrip())
                {
                    // Titan's Grip: slow 2H (>=3400) in both hands.
                    if (delay >= 3400)
                        return boost;
                }
                else
                {
                    // 1H DW: slow 1H (>=2600) in both hands.
                    // 2H must be excluded — delay >= 2600 would otherwise pass
                    // for a 2H heirloom (~3600ms) just as it did for Enhancement.
                    if (proto->InventoryType == INVTYPE_2HWEAPON)
                        break;
                    if (delay >= 2600)
                        return boost;
                }
            }
            else if (tab == WARRIOR_TAB_PROTECTION)
            {
                // Prot: slow 1H (>=2600) in mainhand. Shield in offhand, no speed bonus.
                if (slot == EQUIPMENT_SLOT_MAINHAND && delay >= 2600)
                    return boost;
            }
            break;

        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_RETRIBUTION)
            {
                // Ret: slow 2H in mainhand only.
                if (slot == EQUIPMENT_SLOT_MAINHAND && delay >= 3400)
                    return boost;
            }
            else if (tab == PALADIN_TAB_PROTECTION)
            {
                // Prot: slow 1H (>=2600) in mainhand. Shield in offhand.
                if (slot == EQUIPMENT_SLOT_MAINHAND && delay >= 2600)
                    return boost;
            }
            break;

        case CLASS_DEATH_KNIGHT:
            if (tab == DEATH_KNIGHT_TAB_BLOOD || tab == DEATH_KNIGHT_TAB_UNHOLY)
            {
                // Blood / Unholy: slow 2H in mainhand only.
                if (slot == EQUIPMENT_SLOT_MAINHAND && delay >= 3400)
                    return boost;
            }
            else if (tab == DEATH_KNIGHT_TAB_FROST)
            {
                // Frost DK has Dual Wield innately — always dual-wields 1H.
                if (proto->InventoryType == INVTYPE_2HWEAPON)
                    break;
                if (delay >= 2600)
                    return boost;
            }
            break;

        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_ENHANCEMENT)
            {
                if (!player_->CanDualWield())
                {
                    // Pre-Dual Wield: Enhancement plays like a 2H spec.
                    if (slot == EQUIPMENT_SLOT_MAINHAND && delay >= 3400)
                        return boost;
                }
                else
                {
                    // Post-Dual Wield: slow 1H (>=2600) in both hands.
                    if (proto->InventoryType == INVTYPE_2HWEAPON)
                        break;

                    if (delay >= 2600)
                    {
                        float mult = boost;
                        if (slot == EQUIPMENT_SLOT_OFFHAND)
                        {
                            Item* mh = player_->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                            if (mh && mh->GetTemplate() && mh->GetTemplate()->Delay == delay)
                                mult *= boost;  // synchronized: ×(1+weight)² total = ×9 for 2.0f weight
                        }
                        return mult;
                    }
                }
            }
            break;

        case CLASS_ROGUE:
            if (tab == ROGUE_TAB_COMBAT)
            {
                // Combat: slow MH (>=2600), fast OH (<=1500).
                if (slot == EQUIPMENT_SLOT_MAINHAND && delay >= 2600)
                    return boost;
                if (slot == EQUIPMENT_SLOT_OFFHAND && delay <= 1500)
                    return boost;
            }
            else  // Assassination or Subtlety: slow dagger MH, fast dagger OH.
            {
                bool isDagger = (proto->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER);
                if (slot == EQUIPMENT_SLOT_MAINHAND && isDagger && delay >= 1700)
                    return boost;
                if (slot == EQUIPMENT_SLOT_OFFHAND && isDagger && delay <= 1500)
                    return boost;
            }
            break;

        default:
            break;
    }

    return 1.0f;
}
