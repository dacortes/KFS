// SPDX-License-Identifier: GPL-2.0

#include <gtest/gtest.h>
#include <kernel/wrappers/helper.h>
#include <cstring>


class StrcpyTest : public ::testing::Test {
protected:
	char dst[64];

	void SetUp() override
	{
		memset(dst, 'X', sizeof(dst));
	}
};

TEST_F(StrcpyTest, BasicCopy)
{
	char *ret = ft_strcpy(dst, "Hello");

	EXPECT_EQ(dst, ret);
	EXPECT_STREQ("Hello", dst);
}

TEST_F(StrcpyTest, EmptyString)
{
	char *ret = ft_strcpy(dst, "");

	EXPECT_EQ(dst, ret);
	EXPECT_STREQ("", dst);
}

TEST_F(StrcpyTest, SingleCharacter)
{
	ft_strcpy(dst, "A");

	EXPECT_STREQ("A", dst);
}

TEST_F(StrcpyTest, CopiesToNulTerminator)
{
	ft_strcpy(dst, "Hi");

	EXPECT_EQ('H', dst[0]);
	EXPECT_EQ('i', dst[1]);
	EXPECT_EQ('\0', dst[2]);
	/* Bytes beyond should be untouched */
	EXPECT_EQ('X', dst[3]);
}

TEST_F(StrcpyTest, ReturnsDestination)
{
	char *ret = ft_strcpy(dst, "test");

	EXPECT_EQ(dst, ret);
}

TEST_F(StrcpyTest, NullDstReturnsNull)
{
	char *ret = ft_strcpy(NULL, "test");

	EXPECT_EQ(nullptr, ret);
}

TEST_F(StrcpyTest, NullSrcReturnsNull)
{
	char *ret = ft_strcpy(dst, NULL);

	EXPECT_EQ(nullptr, ret);
}

TEST_F(StrcpyTest, LongerString)
{
	const char *src = "The quick brown fox";

	ft_strcpy(dst, src);

	EXPECT_STREQ(src, dst);
}
