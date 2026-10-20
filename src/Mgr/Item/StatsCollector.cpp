/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "StatsCollector.h"
#include "DBCStores.h"
#include "ItemTemplate.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIAware.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Util.h"

StatsCollector::StatsCollector(CollectorType type, int32 cls, int32 lvl) : type_(type), cls_(cls), lvl_(lvl)
{
    Reset();
}

void StatsCollector::Reset()
{
    for (uint32 i = 0; i < STATS_TYPE_MAX; i++)
    {
        stats[i] = 0;
    }
}

void StatsCollector::CollectItemStats(ItemTemplate const* proto)
{
    // Heirlooms and other scaling items hold their real stats in DBC, keyed by wearer level (Player::_ApplyItemBonuses)
    ScalingStatDistributionEntry const* ssd =
        proto->ScalingStatDistribution ? sScalingStatDistributionStore.LookupEntry(proto->ScalingStatDistribution)
                                       : nullptr;

    uint32 ssdLevel = lvl_ > 0 ? uint32(lvl_) : 0;
    if (ssd && ssdLevel > ssd->MaxLevel)
        ssdLevel = ssd->MaxLevel;

    // Without a wearer level (lvl_ -1) a scaling item keeps its template values.
    ScalingStatValuesEntry const* ssv =
        proto->ScalingStatValue && ssdLevel ? sScalingStatValuesStore.LookupEntry(ssdLevel) : nullptr;

    CollectWeaponDamageStats(proto, ssv);
    CollectItemStatValues(proto, ssd, ssv);

    if (ssv)
    {
        if (uint32 spellBonus = ssv->getSpellBonus(proto->ScalingStatValue))
            CollectByItemStatType(ITEM_MOD_SPELL_POWER, spellBonus);
    }

    uint32 armor = proto->Armor;
    if (ssv)
    {
        // An ssv entry exists only for scaling items, so the scaled armor always replaces the template value.
        if (uint32 ssvArmor = ssv->getArmorMod(proto->ScalingStatValue))
            armor = ssvArmor;
    }

    stats[STATS_TYPE_ARMOR] += armor;
    stats[STATS_TYPE_BLOCK_VALUE] += proto->Block;
    for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
    {
        switch (proto->Spells[j].SpellTrigger)
        {
            case ITEM_SPELLTRIGGER_ON_USE:
                CollectSpellStats(proto->Spells[j].SpellId, 1.0f, Milliseconds(proto->Spells[j].SpellCooldown));
                break;
            case ITEM_SPELLTRIGGER_ON_EQUIP:
                CollectSpellStats(proto->Spells[j].SpellId, 1.0f, Milliseconds(0));
                break;
            case ITEM_SPELLTRIGGER_CHANCE_ON_HIT:
            {
                // CanBeTriggeredByType inside CollectSpellStats gates which collector types a proc can trigger
                // for, so caster on-hit procs are valued for spell damage dealers too.
                if (proto->Spells[j].SpellPPMRate > 0.01f)
                    CollectSpellStats(proto->Spells[j].SpellId, 1.0f, Milliseconds(static_cast<int>(60000 / proto->Spells[j].SpellPPMRate)));
                else
                    CollectSpellStats(proto->Spells[j].SpellId, 1.0f, Milliseconds(static_cast<int>(60000 / 1.8f)));  // Default PPM = 1.8
                break;
            }
            default:
                break;
        }
    }

    if (proto->socketBonus)
    {
        if (SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(proto->socketBonus))
            CollectEnchantStats(enchant);
    }
}

