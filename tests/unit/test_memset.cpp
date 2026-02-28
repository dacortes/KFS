// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_memset.cpp
 * @brief Unit tests for ft_memset
 */

#include <gtest/gtest.h>
#include <cstring>

extern "C" {
void *ft_memset(void *b, int c, unsigned int len);
}

TEST(MemsetTest, FillsBufferWithByte)
{
	char buf[10];

	ft_memset(buf, 'A', sizeof(buf));

	for (unsigned int i = 0; i < sizeof(buf); i++)
		EXPECT_EQ(buf[i], 'A');
}

TEST(MemsetTest, FillsWithZero)
{
	char buf[10];

	memset(buf, 0xFF, sizeof(buf));

	ft_memset(buf, 0, sizeof(buf));

	for (unsigned int i = 0; i < sizeof(buf); i++)
		EXPECT_EQ(buf[i], 0);
}

TEST(MemsetTest, ZeroLengthDoesNothing)
{
	char buf[4] = {'A', 'B', 'C', 'D'};

	ft_memset(buf, 'X', 0);

	EXPECT_EQ(buf[0], 'A');
	EXPECT_EQ(buf[1], 'B');
	EXPECT_EQ(buf[2], 'C');
	EXPECT_EQ(buf[3], 'D');
}

TEST(MemsetTest, PartialFill)
{
	char buf[6] = {'a', 'b', 'c', 'd', 'e', 'f'};

	ft_memset(buf, 'Z', 3);

	EXPECT_EQ(buf[0], 'Z');
	EXPECT_EQ(buf[1], 'Z');
	EXPECT_EQ(buf[2], 'Z');
	EXPECT_EQ(buf[3], 'd');
	EXPECT_EQ(buf[4], 'e');
	EXPECT_EQ(buf[5], 'f');
}

TEST(MemsetTest, ReturnsPointerToBuffer)
{
	char buf[4];
	void *ret = ft_memset(buf, 0, sizeof(buf));

	EXPECT_EQ(ret, buf);
}

TEST(MemsetTest, TruncatesValueToUnsignedChar)
{
	/* Value 0x141 should be truncated to 0x41 = 'A' */
	char buf[4];

	ft_memset(buf, 0x141, 4);

	for (int i = 0; i < 4; i++)
		EXPECT_EQ((unsigned char)buf[i], 0x41);
}
