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

StatsCollector::StatsCollector(CollectorType type, int32 cls) : type_(type), cls_(cls) { Reset(); }

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
        switch (proto->Spells[j].SpellTrigger)
        {
            case ITEM_SPELLTRIGGER_ON_USE:
                CollectSpellStats(proto->Spells[j].SpellId, 1.0f, proto->Spells[j].SpellCooldown);
                break;
            case ITEM_SPELLTRIGGER_ON_EQUIP:
                CollectSpellStats(proto->Spells[j].SpellId, 1.0f, 0);
                break;
            case ITEM_SPELLTRIGGER_CHANCE_ON_HIT:
                if (type_ == CollectorType::MELEE)
                {
                    if (proto->Spells[j].SpellPPMRate > 0.01f)
                        CollectSpellStats(proto->Spells[j].SpellId, 1.0f, 60000 / proto->Spells[j].SpellPPMRate);
                    else
                        CollectSpellStats(proto->Spells[j].SpellId, 1.0f, 60000 / 1.8f); // Default PPM = 1.8
                }
                break;
            default:
                break;
        }
    }

    if (proto->socketBonus)
    {
        if (const SpellItemEnchantmentEntry *enchant = sSpellItemEnchantmentStore.LookupEntry(proto->socketBonus))
            CollectEnchantStats(enchant);
    }
}

void StatsCollector::CollectSpellStats(uint32 spellId, float multiplier, int32 spellCooldown)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);

    if (!spellInfo)
        return;

    if (SpecialSpellFilter(spellId))
        return;

    const SpellProcEventEntry* eventEntry = sSpellMgr->GetSpellProcEvent(spellInfo->Id);

    uint32 triggerCooldown = eventEntry ? eventEntry->cooldown : 0;

    bool canNextTrigger = true;

    uint32 procFlags;
    uint32 procChance;
    if (eventEntry && eventEntry->procFlags)
        procFlags = eventEntry->procFlags;
    else
        procFlags = spellInfo->ProcFlags;
    
    if (eventEntry && eventEntry->customChance)
        procChance = eventEntry->customChance;
    else
        procChance = spellInfo->ProcChance;
    bool lowChance =  procChance <= 5;
    
    if (lowChance || (procFlags && !CanBeTriggeredByType(spellInfo, procFlags)))
        canNextTrigger = false;

    if (spellInfo->StackAmount)
    {
        // Heuristic multiplier for spell with stackAmount since high stackAmount may not be available
        if (spellInfo->StackAmount <= 10)
            multiplier *= spellInfo->StackAmount * 0.6;
        else if (spellInfo->StackAmount <= 20)
            multiplier *= 6 + (spellInfo->StackAmount - 10) * 0.4;
        else
            multiplier *= 10;
    }
    
    for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
    {
        const SpellEffectInfo& effectInfo = spellInfo->Effects[i];
        switch (effectInfo.Effect)
        {
            case SPELL_EFFECT_APPLY_AURA:
            {
                /// @todo Handle negative spell
                if (!spellInfo->IsPositive())
                    break;

                float coverage;
                if (spellCooldown <= 2000 || spellInfo->GetDuration() == -1)
                    coverage = 1.0f;
                else
                    coverage = std::min(1.0f, (float)spellInfo->GetDuration() / (spellInfo->GetDuration() + spellCooldown));

                multiplier *= coverage;
                HandleApplyAura(effectInfo, multiplier, canNextTrigger, triggerCooldown);
                break;
            }
            case SPELL_EFFECT_HEAL:
            {
                /// @todo Handle spell without cooldown
                if (!spellCooldown)
                    break;
                float normalizedCd = std::max((float)spellCooldown / 1000, 5.0f);
                int32 val = AverageValue(effectInfo);
                float transfer_multiplier = 1;
                stats[STATS_TYPE_HEAL_POWER] += (float)val / normalizedCd * multiplier * transfer_multiplier;
                break;
            }
            case SPELL_EFFECT_ENERGIZE:
            {
                /// @todo Handle spell without cooldown
                if (!spellCooldown)
                    break;
                if (effectInfo.MiscValue != POWER_MANA)
                    break;
                float normalizedCd = std::max((float)spellCooldown / 1000, 5.0f);
                int32 val = AverageValue(effectInfo);
                float transfer_multiplier = 0.2;
                stats[STATS_TYPE_MANA_REGENERATION] += (float)val / normalizedCd * multiplier * transfer_multiplier;
                break;
            }
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            {
                /// @todo Handle spell without cooldown
                if (!spellCooldown)
                    break;
                float normalizedCd = std::max((float)spellCooldown / 1000, 5.0f);
                int32 val = AverageValue(effectInfo);
                if (type_ == CollectorType::MELEE || type_ == CollectorType::RANGED)
                {
                    float transfer_multiplier = 1;
                    stats[STATS_TYPE_ATTACK_POWER] += (float)val / normalizedCd * multiplier * transfer_multiplier;
                }
                else if (type_ == CollectorType::SPELL_DMG)
                {
                    float transfer_multiplier = 0.5;
                    stats[STATS_TYPE_SPELL_POWER] += (float)val / normalizedCd * multiplier * transfer_multiplier;
                }
                break;
            }
            default:
                break;
        }
    }
}