void StatsCollector::CollectWeaponDamageStats(ItemTemplate const* proto, ScalingStatValuesEntry const* ssv)
{
    if (!proto->IsWeapon() || proto->Delay == 0)
        return;

    int32 extraDps = ssv ? ssv->getDPSMod(proto->ScalingStatValue) : 0;
    float dps = 0.0f;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
    {
        float minDamage = proto->Damage[i].DamageMin;
        float maxDamage = proto->Damage[i].DamageMax;

        // Scaling replaces the first damage entry: min 70% / max 130% of the scaled average, 80% / 120% two-hand.
        if (extraDps && i == 0)
        {
            float average = extraDps * proto->Delay / 1000.0f;
            float mod = ssv->IsTwoHand(proto->ScalingStatValue) ? 0.2f : 0.3f;

            minDamage = (1.0f - mod) * average;
            maxDamage = (1.0f + mod) * average;
        }

        dps += (minDamage + maxDamage) * 1000 / 2 / proto->Delay;
    }

    if (proto->IsRangedWeapon())
    {
        stats[STATS_TYPE_RANGED_DPS] += dps;
        return;
    }

    stats[STATS_TYPE_MELEE_DPS] += dps;
    // Feral forms convert weapon DPS into attack power, so treat it as attack power for Feral Druids.
    if (cls_ == CLASS_DRUID && (type_ & CollectorType::MELEE))
        stats[STATS_TYPE_ATTACK_POWER] += proto->getFeralBonus(extraDps);
}

