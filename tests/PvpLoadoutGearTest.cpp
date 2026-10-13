/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpLoadoutRules.h"
#include "gtest/gtest.h"

using namespace PvpLoadout;

namespace
{
constexpr uint32 EPIC = 4;
constexpr uint32 RARE = 3;

// A free-item cap above every test item, so only the cases about the cap are affected by it.
RatingRules const RULES = {1400, 100, 1000};
GearLimits const NO_CAP = {EPIC, 0, 80};

// Mixed gear score defaults to the item level; the real multiplier lives in PlayerbotAI.
ItemCandidate Item(uint32 id, uint32 itemLevel, uint32 requiredRating = 0, uint32 quality = EPIC,
                   uint32 requiredLevel = 80)
{
    return {id, itemLevel, quality, requiredLevel, requiredRating, itemLevel};
}
}  // namespace

TEST(PvpLoadoutRequirements, LowestVendorRequirementWins)
{
    auto const resolved = ResolveRequiredRatings({{1, 2000}, {1, 0}, {2, 1800}, {2, 1900}});
    EXPECT_EQ(resolved.at(1), 0u);
    EXPECT_EQ(resolved.at(2), 1800u);
}

TEST(PvpLoadoutRequirements, GoldVendorsDoNotWaiveARatingRequirement)
{
    auto const resolved = ResolveRequiredRatings({{1, 0, true}, {1, 2000}});
    EXPECT_EQ(resolved.at(1), 2000u);
}

TEST(PvpLoadoutRequirements, ItemsOnlySoldForGoldNeedNoRating)
{
    auto const resolved = ResolveRequiredRatings({{1, 0, true}});
    EXPECT_EQ(resolved.at(1), 0u);
}

TEST(PvpLoadoutRequirements, ItemsWithNoSourceAreDropped)
{
    auto const resolved = ResolveRequiredRatings({{1, 0}});
    EXPECT_EQ(resolved.count(4), 0u);
}

TEST(PvpLoadoutLimits, RespectsQualityGearScoreAndRequiredLevel)
{
    GearLimits const limits = {RARE, 245, 80};
    EXPECT_TRUE(WithinLimits(Item(1, 245, 0, RARE), limits));
    EXPECT_FALSE(WithinLimits(Item(1, 245, 0, EPIC), limits));
    EXPECT_FALSE(WithinLimits(Item(1, 246, 0, RARE), limits));
    EXPECT_FALSE(WithinLimits(Item(1, 200, 0, RARE, 81), limits));
}

TEST(PvpLoadoutLimits, GearScoreUsesTheMixedScoreNotTheItemLevel)
{
    GearLimits const limits = {EPIC, 300, 80};
    ItemCandidate item = Item(1, 250);
    item.mixedGearScore = 325;
    EXPECT_FALSE(WithinLimits(item, limits));
}

TEST(PvpLoadoutLimits, ZeroGearScoreLimitMeansNoCap) { EXPECT_TRUE(WithinLimits(Item(1, 290), NO_CAP)); }

TEST(PvpLoadoutEligibility, RequirementFreeItemsAreEligibleAtAnyRating)
{
    EXPECT_TRUE(IsPvpEligible(Item(1, 232, 0), 0, RULES));
}

TEST(PvpLoadoutEligibility, RequirementFreeItemsStopAtTheCurveItemLevel)
{
    RatingRules const capped = {1400, 100, 200};
    EXPECT_TRUE(IsPvpEligible(Item(1, 200, 0), 0, capped));
    EXPECT_FALSE(IsPvpEligible(Item(1, 251, 0), 0, capped));
}

TEST(PvpLoadoutEligibility, TheFreeItemCapDoesNotLimitRatedItems)
{
    RatingRules const capped = {1400, 100, 200};
    EXPECT_TRUE(IsPvpEligible(Item(1, 270, 1500), 1600, capped));
}

TEST(PvpLoadoutEligibility, RatedItemsNeedTheMinimumRating)
{
    EXPECT_FALSE(IsPvpEligible(Item(1, 245, 1400), 1399, RULES));
    EXPECT_TRUE(IsPvpEligible(Item(1, 245, 1400), 1400, RULES));
}

TEST(PvpLoadoutEligibility, RatedItemsAreEligibleWithinTheMargin)
{
    // The 1930-rated opponent from the design example faces 2000-rated shoulders.
    EXPECT_TRUE(IsPvpEligible(Item(1, 270, 2000), 1930, RULES));
    EXPECT_TRUE(IsPvpEligible(Item(1, 270, 2030), 1930, RULES));
    EXPECT_FALSE(IsPvpEligible(Item(1, 270, 2031), 1930, RULES));
}

