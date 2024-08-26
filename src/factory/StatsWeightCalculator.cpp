/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "StatsWeightCalculator.h"

#include <memory>

#include "AiFactory.h"
#include "DBCStores.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "PlayerbotAI.h"
#include "SharedDefines.h"
#include "StatsCollector.h"
#include "Unit.h"

StatsWeightCalculator::StatsWeightCalculator(Player* player) : player_(player)
{
    if (PlayerbotAI::IsHeal(player))
        type_ = CollectorType::SPELL_HEAL;
    else if (PlayerbotAI::IsCaster(player))
        type_ = CollectorType::SPELL_DMG;
    else if (PlayerbotAI::IsMelee(player))
        type_ = CollectorType::MELEE;
    else
        type_ = CollectorType::RANGED;
    collector_ = std::make_unique<StatsCollector>(type_);

    cls = player->getClass();
    tab = AiFactory::GetPlayerSpecTab(player);

    enable_overflow_penalty_ = true;
    enable_item_set_bonus_ = true;
    enable_quality_blend_ = true;
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

float StatsWeightCalculator::CalculateItem(uint32 itemId)
{
    ItemTemplate const* proto = &sObjectMgr->GetItemTemplateStore()->at(itemId);
    
    if (!proto)
        return 0.0f;

    Reset();
    
    collector_->CollectItemStats(proto);

    GenerateWeights(player_);
    for (uint32 i = 0; i < STATS_TYPE_MAX; i++)
    {
        weight_ += stats_weights_[i] * collector_->stats[i];
    }

    CalculateItemTypePenalty(proto);

    if (enable_item_set_bonus_)
        CalculateItemSetBonus(player_, proto);
    
    CalculateSocketBonus(player_, proto);

    if (enable_quality_blend_)
        // Blend with item quality and level
        weight_ *= (proto->Quality + 1) * proto->ItemLevel;
    
    return weight_;
}

float StatsWeightCalculator::CalculateEnchant(uint32 enchantId)
{
    SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchantId);
    
    if (!enchant)
        return 0.0f;

    Reset();
    
    collector_->CollectEnchantStats(enchant);

    GenerateWeights(player_);
    for (uint32 i = 0; i < STATS_TYPE_MAX; i++)
    {
        weight_ += stats_weights_[i] * collector_->stats[i];
    }

    return weight_;
}

void StatsWeightCalculator::GenerateWeights(Player* player)
{
    GenerateBasicWeights(player);
    GenerateAdditionalWeights(player);

    if (enable_overflow_penalty_)
        ApplyOverflowPenalty(player);
}