void StatsCollector::CollectItemStatValues(ItemTemplate const* proto, ScalingStatDistributionEntry const* ssd,
                                           ScalingStatValuesEntry const* ssv)
{
    if (ssv)
    {
        // A scaling item draws its stats from the distribution entry alone; its template stats are ignored, as in core.
        if (!ssd)
            return;

        uint32 multiplier = ssv->getssdMultiplier(proto->ScalingStatValue);
        for (uint8 i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
        {
            if (ssd->StatMod[i] < 0)
                continue;

            CollectByItemStatType(ssd->StatMod[i], int32(multiplier * ssd->Modifier[i] / 10000));
        }
        return;
    }

    for (uint32 i = 0; i < proto->StatsCount; ++i)
    {
        _ItemStat const& stat = proto->ItemStat[i];
        CollectByItemStatType(stat.ItemStatType, stat.ItemStatValue);
    }
}

void StatsCollector::CollectSpellStats(uint32 spellId, float multiplier, Milliseconds spellCooldown)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);

    if (!spellInfo)
        return;

    if (SpecialSpellFilter(spellId))
        return;

    // Form-restricted item auras (e.g., feral attack power, idol boosts) should be considered by Druids only.
    if (spellInfo->Stances && cls_ != CLASS_DRUID)
        return;

    SpellProcEntry const* eventEntry = sSpellMgr->GetSpellProcEntry(spellInfo->Id);

    Milliseconds triggerCooldown = eventEntry ? eventEntry->Cooldown : 0ms;

    bool canNextTrigger = true;

    uint32 procFlags;
    uint32 procChance;
    if (eventEntry && eventEntry->ProcFlags)
        procFlags = eventEntry->ProcFlags;
    else
        procFlags = spellInfo->ProcFlags;

    if (eventEntry && eventEntry->Chance)
        procChance = eventEntry->Chance;
    else
        procChance = spellInfo->ProcChance;
    bool lowChance = procChance <= 5;

    if (lowChance || (procFlags && !CanBeTriggeredByType(spellInfo, procFlags)))
        canNextTrigger = false;

    if (spellInfo->StackAmount)
    {
        // Heuristic multiplier for spell with stackAmount since high stackAmount may not be available
        if (spellInfo->StackAmount <= 1)
            multiplier *= spellInfo->StackAmount * 1;
        else if (spellInfo->StackAmount <= 5)
            multiplier *= 1 + (spellInfo->StackAmount - 1) * 0.75;
        else if (spellInfo->StackAmount <= 10)
            multiplier *= 4 + (spellInfo->StackAmount - 5) * 0.6;
        else if (spellInfo->StackAmount <= 20)
            multiplier *= 7 + (spellInfo->StackAmount - 10) * 0.4;
        else
            multiplier *= 11;
    }

    for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
    {
        SpellEffectInfo const& effectInfo = spellInfo->Effects[i];
        if (!effectInfo.Effect)
            continue;
        switch (effectInfo.Effect)
        {
            case SPELL_EFFECT_APPLY_AURA:
            {
                if (spellInfo->SpellFamilyName && /*effectInfo.ApplyAuraName != SPELL_AURA_DUMMY &&*/
                    effectInfo.ApplyAuraName != SPELL_AURA_PROC_TRIGGER_SPELL)
                {
                    if (!CheckSpellValidation(spellInfo->SpellFamilyName, effectInfo.SpellClassMask))
                        return;

                    // Some dummy effects cannot be recognized, make some bonus to identify
                    stats[STATS_TYPE_BONUS] += 1;
                }

                /// @todo Handle negative spell
                if (!spellInfo->IsPositive())
                    break;

                float coverage;
                if (spellCooldown.count() <= 2000 || spellInfo->GetDuration() == -1)
                    coverage = 1.0f;
                else
                    coverage =
                        std::min(1.0f, (float)spellInfo->GetDuration() / (spellInfo->GetDuration() + spellCooldown.count()));

                // Do not mutate `multiplier`: a spell with several APPLY_AURA effects would otherwise
                // get its coverage applied once per effect (e.g. Deathbringer's Will buffs carry a
                // TRANSFORM visual + the stat aura).
                HandleApplyAura(effectInfo, spellInfo, multiplier * coverage, canNextTrigger, triggerCooldown);
                break;
            }
            case SPELL_EFFECT_HEAL:
            {
                /// @todo Handle spell without cooldown
                if (!spellCooldown.count())
                    break;
                float normalizedCd = std::max((float)spellCooldown.count() / 1000, 5.0f);
                int32 val = AverageValue(effectInfo, spellInfo);
                float transfer_multiplier = 1;
                stats[STATS_TYPE_HEAL_POWER] += (float)val / normalizedCd * multiplier * transfer_multiplier;
                break;
            }
            case SPELL_EFFECT_ENERGIZE:
            {
                /// @todo Handle spell without cooldown
                if (!spellCooldown.count())
                    break;
                if (effectInfo.MiscValue != POWER_MANA)
                    break;
                float normalizedCd = std::max((float)spellCooldown.count() / 1000, 5.0f);
                int32 val = AverageValue(effectInfo, spellInfo);
                float transfer_multiplier = 0.2;
                stats[STATS_TYPE_MANA_REGENERATION] += (float)val / normalizedCd * multiplier * transfer_multiplier;
                break;
            }
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            {
                /// @todo Handle spell without cooldown
                if (!spellCooldown.count())
                    break;
                float normalizedCd = std::max((float)spellCooldown.count() / 1000, 5.0f);
                int32 val = AverageValue(effectInfo, spellInfo);
                if (type_ & (CollectorType::MELEE | CollectorType::RANGED))
                {
                    float transfer_multiplier = 1;
                    stats[STATS_TYPE_ATTACK_POWER] += (float)val / normalizedCd * multiplier * transfer_multiplier;
                }
                else if (type_ & CollectorType::SPELL_DMG)
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

void StatsCollector::CollectEnchantStats(SpellItemEnchantmentEntry const* enchant, uint32 default_enchant_amount)
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
                if (type_ & CollectorType::MELEE)
                    CollectSpellStats(enchant_spell_id, 0.25f);
                break;
            }
            case ITEM_ENCHANTMENT_TYPE_DAMAGE:
            {
                // Flat weapon-damage enchant; approximated as raw DPS (true gain depends on weapon speed)
                if (type_ & CollectorType::MELEE)
                    stats[STATS_TYPE_MELEE_DPS] += enchant_amount;
                break;
            }
            case ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL:
            {
                CollectSpellStats(enchant_spell_id, 1.0f);
                break;
            }
            case ITEM_ENCHANTMENT_TYPE_STAT:
            {
                // for item random suffix
                if (!enchant_amount)
                    enchant_amount = default_enchant_amount;

                if (!enchant_amount)
                {
                    break;
                }
                CollectByItemStatType(enchant_spell_id, enchant_amount);
                break;
            }
            case ITEM_ENCHANTMENT_TYPE_RESISTANCE:
            {
                // spellid holds the school; school 0 is armor (e.g. Enchant Cloak - Superior Defense)
                if (enchant_spell_id != SPELL_SCHOOL_NORMAL)
                    break;

                // for item random suffix
                if (!enchant_amount)
                    enchant_amount = default_enchant_amount;

                stats[STATS_TYPE_ARMOR] += enchant_amount;
                break;
            }
            default:
                break;
        }
    }
}

/// @todo Special case for some spell that hard to calculate, like trinket, relic, etc.

