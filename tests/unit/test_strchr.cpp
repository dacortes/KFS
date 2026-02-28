// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_strchr.cpp
 * @brief Unit tests for ft_strchr
 */

#include <gtest/gtest.h>

extern "C" {
#include <kernel/wrappers/helper.h>
}

TEST(StrchrTest, FindsFirstOccurrence)
{
	const char *s = "Hello, World!";
	char *result = ft_strchr(s, 'o');

	ASSERT_NE(result, nullptr);
	EXPECT_EQ(result - s, 4);
}

TEST(StrchrTest, FindsFirstCharacter)
{
	const char *s = "abcdef";
	char *result = ft_strchr(s, 'a');

	ASSERT_NE(result, nullptr);
	EXPECT_EQ(result, s);
}

TEST(StrchrTest, FindsLastCharacter)
{
	const char *s = "abcdef";
	char *result = ft_strchr(s, 'f');

	ASSERT_NE(result, nullptr);
	EXPECT_EQ(result - s, 5);
}

TEST(StrchrTest, FindsNullTerminator)
{
	const char *s = "abc";
	char *result = ft_strchr(s, '\0');

	ASSERT_NE(result, nullptr);
	EXPECT_EQ(result - s, 3);
}

TEST(StrchrTest, ReturnsNullWhenNotFound)
{
	const char *s = "Hello";
	char *result = ft_strchr(s, 'z');

	EXPECT_EQ(result, nullptr);
}

TEST(StrchrTest, EmptyStringFindsNull)
{
	const char *s = "";
	char *result = ft_strchr(s, '\0');

	ASSERT_NE(result, nullptr);
	EXPECT_EQ(result, s);
}

TEST(StrchrTest, EmptyStringReturnsNullForNonNull)
{
	const char *s = "";
	char *result = ft_strchr(s, 'a');

	EXPECT_EQ(result, nullptr);
}

TEST(StrchrTest, FindsDuplicateAtFirstPosition)
{
	const char *s = "aabaa";
	char *result = ft_strchr(s, 'a');

	ASSERT_NE(result, nullptr);
	EXPECT_EQ(result, s);
}
