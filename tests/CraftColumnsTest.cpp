/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CraftRules.h"
#include "gtest/gtest.h"

using namespace CraftRules;

TEST(CraftReagentsColumn, RoundTrips)
{
    Reagents const reagents = {{2589, {2, 4, 20}}, {2320, {1, 0, 0}}, {4000000000u, {7, 8, 9}}};
    Reagents parsed;
    ASSERT_TRUE(ParseReagents(FormatReagents(reagents), parsed));
    ASSERT_EQ(parsed.size(), reagents.size());
    for (auto const& [item, reagent] : reagents)
    {
        EXPECT_EQ(parsed[item].perCraft, reagent.perCraft);
        EXPECT_EQ(parsed[item].obtained, reagent.obtained);
        EXPECT_EQ(parsed[item].own, reagent.own);
    }
}

TEST(CraftReagentsColumn, EmptyRoundTrips)
{
    Reagents parsed = {{1, {}}};
    ASSERT_TRUE(ParseReagents(FormatReagents({}), parsed));
    EXPECT_TRUE(parsed.empty());
}

TEST(CraftReagentsColumn, MalformedEntriesAreRejected)
{
    for (std::string const text :
         {"1:2:3", "1:2:3:4:5", "a:2:3:4", "1:2:3:4x", "1::3:4", "-1:2:3:4", "4294967296:1:1:1", "2589:2:4:20 1:2"})
    {
        Reagents parsed;
        EXPECT_FALSE(ParseReagents(text, parsed)) << text;
        EXPECT_TRUE(parsed.empty()) << text;
    }
}