// Classes whose Death's Choice / Deathbringer's Will proc buff is Strength.
// The core script (spell_item_death_choice / spell_item_deathbringers_will_*) picks the buff
// from max(str, agi) or a per-class pool; Strength users get the STR variant.
static bool UsesStrengthProcBuff(uint32 cls)
{
    return cls == CLASS_WARRIOR || cls == CLASS_PALADIN || cls == CLASS_DEATH_KNIGHT;
}

void StatsCollector::CollectScriptedProcBuff(uint32 procSpellId, uint32 buffSpellId, float multiplier)
{
    Milliseconds icd = 0ms;
    if (SpellProcEntry const* entry = sSpellMgr->GetSpellProcEntry(procSpellId))
        icd = entry->Cooldown;
    CollectSpellStats(buffSpellId, multiplier, icd);
}

void StatsCollector::CollectStackTriggerProc(uint32 procSpellId, uint32 triggerSpellId)
{
    SpellInfo const* procInfo = sSpellMgr->GetSpellInfo(procSpellId);
    if (!procInfo)
        return;

    Milliseconds icd = 0ms;
    if (SpellProcEntry const* entry = sSpellMgr->GetSpellProcEntry(procSpellId))
        icd = entry->Cooldown;

    // Stacks required to fire the trigger = the proc aura's amount (base points + die side).
    uint32 stacks = uint32(std::max(AverageValue(procInfo->Effects[0], procInfo), 1.0f));
    Milliseconds cycle = icd * stacks;
    if (cycle <= 0ms)
        cycle = Milliseconds(1000);

    CollectSpellStats(triggerSpellId, 1.0f, cycle);
}

