/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpLoadoutRules.h"
#include "gtest/gtest.h"

using namespace PvpLoadout;

TEST(PvpLoadoutCopyFormat, ChargesMatchTheItemInstanceColumn)
{
    ItemCharges const charges = {-1, 0, 3, 0, 0};
    EXPECT_EQ(FormatCharges(charges), "-1 0 3 0 0 ");

    ItemCharges parsed{};
    ASSERT_TRUE(ParseCharges("-1 0 3 0 0 ", parsed));
    EXPECT_EQ(parsed, charges);
}

TEST(PvpLoadoutCopyFormat, EnchantmentsRoundTripAsTriples)
{
    // A permanent enchant, a temporary one with time and charges left, a gem and the belt buckle's prismatic socket.
    ItemEnchantments enchantments{};
    enchantments[0] = {3832, 0, 0};
    enchantments[1] = {2629, 1800000, 5};
    enchantments[2] = {3525, 0, 0};
    enchantments[6] = {3729, 0, 0};

    std::string const text = FormatEnchantments(enchantments);
    EXPECT_EQ(text.rfind("3832 0 0 2629 1800000 5 3525 0 0 ", 0), 0u);

    ItemEnchantments parsed{};
    ASSERT_TRUE(ParseEnchantments(text, parsed));
    for (size_t i = 0; i < enchantments.size(); ++i)
    {
        EXPECT_EQ(parsed[i].id, enchantments[i].id) << "slot " << i;
        EXPECT_EQ(parsed[i].duration, enchantments[i].duration) << "slot " << i;
        EXPECT_EQ(parsed[i].charges, enchantments[i].charges) << "slot " << i;
    }
}

TEST(PvpLoadoutCopyFormat, RejectsWrongCountsAndNonNumbers)
{
    ItemCharges charges{};
    EXPECT_FALSE(ParseCharges("0 0 0 0 ", charges));
    EXPECT_FALSE(ParseCharges("0 0 0 0 0 0 ", charges));
    EXPECT_FALSE(ParseCharges("0 0 x 0 0 ", charges));
    EXPECT_FALSE(ParseCharges("", charges));

    ItemEnchantments enchantments{};
    EXPECT_FALSE(ParseEnchantments("3832 0 0 ", enchantments));
    EXPECT_FALSE(ParseEnchantments(FormatEnchantments({}) + "1 ", enchantments));
}

TEST(PvpLoadoutCopyFormat, AFailedParseLeavesTheValuesUntouched)
{
    ItemCharges charges = {1, 2, 3, 4, 5};
    EXPECT_FALSE(ParseCharges("9 9 9 ", charges));
    EXPECT_EQ(charges, (ItemCharges{1, 2, 3, 4, 5}));
}
