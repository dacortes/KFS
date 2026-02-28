// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_memchr.cpp
 * @brief Unit tests for ft_memchr
 */

#include <gtest/gtest.h>
#include <cstring>

extern "C" {
#include <kernel/wrappers/helper.h>
}

TEST(MemchrTest, FindsFirstOccurrence)
{
	const char *s = "Hello, World!";
	void *result = ft_memchr(s, 'o', strlen(s));

	ASSERT_NE(result, nullptr);
	EXPECT_EQ((const char *)result - s, 4);
}

TEST(MemchrTest, FindsFirstCharacter)
{
	const char *s = "abcdef";
	void *result = ft_memchr(s, 'a', 6);

	ASSERT_NE(result, nullptr);
	EXPECT_EQ((const char *)result, s);
}

TEST(MemchrTest, FindsLastCharacter)
{
	const char *s = "abcdef";
	void *result = ft_memchr(s, 'f', 6);

	ASSERT_NE(result, nullptr);
	EXPECT_EQ((const char *)result - s, 5);
}

TEST(MemchrTest, ReturnsNullWhenNotFound)
{
	const char *s = "Hello";
	void *result = ft_memchr(s, 'z', strlen(s));

	EXPECT_EQ(result, nullptr);
}

TEST(MemchrTest, ReturnsNullForZeroLength)
{
	const char *s = "Hello";
	void *result = ft_memchr(s, 'H', 0);

	EXPECT_EQ(result, nullptr);
}

TEST(MemchrTest, FindsNullTerminator)
{
	const char *s = "abc";
	void *result = ft_memchr(s, '\0', 4);

	ASSERT_NE(result, nullptr);
	EXPECT_EQ((const char *)result - s, 3);
}

TEST(MemchrTest, SearchesOnlyNBytes)
{
	const char *s = "abcdef";

	/* 'f' is at index 5, but we only search 3 bytes */
	void *result = ft_memchr(s, 'f', 3);

	EXPECT_EQ(result, nullptr);
}

TEST(MemchrTest, WorksWithBinaryData)
{
	unsigned char data[] = {0x00, 0x10, 0xFF, 0x42, 0x10};
	void *result = ft_memchr(data, 0xFF, sizeof(data));

	ASSERT_NE(result, nullptr);
	EXPECT_EQ((unsigned char *)result - data, 2);
}

TEST(MemchrTest, CastsCharToUnsigned)
{
	unsigned char data[] = {0x80, 0x90, 0xFF};
	void *result = ft_memchr(data, 0x90, 3);

	ASSERT_NE(result, nullptr);
	EXPECT_EQ((unsigned char *)result - data, 1);
}