bool StatsCollector::SpecialSpellFilter(uint32 spellId)
{
    // trinket
    switch (spellId)
    {
        case 60764:  // Totem of Splintering: enhancement-shaman relic (dummy aura), worthless for casters
            if (type_ & (CollectorType::SPELL))
                return true;
            break;
        // NOTE: Insightful Earthstorm/Earthsiege (27521/55381), Darkmoon Card: Wrath (39442) and
        // Berserking (59620) have their trigger/value in the DBC and are handled by the generic
        // trigger-chase / combat-enchant path - no special case needed.
        case 67702:  // Death's Verdict (normal): spell_item_death_choice casts +450 STR or AGI (67708/67703), max(str, agi)
            CollectScriptedProcBuff(67702, UsesStrengthProcBuff(cls_) ? 67708 : 67703);
            return true;
        case 67771:  // Death's Verdict (heroic): +510 STR or AGI (67773/67772)
            CollectScriptedProcBuff(67771, UsesStrengthProcBuff(cls_) ? 67773 : 67772);
            return true;
        case 71406:  // Tiny Abomination in a Jar: 8 motes (50% on hit) -> Manifest Anger (71433/71434), a normalized
                     // weapon-damage attack with no fixed DBC value; approximated as AP (paladins gain more from it)
            if (cls_ == CLASS_PALADIN)
                stats[STATS_TYPE_ATTACK_POWER] += 700;
            else
                stats[STATS_TYPE_ATTACK_POWER] += 500;
            return true;
        case 71545:  // Tiny Abomination in a Jar (heroic): 7 motes to trigger
            if (cls_ == CLASS_PALADIN)
                stats[STATS_TYPE_ATTACK_POWER] += 800;
            else
                stats[STATS_TYPE_ATTACK_POWER] += 600;
            return true;
        case 67712:  // Reign of the Dead (normal): spell_item_trinket_stack - mote 67713 (3 stacks) -> Pillar of Flame 67714
            CollectStackTriggerProc(67712, 67714);
            return true;
        case 67758:  // Reign of the Dead (heroic): mote 67759 -> Pillar of Flame 67760
            CollectStackTriggerProc(67758, 67760);
            return true;
        case 57345:  // Darkmoon Card: Greatness - script casts the caster's highest stat (60229 STR/60233 AGI/60234 INT/60235 SPI)
        {
            uint32 buff = UsesStrengthProcBuff(cls_) ? 60229 : 60233;
            if (type_ & CollectorType::SPELL)
                buff = 60234;  // intellect for casters
            CollectScriptedProcBuff(57345, buff);
            return true;
        }
        case 71519:  // Deathbringer's Will (normal): per-class pool of 3 random buffs -> value = average of the pool
        case 71562:  // Deathbringer's Will (heroic)
        {
            bool heroic = spellId == 71562;
            uint32 str   = heroic ? 71561 : 71484;  // Strength of the Taunka
            uint32 agi   = heroic ? 71556 : 71485;  // Agility of the Vrykul
            uint32 power = heroic ? 71558 : 71486;  // Power of the Taunka (AP)
            uint32 aim   = heroic ? 71559 : 71491;  // Aim of the Iron Dwarves (crit)
            uint32 speed = heroic ? 71560 : 71492;  // Speed of the Vrykul (haste)

            auto addPool = [&](uint32 a, uint32 b, uint32 c)
            {
                CollectScriptedProcBuff(spellId, a, 1.0f / 3.0f);
                CollectScriptedProcBuff(spellId, b, 1.0f / 3.0f);
                CollectScriptedProcBuff(spellId, c, 1.0f / 3.0f);
            };
            switch (cls_)
            {
                case CLASS_WARRIOR:
                case CLASS_PALADIN:
                case CLASS_DEATH_KNIGHT:
                    addPool(str, aim, speed);
                    break;
                case CLASS_HUNTER:
                    addPool(agi, aim, power);
                    break;
                case CLASS_ROGUE:
                case CLASS_SHAMAN:
                    addPool(agi, speed, power);
                    break;
                case CLASS_DRUID:
                    addPool(str, agi, speed);
                    break;
                default:
                    break;  // priest/mage/warlock: script does not proc
            }
            return true;
        }
        case 71602:  // Dislodged Foreign Object
            /// @todo The item can be triggered by heal spell, which mismatch with it's description
            /// Noticing that heroic item can not be triggered, probably a bug to report to AC
            if (type_ & CollectorType::SPELL_HEAL)
                return true;
            break;
        case 71903:  // Shadowmourne: soul fragment 71905 (+30 STR x10, resets at 10) -> chaos bane 73422 (+270 STR 10 sec)
            CollectScriptedProcBuff(71903, 71905);        // fragments: +30 STR x stack heuristic
            CollectScriptedProcBuff(71903, 73422, 0.4f);  // chaos bane: ~40% uptime (10 sec per ~25 sec cycle)
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
            if (type_ & CollectorType::MELEE)
            {
                stats[STATS_TYPE_PARRY] += 50;
            }
            return true;
        case 53365:  // Rune of the Fallen Crusader
            if (type_ & CollectorType::MELEE)
            {
                stats[STATS_TYPE_STRENGTH] += 75;
            }
            return true;
        case 62157:  // Rune of the Stoneskin Gargoyle
            if (type_ & CollectorType::MELEE)
            {
                stats[STATS_TYPE_DEFENSE] += 25;
                stats[STATS_TYPE_STAMINA] += 50;
            }
            return true;
        case 64571:  // Blood draining
            if (type_ & CollectorType::MELEE)
            {
                stats[STATS_TYPE_STAMINA] += 50;
            }
            return true;
        default:
            break;
    }
    return false;
}

