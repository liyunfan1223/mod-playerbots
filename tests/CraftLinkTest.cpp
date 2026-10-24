/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CraftRules.h"
#include "gtest/gtest.h"

using namespace CraftRules;

TEST(CraftRecipeBits, NoEntriesNoCharacters) { EXPECT_EQ(EncodeRecipeBits({}), ""); }

TEST(CraftRecipeBits, PacksSixEntriesPerCharacter)
{
    EXPECT_EQ(EncodeRecipeBits(std::vector<bool>(6, false)), "A");
    EXPECT_EQ(EncodeRecipeBits(std::vector<bool>(7, false)), "AA");
    EXPECT_EQ(EncodeRecipeBits(std::vector<bool>(12, false)), "AA");
    EXPECT_EQ(EncodeRecipeBits(std::vector<bool>(13, false)), "AAA");
}

TEST(CraftRecipeBits, TheFirstEntryIsTheLowestBit)
{
    EXPECT_EQ(EncodeRecipeBits({true}), "B");
    EXPECT_EQ(EncodeRecipeBits({false, true}), "C");
    EXPECT_EQ(EncodeRecipeBits({false, false, false, false, false, true}), "g");
}

TEST(CraftRecipeBits, UsesStandardBase64)
{
    EXPECT_EQ(EncodeRecipeBits(std::vector<bool>(6, true)), "/");
    EXPECT_EQ(EncodeRecipeBits({false, true, true, true, true, true}), "+");
    EXPECT_EQ(EncodeRecipeBits({false, false, true, true, false, true}), "s");
}

TEST(CraftRecipeBits, TheSeventhEntryStartsTheNextCharacter)
{
    EXPECT_EQ(EncodeRecipeBits({false, false, false, false, false, false, true}), "AB");
    EXPECT_EQ(EncodeRecipeBits({true, false, false, false, false, false, true}), "BB");
}
