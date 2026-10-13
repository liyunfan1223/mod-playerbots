/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PVPLOADOUTRULES_H
#define PLAYERBOTS_PVPLOADOUTRULES_H

#include "Define.h"
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

// Pure decision logic for the PvP loadout swap. No core types, so it is unit-testable without a server.
namespace PvpLoadout
{
constexpr uint8 GLYPH_SLOT_COUNT = 6;
constexpr uint8 TALENT_TAB_COUNT = 3;
constexpr int32 NO_SPEC = -1;

// A whole, trimmed, unsigned number; false on anything else.
bool ParseNumber(std::string text, uint32& value);

struct CurveStep
{
    uint32 minRating;
    uint32 itemLevel;
};

// "0:200,1400:224" -> steps sorted by rating. Any malformed entry makes the whole curve empty.
std::vector<CurveStep> ParseItemLevelCurve(std::string const& text);

// Item level of the highest step whose rating floor is <= rating; 0 when no step applies.
uint32 CurveItemLevel(std::vector<CurveStep> const& curve, uint32 rating);

// mainTabs[specNo] is the tree holding most of the premade's points, NO_SPEC when it has no link.
// For each tab, the first premade whose name contains "pvp" with that main tree; NO_SPEC if none.
std::array<int32, TALENT_TAB_COUNT> MapPvpSpecsByTab(std::vector<std::string> const& names,
                                                     std::vector<int32> const& mainTabs);

struct VendorCost
{
    uint32 itemId;
    uint32 requiredRating;
    bool soldForGold = false;  // no extended cost: a custom gold vendor, not a PvP currency vendor
};

// Required personal rating per item: the lowest requirement across the vendor rows that charge PvP currency. Gold
// vendors only count when no PvP currency vendor sells the item, and then it needs no rating.
std::unordered_map<uint32, uint32> ResolveRequiredRatings(std::vector<VendorCost> const& vendorCosts);

struct ItemCandidate
{
    uint32 itemId;
    uint32 itemLevel;
    uint32 quality;
    uint32 requiredLevel;
    uint32 requiredRating;
    uint32 mixedGearScore;  // PlayerbotFactory::CalcMixedGearScore(itemLevel, quality)
};

// Gear limits in autogear's terms (PvpLoadoutQualityLimit / PvpLoadoutScoreLimit).
struct GearLimits
{
    uint32 maxQuality;
    uint32 maxMixedGearScore;  // CalcMixedGearScore(score limit, quality limit); 0 = no limit
    uint32 botLevel;
};

struct RatingRules
{
    uint32 minRating;
    uint32 margin;
    uint32 freeItemLevelCap;  // requirement-free items only substitute for PvE gear up to this item level
};

bool WithinLimits(ItemCandidate const& item, GearLimits const& limits);

// Requirement-free items are eligible up to freeItemLevelCap; rated items need rating >= minRating and
// requiredRating <= rating + margin.
bool IsPvpEligible(ItemCandidate const& item, uint32 rating, RatingRules const& rules);

// The highest of `requirements` (ascending, non-zero) that rating unlocks; 0 when none does. Ratings with the same
// unlocked requirement and the same curve item level see exactly the same eligible items.
uint32 UnlockedRequirement(std::vector<uint32> const& requirements, uint32 rating, RatingRules const& rules);

struct MatchRating
{
    bool ready;
    uint32 rating;
};

struct Opponent
{
    uint32 personalRating;
    bool realPlayer;
};

// Rated: the opposing MMR. Skirmish: the highest rating among the opposing real players, else its bots, once the whole
// team is present; near the end of prep, whoever is there (0 with nobody).
MatchRating ResolveMatchRating(bool rated, uint32 opposingMmr, std::vector<Opponent> const& opponents,
                               uint32 opposingTeamSize, uint32 prepRemainingMs, uint32 fallbackThresholdMs);

enum class Transition
{
    None,
    Enter,
    Reapply,
    ApplyGear,  // the planned match gear goes on once the copies of the PvE items it replaces are saved
    Restore
};

// Restore happens at match end or anywhere outside an arena (crash, relog), even with the feature disabled.
// arenaInstanceId is 0 outside an arena; gearPending: planned match gear not yet equipped.
Transition Decide(bool featureEnabled, uint32 arenaInstanceId, bool matchOver, bool hasLoadout,
                  uint32 loadoutInstanceId, bool gearPending = false);

using Glyphs = std::array<uint32, GLYPH_SLOT_COUNT>;

constexpr uint8 ITEM_SPELL_CHARGE_COUNT = 5;       // MAX_ITEM_PROTO_SPELLS
constexpr uint8 ITEM_ENCHANTMENT_SLOT_COUNT = 12;  // MAX_ENCHANTMENT_SLOT

using ItemCharges = std::array<int32, ITEM_SPELL_CHARGE_COUNT>;

struct ItemEnchantment
{
    uint32 id;
    uint32 duration;
    uint32 charges;
};

using ItemEnchantments = std::array<ItemEnchantment, ITEM_ENCHANTMENT_SLOT_COUNT>;

// A replaced PvE item as the core persists it (Item::SaveToDB's item_instance columns), so the item can be destroyed
// for the match and recreated identically afterwards: gems, enchants, random suffix, durability, crafter and all.
struct ItemCopy
{
    uint8 slot;
    uint32 itemGuid;  // the original's guid counter: recognises it if it was never destroyed
    uint32 entry;
    uint32 creatorGuid;  // the crafter, shown as "<Made by Name>"
    uint32 giftCreatorGuid;
    uint32 flags;
    uint32 duration;
    ItemCharges charges;
    ItemEnchantments enchantments;
    int32 randomPropertyId;
    uint32 durability;
    uint32 playedTime;
    std::string text;
};

struct Snapshot
{
    std::string talentLink;
    Glyphs glyphs;
    std::vector<ItemCopy> pveCopies;  // the PvE items the match gear replaced, recreated at restore
    std::vector<uint32> matchItems;   // item guid counters created for the match
    uint32 arenaInstanceId;

    // In memory only, between planning the match gear and equipping it: (slot, item entry), entry 0 to empty the
    // slot, and the rating the gear was scaled to. Lost on a restart, which then restores nothing it never replaced.
    std::vector<std::pair<uint8, uint32>> plannedItems;
    uint32 plannedRating;
};

// Column formats for playerbots_pvp_loadout: comma-separated numbers.
std::string FormatGlyphs(Glyphs const& glyphs);
bool ParseGlyphs(std::string const& text, Glyphs& glyphs);
std::string FormatItemList(std::vector<uint32> const& items);
bool ParseItemList(std::string const& text, std::vector<uint32>& items);

// The item_instance column formats: each number followed by a space, charges as 5 numbers, enchantments as 12
// (id, duration, charges) triples. Parsing fails on a wrong count or anything that is not a number.
std::string FormatCharges(ItemCharges const& charges);
bool ParseCharges(std::string const& text, ItemCharges& charges);
std::string FormatEnchantments(ItemEnchantments const& enchantments);
bool ParseEnchantments(std::string const& text, ItemEnchantments& enchantments);

}  // namespace PvpLoadout

#endif
