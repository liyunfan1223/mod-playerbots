/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpLoadoutEp.h"
#include "gtest/gtest.h"

using namespace PvpLoadout;

namespace
{
StatVector Stats(std::initializer_list<std::pair<Stat, float>> values)
{
    StatVector stats{};
    for (auto const& [stat, value] : values)
        At(stats, stat) += value;
    return stats;
}

Option Own(uint32 itemId, StatVector stats, bool unique = false) { return {{itemId}, stats, true, unique}; }
Option New(uint32 itemId, StatVector stats, bool unique = false) { return {{itemId}, stats, false, unique}; }
}  // namespace

TEST(PvpLoadoutProfile, DefaultsMatchTheRoleTable)
{
    Profile const caster = DefaultProfile(Role::Caster);
    EXPECT_EQ(caster.hitTarget, 105u);
    EXPECT_EQ(caster.spellPenetrationTarget, 130u);
    EXPECT_EQ(caster.resilienceTarget, 1250u);
    std::vector<Stat> const priority = {Stat::SpellPower, Stat::Haste, Stat::Crit};
    EXPECT_EQ(caster.priority, priority);
    EXPECT_EQ(caster.weaponDpsWeight, 0.0f);

    EXPECT_EQ(DefaultProfile(Role::Healer).hitTarget, 0u);
    EXPECT_GT(DefaultProfile(Role::Melee).weaponDpsWeight, 0.0f);
}

TEST(PvpLoadoutProfile, ParsesAConfigLine)
{
    Profile profile = DefaultProfile(Role::Hunter);
    ASSERT_TRUE(ParseProfile("hit:150, spellpen:100, resilience:900, priority:agility,crit", Role::Hunter, profile));
    EXPECT_EQ(profile.hitTarget, 150u);
    EXPECT_EQ(profile.spellPenetrationTarget, 100u);
    EXPECT_EQ(profile.resilienceTarget, 900u);
    std::vector<Stat> const priority = {Stat::Agility, Stat::Crit};
    EXPECT_EQ(profile.priority, priority);
    EXPECT_EQ(profile.weaponDpsWeight, DefaultProfile(Role::Hunter).weaponDpsWeight);
}

TEST(PvpLoadoutProfile, RejectsUnknownKeysStatsAndNumbers)
{
    Profile profile = DefaultProfile(Role::Melee);
    EXPECT_FALSE(ParseProfile("hit:abc", Role::Melee, profile));
    EXPECT_FALSE(ParseProfile("dodge:5", Role::Melee, profile));
    EXPECT_FALSE(ParseProfile("priority:strength,luck", Role::Melee, profile));
    EXPECT_EQ(profile.hitTarget, DefaultProfile(Role::Melee).hitTarget);
}

TEST(PvpLoadoutEp, HitStopsCountingAtTheTarget)
{
    Profile const melee = DefaultProfile(Role::Melee);
    StatVector const atCap = Stats({{Stat::Hit, 164.0f}});
    EXPECT_GT(MarginalEp({}, Stats({{Stat::Hit, 20.0f}}), melee), 0.0f);
    EXPECT_FLOAT_EQ(MarginalEp(atCap, Stats({{Stat::Hit, 20.0f}}), melee), 0.0f);
}

TEST(PvpLoadoutEp, SpellPenetrationOnlyCountsForRolesThatNeedIt)
{
    StatVector const spellPen = Stats({{Stat::SpellPenetration, 30.0f}});
    EXPECT_GT(MarginalEp({}, spellPen, DefaultProfile(Role::Caster)), 0.0f);
    EXPECT_FLOAT_EQ(MarginalEp({}, spellPen, DefaultProfile(Role::Melee)), 0.0f);
    EXPECT_FLOAT_EQ(MarginalEp({}, spellPen, DefaultProfile(Role::Healer)), 0.0f);
}

