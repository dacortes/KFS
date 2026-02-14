// SPDX-License-Identifier: GPL-2.0

#include <gtest/gtest.h>
#include <kernel/wrappers/helper.h>


class StrcmpTest : public ::testing::Test {
};

TEST_F(StrcmpTest, EqualStrings)
{
	EXPECT_EQ(0, ft_strcmp("hello", "hello"));
}

TEST_F(StrcmpTest, EmptyStrings)
{
	EXPECT_EQ(0, ft_strcmp("", ""));
}

TEST_F(StrcmpTest, FirstLessThanSecond)
{
	EXPECT_LT(ft_strcmp("abc", "abd"), 0);
}

TEST_F(StrcmpTest, FirstGreaterThanSecond)
{
	EXPECT_GT(ft_strcmp("abd", "abc"), 0);
}

TEST_F(StrcmpTest, FirstShorterThanSecond)
{
	EXPECT_LT(ft_strcmp("abc", "abcd"), 0);
}

TEST_F(StrcmpTest, FirstLongerThanSecond)
{
	EXPECT_GT(ft_strcmp("abcd", "abc"), 0);
}

TEST_F(StrcmpTest, EmptyVsNonEmpty)
{
	EXPECT_LT(ft_strcmp("", "a"), 0);
}

TEST_F(StrcmpTest, NonEmptyVsEmpty)
{
	EXPECT_GT(ft_strcmp("a", ""), 0);
}

TEST_F(StrcmpTest, SingleCharEqual)
{
	EXPECT_EQ(0, ft_strcmp("x", "x"));
}

TEST_F(StrcmpTest, SingleCharDifferent)
{
	EXPECT_LT(ft_strcmp("a", "b"), 0);
	EXPECT_GT(ft_strcmp("b", "a"), 0);
}
