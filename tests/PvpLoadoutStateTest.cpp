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
constexpr uint32 NOT_IN_ARENA = 0;
constexpr uint32 ARENA = 7;
constexpr uint32 OTHER_ARENA = 8;
}  // namespace

TEST(PvpLoadoutDecide, EntersAnArenaWithoutALoadout)
{
    EXPECT_EQ(Decide(true, ARENA, false, false, 0), Transition::Enter);
}

TEST(PvpLoadoutDecide, DoesNotEnterWhenDisabled) { EXPECT_EQ(Decide(false, ARENA, false, false, 0), Transition::None); }

TEST(PvpLoadoutDecide, NothingToDoInTheArenaTheLoadoutWasBuiltFor)
{
    EXPECT_EQ(Decide(true, ARENA, false, true, ARENA), Transition::None);
}

TEST(PvpLoadoutDecide, ReappliesForANewArenaWithoutRestoringInBetween)
{
    EXPECT_EQ(Decide(true, OTHER_ARENA, false, true, ARENA), Transition::Reapply);
}

TEST(PvpLoadoutDecide, RestoresOutsideAnArena)
{
    EXPECT_EQ(Decide(true, NOT_IN_ARENA, false, true, ARENA), Transition::Restore);
}

TEST(PvpLoadoutDecide, RestoresEvenWhenTheFeatureWasSwitchedOff)
{
    EXPECT_EQ(Decide(false, NOT_IN_ARENA, false, true, ARENA), Transition::Restore);
}

TEST(PvpLoadoutDecide, SwitchedOffMidMatchWaitsForTheExitToRestore)
{
    EXPECT_EQ(Decide(false, ARENA, false, true, ARENA), Transition::None);
    EXPECT_EQ(Decide(false, OTHER_ARENA, false, true, ARENA), Transition::None);
}

TEST(PvpLoadoutDecide, NothingToDoOutsideAnArenaWithoutALoadout)
{
    EXPECT_EQ(Decide(true, NOT_IN_ARENA, false, false, 0), Transition::None);
    EXPECT_EQ(Decide(false, NOT_IN_ARENA, false, false, 0), Transition::None);
}

TEST(PvpLoadoutDecide, RestoresWhenTheMatchEndsWhileStillInTheArena)
{
    EXPECT_EQ(Decide(true, ARENA, true, true, ARENA), Transition::Restore);
}

TEST(PvpLoadoutDecide, RestoresAtMatchEndEvenWhenTheFeatureWasSwitchedOff)
{
    EXPECT_EQ(Decide(false, ARENA, true, true, ARENA), Transition::Restore);
}

TEST(PvpLoadoutDecide, DoesNotEnterAnArenaWhoseMatchHasEnded)
{
    EXPECT_EQ(Decide(true, ARENA, true, false, 0), Transition::None);
}

TEST(PvpLoadoutDecide, PlannedGearGoesOnInItsArena)
{
    EXPECT_EQ(Decide(true, ARENA, false, true, ARENA, true), Transition::ApplyGear);
}

TEST(PvpLoadoutDecide, PlannedGearWaitsWhenTheFeatureIsTurnedOff)
{
    // The PvE items were never touched, so the bot just keeps them until the match ends.
    EXPECT_EQ(Decide(false, ARENA, false, true, ARENA, true), Transition::None);
}

TEST(PvpLoadoutDecide, PlannedGearIsDroppedWhenTheMatchEndsOrTheBotLeaves)
{
    EXPECT_EQ(Decide(true, ARENA, true, true, ARENA, true), Transition::Restore);
    EXPECT_EQ(Decide(true, NOT_IN_ARENA, false, true, ARENA, true), Transition::Restore);
}

TEST(PvpLoadoutDecide, ANewArenaReappliesEvenWithGearPlanned)
{
    EXPECT_EQ(Decide(true, OTHER_ARENA, false, true, ARENA, true), Transition::Reapply);
}
