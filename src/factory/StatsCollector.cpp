#include "StatsCollector.h"

#include <cstdint>

#include "DBCStores.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "UpdateFields.h"

StatsCollector::StatsCollector(CollectorType type) : type_(type) { Reset(); }

void StatsCollector::Reset()
{
    for (uint32 i = 0; i < STATS_TYPE_MAX; i++)
    {
        stats[i] = 0;
    }
}

void StatsCollector::CollectItemStats(ItemTemplate const* proto)
{
    if (proto->IsRangedWeapon())
    {
        uint32 val = (proto->Damage[0].DamageMin + proto->Damage[0].DamageMax) * 1000 / 2 / proto->Delay;
        stats[STATS_TYPE_RANGED_DPS] += val;
    }
    else if (proto->IsWeapon())
    {
        uint32 val = (proto->Damage[0].DamageMin + proto->Damage[0].DamageMax) * 1000 / 2 / proto->Delay;
        stats[STATS_TYPE_MELEE_DPS] += val;
    }
    stats[STATS_TYPE_ARMOR] += proto->Armor;
    stats[STATS_TYPE_BLOCK_VALUE] += proto->Block;
    for (int i = 0; i < proto->StatsCount; i++)
    {
        const _ItemStat& stat = proto->ItemStat[i];
        const int32& val = stat.ItemStatValue;
        CollectByItemStatType(stat.ItemStatType, val);
    }
    for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
    {
        CollectSpellStats(proto->Spells[j].SpellId, proto->Spells[j].SpellTrigger != ITEM_SPELLTRIGGER_ON_EQUIP);
    }

    if (proto->socketBonus)
    {
        if (const SpellItemEnchantmentEntry *enchant = sSpellItemEnchantmentStore.LookupEntry(proto->socketBonus))
            CollectEnchantStats(enchant);
    }
}

void StatsCollector::CollectSpellStats(uint32 spellId, bool isTriggered)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);

    if (!spellInfo)
        return;

    if (CollectSpecialCaseSpellStats(spellId))
        return;

    /// @todo Not all triggered spell need this penalty
    float multiplier = isTriggered ? 0.25f : 1.0f;

    bool canNextTrigger = true;

    uint32 procFlags;
    const SpellProcEventEntry* eventEntry = sSpellMgr->GetSpellProcEvent(spellInfo->Id);
    if (eventEntry && eventEntry->procFlags)
        procFlags = eventEntry->procFlags;
    else
        procFlags = spellInfo->ProcFlags;

    if (procFlags && !CanBeTriggeredByType(spellInfo, procFlags))
        canNextTrigger = false;
    
    // if (!eventEntry || eventEntry->cooldown == 0)
    // {

    // }
    // if (spellInfo->ProcChance == 100)
    
    if (spellInfo->StackAmount)
        multiplier *= spellInfo->StackAmount;

    for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
    {
        if (spellInfo->IsPositive())
            CollectPositiveSpellEffectStats(spellInfo->Effects[i], multiplier, canNextTrigger);
    }
}

void StatsCollector::CollectPositiveSpellEffectStats(const SpellEffectInfo& effectInfo, float multiplier, bool canNextTrigger)
{

    switch (effectInfo.Effect)
    {
        case SPELL_EFFECT_APPLY_AURA:
            HandleApplyAura(effectInfo, multiplier, canNextTrigger);
            break;
        // case SPELL_EFFECT_HEAL:
        //     int32 val = effectInfo.CalcValue();
        //     stats[STATS_TYPE_HEAL_POWER] += (float)val / 5 * multiplier;
        //     break;
        default:
            break;
    }
}

void StatsCollector::CollectEnchantStats(SpellItemEnchantmentEntry const* enchant)
{
    for (int s = 0; s < MAX_SPELL_ITEM_ENCHANTMENT_EFFECTS; ++s)
    {
        uint32 enchant_display_type = enchant->type[s];
        uint32 enchant_amount = enchant->amount[s];
        uint32 enchant_spell_id = enchant->spellid[s];

        if (CollectSpecialEnchantSpellStats(enchant_spell_id))
            continue;

        switch (enchant_display_type)
        {
            case ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL:
            {
                CollectSpellStats(enchant_spell_id, true);
                break;
            }
            case ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL:
            {
                CollectSpellStats(enchant_spell_id, false);
                break;
            }
            case ITEM_ENCHANTMENT_TYPE_STAT:
            {
                if (!enchant_amount)
                {
                    break;
                }
                CollectByItemStatType(enchant_spell_id, enchant_amount);
                break;
            }
            default:
                break;
        }
    }
}

