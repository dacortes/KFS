// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_isdigit.cpp
 * @brief Unit tests for ft_isdigit
 */

#include <gtest/gtest.h>

extern "C" {
#include <kernel/wrappers/helper.h>
}

TEST(IsdigitTest, ZeroIsDigit)
{
	EXPECT_EQ(ft_isdigit('0'), 1);
}

TEST(IsdigitTest, NineIsDigit)
{
	EXPECT_EQ(ft_isdigit('9'), 1);
}

TEST(IsdigitTest, AllDigitsReturnOne)
{
	for (char c = '0'; c <= '9'; c++)
		EXPECT_EQ(ft_isdigit(c), 1);
}

TEST(IsdigitTest, LetterIsNotDigit)
{
	EXPECT_EQ(ft_isdigit('a'), 0);
	EXPECT_EQ(ft_isdigit('Z'), 0);
}

TEST(IsdigitTest, SpaceIsNotDigit)
{
	EXPECT_EQ(ft_isdigit(' '), 0);
}

TEST(IsdigitTest, NullIsNotDigit)
{
	EXPECT_EQ(ft_isdigit('\0'), 0);
}

TEST(IsdigitTest, SpecialCharIsNotDigit)
{
	EXPECT_EQ(ft_isdigit('!'), 0);
	EXPECT_EQ(ft_isdigit('/'), 0);
	EXPECT_EQ(ft_isdigit(':'), 0);
}

TEST(IsdigitTest, BoundaryBeforeZero)
{
	/* '/' is ASCII 47, just before '0' (48) */
	EXPECT_EQ(ft_isdigit('/'), 0);
}

TEST(IsdigitTest, BoundaryAfterNine)
{
	/* ':' is ASCII 58, just after '9' (57) */
	EXPECT_EQ(ft_isdigit(':'), 0);
}
