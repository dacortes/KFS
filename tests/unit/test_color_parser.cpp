// SPDX-License-Identifier: GPL-2.0

#include <gtest/gtest.h>
#include <kernel/terminal/color_parser.h>
#include <kernel/terminal/terminal.h> /* for DEVICE_BUFFER_SIZE */
#include <kernel/display/color.h>

class ColorParserTest : public ::testing::Test {
protected:
    color_parser_t parser;

    void SetUp() override
    {
        color_parser_init(&parser);
    }
};

TEST_F(ColorParserTest, InitDefaults)
{
    EXPECT_EQ(parser.state, COLOR_STATE_NORMAL);
    EXPECT_EQ(parser.last_foreground, WHITE_ON_BLACK);
    EXPECT_EQ(parser.last_background, BLACK_ON_BLACK);
}

TEST_F(ColorParserTest, StripPlainStringLeavesUnchanged)
{
    char out[DEVICE_BUFFER_SIZE];
    const char *input = "no escape";
    parser.parser_strip(&parser, input, out, sizeof(out));
    EXPECT_STREQ(out, input);
    EXPECT_EQ(color_parser_get_color(&parser), WHITE_ON_BLACK);
}

TEST_F(ColorParserTest, ForegroundColorIsApplied)
{
    char out[DEVICE_BUFFER_SIZE];
    parser.parser_strip(&parser, "\033[31mHello", out, sizeof(out));
    EXPECT_STREQ(out, "Hello");
    EXPECT_EQ(color_parser_get_color(&parser), RED_ON_BLACK);
}

TEST_F(ColorParserTest, BackgroundColorIsApplied)
{
    char out[DEVICE_BUFFER_SIZE];
    parser.parser_strip(&parser, "\033[42mX", out, sizeof(out));
    EXPECT_STREQ(out, "X");
    /* background code 42 maps to WHITE_ON_GREEN; combine with default
       foreground which is WHITE_ON_BLACK low nibble. */
    uint8_t expected = (WHITE_ON_BLACK & 0x0F) | (WHITE_ON_GREEN & 0xF0);
    EXPECT_EQ(color_parser_get_color(&parser), expected);
}

TEST_F(ColorParserTest, ForegroundAndBackgroundBoth)
{
    char out[DEVICE_BUFFER_SIZE];
    /* 31 = red fg, 46 = cyan bg */
    parser.parser_strip(&parser, "\033[31;46mZ", out, sizeof(out));
    EXPECT_STREQ(out, "Z");
    uint8_t combined = (RED_ON_BLACK & 0x0F) | (WHITE_ON_CYAN & 0xF0);
    EXPECT_EQ(color_parser_get_color(&parser), combined);
}

TEST_F(ColorParserTest, ResetReturnsDefault)
{
    char out[DEVICE_BUFFER_SIZE];
    parser.parser_strip(&parser, "\033[31mA", out, sizeof(out));
    EXPECT_NE(parser.last_foreground, WHITE_ON_BLACK);
    parser.parser_reset(&parser);
    EXPECT_EQ(parser.last_foreground, WHITE_ON_BLACK);
    EXPECT_EQ(parser.last_background, BLACK_ON_BLACK);
}

TEST_F(ColorParserTest, AnsiResetCodeZeroResetsForegroundAndBackground)
{
    char out[DEVICE_BUFFER_SIZE];

    parser.parser_strip(&parser, "\033[31;46mA", out, sizeof(out));
    EXPECT_NE(color_parser_get_color(&parser), WHITE_ON_BLACK);

    parser.parser_strip(&parser, "\033[0mB", out, sizeof(out));
    EXPECT_STREQ(out, "B");
    EXPECT_EQ(color_parser_get_color(&parser), WHITE_ON_BLACK);
}

TEST_F(ColorParserTest, EmptyAnsiResetCodeResetsForegroundAndBackground)
{
    char out[DEVICE_BUFFER_SIZE];

    parser.parser_strip(&parser, "\033[31;46mA", out, sizeof(out));
    EXPECT_NE(color_parser_get_color(&parser), WHITE_ON_BLACK);

    parser.parser_strip(&parser, "\033[mB", out, sizeof(out));
    EXPECT_STREQ(out, "B");
    EXPECT_EQ(color_parser_get_color(&parser), WHITE_ON_BLACK);
}
