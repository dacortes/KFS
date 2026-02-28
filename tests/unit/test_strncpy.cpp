// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_strncpy.cpp
 * @brief Unit tests for ft_strncpy function
 */

#include <gtest/gtest.h>
#include <kernel/wrappers/helper.h>
#include <cstring>

class StrncpyTest : public ::testing::Test {
protected:
	char dest[100];
	char src[100];

	void SetUp() override
	{
		/* Clear buffers before each test */
		memset(dest, 'X', sizeof(dest));
		memset(src, 0, sizeof(src));
		dest[99] = '\0';
		src[99] = '\0';
	}
};

TEST_F(StrncpyTest, BasicCopy)
{
	strcpy(src, "Hello");
	
	char *result = ft_strncpy(dest, src, 10);

	EXPECT_STREQ("Hello", dest);
	EXPECT_EQ(dest, result);
}

TEST_F(StrncpyTest, CopyExactLength)
{
	strcpy(src, "Test");
	
	ft_strncpy(dest, src, 4);

	/* Should copy exactly 4 characters without null terminator */
	EXPECT_EQ('T', dest[0]);
	EXPECT_EQ('e', dest[1]);
	EXPECT_EQ('s', dest[2]);
	EXPECT_EQ('t', dest[3]);
}

TEST_F(StrncpyTest, CopyLessThanLength)
{
	strcpy(src, "Hi");
	memset(dest, 'Z', sizeof(dest));
	
	ft_strncpy(dest, src, 10);

	/* Should copy "Hi" and fill remaining with '\0' */
	EXPECT_EQ('H', dest[0]);
	EXPECT_EQ('i', dest[1]);
	EXPECT_EQ('\0', dest[2]);
	EXPECT_EQ('\0', dest[3]);
}

TEST_F(StrncpyTest, CopyMoreThanLength)
{
	strcpy(src, "This is a long string");
	
	ft_strncpy(dest, src, 10);

	/* Should copy only first 10 characters */
	EXPECT_EQ('T', dest[0]);
	EXPECT_EQ('h', dest[1]);
	EXPECT_EQ('i', dest[2]);
	EXPECT_EQ('s', dest[3]);
	EXPECT_EQ(' ', dest[4]);
	EXPECT_EQ('i', dest[5]);
	EXPECT_EQ('s', dest[6]);
	EXPECT_EQ(' ', dest[7]);
	EXPECT_EQ('a', dest[8]);
	EXPECT_EQ(' ', dest[9]);
}

TEST_F(StrncpyTest, EmptyString)
{
	strcpy(src, "");
	memset(dest, 'A', sizeof(dest));
	
	ft_strncpy(dest, src, 5);

	/* Should fill with null terminators */
	EXPECT_EQ('\0', dest[0]);
	EXPECT_EQ('\0', dest[1]);
	EXPECT_EQ('\0', dest[2]);
}

TEST_F(StrncpyTest, ZeroLength)
{
	strcpy(src, "Hello");
	dest[0] = 'X';
	
	ft_strncpy(dest, src, 0);

	/* Should not copy anything */
	EXPECT_EQ('X', dest[0]);
}

TEST_F(StrncpyTest, NullDstReturnsEmptyString)
{
	strcpy(src, "Test");
	
	char *result = ft_strncpy(NULL, src, 5);

	/* Should return empty string */
	EXPECT_EQ(NULL, result);
}

TEST_F(StrncpyTest, NullSrcReturnsEmptyString)
{
	char *result = ft_strncpy(dest, NULL, 5);

	/* Should return empty string */
	EXPECT_EQ(NULL, result);
}

TEST_F(StrncpyTest, BothNullReturnsEmptyString)
{
	char *result = ft_strncpy(NULL, NULL, 5);

	/* Should return empty string */
	EXPECT_EQ(NULL, result);
}

TEST_F(StrncpyTest, SingleCharacter)
{
	strcpy(src, "A");
	
	ft_strncpy(dest, src, 5);

	EXPECT_EQ('A', dest[0]);
	EXPECT_EQ('\0', dest[1]);
	EXPECT_EQ('\0', dest[2]);
}

TEST_F(StrncpyTest, ReturnsDestination)
{
	strcpy(src, "Return test");
	
	char *result = ft_strncpy(dest, src, 20);

	EXPECT_EQ(dest, result);
	EXPECT_STREQ("Return test", result);
}

TEST_F(StrncpyTest, OverwriteExistingData)
{
	strcpy(dest, "XXXXXXXXXX");
	strcpy(src, "New");
	
	ft_strncpy(dest, src, 10);

	/* Should overwrite with "New" and nulls */
	EXPECT_EQ('N', dest[0]);
	EXPECT_EQ('e', dest[1]);
	EXPECT_EQ('w', dest[2]);
	EXPECT_EQ('\0', dest[3]);
}

TEST_F(StrncpyTest, LargeString)
{
	/* Fill source with known pattern */
	for (int i = 0; i < 50; i++)
		src[i] = 'A' + (i % 26);
	src[50] = '\0';
	
	ft_strncpy(dest, src, 51);

	/* Verify pattern copied correctly */
	for (int i = 0; i < 50; i++)
		EXPECT_EQ('A' + (i % 26), dest[i]);
	EXPECT_EQ('\0', dest[50]);
}