void StatsCollector::CollectEnchantStats(SpellItemEnchantmentEntry const* enchant)
{
    for (int s = 0; s < MAX_SPELL_ITEM_ENCHANTMENT_EFFECTS; ++s)
    {
        uint32 enchant_display_type = enchant->type[s];
        uint32 enchant_amount = enchant->amount[s];
        uint32 enchant_spell_id = enchant->spellid[s];

        if (SpecialEnchantFilter(enchant_spell_id))
            continue;

        switch (enchant_display_type)
        {
            case ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL:
            {
                if (type_ == CollectorType::MELEE)
                    CollectSpellStats(enchant_spell_id, 0.25f);
                break;
            }
            case ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL:
            {
                CollectSpellStats(enchant_spell_id, 1.0f);
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
bool StatsCollector::SpecialSpellFilter(uint32 spellId) {
    // trinket
    switch (spellId)
    {
        case 27521: // Insightful Earthstorm Diamond
            stats[STATS_TYPE_MANA_REGENERATION] += 20;
            return true;
        case 55381: // Insightful Earthsiege Diamond
            stats[STATS_TYPE_MANA_REGENERATION] += 40;
            return true;
        case 39442: // Darkmoon Card: Wrath
            if (type_ != CollectorType::SPELL_HEAL)
                stats[STATS_TYPE_CRIT] += 50;
            return true;
        case 59620: // Berserk
            if (type_ == CollectorType::MELEE)
                stats[STATS_TYPE_ATTACK_POWER] += 120;
            return true;
        case 67702: // Death's Verdict
            stats[STATS_TYPE_ATTACK_POWER] += 225;
            return true;
        case 67771: // Death's Verdict (heroic)
            stats[STATS_TYPE_ATTACK_POWER] += 260;
            return true;
        case 71406: // Tiny Abomination in a Jar
            if (cls_ == CLASS_PALADIN)
                stats[STATS_TYPE_ATTACK_POWER] += 600;
            else
                stats[STATS_TYPE_ATTACK_POWER] += 150;
            return true;
        case 71545: // Tiny Abomination in a Jar (heroic)
            if (cls_ == CLASS_PALADIN)
                stats[STATS_TYPE_ATTACK_POWER] += 800;
            else
                stats[STATS_TYPE_ATTACK_POWER] += 200;
            return true;    
        case 71519: // Deathbringer's Will
            stats[STATS_TYPE_ATTACK_POWER] += 350;
            return true;
        case 71562: // Deathbringer's Will (heroic)
            stats[STATS_TYPE_ATTACK_POWER] += 400;
            return true;
        case 71602: // Dislodged Foreign Object
            /// @todo The item can be triggered by heal spell, which mismatch with it's description
            /// Noticing that heroic item can not be triggered, probably a bug to report to AC
            if (type_ == CollectorType::SPELL_HEAL)
                return true;
            break;
        case 71903: // Shadowmourne
            stats[STATS_TYPE_STRENGTH] += 200;
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

bool StatsCollector::SpecialEnchantFilter(uint32 enchantSpellId)
{
    switch (enchantSpellId)
    {
        case 64440:
            if (type_ == CollectorType::MELEE)
            {
                stats[STATS_TYPE_PARRY] += 50;
            }
            return true;
        case 53365: // Rune of the Fallen Crusader
            if (type_ == CollectorType::MELEE)
            {
                stats[STATS_TYPE_STRENGTH] += 75;
            }
            return true;
        case 62157: // Rune of the Stoneskin Gargoyle
            if (type_ == CollectorType::MELEE)
            {
                stats[STATS_TYPE_DEFENSE] += 25;
                stats[STATS_TYPE_STAMINA] += 50;
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
            stats[STATS_TYPE_MANA_REGENERATION] += val / 10;
            break;
        case ITEM_MOD_HEALTH:
            stats[STATS_TYPE_STAMINA] += val / 15;
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

void StatsCollector::HandleApplyAura(const SpellEffectInfo& effectInfo, float multiplier, bool canNextTrigger, uint32 triggerCooldown)
{
    if (effectInfo.Effect != SPELL_EFFECT_APPLY_AURA)
        return;
    
    int32 val = AverageValue(effectInfo);
    
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
        case SPELL_AURA_MOD_INCREASE_HEALTH:
            stats[STATS_TYPE_STAMINA] += val * multiplier / 15;
            break;
        case SPELL_AURA_SCHOOL_ABSORB:
        {
            int32 schoolType = effectInfo.MiscValue;
            if (schoolType & SPELL_SCHOOL_MASK_NORMAL)
                stats[STATS_TYPE_STAMINA] += val * multiplier / 15;
            break;
        }
        case SPELL_AURA_MOD_ATTACK_POWER:
            if (type_ == CollectorType::MELEE)
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
                CollectSpellStats(effectInfo.TriggerSpell, multiplier, triggerCooldown);
            break;
        }
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        {
            if (canNextTrigger)
                CollectSpellStats(effectInfo.TriggerSpell, multiplier, triggerCooldown);
            break;
        }
        case SPELL_AURA_MOD_CRIT_DAMAGE_BONUS:
        {
            if (type_ != CollectorType::SPELL_HEAL)
            {
                int32 statType = effectInfo.MiscValue;
                if (statType & SPELL_SCHOOL_MASK_NORMAL) // physical
                    stats[STATS_TYPE_CRIT] += 30 * val * multiplier;
            }
            break;
        }
        default:
            break;
    }
}

int32 StatsCollector::AverageValue(const SpellEffectInfo& effectInfo)
{
    float basePointsPerLevel = effectInfo.RealPointsPerLevel;
    int32 basePoints = effectInfo.BasePoints;
    int32 randomPoints = int32(effectInfo.DieSides);

    switch (randomPoints)
    {
        case 0:
            break;
        case 1:
            basePoints += 1;
            break;
        default:
            int32 randvalue = (1 + randomPoints) / 2;
            basePoints += randvalue;
            break;
    }
    return basePoints;
}