TEST(PvpLoadoutEligibility, MarginIsConfigurable)
{
    RatingRules const tight = {1400, 0, 1000};
    EXPECT_FALSE(IsPvpEligible(Item(1, 270, 2000), 1930, tight));
    EXPECT_TRUE(IsPvpEligible(Item(1, 270, 1930), 1930, tight));
}

TEST(PvpLoadoutTier, UnlocksTheHighestRequirementWithinTheMargin)
{
    std::vector<uint32> const requirements = {1350, 1600, 1800, 2000, 2200};
    EXPECT_EQ(UnlockedRequirement(requirements, 1930, RULES), 2000u);
    EXPECT_EQ(UnlockedRequirement(requirements, 1899, RULES), 1800u);
    EXPECT_EQ(UnlockedRequirement(requirements, 1900, RULES), 2000u);
}

TEST(PvpLoadoutTier, NothingUnlocksBelowTheMinimumRating)
{
    EXPECT_EQ(UnlockedRequirement({1350, 1600}, 1399, RULES), 0u);
}

TEST(PvpLoadoutTier, NothingUnlocksBelowTheLowestRequirement)
{
    EXPECT_EQ(UnlockedRequirement({1600, 1800}, 1450, RULES), 0u);
    EXPECT_EQ(UnlockedRequirement({}, 2400, RULES), 0u);
}

TEST(PvpLoadoutTier, RatingsInOneTierSeeTheSameEligibleItems)
{
    // The cache shares one loadout per tier, so eligibility must not change within it.
    std::vector<uint32> const requirements = {1600, 1800, 2000};
    std::vector<ItemCandidate> const items = {Item(1, 245, 1600), Item(2, 258, 1800), Item(3, 270, 2000)};
    for (uint32 rating : {1700u, 1750u, 1899u})
    {
        EXPECT_EQ(UnlockedRequirement(requirements, rating, RULES), 1800u);
        EXPECT_TRUE(IsPvpEligible(items[0], rating, RULES));
        EXPECT_TRUE(IsPvpEligible(items[1], rating, RULES));
        EXPECT_FALSE(IsPvpEligible(items[2], rating, RULES));
    }
}

TEST(PvpLoadoutRating, RatedMatchesUseTheOpposingMmr)
{
    MatchRating const rating = ResolveMatchRating(true, 1930, {}, 2, 60000, 5000);
    EXPECT_TRUE(rating.ready);
    EXPECT_EQ(rating.rating, 1930u);
}

TEST(PvpLoadoutRating, SkirmishWithoutRealPlayersUsesTheHighestBotRating)
{
    MatchRating const rating =
        ResolveMatchRating(false, 0, {{1500, false}, {2100, false}, {1800, false}}, 3, 60000, 5000);
    EXPECT_TRUE(rating.ready);
    EXPECT_EQ(rating.rating, 2100u);
}

TEST(PvpLoadoutRating, SkirmishScalesToRealPlayersOverHigherRatedBots)
{
    MatchRating const rating = ResolveMatchRating(false, 0, {{1500, true}, {2100, false}}, 2, 60000, 5000);
    EXPECT_TRUE(rating.ready);
    EXPECT_EQ(rating.rating, 1500u);
}

TEST(PvpLoadoutRating, UnratedRealPlayersFallBackToTheirBotTeammates)
{
    // Most players have no arena team, so their personal rating is 0.
    MatchRating const rating = ResolveMatchRating(false, 0, {{0, true}, {1877, false}}, 2, 60000, 5000);
    EXPECT_TRUE(rating.ready);
    EXPECT_EQ(rating.rating, 1877u);
}

TEST(PvpLoadoutRating, SkirmishWaitsForOpponentsDuringPrep)
{
    EXPECT_FALSE(ResolveMatchRating(false, 0, {}, 2, 30000, 5000).ready);
}

TEST(PvpLoadoutRating, SkirmishWaitsForTheWholeOpposingTeam)
{
    // A bot partner porting in before the player must not set the rating on its own.
    EXPECT_FALSE(ResolveMatchRating(false, 0, {{0, false}}, 2, 30000, 5000).ready);
}

TEST(PvpLoadoutRating, SkirmishUsesWhoeverIsPresentWhenPrepRunsOut)
{
    MatchRating const rating = ResolveMatchRating(false, 0, {{1700, true}}, 2, 5000, 5000);
    EXPECT_TRUE(rating.ready);
    EXPECT_EQ(rating.rating, 1700u);
}

TEST(PvpLoadoutRating, SkirmishFallsBackToTheBottomOfTheCurveWhenPrepRunsOut)
{
    MatchRating const rating = ResolveMatchRating(false, 0, {}, 2, 5000, 5000);
    EXPECT_TRUE(rating.ready);
    EXPECT_EQ(rating.rating, 0u);
}
