/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpLoadoutStats.h"
#include "DBCStores.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "StatsCollector.h"
#include <algorithm>

namespace
{
CollectorType CollectorFor(PvpLoadout::Role role)
{
    switch (role)
    {
        case PvpLoadout::Role::Hunter:
            return CollectorType::RANGED;
        case PvpLoadout::Role::Caster:
            return CollectorType::SPELL_DMG;
        case PvpLoadout::Role::Healer:
            return CollectorType::SPELL_HEAL;
        case PvpLoadout::Role::Melee:
        default:
            return CollectorType::MELEE_DMG;
    }
}

CombatRating HitRating(PvpLoadout::Role role)
{
    return role == PvpLoadout::Role::Melee    ? CR_HIT_MELEE
           : role == PvpLoadout::Role::Hunter ? CR_HIT_RANGED
                                              : CR_HIT_SPELL;
}

// How many level-80 rating points (the profiles' level) one rating point at the bot's level is worth. The class scalar
// applies at every level, so it cancels out.
float RatingScale(Player const* bot, CombatRating rating)
{
    uint32 const level = std::clamp<uint32>(bot->GetLevel(), 1, GT_MAX_LEVEL);
    GtCombatRatingsEntry const* atLevel = sGtCombatRatingsStore.LookupEntry(rating * GT_MAX_LEVEL + level - 1);
    GtCombatRatingsEntry const* atProfile =
        sGtCombatRatingsStore.LookupEntry(rating * GT_MAX_LEVEL + DEFAULT_MAX_LEVEL - 1);
    return atLevel && atProfile && atLevel->ratio > 0.0f ? atProfile->ratio / atLevel->ratio : 1.0f;
}

PvpLoadout::StatVector ToStatVector(StatsCollector const& collector, Player const* bot, PvpLoadout::Role role)
{
    using PvpLoadout::At;
    using PvpLoadout::Stat;

    bool const spell = role == PvpLoadout::Role::Caster || role == PvpLoadout::Role::Healer;
    float const* stats = collector.stats;
    PvpLoadout::StatVector vector{};
    At(vector, Stat::Hit) = stats[STATS_TYPE_HIT] * RatingScale(bot, HitRating(role));
    At(vector, Stat::SpellPenetration) = stats[STATS_TYPE_SPELL_PENETRATION];
    At(vector, Stat::Resilience) = stats[STATS_TYPE_RESILIENCE] * RatingScale(bot, CR_CRIT_TAKEN_MELEE);
    At(vector, Stat::Stamina) = stats[STATS_TYPE_STAMINA];
    At(vector, Stat::Strength) = stats[STATS_TYPE_STRENGTH];
    At(vector, Stat::Agility) = stats[STATS_TYPE_AGILITY];
    At(vector, Stat::AttackPower) = stats[STATS_TYPE_ATTACK_POWER];
    At(vector, Stat::ArmorPenetration) = stats[STATS_TYPE_ARMOR_PENETRATION] * RatingScale(bot, CR_ARMOR_PENETRATION);
    // StatsCollector counts plain spell power as both; healing-only power is a healer's spell power.
    At(vector, Stat::SpellPower) =
        role == PvpLoadout::Role::Healer ? stats[STATS_TYPE_HEAL_POWER] : stats[STATS_TYPE_SPELL_POWER];
    At(vector, Stat::Haste) = stats[STATS_TYPE_HASTE] * RatingScale(bot, spell ? CR_HASTE_SPELL : CR_HASTE_MELEE);
    At(vector, Stat::Crit) = stats[STATS_TYPE_CRIT] * RatingScale(bot, spell ? CR_CRIT_SPELL : CR_CRIT_MELEE);
    At(vector, Stat::Mp5) = stats[STATS_TYPE_MANA_REGENERATION];
    At(vector, Stat::Spirit) = stats[STATS_TYPE_SPIRIT];
    At(vector, Stat::Intellect) = stats[STATS_TYPE_INTELLECT];
    At(vector, Stat::WeaponDps) =
        role == PvpLoadout::Role::Hunter ? stats[STATS_TYPE_RANGED_DPS] : stats[STATS_TYPE_MELEE_DPS];
    return vector;
}

uint16 CombatRatingField(CombatRating rating) { return static_cast<uint16>(PLAYER_FIELD_COMBAT_RATING_1) + rating; }
}  // namespace

PvpLoadout::Role PvpLoadoutStats::RoleOf(Player* bot)
{
    if (bot->getClass() == CLASS_HUNTER)
        return PvpLoadout::Role::Hunter;

    if (PlayerbotAI::IsHeal(bot, true))
        return PvpLoadout::Role::Healer;

    if (PlayerbotAI::IsCaster(bot, true))
        return PvpLoadout::Role::Caster;

    return PvpLoadout::Role::Melee;
}

PvpLoadout::StatVector PvpLoadoutStats::ItemStats(Player const* bot, PvpLoadout::Role role, ItemTemplate const* proto)
{
    StatsCollector collector(CollectorFor(role), bot->getClass());
    collector.CollectItemStats(proto);
    return ToStatVector(collector, bot, role);
}

PvpLoadout::StatVector PvpLoadoutStats::EnchantStats(Player const* bot, PvpLoadout::Role role, uint32 enchantId)
{
    SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchantId);
    if (!enchant)
        return {};

    StatsCollector collector(CollectorFor(role), bot->getClass());
    collector.CollectEnchantStats(enchant);
    return ToStatVector(collector, bot, role);
}

PvpLoadout::StatVector PvpLoadoutStats::MeasureCappedTotals(Player* bot, PvpLoadout::Role role)
{
    CombatRating const hitRating = HitRating(role);

    PvpLoadout::StatVector totals{};
    PvpLoadout::At(totals, PvpLoadout::Stat::Hit) =
        float(bot->GetUInt32Value(CombatRatingField(hitRating))) * RatingScale(bot, hitRating);
    PvpLoadout::At(totals, PvpLoadout::Stat::Resilience) =
        float(bot->GetUInt32Value(CombatRatingField(CR_CRIT_TAKEN_MELEE))) * RatingScale(bot, CR_CRIT_TAKEN_MELEE);
    // Spell penetration is applied as a negative target resistance.
    PvpLoadout::At(totals, PvpLoadout::Stat::SpellPenetration) =
        float(std::max(0, -bot->GetInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE)));
    return totals;
}