bool StatsCollector::CanBeTriggeredByType(SpellInfo const* spellInfo, uint32 procFlags, bool strict)
{
    SpellProcEntry const* eventEntry = sSpellMgr->GetSpellProcEntry(spellInfo->Id);
    uint32 spellFamilyName = 0;
    if (eventEntry)
    {
        spellFamilyName = eventEntry->SpellFamilyName;
        flag96 spellFamilyMask = eventEntry->SpellFamilyMask;
        if (spellFamilyName != 0)
        {
            if (!CheckSpellValidation(spellFamilyName, spellFamilyMask, strict))
                return false;
        }
    }

    uint32 triggerMask = TAKEN_HIT_PROC_FLAG_MASK;  // Generic trigger mask
    switch (type_)
    {
        case CollectorType::MELEE_DMG:
        {
            triggerMask |= MELEE_PROC_FLAG_MASK;
            triggerMask |= SPELL_PROC_FLAG_MASK;
            triggerMask |= PROC_FLAG_DONE_PERIODIC;
            if (procFlags & triggerMask)
                return true;
            break;
        }
        case CollectorType::MELEE_TANK:
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
            triggerMask |= PROC_FLAG_DONE_PERIODIC;
            if (procFlags & triggerMask)
                return true;
            break;
        }
        case CollectorType::SPELL_DMG:
        {
            triggerMask |= SPELL_PROC_FLAG_MASK;
            triggerMask |= PROC_FLAG_DONE_PERIODIC;
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
            triggerMask |= PROC_FLAG_DONE_PERIODIC;
            // Dmg spell should not trigger
            triggerMask &= ~PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;
            triggerMask &= ~PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG;
            if (!spellFamilyName)
                triggerMask &=
                    ~PROC_FLAG_DONE_PERIODIC;  // spellFamilyName = 0 and PROC_FLAG_DONE_PERIODIC -> it is a dmg spell
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
            stats[STATS_TYPE_MANA_REGENERATION] += (float)val / 10;
            break;
        case ITEM_MOD_HEALTH:
            stats[STATS_TYPE_STAMINA] += (float)val / 15;
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
            if (type_ & CollectorType::MELEE)
                stats[STATS_TYPE_HIT] += val;
            break;
        case ITEM_MOD_HIT_RANGED_RATING:
            if (type_ & CollectorType::RANGED)
                stats[STATS_TYPE_HIT] += val;
            break;
        case ITEM_MOD_HIT_SPELL_RATING:
            if (type_ & CollectorType::SPELL)
                stats[STATS_TYPE_HIT] += val;
            break;
        case ITEM_MOD_CRIT_MELEE_RATING:
            if (type_ & CollectorType::MELEE)
                stats[STATS_TYPE_CRIT] += val;
            break;
        case ITEM_MOD_CRIT_RANGED_RATING:
            if (type_ & CollectorType::RANGED)
                stats[STATS_TYPE_CRIT] += val;
            break;
        case ITEM_MOD_CRIT_SPELL_RATING:
            if (type_ & CollectorType::SPELL)
                stats[STATS_TYPE_CRIT] += val;
            break;
        case ITEM_MOD_HASTE_MELEE_RATING:
            if (type_ & CollectorType::MELEE)
                stats[STATS_TYPE_HASTE] += val;
            break;
        case ITEM_MOD_HASTE_RANGED_RATING:
            if (type_ & CollectorType::RANGED)
                stats[STATS_TYPE_HASTE] += val;
            break;
        case ITEM_MOD_HASTE_SPELL_RATING:
            if (type_ & CollectorType::SPELL)
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

void StatsCollector::HandleApplyAura(SpellEffectInfo const& effectInfo, SpellInfo const* spellInfo, float multiplier,
                                     bool canNextTrigger, Milliseconds triggerCooldown)
{
    if (effectInfo.Effect != SPELL_EFFECT_APPLY_AURA)
        return;

    int32 val = AverageValue(effectInfo, spellInfo);

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
        case SPELL_AURA_PERIODIC_DAMAGE:
        {
            // Value is damage per tick; Amplitude is the tick interval in ms.
            float perSecond = effectInfo.Amplitude ? (val * 1000.0f / effectInfo.Amplitude) : val;
            if (type_ & (CollectorType::MELEE | CollectorType::RANGED))
                stats[STATS_TYPE_ATTACK_POWER] += perSecond * multiplier;
            else if (type_ & CollectorType::SPELL_DMG)
                stats[STATS_TYPE_SPELL_POWER] += perSecond * multiplier * 0.5f;
            break;
        }
        case SPELL_AURA_PERIODIC_HEAL:
        {
            float perSecond = effectInfo.Amplitude ? (val * 1000.0f / effectInfo.Amplitude) : val;
            stats[STATS_TYPE_HEAL_POWER] += perSecond * multiplier;
            break;
        }
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
            if (type_ & CollectorType::MELEE)
                stats[STATS_TYPE_ATTACK_POWER] += val * multiplier;
            break;
        case SPELL_AURA_MOD_RANGED_ATTACK_POWER:
            if (type_ & CollectorType::RANGED)
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
                case -1:  // Stat all
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
            if (statType & SPELL_SCHOOL_MASK_NORMAL)  // physical
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
                            if (type_ & CollectorType::MELEE)
                                stats[STATS_TYPE_HIT] += val * multiplier;
                            break;
                        case CR_HIT_RANGED:
                            if (type_ & CollectorType::RANGED)
                                stats[STATS_TYPE_HIT] += val * multiplier;
                            break;
                        case CR_HIT_SPELL:
                            if (type_ & CollectorType::SPELL)
                                stats[STATS_TYPE_HIT] += val * multiplier;
                            break;
                        case CR_CRIT_MELEE:
                            if (type_ & CollectorType::MELEE)
                                stats[STATS_TYPE_CRIT] += val * multiplier;
                            break;
                        case CR_CRIT_RANGED:
                            if (type_ & CollectorType::RANGED)
                                stats[STATS_TYPE_CRIT] += val * multiplier;
                            break;
                        case CR_CRIT_SPELL:
                            if (type_ & CollectorType::SPELL)
                                stats[STATS_TYPE_CRIT] += val * multiplier;
                            break;
                        case CR_HASTE_MELEE:
                            if (type_ & CollectorType::MELEE)
                                stats[STATS_TYPE_HASTE] += val * multiplier;
                            break;
                        case CR_HASTE_RANGED:
                            if (type_ & CollectorType::RANGED)
                                stats[STATS_TYPE_HASTE] += val * multiplier;
                            break;
                        case CR_HASTE_SPELL:
                            if (type_ & CollectorType::SPELL)
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
            break;
        }
        case SPELL_AURA_MOD_POWER_REGEN:
        {
            int32 powerType = effectInfo.MiscValue;
            switch (powerType)
            {
                case POWER_MANA:
                    stats[STATS_TYPE_MANA_REGENERATION] += val * multiplier;
                    break;
                default:
                    break;
            }
            break;
        }
        case SPELL_AURA_MOD_REGEN:
            // Health regeneration (mana is handled by SPELL_AURA_MOD_POWER_REGEN); per-5s value like ITEM_MOD_HEALTH_REGEN
            stats[STATS_TYPE_HEALTH_REGENERATION] += val * multiplier;
            break;
        case SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT:
            stats[STATS_TYPE_HEALTH_REGENERATION] += val * multiplier;
            break;
        case SPELL_AURA_MOD_INCREASE_HEALTH_2:
            // Same as SPELL_AURA_MOD_INCREASE_HEALTH: flat max health -> stamina (15 hp per stamina)
            stats[STATS_TYPE_STAMINA] += val * multiplier / 15;
            break;
        case SPELL_AURA_MOD_TARGET_RESISTANCE:
        {
            // Mirrors AuraEffect::HandleModTargetResistance: physical -> armor pen, full spell -> spell pen
            int32 schoolType = effectInfo.MiscValue;
            if (schoolType & SPELL_SCHOOL_MASK_NORMAL)
                stats[STATS_TYPE_ARMOR_PENETRATION] += val * multiplier;
            if ((schoolType & SPELL_SCHOOL_MASK_SPELL) == SPELL_SCHOOL_MASK_SPELL)
                stats[STATS_TYPE_SPELL_PENETRATION] += val * multiplier;
            break;
        }
        case SPELL_AURA_MOD_POWER_COST_SCHOOL:
            // Flat mana-cost reduction (e.g. Spark of Hope); stored negative in the DBC,
            // so negate to add as mana saved per cast.
            stats[STATS_TYPE_MANA_REGENERATION] += -val * multiplier;
            break;
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
        case SPELL_AURA_ADD_TARGET_TRIGGER:
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
                if (statType & SPELL_SCHOOL_MASK_NORMAL)  // physical
                    stats[STATS_TYPE_CRIT] += 30 * val * multiplier;
            }
            break;
        }
        default:
            break;
    }
}

