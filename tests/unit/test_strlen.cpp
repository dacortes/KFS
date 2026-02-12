// SPDX-License-Identifier: GPL-2.0

#include <gtest/gtest.h>
#include <kernel/wrappers/helper.h>


class StrlenTest : public ::testing::Test {
};

TEST_F(StrlenTest, EmptyString)
{
	EXPECT_EQ(0u, ft_strlen(""));
}

TEST_F(StrlenTest, SingleCharacter)
{
	EXPECT_EQ(1u, ft_strlen("A"));
}

TEST_F(StrlenTest, RegularString)
{
	EXPECT_EQ(5u, ft_strlen("Hello"));
}

TEST_F(StrlenTest, StringWithSpaces)
{
	EXPECT_EQ(11u, ft_strlen("Hello World"));
}

TEST_F(StrlenTest, LongerString)
{
	EXPECT_EQ(26u, ft_strlen("abcdefghijklmnopqrstuvwxyz"));
}

TEST_F(StrlenTest, NullPointer)
{
	EXPECT_EQ(0u, ft_strlen(NULL));
}
