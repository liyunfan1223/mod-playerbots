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
constexpr uint32 NOW = 1'800'000'000;

OrderState Order()
{
    OrderState order;
    order.itemId = 2996;
    order.count = 1;
    order.reagents[2589] = {2, 2, 0};
    return order;
}
}  // namespace

TEST(CraftMailWatch, BotsThatNeverRefundedDoNotLook)
{
    EXPECT_FALSE(OrderState().IsWatchingMail(NOW));
    EXPECT_FALSE(Order().IsWatchingMail(NOW));
    EXPECT_TRUE(OrderState().IsIdle(NOW));
}

TEST(CraftMailWatch, LooksUntilRefundMailCanNoLongerComeBack)
{
    OrderState order;
    order.WatchMail(NOW);
    EXPECT_TRUE(order.IsWatchingMail(NOW));
    EXPECT_TRUE(order.IsWatchingMail(NOW + MAIL_WATCH_SECONDS - 1));
    EXPECT_FALSE(order.IsWatchingMail(NOW + MAIL_WATCH_SECONDS));
}

TEST(CraftMailWatch, CoversMailReturnedAfterThirtyDays)
{
    OrderState order;
    order.WatchMail(NOW);
    EXPECT_TRUE(order.IsWatchingMail(NOW + 30 * 24 * 60 * 60));
}

TEST(CraftMailWatch, ALaterRefundExtendsTheWindow)
{
    OrderState order;
    order.WatchMail(NOW);
    order.WatchMail(NOW + 100);
    EXPECT_EQ(order.mailWatchUntil, NOW + 100 + MAIL_WATCH_SECONDS);

    // Never shortened, whatever order the refunds are counted in.
    order.WatchMail(NOW);
    EXPECT_EQ(order.mailWatchUntil, NOW + 100 + MAIL_WATCH_SECONDS);
}

TEST(CraftMailWatch, EndingTheOrderKeepsLooking)
{
    OrderState order = Order();
    order.WatchMail(NOW);
    order.Clear();

    EXPECT_TRUE(order.IsEmpty());
    EXPECT_TRUE(order.reagents.empty());
    EXPECT_TRUE(order.IsWatchingMail(NOW));
    EXPECT_FALSE(order.IsIdle(NOW));
    EXPECT_TRUE(order.IsIdle(NOW + MAIL_WATCH_SECONDS));
}

TEST(CraftMailWatch, AnOpenOrderIsNeverIdle)
{
    EXPECT_FALSE(Order().IsIdle(NOW));
    EXPECT_FALSE(Order().IsIdle(NOW + MAIL_WATCH_SECONDS));
}

namespace
{
constexpr uint32 THREE_DAYS = 3 * 24 * 60 * 60;
}

TEST(CraftMailWait, CountsFromDelivery)
{
    EXPECT_FALSE(HasWaited(NOW, NOW, THREE_DAYS));
    EXPECT_FALSE(HasWaited(NOW, NOW + THREE_DAYS - 1, THREE_DAYS));
    EXPECT_TRUE(HasWaited(NOW, NOW + THREE_DAYS, THREE_DAYS));
    EXPECT_TRUE(HasWaited(NOW, NOW + 10 * THREE_DAYS, THREE_DAYS));
}

TEST(CraftMailWait, NoWaitMeansAnyDeliveredMail) { EXPECT_TRUE(HasWaited(NOW, NOW, 0)); }

TEST(CraftMailWait, MailNotYetDeliveredHasNotWaited)
{
    // Mail sent with a delivery delay is not in the mailbox yet.
    EXPECT_FALSE(HasWaited(NOW + 60, NOW, 0));
    EXPECT_FALSE(HasWaited(NOW + 60, NOW, THREE_DAYS));
}