float StatsCollector::AverageValue(SpellEffectInfo const& effectInfo, SpellInfo const* spellInfo)
{
    float basePoints = effectInfo.BasePoints;
    int32 randomPoints = effectInfo.DieSides;

    switch (randomPoints)
    {
        case 0:
            break;
        case 1:
            basePoints += 1;
            break;
        default:
            float randvalue = (1 + randomPoints) / 2.0f;
            basePoints += randvalue;
            break;
    }

    // Level-scaled effects (mirrors SpellEffectInfo::CalcValue in the core)
    if (spellInfo && lvl_ > 0 && effectInfo.RealPointsPerLevel != 0.0f)
    {
        int32 level = lvl_;
        if (spellInfo->MaxLevel > 0 && level > int32(spellInfo->MaxLevel))
            level = int32(spellInfo->MaxLevel);
        else if (level < int32(spellInfo->BaseLevel))
            level = int32(spellInfo->BaseLevel);
        level -= int32(std::max(spellInfo->BaseLevel, spellInfo->SpellLevel));
        basePoints += level * effectInfo.RealPointsPerLevel;
    }

    return basePoints;
}

bool StatsCollector::CheckSpellValidation(uint32 spellFamilyName, flag96 spelFalimyFlags, bool strict)
{
    if (PlayerbotAI::Class2SpellFamilyName(cls_) != spellFamilyName)
        return false;

    bool isHealingSpell = PlayerbotAI::IsHealingSpell(spellFamilyName, spelFalimyFlags);
    // strict to healer
    if (strict && (type_ & CollectorType::SPELL_HEAL))
    {
        return isHealingSpell;
    }

    if (!(type_ & CollectorType::SPELL_HEAL) && isHealingSpell)
        return false;

    // spells for caster/melee/tank are ambiguous
    if (cls_ == CLASS_DRUID && spellFamilyName == SPELLFAMILY_DRUID && (type_ & CollectorType::MELEE))
    {
        uint32 castingFlagsA = 0x4 | 0x2 | 0x1 | 0x200000;  // starfire | moonfire | wrath | insect swarm
        uint32 castingFlagsB = 0x0;
        uint32 castingFlagsC = 0x0;
        flag96 invalidFlags = {castingFlagsA, castingFlagsB, castingFlagsC};
        if (spelFalimyFlags & invalidFlags)
            return false;
    }

    if (cls_ == CLASS_PALADIN && spellFamilyName == SPELLFAMILY_PALADIN && (type_ & CollectorType::MELEE_TANK))
    {
        uint32 retributionFlagsA = 0x0;
        uint32 retributionFlagsB = 0x8000;  // crusader strike
        uint32 retributionFlagsC = 0x0;
        flag96 invalidFlags = {retributionFlagsA, retributionFlagsB, retributionFlagsC};
        if (spelFalimyFlags & invalidFlags)
            return false;
    }

    if (cls_ == CLASS_PALADIN && spellFamilyName == SPELLFAMILY_PALADIN && (type_ & CollectorType::MELEE_DMG))
    {
        uint32 retributionFlagsA = 0x0;
        uint32 retributionFlagsB = 0x100000 | 0x40;  // shield of righteouness | holy shield
        uint32 retributionFlagsC = 0x0;
        flag96 invalidFlags = {retributionFlagsA, retributionFlagsB, retributionFlagsC};
        if (spelFalimyFlags & invalidFlags)
            return false;
    }

    if (cls_ == CLASS_SHAMAN && spellFamilyName == SPELLFAMILY_SHAMAN && (type_ & CollectorType::SPELL_DMG))
    {
        uint32 meleeFlagsA = 0x0;
        uint32 meleeFlagsB = 0x1000010;  // stromstrike
        uint32 meleeFlagsC = 0x4;        // lava lash
        flag96 invalidFlags = {meleeFlagsA, meleeFlagsB, meleeFlagsC};
        if (spelFalimyFlags & invalidFlags)
            return false;
    }

    if (cls_ == CLASS_SHAMAN && spellFamilyName == SPELLFAMILY_SHAMAN && (type_ & CollectorType::MELEE_DMG))
    {
        uint32 casterFlagsA = 0x0;
        uint32 casterFlagsB = 0x1000;  // lava burst
        uint32 casterFlagsC = 0x0;
        flag96 invalidFlags = {casterFlagsA, casterFlagsB, casterFlagsC};
        if (spelFalimyFlags & invalidFlags)
            return false;
    }

    return true;
}
