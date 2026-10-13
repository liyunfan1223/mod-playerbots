/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpLoadoutEp.h"
#include "Helpers.h"
#include "PvpLoadoutRules.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace PvpLoadout
{
namespace
{
// The profile's order turned into weights: a target still open outranks anything spent past it.
constexpr float HIT_WEIGHT = 3.0f;
constexpr float SPELL_PENETRATION_WEIGHT = 2.5f;
constexpr float RESILIENCE_WEIGHT = 2.0f;
constexpr float RESILIENCE_PAST_TARGET_WEIGHT = 0.5f;
constexpr float STAMINA_WEIGHT = 0.5f;
constexpr std::array<float, 4> PRIORITY_WEIGHTS = {1.5f, 1.2f, 1.0f, 0.8f};
constexpr float WEAPON_DPS_WEIGHT = 3.0f;
// Marginal EP within this of each other counts as a tie, so float noise cannot replace an item the bot already wears.
constexpr float EP_TIE = 0.001f;

std::unordered_map<std::string, Stat> const& StatNames()
{
    static std::unordered_map<std::string, Stat> const names = {
        {"hit", Stat::Hit},
        {"spellpen", Stat::SpellPenetration},
        {"resilience", Stat::Resilience},
        {"stamina", Stat::Stamina},
        {"strength", Stat::Strength},
        {"agility", Stat::Agility},
        {"attackpower", Stat::AttackPower},
        {"armorpen", Stat::ArmorPenetration},
        {"spellpower", Stat::SpellPower},
        {"haste", Stat::Haste},
        {"crit", Stat::Crit},
        {"mp5", Stat::Mp5},
        {"spirit", Stat::Spirit},
        {"intellect", Stat::Intellect},
    };
    return names;
}

float Capped(float value, float target, float weightBelow, float weightAbove)
{
    return weightBelow * std::min(value, target) + weightAbove * std::max(0.0f, value - target);
}
}  // namespace

float& At(StatVector& stats, Stat stat) { return stats[static_cast<size_t>(stat)]; }

float At(StatVector const& stats, Stat stat) { return stats[static_cast<size_t>(stat)]; }

StatVector Add(StatVector lhs, StatVector const& rhs)
{
    for (size_t i = 0; i < lhs.size(); ++i)
        lhs[i] += rhs[i];
    return lhs;
}

StatVector Subtract(StatVector lhs, StatVector const& rhs)
{
    for (size_t i = 0; i < lhs.size(); ++i)
        lhs[i] = std::max(0.0f, lhs[i] - rhs[i]);
    return lhs;
}

Profile DefaultProfile(Role role)
{
    switch (role)
    {
        case Role::Hunter:
            return {164, 130, 1050, {Stat::Agility, Stat::ArmorPenetration, Stat::Crit}, WEAPON_DPS_WEIGHT};
        case Role::Caster:
            return {105, 130, 1250, {Stat::SpellPower, Stat::Haste, Stat::Crit}, 0.0f};
        case Role::Healer:
            return {0, 0, 1400, {Stat::Mp5, Stat::Spirit, Stat::SpellPower, Stat::Haste}, 0.0f};
        case Role::Melee:
        default:
            return {164, 0, 1000, {Stat::ArmorPenetration, Stat::Strength, Stat::Agility}, WEAPON_DPS_WEIGHT};
    }
}

bool ParseProfile(std::string const& text, Role role, Profile& profile)
{
    Profile parsed = DefaultProfile(role);
    auto const addPriority = [&parsed](std::string const& name)
    {
        auto const stat = StatNames().find(name);
        if (stat == StatNames().end())
            return false;

        parsed.priority.push_back(stat->second);
        return true;
    };

    // Priority stats are comma separated too, so tokens after "priority:" without a key belong to it.
    bool inPriority = false;
    for (std::string token : split(text, ','))
    {
        trim(token);
        size_t const colon = token.find(':');
        if (colon == std::string::npos)
        {
            if (!inPriority || !addPriority(token))
                return false;
            continue;
        }

        std::string key = token.substr(0, colon);
        std::string value = token.substr(colon + 1);
        trim(key);
        trim(value);

        inPriority = key == "priority";
        if (inPriority)
        {
            parsed.priority.clear();
            if (!addPriority(value))
                return false;
            continue;
        }

        uint32* target = key == "hit"          ? &parsed.hitTarget
                         : key == "spellpen"   ? &parsed.spellPenetrationTarget
                         : key == "resilience" ? &parsed.resilienceTarget
                                               : nullptr;
        if (!target || !ParseNumber(value, *target))
            return false;
    }

    profile = std::move(parsed);
    return true;
}

float Ep(StatVector const& totals, Profile const& profile)
{
    float ep = Capped(At(totals, Stat::Hit), float(profile.hitTarget), HIT_WEIGHT, 0.0f);
    if (profile.spellPenetrationTarget)
        ep += Capped(At(totals, Stat::SpellPenetration), float(profile.spellPenetrationTarget),
                     SPELL_PENETRATION_WEIGHT, 0.0f);
    ep += Capped(At(totals, Stat::Resilience), float(profile.resilienceTarget), RESILIENCE_WEIGHT,
                 RESILIENCE_PAST_TARGET_WEIGHT);
    ep += STAMINA_WEIGHT * At(totals, Stat::Stamina);
    ep += profile.weaponDpsWeight * At(totals, Stat::WeaponDps);

    for (size_t i = 0; i < profile.priority.size() && i < PRIORITY_WEIGHTS.size(); ++i)
        ep += PRIORITY_WEIGHTS[i] * At(totals, profile.priority[i]);

    return ep;
}

float MarginalEp(StatVector const& totals, StatVector const& added, Profile const& profile)
{
    return Ep(Add(totals, added), profile) - Ep(totals, profile);
}

std::string FormatTargets(StatVector const& totals, Profile const& profile)
{
    auto const part = [&totals](char const* name, Stat stat, uint32 target)
    { return std::string(name) + " " + std::to_string(uint32(At(totals, stat))) + "/" + std::to_string(target); };
    return part("hit", Stat::Hit, profile.hitTarget) + ", " +
           part("spellpen", Stat::SpellPenetration, profile.spellPenetrationTarget) + ", " +
           part("resilience", Stat::Resilience, profile.resilienceTarget);
}

std::map<uint8, uint32> PickSetPieces(std::vector<SetPiece> const& pieces, uint32 required)
{
    // Best piece per slot within each set.
    std::map<uint32, std::map<uint8, SetPiece>> sets;
    for (SetPiece const& piece : pieces)
    {
        auto const [it, inserted] = sets[piece.itemSet].emplace(piece.slot, piece);
        if (!inserted && piece.itemLevel > it->second.itemLevel)
            it->second = piece;
    }

    std::vector<SetPiece> best;
    uint32 bestTotalItemLevel = 0;
    uint32 bestSet = 0;
    for (auto const& [itemSet, bySlot] : sets)
    {
        if (bySlot.size() < required)
            continue;

        std::vector<SetPiece> chosen;
        for (auto const& [slot, piece] : bySlot)
            chosen.push_back(piece);
        std::sort(chosen.begin(), chosen.end(), [](SetPiece const& lhs, SetPiece const& rhs)
                  { return lhs.itemLevel != rhs.itemLevel ? lhs.itemLevel > rhs.itemLevel : lhs.slot < rhs.slot; });
        chosen.resize(required);

        uint32 totalItemLevel = 0;
        for (SetPiece const& piece : chosen)
            totalItemLevel += piece.itemLevel;

        // Equal sizes, so the total stands in for the average.
        if (best.empty() || totalItemLevel > bestTotalItemLevel ||
            (totalItemLevel == bestTotalItemLevel && itemSet > bestSet))
        {
            best = std::move(chosen);
            bestTotalItemLevel = totalItemLevel;
            bestSet = itemSet;
        }
    }

    std::map<uint8, uint32> result;
    for (SetPiece const& piece : best)
        result[piece.slot] = piece.itemId;
    return result;
}

std::vector<int32> SelectBiggestGainFirst(StatVector const& base, std::vector<Group> const& groups,
                                          Profile const& profile)
{
    std::vector<int32> chosen(groups.size(), -1);
    std::vector<bool> decided(groups.size(), false);
    std::unordered_set<uint32> takenUnique;
    StatVector totals = base;

    for (size_t round = 0; round < groups.size(); ++round)
    {
        int32 bestGroup = -1;
        int32 bestOption = -1;
        float bestGain = 0.0f;
        bool bestOwn = false;
        for (size_t g = 0; g < groups.size(); ++g)
        {
            if (decided[g])
                continue;

            for (size_t o = 0; o < groups[g].options.size(); ++o)
            {
                Option const& option = groups[g].options[o];
                bool const blocked =
                    option.unique && std::any_of(option.itemIds.begin(), option.itemIds.end(),
                                                 [&](uint32 itemId) { return takenUnique.count(itemId) != 0; });
                if (blocked)
                    continue;

                float const gain = MarginalEp(totals, option.stats, profile);
                bool const better =
                    bestGroup < 0 || gain > bestGain + EP_TIE || (gain > bestGain - EP_TIE && option.own && !bestOwn);
                if (better)
                {
                    bestGroup = static_cast<int32>(g);
                    bestOption = static_cast<int32>(o);
                    bestGain = gain;
                    bestOwn = option.own;
                }
            }
        }

        // Groups whose every option is blocked stay empty.
        if (bestGroup < 0)
            break;

        Option const& option = groups[bestGroup].options[bestOption];
        chosen[bestGroup] = bestOption;
        decided[bestGroup] = true;
        totals = Add(totals, option.stats);
        if (option.unique)
            takenUnique.insert(option.itemIds.begin(), option.itemIds.end());
    }

    return chosen;
}
}  // namespace PvpLoadout