void StatsWeightCalculator::GenerateBasicWeights(Player* player)
{
    // Basic weights
    stats_weights_[STATS_TYPE_STAMINA] += 0.01f;
    stats_weights_[STATS_TYPE_ARMOR] += 0.001f;

    if (cls == CLASS_HUNTER && (tab == HUNTER_TAB_BEASTMASTER || tab == HUNTER_TAB_SURVIVAL))
    {
        stats_weights_[STATS_TYPE_AGILITY] += 2.4f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 1.3f;
        stats_weights_[STATS_TYPE_HIT] += 1.6f;
        stats_weights_[STATS_TYPE_CRIT] += 1.5f;
        stats_weights_[STATS_TYPE_HASTE] += 1.4f;
        stats_weights_[STATS_TYPE_RANGED_DPS] += 5.0f;
    } 
    else if (cls == CLASS_HUNTER && tab == HUNTER_TAB_MARKSMANSHIP)
    {
        stats_weights_[STATS_TYPE_AGILITY] += 2.2f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 2.2f;
        stats_weights_[STATS_TYPE_HIT] += 2.1f;
        stats_weights_[STATS_TYPE_CRIT] += 2.0f;
        stats_weights_[STATS_TYPE_HASTE] += 1.8f;
        stats_weights_[STATS_TYPE_RANGED_DPS] += 5.0f;
    }
    else if ((cls == CLASS_ROGUE && tab == ROGUE_TAB_COMBAT) || (cls == CLASS_DRUID && tab == DRUID_TAB_FERAL && !PlayerbotAI::IsTank(player)))
    {
        stats_weights_[STATS_TYPE_AGILITY] += 1.8f;
        stats_weights_[STATS_TYPE_STRENGTH] += 1.1f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 1.2f;
        stats_weights_[STATS_TYPE_HIT] += 2.0f;
        stats_weights_[STATS_TYPE_CRIT] += 1.6f;
        stats_weights_[STATS_TYPE_HASTE] += 1.4f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 2.0f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 5.0f;
    } 
    else if (cls == CLASS_ROGUE && (tab == ROGUE_TAB_ASSASSINATION || tab == ROGUE_TAB_SUBTLETY))
    {
        stats_weights_[STATS_TYPE_AGILITY] += 1.7f;
        stats_weights_[STATS_TYPE_STRENGTH] += 1.1f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 1.0f;
        stats_weights_[STATS_TYPE_HIT] += 1.6f;
        stats_weights_[STATS_TYPE_CRIT] += 1.3f;
        stats_weights_[STATS_TYPE_HASTE] += 1.5f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 2.0f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 5.0f;
    }
    else if (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_FURY)       //  fury
    {
        stats_weights_[STATS_TYPE_AGILITY] += 1.8f;
        stats_weights_[STATS_TYPE_STRENGTH] += 2.6f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 2.1f;
        stats_weights_[STATS_TYPE_HIT] += 2.3f;
        stats_weights_[STATS_TYPE_CRIT] += 2.2f;
        stats_weights_[STATS_TYPE_HASTE] += 1.8f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 2.5f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 7.0f;
    }
    else if (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_ARMS)       // arm 
    {
        stats_weights_[STATS_TYPE_AGILITY] += 1.6f;
        stats_weights_[STATS_TYPE_STRENGTH] += 2.3f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 1.7f;
        stats_weights_[STATS_TYPE_HIT] += 2.0f;
        stats_weights_[STATS_TYPE_CRIT] += 1.9f;
        stats_weights_[STATS_TYPE_HASTE] += 0.8f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 1.4f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 7.0f;
    }
    else if (cls == CLASS_DEATH_KNIGHT && tab == DEATHKNIGHT_TAB_FROST) // frost dk
    {
        stats_weights_[STATS_TYPE_AGILITY] += 1.8f;
        stats_weights_[STATS_TYPE_STRENGTH] += 2.6f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 2.1f;
        stats_weights_[STATS_TYPE_HIT] += 2.3f;
        stats_weights_[STATS_TYPE_CRIT] += 2.2f;
        stats_weights_[STATS_TYPE_HASTE] += 1.8f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 2.5f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 7.0f;
    }
    else if (cls == CLASS_DEATH_KNIGHT && tab == DEATHKNIGHT_TAB_UNHOLY)
    {
        stats_weights_[STATS_TYPE_AGILITY] += 0.5f;
        stats_weights_[STATS_TYPE_STRENGTH] += 2.5f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 1.0f;
        stats_weights_[STATS_TYPE_HIT] += 1.8f;
        stats_weights_[STATS_TYPE_CRIT] += 1.0f;
        stats_weights_[STATS_TYPE_HASTE] += 1.7f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 1.0f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 5.0f;
    }
    else if (cls == CLASS_PALADIN && tab == PALADIN_TAB_RETRIBUTION)    // retribution
    {
        stats_weights_[STATS_TYPE_AGILITY] += 1.1f;
        stats_weights_[STATS_TYPE_STRENGTH] += 2.5f;
        stats_weights_[STATS_TYPE_INTELLECT] += 0.15f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_SPELL_POWER] += 0.3f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 0.8f;
        stats_weights_[STATS_TYPE_HIT] += 1.9f;
        stats_weights_[STATS_TYPE_CRIT] += 1.2f;
        stats_weights_[STATS_TYPE_HASTE] += 1.3f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 2.0f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 7.0f;
    }
    else if ((cls == CLASS_SHAMAN && tab == SHAMAN_TAB_ENHANCEMENT))  // enhancement
    {
        stats_weights_[STATS_TYPE_AGILITY] += 1.6f;
        stats_weights_[STATS_TYPE_STRENGTH] += 1.1f;
        stats_weights_[STATS_TYPE_INTELLECT] += 0.5f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_SPELL_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR_PENETRATION] += 1.2f;
        stats_weights_[STATS_TYPE_HIT] += 1.7f;
        stats_weights_[STATS_TYPE_CRIT] += 1.4f;
        stats_weights_[STATS_TYPE_HASTE] += 1.8f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 2.0f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 5.2f;
    }
    else if (cls == CLASS_WARLOCK || 
             cls == CLASS_MAGE ||
             (cls == CLASS_PRIEST && tab == PRIEST_TAB_SHADOW) ||     // shadow
             (cls == CLASS_SHAMAN && tab == SHAMAN_TAB_ELEMENTAL) ||  // element
             (cls == CLASS_DRUID && tab == DRUID_TAB_BALANCE))        // balance
    {
        stats_weights_[STATS_TYPE_INTELLECT] += 0.5f;
        stats_weights_[STATS_TYPE_SPIRIT] += 0.4f;
        stats_weights_[STATS_TYPE_SPELL_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_SPELL_PENETRATION] += 1.0f;
        stats_weights_[STATS_TYPE_HIT] += 1.1f;
        stats_weights_[STATS_TYPE_CRIT] += 0.8f;
        stats_weights_[STATS_TYPE_HASTE] += 1.0f;
        stats_weights_[STATS_TYPE_RANGED_DPS] += 1.0f;
    }
    else if ((cls == CLASS_PALADIN && tab == PALADIN_TAB_HOLY) ||       // holy
             (cls == CLASS_PRIEST && tab != PRIEST_TAB_SHADOW) ||       // discipline / holy
             (cls == CLASS_SHAMAN && tab == SHAMAN_TAB_RESTORATION) ||  // heal
             (cls == CLASS_DRUID && tab == DRUID_TAB_RESTORATION))
    {
        stats_weights_[STATS_TYPE_INTELLECT] += 0.8f;
        stats_weights_[STATS_TYPE_SPIRIT] += 0.8f;
        stats_weights_[STATS_TYPE_HEAL_POWER] += 1.0f;
        stats_weights_[STATS_TYPE_MANA_REGENERATION] += 1.2f;
        stats_weights_[STATS_TYPE_CRIT] += 0.7f;
        stats_weights_[STATS_TYPE_HASTE] += 1.0f;
        stats_weights_[STATS_TYPE_RANGED_DPS] += 1.0f;
    }
    else if ((cls == CLASS_WARRIOR && tab == WARRIOR_TAB_PROTECTION) || (cls == CLASS_PALADIN && tab == PALADIN_TAB_PROTECTION))
    {
        stats_weights_[STATS_TYPE_AGILITY] += 2.0f;
        stats_weights_[STATS_TYPE_STRENGTH] += 1.0f;
        stats_weights_[STATS_TYPE_STAMINA] += 3.5f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 0.2f;
        stats_weights_[STATS_TYPE_DEFENSE] += 2.5f;
        stats_weights_[STATS_TYPE_PARRY] += 2.0f;
        stats_weights_[STATS_TYPE_DODGE] += 2.0f;
        stats_weights_[STATS_TYPE_RESILIENCE] += 2.0f;
        stats_weights_[STATS_TYPE_BLOCK_RATING] += 1.0f;
        stats_weights_[STATS_TYPE_BLOCK_VALUE] += 0.5f;
        stats_weights_[STATS_TYPE_ARMOR] += 0.15f;
        stats_weights_[STATS_TYPE_HIT] += 2.0f;
        stats_weights_[STATS_TYPE_CRIT] += 0.2f;
        stats_weights_[STATS_TYPE_HASTE] += 0.5f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 3.0f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 2.0f;
    }
    else if (cls == CLASS_DEATH_KNIGHT && tab == DEATHKNIGHT_TAB_BLOOD)
    {
        stats_weights_[STATS_TYPE_AGILITY] += 2.0f;
        stats_weights_[STATS_TYPE_STRENGTH] += 1.0f;
        stats_weights_[STATS_TYPE_STAMINA] += 3.5f;
        stats_weights_[STATS_TYPE_ATTACK_POWER] += 0.2f;
        stats_weights_[STATS_TYPE_DEFENSE] += 3.5f;
        stats_weights_[STATS_TYPE_PARRY] += 2.0f;
        stats_weights_[STATS_TYPE_DODGE] += 2.0f;
        stats_weights_[STATS_TYPE_RESILIENCE] += 2.0f;
        stats_weights_[STATS_TYPE_ARMOR] += 0.15f;
        stats_weights_[STATS_TYPE_HIT] += 2.0f;
        stats_weights_[STATS_TYPE_CRIT] += 0.5f;
        stats_weights_[STATS_TYPE_HASTE] += 0.5f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 3.5f;
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
        stats_weights_[STATS_TYPE_RESILIENCE] += 1.0f;
        stats_weights_[STATS_TYPE_ARMOR] += 0.15f;
        stats_weights_[STATS_TYPE_HIT] += 3.0f;
        stats_weights_[STATS_TYPE_CRIT] += 1.3f;
        stats_weights_[STATS_TYPE_HASTE] += 2.3f;
        stats_weights_[STATS_TYPE_EXPERTISE] += 3.7f;
        stats_weights_[STATS_TYPE_MELEE_DPS] += 3.0f;
    }
}

