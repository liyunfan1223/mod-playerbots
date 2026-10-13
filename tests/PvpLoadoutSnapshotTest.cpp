/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpLoadoutRules.h"
#include "gtest/gtest.h"

using namespace PvpLoadout;

TEST(PvpLoadoutColumns, GlyphsRoundTrip)
{
    Glyphs const glyphs = {0, 1234, 0, 56, 789, 0};
    Glyphs parsed = {};
    ASSERT_TRUE(ParseGlyphs(FormatGlyphs(glyphs), parsed));
    EXPECT_EQ(parsed, glyphs);
}

TEST(PvpLoadoutColumns, GlyphsNeedEverySlot)
{
    Glyphs parsed = {};
    EXPECT_FALSE(ParseGlyphs("", parsed));
    EXPECT_FALSE(ParseGlyphs("1,2,3", parsed));
    EXPECT_FALSE(ParseGlyphs("1,2,3,4,5,6,7", parsed));
    EXPECT_FALSE(ParseGlyphs("1,2,x,4,5,6", parsed));
}

TEST(PvpLoadoutColumns, ItemListsRoundTrip)
{
    std::vector<uint32> const items = {40001, 40002, 40002, 4000000000u};
    std::vector<uint32> parsed;
    ASSERT_TRUE(ParseItemList(FormatItemList(items), parsed));
    EXPECT_EQ(parsed, items);
}

TEST(PvpLoadoutColumns, EmptyItemListRoundTrips)
{
    std::vector<uint32> parsed = {1};
    ASSERT_TRUE(ParseItemList(FormatItemList({}), parsed));
    EXPECT_TRUE(parsed.empty());
}

TEST(PvpLoadoutColumns, MalformedItemListsAreRejected)
{
    std::vector<uint32> parsed;
    EXPECT_FALSE(ParseItemList("1,,2", parsed));
    EXPECT_FALSE(ParseItemList("1,a", parsed));
    EXPECT_FALSE(ParseItemList("-1", parsed));
    EXPECT_FALSE(ParseItemList("4294967296", parsed));
}
