// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_printf.cpp
 * @brief Unit tests for the kernel printf / format subsystem
 *
 * Stubs write_redirectable() so all output is captured into a local
 * buffer, then exercises char_format, str_format, int_format,
 * base_number_format, ptr_format, formats, and printf.
 */

#include <gtest/gtest.h>
#include <cstring>
#include <climits>

/* ------------------------------------------------------------------ */
/*  Capture buffer & stubs                                            */
/* ------------------------------------------------------------------ */

static char  g_buf[4096];
static int   g_pos;

static void reset_capture(void)
{
	memset(g_buf, 0, sizeof(g_buf));
	g_pos = 0;
}

/*
 * Rename the kernel 'write' and 'printf' symbols while pulling in
 * the implementation files so they don't collide with POSIX write()
 * (from unistd.h, included by gtest) or trigger g++ built-in printf
 * format checking.
 */
#define write  kfs_write
#define printf kfs_printf

extern "C" {

/*
 * Stub write_redirectable – captures bytes into g_buf instead of
 * touching video memory or the terminal subsystem.
 */
int write_redirectable(const char *text, unsigned int count)
{
	for (unsigned int i = 0;
	     i < count && g_pos < (int)sizeof(g_buf) - 1; i++)
		g_buf[g_pos++] = text[i];
	g_buf[g_pos] = '\0';
	return (int)count;
}

/*
 * Stub set_global_writer – not exercised in format tests but the
 * symbol is required by print.h.
 */
int set_global_writer(int (*fn)(const char *, unsigned int))
{
	(void)fn;
	return 0;
}

/*
 * Pull in the actual implementations under test.
 * formats.c provides char_format, str_format, int_format, etc.
 * printf.c  provides printf (and the static 'checking' helper).
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#include <kernel/print/formats.c>
#include <kernel/print/printf.c>
#pragma GCC diagnostic pop

} /* extern "C" */

#undef write
#undef printf

/* ------------------------------------------------------------------ */
/*  Test fixture                                                      */
/* ------------------------------------------------------------------ */

class PrintfTest : public ::testing::Test {
protected:
	void SetUp() override
	{
		reset_capture();
	}
};

/* ------------------------------------------------------------------ */
/*  char_format tests                                                 */
/* ------------------------------------------------------------------ */

TEST_F(PrintfTest, CharFormatSingleChar)
{
	int count = 0;
	int ret = char_format('A', &count);

	EXPECT_EQ(ret, 1);
	EXPECT_EQ(count, 1);
	EXPECT_STREQ(g_buf, "A");
}

TEST_F(PrintfTest, CharFormatAccumulatesCount)
{
	int count = 0;

	char_format('H', &count);
	char_format('i', &count);

	EXPECT_EQ(count, 2);
	EXPECT_STREQ(g_buf, "Hi");
}

TEST_F(PrintfTest, CharFormatSpecialChars)
{
	int count = 0;

	char_format('\n', &count);
	char_format('\t', &count);

	EXPECT_EQ(count, 2);
	EXPECT_EQ(g_buf[0], '\n');
	EXPECT_EQ(g_buf[1], '\t');
}

/* ------------------------------------------------------------------ */
/*  str_format tests                                                  */
/* ------------------------------------------------------------------ */

TEST_F(PrintfTest, StrFormatNormalString)
{
	int count = 0;
	int ret = str_format("hello", &count);

	EXPECT_EQ(ret, 5);
	EXPECT_STREQ(g_buf, "hello");
}

TEST_F(PrintfTest, StrFormatNullPrintsNill)
{
	int count = 0;
	int ret = str_format(NULL, &count);

	EXPECT_EQ(ret, 6);
	EXPECT_STREQ(g_buf, "(nill)");
}

TEST_F(PrintfTest, StrFormatEmptyString)
{
	int count = 0;
	int ret = str_format("", &count);

	EXPECT_EQ(ret, 0);
	EXPECT_STREQ(g_buf, "");
}

TEST_F(PrintfTest, StrFormatAccumulatesCount)
{
	int count = 0;

	str_format("ab", &count);
	str_format("cd", &count);

	EXPECT_EQ(count, 4);
	EXPECT_STREQ(g_buf, "abcd");
}

/* ------------------------------------------------------------------ */
/*  int_format tests                                                  */
/* ------------------------------------------------------------------ */

TEST_F(PrintfTest, IntFormatPositive)
{
	int count = 0;

	int_format(42, &count);

	EXPECT_STREQ(g_buf, "42");
	EXPECT_EQ(count, 2);
}

TEST_F(PrintfTest, IntFormatZero)
{
	int count = 0;

	int_format(0, &count);

	EXPECT_STREQ(g_buf, "0");
	EXPECT_EQ(count, 1);
}

TEST_F(PrintfTest, IntFormatNegative)
{
	int count = 0;

	int_format(-123, &count);

	EXPECT_STREQ(g_buf, "-123");
	EXPECT_EQ(count, 4);
}

TEST_F(PrintfTest, IntFormatIntMin)
{
	int count = 0;

	int_format(INT_MIN, &count);

	EXPECT_STREQ(g_buf, "-2147483648");
	EXPECT_EQ(count, 11);
}

TEST_F(PrintfTest, IntFormatIntMax)
{
	int count = 0;

	int_format(INT_MAX, &count);

	EXPECT_STREQ(g_buf, "2147483647");
	EXPECT_EQ(count, 10);
}


/* ------------------------------------------------------------------ */
/*  base_number_format tests                                          */
/* ------------------------------------------------------------------ */

TEST_F(PrintfTest, BaseNumberDecimal)
{
	int count = 0;

	base_number_format(255, &count, 0, 10);

	EXPECT_STREQ(g_buf, "255");
}

TEST_F(PrintfTest, BaseNumberHexLower)
{
	int count = 0;

	base_number_format(255, &count, 1, 16);

	EXPECT_STREQ(g_buf, "ff");
}

TEST_F(PrintfTest, BaseNumberHexUpper)
{
	int count = 0;

	base_number_format(255, &count, 2, 16);

	EXPECT_STREQ(g_buf, "FF");
}

TEST_F(PrintfTest, BaseNumberZero)
{
	int count = 0;

	base_number_format(0, &count, 0, 10);

	EXPECT_STREQ(g_buf, "0");
	EXPECT_EQ(count, 1);
}

TEST_F(PrintfTest, BaseNumberLargeHex)
{
	int count = 0;

	base_number_format(0xDEADBEEF, &count, 1, 16);

	EXPECT_STREQ(g_buf, "deadbeef");
}

/* ------------------------------------------------------------------ */
/*  ptr_format tests                                                  */
/* ------------------------------------------------------------------ */

TEST_F(PrintfTest, PtrFormatNonZero)
{
	int count = 0;

	ptr_format(0x1234, &count);

	EXPECT_STREQ(g_buf, "0x1234");
}

TEST_F(PrintfTest, PtrFormatZero)
{
	int count = 0;

	ptr_format(0, &count);

	EXPECT_STREQ(g_buf, "0x0");
}

/* ------------------------------------------------------------------ */
/*  formats() dispatch tests                                          */
/* ------------------------------------------------------------------ */

TEST_F(PrintfTest, FormatsPercent)
{
	int count = 0;
	va_list dummy;

	formats(&dummy, '%', &count);

	EXPECT_STREQ(g_buf, "%");
	EXPECT_EQ(count, 1);
}

/* Helper: call formats() with a single typed argument via a wrapper
 * so we can safely build a va_list from known arguments. */
static int call_printf_char(char c)
{
	reset_capture();
	return kfs_printf("%c", c);
}

static int call_printf_str(const char *s)
{
	reset_capture();
	return kfs_printf("%s", s);
}

static int call_printf_int(int n)
{
	reset_capture();
	return kfs_printf("%d", n);
}

static int call_printf_uint(unsigned int n)
{
	reset_capture();
	return kfs_printf("%u", n);
}

static int call_printf_hex(unsigned int n)
{
	reset_capture();
	return kfs_printf("%x", n);
}

static int call_printf_hex_upper(unsigned int n)
{
	reset_capture();
	return kfs_printf("%X", n);
}

static int call_printf_ptr(unsigned long n)
{
	reset_capture();
	return kfs_printf("%p", n);
}

TEST_F(PrintfTest, FormatsChar)
{
	int ret = call_printf_char('Z');

	EXPECT_EQ(ret, 1);
	EXPECT_STREQ(g_buf, "Z");
}

TEST_F(PrintfTest, FormatsString)
{
	int ret = call_printf_str("world");

	EXPECT_EQ(ret, 5);
	EXPECT_STREQ(g_buf, "world");
}

TEST_F(PrintfTest, FormatsStringNull)
{
	int ret = call_printf_str(NULL);

	EXPECT_EQ(ret, 6);
	EXPECT_STREQ(g_buf, "(nill)");
}

TEST_F(PrintfTest, FormatsDecimal)
{
	int ret = call_printf_int(-99);

	EXPECT_EQ(ret, 3);
	EXPECT_STREQ(g_buf, "-99");
}

TEST_F(PrintfTest, FormatsUnsigned)
{
	int ret = call_printf_uint(4294967295U);

	EXPECT_EQ(ret, 10);
	EXPECT_STREQ(g_buf, "4294967295");
}

TEST_F(PrintfTest, FormatsHexLower)
{
	int ret = call_printf_hex(0xABC);

	EXPECT_EQ(ret, 3);
	EXPECT_STREQ(g_buf, "abc");
}

TEST_F(PrintfTest, FormatsHexUpper)
{
	int ret = call_printf_hex_upper(0xABC);

	EXPECT_EQ(ret, 3);
	EXPECT_STREQ(g_buf, "ABC");
}

TEST_F(PrintfTest, FormatsPointer)
{
	int ret = call_printf_ptr(0xCAFE);

	EXPECT_EQ(ret, 6);
	EXPECT_STREQ(g_buf, "0xcafe");
}

/* ------------------------------------------------------------------ */
/*  printf tests                                                      */
/* ------------------------------------------------------------------ */

TEST_F(PrintfTest, PrintfPlainString)
{
	int ret = kfs_printf("hello");

	EXPECT_EQ(ret, 5);
	EXPECT_STREQ(g_buf, "hello");
}

TEST_F(PrintfTest, PrintfEmptyString)
{
	int ret = kfs_printf("");

	EXPECT_EQ(ret, 0);
	EXPECT_STREQ(g_buf, "");
}

TEST_F(PrintfTest, PrintfPercentLiteral)
{
	int ret = kfs_printf("100%%");

	EXPECT_EQ(ret, 4);
	EXPECT_STREQ(g_buf, "100%");
}

TEST_F(PrintfTest, PrintfMixedSpecifiers)
{
	int ret = kfs_printf("%s is %d", "age", 25);

	EXPECT_EQ(ret, 9);
	EXPECT_STREQ(g_buf, "age is 25");
}

TEST_F(PrintfTest, PrintfMultipleStrings)
{
	int ret = kfs_printf("%s %s", "hello", "world");

	EXPECT_EQ(ret, 11);
	EXPECT_STREQ(g_buf, "hello world");
}

TEST_F(PrintfTest, PrintfHexAndDecimal)
{
	int ret = kfs_printf("%d=0x%x", 255, 255);

	EXPECT_EQ(ret, 8);
	EXPECT_STREQ(g_buf, "255=0xff");
}

TEST_F(PrintfTest, PrintfCharSpecifier)
{
	int ret = kfs_printf("[%c]", 'X');

	EXPECT_EQ(ret, 3);
	EXPECT_STREQ(g_buf, "[X]");
}

TEST_F(PrintfTest, PrintfISpecifierSameAsD)
{
	int ret = kfs_printf("%i", -7);

	EXPECT_EQ(ret, 2);
	EXPECT_STREQ(g_buf, "-7");
}

TEST_F(PrintfTest, PrintfPointerSpecifier)
{
	int ret = kfs_printf("addr=%p", (unsigned long)0x42);

	EXPECT_EQ(ret, 9);
	EXPECT_STREQ(g_buf, "addr=0x42");
}

TEST_F(PrintfTest, PrintfAllSpecifiersTogether)
{
	int ret = kfs_printf("%c %s %d %u %x %X %p %%",
			     'A', "hi", -1, 10u, 16u, 16u,
			     (unsigned long)0xff);

	/* A hi -1 10 10 10 0xff % */
	EXPECT_STREQ(g_buf, "A hi -1 10 10 10 0xff %");
	EXPECT_EQ(ret, (int)strlen("A hi -1 10 10 10 0xff %"));
}

TEST_F(PrintfTest, PrintfReturnsCorrectCountForLongOutput)
{
	int ret = kfs_printf("%d%d%d", 111, 222, 333);

	EXPECT_EQ(ret, 9);
	EXPECT_STREQ(g_buf, "111222333");
}
