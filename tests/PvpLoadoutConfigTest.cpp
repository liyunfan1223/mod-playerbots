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
std::string const DEFAULT_CURVE = "0:200,1400:224,1600:245,1800:258,2000:264,2200:290";
}

TEST(PvpLoadoutCurve, ParsesTheDefaultCurve)
{
    std::vector<CurveStep> const curve = ParseItemLevelCurve(DEFAULT_CURVE);
    ASSERT_EQ(curve.size(), 6u);
    EXPECT_EQ(curve.front().minRating, 0u);
    EXPECT_EQ(curve.front().itemLevel, 200u);
    EXPECT_EQ(curve.back().minRating, 2200u);
    EXPECT_EQ(curve.back().itemLevel, 290u);
}

TEST(PvpLoadoutCurve, SortsStepsByRating)
{
    std::vector<CurveStep> const curve = ParseItemLevelCurve("1800:258,0:200,1400:224");
    ASSERT_EQ(curve.size(), 3u);
    EXPECT_EQ(curve[0].minRating, 0u);
    EXPECT_EQ(curve[1].minRating, 1400u);
    EXPECT_EQ(curve[2].minRating, 1800u);
}

TEST(PvpLoadoutCurve, ToleratesSpacesAroundEntries) { EXPECT_EQ(ParseItemLevelCurve(" 0:200 , 1400:224 ").size(), 2u); }

TEST(PvpLoadoutCurve, RejectsMalformedCurvesEntirely)
{
    EXPECT_TRUE(ParseItemLevelCurve("").empty());
    EXPECT_TRUE(ParseItemLevelCurve("0:200,abc").empty());
    EXPECT_TRUE(ParseItemLevelCurve("0-200").empty());
    EXPECT_TRUE(ParseItemLevelCurve("1400:").empty());
    EXPECT_TRUE(ParseItemLevelCurve(":224").empty());
    EXPECT_TRUE(ParseItemLevelCurve("0:200,0:224").empty());
}

TEST(PvpLoadoutCurve, ItemLevelIsTheHighestStepAtOrBelowTheRating)
{
    std::vector<CurveStep> const curve = ParseItemLevelCurve(DEFAULT_CURVE);
    EXPECT_EQ(CurveItemLevel(curve, 0), 200u);
    EXPECT_EQ(CurveItemLevel(curve, 1399), 200u);
    EXPECT_EQ(CurveItemLevel(curve, 1400), 224u);
    EXPECT_EQ(CurveItemLevel(curve, 1930), 258u);
    EXPECT_EQ(CurveItemLevel(curve, 2200), 290u);
    EXPECT_EQ(CurveItemLevel(curve, 5000), 290u);
}

TEST(PvpLoadoutCurve, NoItemLevelBelowTheFirstStepOrWithoutACurve)
{
    EXPECT_EQ(CurveItemLevel(ParseItemLevelCurve("1000:224"), 500), 0u);
    EXPECT_EQ(CurveItemLevel({}, 1500), 0u);
}

TEST(PvpLoadoutSpecs, MapsEachTabToItsPvpPremade)
{
    std::vector<std::string> const names = {"arms pve", "fury pve", "prot pve", "arms pvp", "fury pvp", "prot pvp"};
    std::vector<int32> const mainTabs = {0, 1, 2, 0, 1, 2};
    std::array<int32, TALENT_TAB_COUNT> const expected = {3, 4, 5};
    EXPECT_EQ(MapPvpSpecsByTab(names, mainTabs), expected);
}

TEST(PvpLoadoutSpecs, FirstPvpPremadeForATabWins)
{
    std::vector<std::string> const names = {"frost pvp", "frost pvp alt"};
    std::vector<int32> const mainTabs = {2, 2};
    EXPECT_EQ(MapPvpSpecsByTab(names, mainTabs)[2], 0);
}

TEST(PvpLoadoutSpecs, TabsWithoutAPvpPremadeAreNoSpec)
{
    std::vector<std::string> const names = {"arms pve", "arms pvp"};
    std::vector<int32> const mainTabs = {0, 0};
    std::array<int32, TALENT_TAB_COUNT> const expected = {1, NO_SPEC, NO_SPEC};
    EXPECT_EQ(MapPvpSpecsByTab(names, mainTabs), expected);
}

TEST(PvpLoadoutSpecs, PvpPremadesWithoutALinkAreIgnored)
{
    std::vector<std::string> const names = {"arms pvp", "arms pvp 2"};
    std::vector<int32> const mainTabs = {NO_SPEC, 0};
    EXPECT_EQ(MapPvpSpecsByTab(names, mainTabs)[0], 1);
}

TEST(PvpLoadoutSpecs, MatchesPvpCaseSensitivelyLikeIsSpecPvp)
{
    EXPECT_EQ(MapPvpSpecsByTab({"arms PvP"}, {0})[0], NO_SPEC);
}

TEST(PvpLoadoutSpecs, OutOfRangeTabsAreIgnored)
{
    std::array<int32, TALENT_TAB_COUNT> const expected = {NO_SPEC, NO_SPEC, NO_SPEC};
    EXPECT_EQ(MapPvpSpecsByTab({"odd pvp"}, {3}), expected);
}

TEST(PvpLoadoutSpecs, ExtraNamesWithoutTabsAreIgnored)
{
    std::array<int32, TALENT_TAB_COUNT> const expected = {0, NO_SPEC, NO_SPEC};
    EXPECT_EQ(MapPvpSpecsByTab({"arms pvp", "fury pvp"}, {0}), expected);
}