/// @todo Special case for some spell that hard to calculate, like trinket, relic, etc.
bool StatsCollector::CollectSpecialCaseSpellStats(uint32 spellId) {
    // trinket
    switch (spellId)
    {
        case 71519: // Deathbringer's Will
            stats[STATS_TYPE_ATTACK_POWER] += 350;
            return true;
        case 71562: // Deathbringer's Will (heroic)
            stats[STATS_TYPE_ATTACK_POWER] += 400;
            return true;
        default:
            break;
    }
    // switch (spellId)
    // {
    //     case 50457: // Idol of the Lunar Eclipse
    //         stats[STATS_TYPE_CRIT] += 150;
    //         return true;
    //     default:
    //         break;
    // }
    return false;
}

bool StatsCollector::CollectSpecialEnchantSpellStats(uint32 enchantSpellId)
{
    switch (enchantSpellId)
    {
        // case 28093:  // mongoose
        //     if (type_ == CollectorType::MELEE)
        //     {
        //         stats[STATS_TYPE_AGILITY] += 40;
        //     }
        //     return true;
        // case 20007:  // crusader
        //     if (type_ == CollectorType::MELEE)
        //     {
        //         stats[STATS_TYPE_STRENGTH] += 30;
        //     }
        //     return true;
        // case 59620:  // Berserk
        //     if (type_ == CollectorType::MELEE)
        //     {
        //         stats[STATS_TYPE_ATTACK_POWER] += 120;
        //     }
        //     return true;
        // case 64440:  // Blade Warding
        //     if (type_ == CollectorType::MELEE)
        //     {
        //         stats[STATS_TYPE_PARRY] += 50;
        //     }
        //     return true;
        case 53365: // Rune of the Fallen Crusader
            if (type_ == CollectorType::MELEE)
            {
                stats[STATS_TYPE_STRENGTH] += 60;
            }
            return true;
        case 62157: // Rune of the Stoneskin Gargoyle
            if (type_ == CollectorType::MELEE)
            {
                stats[STATS_TYPE_DEFENSE] += 25;
                stats[STATS_TYPE_STAMINA] += 40;
            }
            return true;
        case 64571: // Blood draining
            if (type_ == CollectorType::MELEE)
            {
                stats[STATS_TYPE_STAMINA] += 50;
            }
            return true;
        default:
            break;
    }
    // {
    //     int allStatsAmount = 0;
    //     switch (enchantSpellId)
    //     {
    //         case 13624:
    //             allStatsAmount = 1;
    //             break;
    //         case 13625:
    //             allStatsAmount = 2;
    //             break;
    //         case 13824:
    //             allStatsAmount = 3;
    //             break;
    //         case 19988:
    //         case 44627:
    //         case 56527:
    //             allStatsAmount = 4;
    //             break;
    //         case 27959:
    //         case 56529:
    //             allStatsAmount = 6;
    //             break;
    //         case 44624:
    //             allStatsAmount = 8;
    //             break;
    //         case 60694:
    //         case 68251:
    //             allStatsAmount = 10;
    //             break;
    //         default:
    //             break;
    //     }
    //     if (allStatsAmount != 0)
    //     {
    //         stats[STATS_TYPE_AGILITY] += allStatsAmount;
    //         stats[STATS_TYPE_STRENGTH] += allStatsAmount;
    //         stats[STATS_TYPE_INTELLECT] += allStatsAmount;
    //         stats[STATS_TYPE_SPIRIT] += allStatsAmount;
    //         stats[STATS_TYPE_STAMINA] += allStatsAmount;
    //         return true;
    //     }
    // }

    return false;
}

