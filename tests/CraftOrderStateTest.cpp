/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CraftRules.h"
#include "gtest/gtest.h"

using namespace CraftRules;

namespace
{
constexpr uint32 BOLT = 2996;
constexpr uint32 LINEN = 2589;
constexpr uint32 THREAD = 2320;
constexpr uint32 OTHER = 4306;

// Bolt of Linen Cloth needs two linen; a second reagent shows every reagent is checked.
OrderState Order(uint32 count, uint32 ownLinen = 0, uint32 ownProducts = 0)
{
    OrderState order;
    order.itemId = BOLT;
    order.count = count;
    order.ownProducts = ownProducts;
    order.reagents[LINEN] = {2, 0, ownLinen};
    order.reagents[THREAD] = {1, 0, 0};
    return order;
}
}  // namespace

TEST(CraftOrderState, WaitsForEveryReagentOfTheNextCraft)
{
    OrderState order = Order(2);
    EXPECT_FALSE(order.HasReagentsForNextCraft());

    order.AddObtained(LINEN, 2);
    EXPECT_FALSE(order.HasReagentsForNextCraft());

    order.AddObtained(THREAD, 1);
    EXPECT_TRUE(order.HasReagentsForNextCraft());
}

TEST(CraftOrderState, OnlyCountsTheOrdersReagents)
{
    OrderState order = Order(1);
    EXPECT_FALSE(order.AddObtained(OTHER, 5));
    EXPECT_TRUE(order.AddObtained(LINEN, 5));
    EXPECT_EQ(order.reagents[LINEN].obtained, 5u);
    EXPECT_FALSE(order.IsRequired(OTHER));
}

TEST(CraftOrderState, EachCraftUsesOneCraftsWorth)
{
    OrderState order = Order(2);
    order.AddObtained(LINEN, 4);
    order.AddObtained(THREAD, 2);

    order.OnCrafted(1);
    EXPECT_EQ(order.reagents[LINEN].obtained, 2u);
    EXPECT_EQ(order.reagents[THREAD].obtained, 1u);
    EXPECT_EQ(order.crafted, 1u);
    EXPECT_EQ(order.made, 1u);
    EXPECT_FALSE(order.IsCrafted());

    order.OnCrafted(1);
    EXPECT_TRUE(order.IsCrafted());
    EXPECT_EQ(order.made, 2u);
}

TEST(CraftOrderState, StopsCraftingOnceTheOrderIsDone)
{
    OrderState order = Order(1);
    order.AddObtained(LINEN, 10);
    order.AddObtained(THREAD, 10);
    order.OnCrafted(1);

    // Extra reagents are refunded, not turned into crafts nobody ordered.
    EXPECT_FALSE(order.HasReagentsForNextCraft());
}

TEST(CraftOrderState, ProcsCountEveryItemMade)
{
    OrderState order = Order(1);
    order.AddObtained(LINEN, 2);
    order.AddObtained(THREAD, 1);
    order.OnCrafted(3);
    EXPECT_EQ(order.made, 3u);
}

TEST(CraftOrderState, ReservesTheProductAndReagentsOnly)
{
    OrderState const order = Order(1);
    EXPECT_TRUE(order.IsReserved(BOLT));
    EXPECT_TRUE(order.IsReserved(LINEN));
    EXPECT_FALSE(order.IsReserved(OTHER));
    EXPECT_FALSE(OrderState().IsReserved(BOLT));
}

TEST(CraftOrderState, AnEmptyOrderIsNeverCraftedOrReady)
{
    OrderState const order;
    EXPECT_TRUE(order.IsEmpty());
    EXPECT_FALSE(order.IsCrafted());
    EXPECT_FALSE(order.HasReagentsForNextCraft());
    EXPECT_EQ(order.OwedProducts(20), 0u);
}

TEST(CraftOrderState, TheFeeIsPaidWithTheFirstDelivery)
{
    OrderState order = Order(1);
    order.made = 1;
    EXPECT_FALSE(order.FeePaid());
    order.OnDelivered(1);
    EXPECT_TRUE(order.FeePaid());
}

TEST(CraftOrderScams, TheBotsOwnReagentsAreNeverRefunded)
{
    // The bot held 20 linen of its own; the customer traded 2 and claims nothing more.
    OrderState order = Order(1, 20);
    order.AddObtained(LINEN, 2);
    order.LimitObtained(LINEN, 22);
    EXPECT_EQ(order.reagents[LINEN].obtained, 2u);
}

TEST(CraftOrderScams, ReagentsTakenBackAreNotRefundedAgain)
{
    // The customer traded 2 linen, then took them back with "t".
    OrderState order = Order(1, 20);
    order.AddObtained(LINEN, 2);
    order.LimitObtained(LINEN, 20);
    EXPECT_EQ(order.reagents[LINEN].obtained, 0u);
}

TEST(CraftOrderScams, ReagentsTheBotUsedUpLowerTheCustomersShare)
{
    OrderState order = Order(1, 0);
    order.AddObtained(LINEN, 5);
    order.LimitObtained(LINEN, 3);
    EXPECT_EQ(order.reagents[LINEN].obtained, 3u);

    // The bot's stock fell below what it had before the order: none of it is the customer's.
    OrderState stocked = Order(1, 20);
    stocked.AddObtained(LINEN, 5);
    stocked.LimitObtained(LINEN, 10);
    EXPECT_EQ(stocked.reagents[LINEN].obtained, 0u);
}

TEST(CraftOrderScams, LimitingAnUnknownItemChangesNothing)
{
    OrderState order = Order(1);
    order.AddObtained(LINEN, 2);
    order.LimitObtained(OTHER, 0);
    EXPECT_EQ(order.reagents[LINEN].obtained, 2u);
}

TEST(CraftOrderScams, OnlyWhatWasMadeIsOwed)
{
    // The bot already had 10 bolts of its own and crafted 1.
    OrderState order = Order(1, 0, 10);
    order.made = 1;
    EXPECT_EQ(order.OwedProducts(11), 1u);
    EXPECT_EQ(order.OwedProducts(50), 1u);
}

TEST(CraftOrderScams, ProductsTheBotNoLongerHoldsAreNotOwed)
{
    OrderState order = Order(3, 0, 10);
    order.made = 3;
    EXPECT_EQ(order.OwedProducts(11), 1u);
    EXPECT_EQ(order.OwedProducts(10), 0u);
    EXPECT_EQ(order.OwedProducts(5), 0u);
}

TEST(CraftOrderScams, DeliveriesAreNeverCountedTwice)
{
    OrderState order = Order(3);
    order.made = 3;
    order.OnDelivered(2);
    EXPECT_EQ(order.OwedProducts(3), 1u);

    order.OnDelivered(5);
    EXPECT_EQ(order.delivered, 3u);
    EXPECT_EQ(order.OwedProducts(3), 0u);
}
