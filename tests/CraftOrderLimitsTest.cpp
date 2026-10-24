/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CraftRules.h"
#include "gtest/gtest.h"

using namespace CraftRules;

TEST(CraftOrderCount, DefaultsToOne)
{
    EXPECT_EQ(ParseOrderCount(""), 1u);
    EXPECT_EQ(ParseOrderCount("|Hitem:2996:0:0:0:0:0:0:0:0|h[Bolt of Linen Cloth]|h|r"), 1u);
}

TEST(CraftOrderCount, ReadsTheLastWord)
{
    EXPECT_EQ(ParseOrderCount("|Hitem:2996:0:0:0:0:0:0:0:0|h[Bolt of Linen Cloth]|h|r 3"), 3u);
    EXPECT_EQ(ParseOrderCount("[Bolt of Linen Cloth] 12"), 12u);
}

TEST(CraftOrderCount, IgnoresAnythingButANumber)
{
    EXPECT_EQ(ParseOrderCount("[Bolt of Linen Cloth] 3x"), 1u);
    EXPECT_EQ(ParseOrderCount("[Bolt of Linen Cloth] -3"), 1u);
    EXPECT_EQ(ParseOrderCount("[Bolt of Linen Cloth] "), 1u);
}

TEST(CraftOrderCount, IsAtLeastOneAndAtMostTheLimit)
{
    EXPECT_EQ(ParseOrderCount("[Bolt of Linen Cloth] 0"), 1u);
    EXPECT_EQ(ParseOrderCount("[Bolt of Linen Cloth] 101"), MAX_ORDER_COUNT);
    EXPECT_EQ(ParseOrderCount("[Bolt of Linen Cloth] 99999999999999999999"), MAX_ORDER_COUNT);
}

TEST(CraftFee, ScalesWithTheHigherOfItemAndRequiredLevel)
{
    EXPECT_EQ(CraftFee(20, 15, 1), 10u);
    EXPECT_EQ(CraftFee(15, 20, 1), 10u);
    EXPECT_EQ(CraftFee(80, 80, 1), 160u);
}

TEST(CraftFee, IsChargedPerCraft) { EXPECT_EQ(CraftFee(20, 15, 3), 30u); }

TEST(CraftFee, LowLevelItemsAreFree) { EXPECT_EQ(CraftFee(5, 1, 10), 0u); }

TEST(CraftFeeSplit, OneMailCarriesTheWholeFee) { EXPECT_EQ(SplitFee(90, {18}), (std::vector<uint32>{90})); }

TEST(CraftFeeSplit, IsSplitByTheItemsEachMailCarries)
{
    EXPECT_EQ(SplitFee(90, {12, 6}), (std::vector<uint32>{60, 30}));
}

TEST(CraftFeeSplit, TheLastMailTakesTheRemainder)
{
    std::vector<uint32> const cods = SplitFee(100, {12, 12, 1});
    ASSERT_EQ(cods.size(), 3u);
    EXPECT_EQ(cods[0], 48u);
    EXPECT_EQ(cods[1], 48u);
    EXPECT_EQ(cods[2], 4u);
}

TEST(CraftFeeSplit, AlwaysAddsUpToTheFee)
{
    for (uint32 fee : {0u, 1u, 7u, 99u, 1000001u})
    {
        std::vector<uint32> const cods = SplitFee(fee, {5, 7, 11, 1});
        uint64 total = 0;
        for (uint32 cod : cods)
            total += cod;
        EXPECT_EQ(total, fee) << "fee " << fee;
    }
}

TEST(CraftFeeSplit, NoMailsNoFee) { EXPECT_TRUE(SplitFee(50, {}).empty()); }

namespace
{
// One bolt per craft from two linen, both stacking to 20.
std::vector<StackNeed> const BOLT_NEEDS = {{1, 20}, {2, 20}};
}  // namespace

TEST(CraftBagSpace, CountsWorstCaseStacks)
{
    EXPECT_EQ(SlotsNeeded(BOLT_NEEDS, 1), 2u);
    EXPECT_EQ(SlotsNeeded(BOLT_NEEDS, 10), 2u);
    EXPECT_EQ(SlotsNeeded(BOLT_NEEDS, 11), 3u);
    EXPECT_EQ(SlotsNeeded(BOLT_NEEDS, 100), 15u);
}

TEST(CraftBagSpace, UnstackableItemsNeedASlotEach)
{
    EXPECT_EQ(SlotsNeeded({{1, 1}}, 4), 4u);
    EXPECT_EQ(SlotsNeeded({{1, 0}}, 4), 4u);
}

TEST(CraftBagSpace, LowersTheCountToWhatFits)
{
    EXPECT_EQ(CraftsThatFit(BOLT_NEEDS, 100, 2), 10u);
    EXPECT_EQ(CraftsThatFit(BOLT_NEEDS, 100, 15), 100u);
    EXPECT_EQ(CraftsThatFit(BOLT_NEEDS, 5, 40), 5u);
}

TEST(CraftBagSpace, RefusesWhenNotEvenOneCraftFits)
{
    EXPECT_EQ(CraftsThatFit(BOLT_NEEDS, 3, 1), 0u);
    EXPECT_EQ(CraftsThatFit(BOLT_NEEDS, 3, 0), 0u);
}