TEST(PvpLoadoutEp, ResilienceIsWorthLessPastTheTarget)
{
    Profile const healer = DefaultProfile(Role::Healer);
    StatVector const resilience = Stats({{Stat::Resilience, 50.0f}});
    float const below = MarginalEp({}, resilience, healer);
    float const above = MarginalEp(Stats({{Stat::Resilience, 1400.0f}}), resilience, healer);
    EXPECT_GT(above, 0.0f);
    EXPECT_LT(above, below);
}

TEST(PvpLoadoutEp, PriorityStatsFollowTheirOrder)
{
    Profile const caster = DefaultProfile(Role::Caster);
    EXPECT_GT(MarginalEp({}, Stats({{Stat::SpellPower, 10.0f}}), caster),
              MarginalEp({}, Stats({{Stat::Haste, 10.0f}}), caster));
    EXPECT_FLOAT_EQ(MarginalEp({}, Stats({{Stat::Strength, 10.0f}}), caster), 0.0f);
}

TEST(PvpLoadoutSet, KeepsTheFourPieceOverHigherLevelMixedPieces)
{
    // At 1600: three newer-season pieces are eligible, the older season has all five.
    std::vector<SetPiece> const pieces = {{0, 2, 10, 251}, {2, 2, 11, 251}, {4, 2, 12, 251}, {6, 2, 13, 251},
                                          {9, 2, 14, 251}, {4, 3, 20, 270}, {6, 3, 21, 270}, {9, 3, 22, 270}};
    std::map<uint8, uint32> const chosen = PickSetPieces(pieces, 4);
    ASSERT_EQ(chosen.size(), 4u);
    for (auto const& [slot, itemId] : chosen)
        EXPECT_LT(itemId, 20u);
}

TEST(PvpLoadoutSet, PrefersTheSetWithTheHigherAverageItemLevel)
{
    std::vector<SetPiece> const pieces = {{0, 2, 10, 245}, {2, 2, 11, 245}, {4, 2, 12, 245}, {6, 2, 13, 245},
                                          {0, 3, 20, 264}, {2, 3, 21, 264}, {4, 3, 22, 264}, {6, 3, 23, 264}};
    std::map<uint8, uint32> const chosen = PickSetPieces(pieces, 4);
    ASSERT_EQ(chosen.size(), 4u);
    EXPECT_EQ(chosen.at(0), 20u);
}

TEST(PvpLoadoutSet, TiesGoToTheLaterSeason)
{
    std::vector<SetPiece> const pieces = {{0, 2, 10, 245}, {2, 2, 11, 245}, {4, 2, 12, 245}, {6, 2, 13, 245},
                                          {0, 3, 20, 245}, {2, 3, 21, 245}, {4, 3, 22, 245}, {6, 3, 23, 245}};
    EXPECT_EQ(PickSetPieces(pieces, 4).at(0), 20u);
}

TEST(PvpLoadoutSet, NoSetWithEnoughPiecesGivesNothing)
{
    std::vector<SetPiece> const pieces = {{0, 2, 10, 251}, {2, 2, 11, 251}, {4, 3, 20, 270}, {6, 3, 21, 270}};
    EXPECT_TRUE(PickSetPieces(pieces, 4).empty());
}

TEST(PvpLoadoutSelect, HitGoesWhereItIsNeededThenStops)
{
    // Two slots, each offering hit or resilience. Only one hit item is needed to reach the cap.
    Profile const melee = DefaultProfile(Role::Melee);
    StatVector const base = Stats({{Stat::Hit, 120.0f}});
    Group const slot = {{New(1, Stats({{Stat::Hit, 50.0f}})), New(2, Stats({{Stat::Resilience, 60.0f}}))}};
    std::vector<int32> const chosen = SelectBiggestGainFirst(base, {slot, slot}, melee);
    ASSERT_EQ(chosen.size(), 2u);
    EXPECT_NE(chosen[0], chosen[1]);
}