void StatsWeightCalculator::GenerateAdditionalWeights(Player* player)
{
    uint8 cls = player->getClass();
    // int tab = AiFactory::GetPlayerSpecTab(player);
    if (cls == CLASS_HUNTER)
    {
        if (player->HasAura(34484))
            stats_weights_[STATS_TYPE_INTELLECT] += 1.1f;
    }
    else if (cls == CLASS_WARRIOR)
    {
        if (player->HasAura(61222))
            stats_weights_[STATS_TYPE_ARMOR] += 0.03f;
    }
    else if (cls == CLASS_SHAMAN)
    {
        if (player->HasAura(51885))
            stats_weights_[STATS_TYPE_INTELLECT] += 1.1f;
    }
}

void StatsWeightCalculator::CalculateItemSetBonus(Player* player, ItemTemplate const* proto)
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
            
            const ItemSetEntry *setEntry = sItemSetStore.LookupEntry(setId);
            if (!setEntry)
                continue;
            
            uint32 itemCount = eff->item_count;
            uint32 max_items = 0;
            for (size_t j = 0; j < MAX_ITEM_SET_SPELLS; j++)
                max_items = std::max(max_items, setEntry->items_to_triggerspell[j]);
            if (itemCount < max_items)
            {
                multiplier += 0.1f * itemCount; // 10% bonus for each item already equipped
            }
            else
            {
                multiplier = 1.0f; // All item set effect has been triggerred
            }
            break;
        }
    }

    if (i == player->ItemSetEff.size())
        multiplier = 1.05f; // this is the first item in the item set

    weight_ *= multiplier;
}