bool StatsCollector::CanBeTriggeredByType(SpellInfo const* spellInfo, uint32 procFlags)
{
    const SpellProcEventEntry* eventEntry = sSpellMgr->GetSpellProcEvent(spellInfo->Id);
    uint32 spellFamilyName = eventEntry ? eventEntry->spellFamilyName : 0;

    if (spellFamilyName != 0)
        /// @todo Check specific trigger spell by spellFamilyMask
        return true;

    uint32 triggerMask = TAKEN_HIT_PROC_FLAG_MASK; // Generic trigger mask
    switch (type_) {
        case CollectorType::MELEE:
        {
            triggerMask |= MELEE_PROC_FLAG_MASK;
            triggerMask |= SPELL_PROC_FLAG_MASK;
            triggerMask |= PERIODIC_PROC_FLAG_MASK;
            if (procFlags & triggerMask)
                return true;
            break;
        }
        case CollectorType::RANGED:
        {
            triggerMask |= RANGED_PROC_FLAG_MASK;
            triggerMask |= SPELL_PROC_FLAG_MASK;
            triggerMask |= PERIODIC_PROC_FLAG_MASK;
            if (procFlags & triggerMask)
                return true;
            break;
        }
        case CollectorType::SPELL_DMG:
        {
            triggerMask |= SPELL_PROC_FLAG_MASK;
            // Healing spell cannot trigger
            triggerMask &= ~PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS;
            triggerMask &= ~PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS;
            if (procFlags & triggerMask)
                return true;
            break;
        }
        case CollectorType::SPELL_HEAL:
        {
            triggerMask |= SPELL_PROC_FLAG_MASK;
            // Dmg spell should not trigger
            triggerMask &= ~PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;
            triggerMask &= ~PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG;
            triggerMask &= ~PROC_FLAG_DONE_PERIODIC; // spellFamilyName = 0 and PROC_FLAG_DONE_PERIODIC -> it is a dmg spell
            if (procFlags & triggerMask)
                return true;
            break;
        }
        default:
            break;
    }
    return false;
}

void StatsCollector::CollectByItemStatType(uint32 itemStatType, int32 val)
{
    switch (itemStatType)
    {
        case ITEM_MOD_MANA:
            break;
        case ITEM_MOD_HEALTH:
            stats[STATS_TYPE_AGILITY] += val / 12;
            break;
        case ITEM_MOD_AGILITY:
            stats[STATS_TYPE_AGILITY] += val;
            break;
        case ITEM_MOD_STRENGTH:
            stats[STATS_TYPE_STRENGTH] += val;
            break;
        case ITEM_MOD_INTELLECT:
            stats[STATS_TYPE_INTELLECT] += val;
            break;
        case ITEM_MOD_SPIRIT:
            stats[STATS_TYPE_SPIRIT] += val;
            break;
        case ITEM_MOD_STAMINA:
            stats[STATS_TYPE_STAMINA] += val;
            break;
        case ITEM_MOD_DEFENSE_SKILL_RATING:
            stats[STATS_TYPE_DEFENSE] += val;
            break;
        case ITEM_MOD_DODGE_RATING:
            stats[STATS_TYPE_DODGE] += val;
            break;
        case ITEM_MOD_PARRY_RATING:
            stats[STATS_TYPE_PARRY] += val;
            break;
        case ITEM_MOD_BLOCK_RATING:
            stats[STATS_TYPE_BLOCK_RATING] += val;
            break;
        case ITEM_MOD_HIT_MELEE_RATING:
            if (type_ == CollectorType::MELEE)
                stats[STATS_TYPE_HIT] += val;
            break;
        case ITEM_MOD_HIT_RANGED_RATING:
            if (type_ == CollectorType::RANGED)
                stats[STATS_TYPE_HIT] += val;
            break;
        case ITEM_MOD_HIT_SPELL_RATING:
            if (type_ == CollectorType::SPELL)
                stats[STATS_TYPE_HIT] += val;
            break;
        case ITEM_MOD_CRIT_MELEE_RATING:
            if (type_ == CollectorType::MELEE)
                stats[STATS_TYPE_CRIT] += val;
            break;
        case ITEM_MOD_CRIT_RANGED_RATING:
            if (type_ == CollectorType::RANGED)
                stats[STATS_TYPE_CRIT] += val;
            break;
        case ITEM_MOD_CRIT_SPELL_RATING:
            if (type_ == CollectorType::SPELL)
                stats[STATS_TYPE_CRIT] += val;
            break;
        case ITEM_MOD_HASTE_MELEE_RATING:
            if (type_ == CollectorType::MELEE)
                stats[STATS_TYPE_HASTE] += val;
            break;
        case ITEM_MOD_HASTE_RANGED_RATING:
            if (type_ == CollectorType::RANGED)
                stats[STATS_TYPE_HASTE] += val;
            break;
        case ITEM_MOD_HASTE_SPELL_RATING:
            if (type_ == CollectorType::SPELL)
                stats[STATS_TYPE_HASTE] += val;
            break;
        case ITEM_MOD_HIT_RATING:
            stats[STATS_TYPE_HIT] += val;
            break;
        case ITEM_MOD_CRIT_RATING:
            stats[STATS_TYPE_CRIT] += val;
            break;
        case ITEM_MOD_RESILIENCE_RATING:
            stats[STATS_TYPE_RESILIENCE] += val;
            break;
        case ITEM_MOD_HASTE_RATING:
            stats[STATS_TYPE_HASTE] += val;
            break;
        case ITEM_MOD_EXPERTISE_RATING:
            stats[STATS_TYPE_EXPERTISE] += val;
            break;
        case ITEM_MOD_ATTACK_POWER:
            stats[STATS_TYPE_ATTACK_POWER] += val;
            break;
        case ITEM_MOD_RANGED_ATTACK_POWER:
            if (type_ == CollectorType::RANGED)
                stats[STATS_TYPE_ATTACK_POWER] += val;
            break;
        case ITEM_MOD_MANA_REGENERATION:
            stats[STATS_TYPE_MANA_REGENERATION] += val;
            break;
        case ITEM_MOD_ARMOR_PENETRATION_RATING:
            stats[STATS_TYPE_ARMOR_PENETRATION] += val;
            break;
        case ITEM_MOD_SPELL_POWER:
            stats[STATS_TYPE_SPELL_POWER] += val;
            stats[STATS_TYPE_HEAL_POWER] += val;
            break;
        case ITEM_MOD_HEALTH_REGEN:
            stats[STATS_TYPE_HEALTH_REGENERATION] += val;
            break;
        case ITEM_MOD_SPELL_PENETRATION:
            stats[STATS_TYPE_SPELL_PENETRATION] += val;
            break;
        case ITEM_MOD_BLOCK_VALUE:
            stats[STATS_TYPE_BLOCK_VALUE] += val;
            break;
        case ITEM_MOD_SPELL_HEALING_DONE:  // deprecated
        case ITEM_MOD_SPELL_DAMAGE_DONE:   // deprecated
        default:
            break;
    }
}

