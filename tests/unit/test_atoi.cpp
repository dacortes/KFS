// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_atoi.cpp
 * @brief Unit tests for ft_atoi
 */

#include <gtest/gtest.h>

extern "C" {
#include <kernel/wrappers/helper.h>
}

TEST(AtoiTest, ConvertsSimplePositive)
{
	EXPECT_EQ(ft_atoi("42"), 42);
}

TEST(AtoiTest, ConvertsZero)
{
	EXPECT_EQ(ft_atoi("0"), 0);
}

TEST(AtoiTest, ConvertsNegative)
{
	EXPECT_EQ(ft_atoi("-42"), -42);
}

TEST(AtoiTest, ConvertsWithExplicitPlus)
{
	EXPECT_EQ(ft_atoi("+42"), 42);
}

TEST(AtoiTest, ConvertsWithLeadingSpaces)
{
	EXPECT_EQ(ft_atoi("   123"), 123);
}

TEST(AtoiTest, ConvertsWithLeadingTabs)
{
	EXPECT_EQ(ft_atoi("\t\n\v\f\r 99"), 99);
}

TEST(AtoiTest, StopsAtNonDigit)
{
	EXPECT_EQ(ft_atoi("123abc"), 123);
}

TEST(AtoiTest, ReturnsZeroForNonNumericStart)
{
	EXPECT_EQ(ft_atoi("abc"), 0);
}

TEST(AtoiTest, ConvertsLargeNumber)
{
	EXPECT_EQ(ft_atoi("2147483647"), 2147483647);
}

TEST(AtoiTest, EmptyStringReturnsZero)
{
	EXPECT_EQ(ft_atoi(""), 0);
}

TEST(AtoiTest, WhitespaceOnlyReturnsZero)
{
	EXPECT_EQ(ft_atoi("   "), 0);
}

TEST(AtoiTest, NegativeWithSpaces)
{
	EXPECT_EQ(ft_atoi("  -100"), -100);
}