void StatsWeightCalculator::CalculateSocketBonus(Player* player, ItemTemplate const* proto)
{
    uint32 socketNum = 0;
    for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT + MAX_GEM_SOCKETS; ++enchant_slot)
    {
        uint8 socketColor = proto->Socket[enchant_slot - SOCK_ENCHANTMENT_SLOT].Color;
        
        if (!socketColor) // no socket slot
            continue;

        socketNum++;
    }

    float multiplier = 1.0f + socketNum * 0.03f; // 3% bonus for socket

    weight_ *= multiplier;
}

void StatsWeightCalculator::CalculateItemTypePenalty(ItemTemplate const* proto)
{
    // penalty for different type armor
    if (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass >= ITEM_SUBCLASS_ARMOR_CLOTH &&
        proto->SubClass <= ITEM_SUBCLASS_ARMOR_PLATE && NotBestArmorType(proto->SubClass))
    {
        weight_ *= 0.8;
    }
    // double hand
    if (proto->Class == ITEM_CLASS_WEAPON)
    {
        bool isDoubleHand = proto->Class == ITEM_CLASS_WEAPON &&
                            !(ITEM_SUBCLASS_MASK_SINGLE_HAND & (1 << proto->SubClass)) &&
                            !(ITEM_SUBCLASS_MASK_WEAPON_RANGED & (1 << proto->SubClass));

        if (isDoubleHand)
        {
            weight_ *= 0.5;
        }
        // spec without double hand
        // enhancement, rogue, ice dk, unholy dk, shield tank, fury warrior without titan's grip but with duel wield
        if (isDoubleHand &&
            ((cls == CLASS_SHAMAN && tab == SHAMAN_TAB_ENHANCEMENT && player_->CanDualWield()) ||
             (cls == CLASS_ROGUE) || (cls == CLASS_DEATH_KNIGHT && tab != DEATHKNIGHT_TAB_BLOOD) ||
             (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_FURY && !player_->CanTitanGrip() && player_->CanDualWield()) ||
             (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_PROTECTION) ||
             (cls == CLASS_PALADIN && tab == PALADIN_TAB_PROTECTION)))
        {
            weight_ *= 0.1;
        }
        // spec with double hand
        // fury without duel wield, arms, bear, retribution, blood dk
        if (isDoubleHand &&
            ((cls == CLASS_HUNTER && !player_->CanDualWield()) ||
             (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_FURY && !player_->CanDualWield()) ||
             (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_ARMS) || (cls == CLASS_DRUID && tab == DRUID_TAB_FERAL) ||
             (cls == CLASS_PALADIN && tab == PALADIN_TAB_RETRIBUTION) ||
             (cls == CLASS_DEATH_KNIGHT && tab == DEATHKNIGHT_TAB_BLOOD) ||
             (cls == CLASS_SHAMAN && tab == SHAMAN_TAB_ENHANCEMENT && !player_->CanDualWield())))
        {
            weight_ *= 10;
        }
        // fury with titan's grip
        if (isDoubleHand && proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM &&
            (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_FURY && player_->CanTitanGrip()))
        {
            weight_ *= 10;
        }
    }
    if (proto->Class == ITEM_CLASS_WEAPON)
    {
        if (cls == CLASS_HUNTER && proto->SubClass == ITEM_SUBCLASS_WEAPON_THROWN)
        {
            weight_ *= 0.1;
        }
        if (cls == CLASS_ROGUE && tab == ROGUE_TAB_ASSASSINATION && proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER)
        {
            weight_ *= 0.1;
        }
        if (cls == CLASS_ROGUE && player_->HasAura(13964) 
            && (proto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD || proto->SubClass == ITEM_SUBCLASS_WEAPON_AXE))
        {
            weight_ *= 1.1;
        }
        if (cls == CLASS_WARRIOR && player_->HasAura(12785) 
            && (proto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM || proto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2))
        {
            weight_ *= 1.1;
        }
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
        if (type_ == CollectorType::SPELL)
        {
            hit_current = player->GetRatingBonusValue(CR_HIT_SPELL);
            hit_overflow = SPELL_HIT_OVERFLOW;
        }
        else if (type_ == CollectorType::MELEE)
        {
            hit_current = player->GetRatingBonusValue(CR_HIT_MELEE);
            hit_overflow = MELEE_HIT_OVERFLOW;
        }
        else
        {
            hit_current = player->GetRatingBonusValue(CR_HIT_RANGED);
            hit_overflow = RANGED_HIT_OVERFLOW;
        }
        if (hit_current >= hit_overflow)
            stats_weights_[STATS_TYPE_HIT] = 0.0f;
        else if (hit_current >= hit_overflow * 0.8)
            stats_weights_[STATS_TYPE_HIT] /= 1.5;
    }
    
    {
        if (type_ == CollectorType::MELEE)
        {
            float expertise_current, expertise_overflow;
            expertise_current = player->GetRatingBonusValue(CR_EXPERTISE);
            expertise_overflow = EXPERTISE_OVERFLOW;
            if (expertise_current >= expertise_overflow)
                stats_weights_[STATS_TYPE_EXPERTISE] = 0.0f;
            else if (expertise_current >= expertise_overflow * 0.8)
                stats_weights_[STATS_TYPE_EXPERTISE] /= 1.5;
        }
    }
    
    {
        if (type_ == CollectorType::MELEE)
        {
            float defense_current, defense_overflow;
            defense_current = player->GetRatingBonusValue(CR_DEFENSE_SKILL);
            defense_overflow = DEFENSE_OVERFLOW;
            if (defense_current >= defense_overflow)
                stats_weights_[STATS_TYPE_DEFENSE] /= 2;
            else if (defense_current >= defense_overflow * 0.8)
                stats_weights_[STATS_TYPE_DEFENSE] /= 1.5;
        }
    }

    {
        if (type_ == CollectorType::MELEE || type_ == CollectorType::RANGED)
        {
            float armor_penetration_current, armor_penetration_overflow;
            armor_penetration_current = player->GetRatingBonusValue(CR_ARMOR_PENETRATION);
            armor_penetration_overflow = ARMOR_PENETRATION_OVERFLOW;
            if (armor_penetration_current >= armor_penetration_overflow)
                stats_weights_[STATS_TYPE_ARMOR_PENETRATION] = 0.0f;
            if (armor_penetration_current >= armor_penetration_overflow * 0.8)
                stats_weights_[STATS_TYPE_ARMOR_PENETRATION] /= 1.5;
        }
    }
}