TEST(PvpLoadoutSelect, AUniqueItemIsNotTakenTwice)
{
    Profile const caster = DefaultProfile(Role::Caster);
    Group const ring = {{New(1, Stats({{Stat::SpellPower, 40.0f}}), true), New(2, Stats({{Stat::SpellPower, 20.0f}}))}};
    std::vector<int32> const chosen = SelectBiggestGainFirst({}, {ring, ring}, caster);
    ASSERT_EQ(chosen.size(), 2u);
    EXPECT_EQ(chosen[0] + chosen[1], 1);
}

TEST(PvpLoadoutSelect, ANonUniqueItemMayBeTakenTwice)
{
    Profile const caster = DefaultProfile(Role::Caster);
    Group const ring = {{New(1, Stats({{Stat::SpellPower, 40.0f}})), New(2, Stats({{Stat::SpellPower, 20.0f}}))}};
    std::vector<int32> const expected = {0, 0};
    EXPECT_EQ(SelectBiggestGainFirst({}, {ring, ring}, caster), expected);
}

TEST(PvpLoadoutSelect, TheBotsOwnItemWinsTies)
{
    Profile const melee = DefaultProfile(Role::Melee);
    StatVector const same = Stats({{Stat::Resilience, 40.0f}});
    Group const slot = {{New(1, same), Own(2, same)}};
    std::vector<int32> const expected = {1};
    EXPECT_EQ(SelectBiggestGainFirst({}, {slot}, melee), expected);
}

TEST(PvpLoadoutSelect, WeaponsAreComparedAsWholeOptions)
{
    // A two-hander against a main hand plus off-hand, decided as one group.
    Profile const melee = DefaultProfile(Role::Melee);
    Option const twoHander = {{30}, Stats({{Stat::WeaponDps, 300.0f}, {Stat::Strength, 60.0f}}), false, false};
    Option const pair = {{31, 32}, Stats({{Stat::WeaponDps, 200.0f}, {Stat::Strength, 60.0f}}), false, false};
    std::vector<int32> const expected = {0};
    EXPECT_EQ(SelectBiggestGainFirst({}, {Group{{twoHander, pair}}}, melee), expected);
}

TEST(PvpLoadoutSelect, AGroupWithNoOptionsIsLeftEmpty)
{
    std::vector<int32> const expected = {-1};
    EXPECT_EQ(SelectBiggestGainFirst({}, {Group{}}, DefaultProfile(Role::Melee)), expected);
}

TEST(PvpLoadoutTargets, FormatsTotalsAgainstTheProfile)
{
    StatVector const totals = Stats({{Stat::Hit, 120.6f}, {Stat::Resilience, 900.0f}});
    EXPECT_EQ(FormatTargets(totals, DefaultProfile(Role::Hunter)), "hit 120/164, spellpen 0/130, resilience 900/1050");
}

TEST(PvpLoadoutVector, SubtractNeverGoesBelowZero)
{
    StatVector const totals = Subtract(Stats({{Stat::Hit, 30.0f}}), Stats({{Stat::Hit, 50.0f}, {Stat::Crit, 10.0f}}));
    EXPECT_FLOAT_EQ(At(totals, Stat::Hit), 0.0f);
    EXPECT_FLOAT_EQ(At(totals, Stat::Crit), 0.0f);
}

TEST(PvpLoadoutEp, AHitGemIsWorthLessThanAPowerGemOnceTheCapIsReached)
{
    // Each gem is scored against the totals so far: hit wins while short of the target, then stops counting.
    Profile const caster = DefaultProfile(Role::Caster);
    StatVector const hitGem = Stats({{Stat::Hit, 20.0f}});
    StatVector const powerGem = Stats({{Stat::SpellPower, 23.0f}});
    StatVector totals = Stats({{Stat::Hit, 85.0f}});
    EXPECT_GT(MarginalEp(totals, hitGem, caster), MarginalEp(totals, powerGem, caster));

    totals = Add(totals, hitGem);
    EXPECT_LT(MarginalEp(totals, hitGem, caster), MarginalEp(totals, powerGem, caster));
}