void StatsCollector::HandleApplyAura(const SpellEffectInfo& effectInfo, float multiplier, bool canNextTrigger)
{
    if (effectInfo.Effect != SPELL_EFFECT_APPLY_AURA)
        return;
    
    int32 val = effectInfo.CalcValue();
    
    switch (effectInfo.ApplyAuraName)
    {
        case SPELL_AURA_MOD_DAMAGE_DONE:
        {
            int32 schoolType = effectInfo.MiscValue;
            if (schoolType & SPELL_SCHOOL_MASK_NORMAL)
                stats[STATS_TYPE_ATTACK_POWER] += val * multiplier;
            if ((schoolType & SPELL_SCHOOL_MASK_MAGIC) == SPELL_SCHOOL_MASK_MAGIC)
                stats[STATS_TYPE_SPELL_POWER] += val * multiplier;
            break;
        }
        case SPELL_AURA_MOD_HEALING_DONE:
            stats[STATS_TYPE_HEAL_POWER] += val * multiplier;
            break;
        case SPELL_AURA_MOD_ATTACK_POWER:
            stats[STATS_TYPE_ATTACK_POWER] += val * multiplier;
            break;
        case SPELL_AURA_MOD_RANGED_ATTACK_POWER:
            if (type_ == CollectorType::RANGED)
                stats[STATS_TYPE_ATTACK_POWER] += val * multiplier;
            break;
        case SPELL_AURA_MOD_SHIELD_BLOCKVALUE:
            stats[STATS_TYPE_BLOCK_VALUE] += val * multiplier;
            break;
        case SPELL_AURA_MOD_STAT:
        {
            int32 statType = effectInfo.MiscValue;
            switch (statType)
            {
                case STAT_STRENGTH:
                    stats[STATS_TYPE_STRENGTH] += val * multiplier;
                    break;
                case STAT_AGILITY:
                    stats[STATS_TYPE_AGILITY] += val * multiplier;
                    break;
                case STAT_STAMINA:
                    stats[STATS_TYPE_STAMINA] += val * multiplier;
                    break;
                case STAT_INTELLECT:
                    stats[STATS_TYPE_INTELLECT] += val * multiplier;
                    break;
                case STAT_SPIRIT:
                    stats[STATS_TYPE_SPIRIT] += val * multiplier;
                    break;
                case -1: // Stat all
                    stats[STATS_TYPE_STRENGTH] += val * multiplier;
                    stats[STATS_TYPE_AGILITY] += val * multiplier;
                    stats[STATS_TYPE_STAMINA] += val * multiplier;
                    stats[STATS_TYPE_INTELLECT] += val * multiplier;
                    stats[STATS_TYPE_SPIRIT] += val * multiplier;
                    break;
                default:
                    break;
            }
            break;
        }
        case SPELL_AURA_MOD_RESISTANCE:
        {
            int32 statType = effectInfo.MiscValue;
            if (statType & SPELL_SCHOOL_MASK_NORMAL) // physical
                stats[STATS_TYPE_ARMOR] += val * multiplier;
            break;
        }
        case SPELL_AURA_MOD_RATING:
        {
            for (uint32 rating = CR_WEAPON_SKILL; rating < MAX_COMBAT_RATING; ++rating)
            {
                if (effectInfo.MiscValue & (1 << rating))
                {
                    switch (rating)
                    {
                        case CR_DEFENSE_SKILL:
                            stats[STATS_TYPE_DEFENSE] += val * multiplier;
                            break;
                        case CR_DODGE:
                            stats[STATS_TYPE_DODGE] += val * multiplier;
                            break;
                        case CR_PARRY:
                            stats[STATS_TYPE_PARRY] += val * multiplier;
                            break;
                        case CR_BLOCK:
                            stats[STATS_TYPE_BLOCK_RATING] += val * multiplier;
                            break;
                        case CR_HIT_MELEE:
                            if (type_ == CollectorType::MELEE)
                                stats[STATS_TYPE_HIT] += val * multiplier;
                            break;
                        case CR_HIT_RANGED:
                            if (type_ == CollectorType::RANGED)
                                stats[STATS_TYPE_HIT] += val * multiplier;
                            break;
                        case CR_HIT_SPELL:
                            if (type_ == CollectorType::SPELL)
                                stats[STATS_TYPE_HIT] += val * multiplier;
                            break;
                        case CR_CRIT_MELEE:
                            if (type_ == CollectorType::MELEE)
                                stats[STATS_TYPE_CRIT] += val * multiplier;
                            break;
                        case CR_CRIT_RANGED:
                            if (type_ == CollectorType::RANGED)
                                stats[STATS_TYPE_CRIT] += val * multiplier;
                            break;
                        case CR_CRIT_SPELL:
                            if (type_ == CollectorType::SPELL)
                                stats[STATS_TYPE_CRIT] += val * multiplier;
                            break;
                        case CR_HASTE_MELEE:
                            if (type_ == CollectorType::MELEE)
                                stats[STATS_TYPE_HASTE] += val * multiplier;
                            break;
                        case CR_HASTE_RANGED:
                            if (type_ == CollectorType::RANGED)
                                stats[STATS_TYPE_HASTE] += val * multiplier;
                            break;
                        case CR_HASTE_SPELL:
                            if (type_ == CollectorType::SPELL)
                                stats[STATS_TYPE_HASTE] += val * multiplier;
                            break;
                        case CR_EXPERTISE:
                            stats[STATS_TYPE_EXPERTISE] += val * multiplier;
                            break;
                        case CR_ARMOR_PENETRATION:
                            stats[STATS_TYPE_ARMOR_PENETRATION] += val * multiplier;
                            break;
                        default:
                            break;
                    }
                }
                break;
            }
        }
        case SPELL_AURA_MOD_POWER_REGEN:
        {
            int32 powerType = effectInfo.MiscValue;
            switch (powerType)
            {
                case POWER_MANA:
                    stats[STATS_TYPE_MANA_REGENERATION] += val * multiplier;
                default:
                    break;
            }
            break;
        }
        case SPELL_AURA_PROC_TRIGGER_SPELL:
        {
            if (canNextTrigger)
                CollectSpellStats(effectInfo.TriggerSpell, true);
            break;
        }
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        {
            if (canNextTrigger)
                CollectSpellStats(effectInfo.TriggerSpell, true);
            break;
        }
        default:
            break;
    }
}