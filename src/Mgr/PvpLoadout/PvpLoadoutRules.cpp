/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpLoadoutRules.h"
#include "Helpers.h"
#include <algorithm>
#include <charconv>
#include <iterator>
#include <sstream>

namespace PvpLoadout
{
namespace
{
template <typename T>
bool ParseInteger(std::string text, T& value)
{
    trim(text);
    if (text.empty())
        return false;

    char const* end = text.data() + text.size();
    auto const [ptr, error] = std::from_chars(text.data(), end, value);
    return error == std::errc() && ptr == end;
}

// Space-separated numbers, exactly count of them.
template <typename T>
bool ParseFields(std::string const& text, size_t count, std::vector<T>& values)
{
    std::vector<std::string> tokens;
    split(tokens, text, " ");
    if (tokens.size() != count)
        return false;

    values.resize(count);
    for (size_t i = 0; i < count; ++i)
        if (!ParseInteger(tokens[i], values[i]))
            return false;

    return true;
}
}  // namespace

bool ParseNumber(std::string text, uint32& value) { return ParseInteger(std::move(text), value); }

std::vector<CurveStep> ParseItemLevelCurve(std::string const& text)
{
    std::vector<CurveStep> curve;
    for (std::string const& entry : split(text, ','))
    {
        std::vector<std::string> const parts = split(entry, ':');
        CurveStep step{};
        if (parts.size() != 2 || !ParseNumber(parts[0], step.minRating) || !ParseNumber(parts[1], step.itemLevel))
            return {};
        curve.push_back(step);
    }

    std::sort(curve.begin(), curve.end(),
              [](CurveStep const& lhs, CurveStep const& rhs) { return lhs.minRating < rhs.minRating; });

    auto const duplicate = std::adjacent_find(curve.begin(), curve.end(), [](CurveStep const& lhs, CurveStep const& rhs)
                                              { return lhs.minRating == rhs.minRating; });
    if (duplicate != curve.end())
        return {};

    return curve;
}

uint32 CurveItemLevel(std::vector<CurveStep> const& curve, uint32 rating)
{
    uint32 itemLevel = 0;
    for (CurveStep const& step : curve)
    {
        if (step.minRating > rating)
            break;
        itemLevel = step.itemLevel;
    }
    return itemLevel;
}

std::array<int32, TALENT_TAB_COUNT> MapPvpSpecsByTab(std::vector<std::string> const& names,
                                                     std::vector<int32> const& mainTabs)
{
    std::array<int32, TALENT_TAB_COUNT> specs = {NO_SPEC, NO_SPEC, NO_SPEC};
    size_t const count = std::min(names.size(), mainTabs.size());
    for (size_t specNo = 0; specNo < count; ++specNo)
    {
        int32 const tab = mainTabs[specNo];
        if (tab < 0 || tab >= TALENT_TAB_COUNT || names[specNo].find("pvp") == std::string::npos)
            continue;

        if (specs[tab] == NO_SPEC)
            specs[tab] = static_cast<int32>(specNo);
    }
    return specs;
}

std::unordered_map<uint32, uint32> ResolveRequiredRatings(std::vector<VendorCost> const& vendorCosts)
{
    std::unordered_map<uint32, uint32> required;
    for (VendorCost const& cost : vendorCosts)
    {
        if (cost.soldForGold)
            continue;

        auto const [it, inserted] = required.emplace(cost.itemId, cost.requiredRating);
        if (!inserted)
            it->second = std::min(it->second, cost.requiredRating);
    }

    for (VendorCost const& cost : vendorCosts)
        if (cost.soldForGold)
            required.emplace(cost.itemId, 0);

    return required;
}

bool WithinLimits(ItemCandidate const& item, GearLimits const& limits)
{
    if (item.quality > limits.maxQuality || item.requiredLevel > limits.botLevel)
        return false;

    return !limits.maxMixedGearScore || item.mixedGearScore <= limits.maxMixedGearScore;
}

bool IsPvpEligible(ItemCandidate const& item, uint32 rating, RatingRules const& rules)
{
    if (!item.requiredRating)
        return item.itemLevel <= rules.freeItemLevelCap;

    return rating >= rules.minRating && item.requiredRating <= rating + rules.margin;
}

uint32 UnlockedRequirement(std::vector<uint32> const& requirements, uint32 rating, RatingRules const& rules)
{
    if (rating < rules.minRating)
        return 0;

    auto const above = std::upper_bound(requirements.begin(), requirements.end(), rating + rules.margin);
    return above == requirements.begin() ? 0 : *std::prev(above);
}

MatchRating ResolveMatchRating(bool rated, uint32 opposingMmr, std::vector<Opponent> const& opponents,
                               uint32 opposingTeamSize, uint32 prepRemainingMs, uint32 fallbackThresholdMs)
{
    if (rated)
        return {true, opposingMmr};

    uint32 highestRealPlayer = 0;
    uint32 highestBot = 0;
    for (Opponent const& opponent : opponents)
    {
        uint32& highest = opponent.realPlayer ? highestRealPlayer : highestBot;
        highest = std::max(highest, opponent.personalRating);
    }

    bool const wholeTeam = opponents.size() >= opposingTeamSize;
    return {wholeTeam || prepRemainingMs <= fallbackThresholdMs, highestRealPlayer ? highestRealPlayer : highestBot};
}

Transition Decide(bool featureEnabled, uint32 arenaInstanceId, bool matchOver, bool hasLoadout,
                  uint32 loadoutInstanceId, bool gearPending)
{
    if (!arenaInstanceId || matchOver)
        return hasLoadout ? Transition::Restore : Transition::None;

    if (!featureEnabled)
        return Transition::None;

    if (!hasLoadout)
        return Transition::Enter;

    if (loadoutInstanceId != arenaInstanceId)
        return Transition::Reapply;

    return gearPending ? Transition::ApplyGear : Transition::None;
}

std::string FormatGlyphs(Glyphs const& glyphs) { return FormatItemList({glyphs.begin(), glyphs.end()}); }

bool ParseGlyphs(std::string const& text, Glyphs& glyphs)
{
    std::vector<uint32> values;
    if (!ParseItemList(text, values) || values.size() != GLYPH_SLOT_COUNT)
        return false;

    std::copy(values.begin(), values.end(), glyphs.begin());
    return true;
}

std::string FormatItemList(std::vector<uint32> const& items)
{
    std::ostringstream out;
    for (size_t i = 0; i < items.size(); ++i)
    {
        if (i)
            out << ',';
        out << items[i];
    }
    return out.str();
}

bool ParseItemList(std::string const& text, std::vector<uint32>& items)
{
    std::vector<uint32> parsed;
    for (std::string const& token : split(text, ','))
    {
        uint32 value = 0;
        if (!ParseNumber(token, value))
            return false;
        parsed.push_back(value);
    }
    items = std::move(parsed);
    return true;
}

std::string FormatCharges(ItemCharges const& charges)
{
    std::ostringstream out;
    for (int32 charge : charges)
        out << charge << ' ';
    return out.str();
}

bool ParseCharges(std::string const& text, ItemCharges& charges)
{
    std::vector<int32> values;
    if (!ParseFields(text, ITEM_SPELL_CHARGE_COUNT, values))
        return false;

    std::copy(values.begin(), values.end(), charges.begin());
    return true;
}

std::string FormatEnchantments(ItemEnchantments const& enchantments)
{
    std::ostringstream out;
    for (ItemEnchantment const& enchantment : enchantments)
        out << enchantment.id << ' ' << enchantment.duration << ' ' << enchantment.charges << ' ';
    return out.str();
}

bool ParseEnchantments(std::string const& text, ItemEnchantments& enchantments)
{
    std::vector<uint32> values;
    if (!ParseFields(text, ITEM_ENCHANTMENT_SLOT_COUNT * 3, values))
        return false;

    for (size_t i = 0; i < enchantments.size(); ++i)
        enchantments[i] = {values[i * 3], values[i * 3 + 1], values[i * 3 + 2]};
    return true;
}
}  // namespace PvpLoadout
