// SPDX-License-Identifier: GPL-2.0

#include <gtest/gtest.h>
#include <display/display.h>
#include <cstring>

class DisplayTest : public ::testing::Test {
protected:
	display_t display;
	char video_memory[4000]; /* 80 * 25 * 2 */

	void SetUp() override
	{
		/* Initialize test video memory buffer */
		memset(video_memory, 0, sizeof(video_memory));
		
		/* Initialize display struct with display_init */
		display_init(&display);
		
		/* Override videomemptr with test buffer */
		display.videomemptr = video_memory;
	}

	void TearDown() override
	{
		/* Cleanup after each test */
	}
};

TEST_F(DisplayTest, ClearFillsWithSpaces)
{
	/* Fill memory with non-space characters */
	for (int i = 0; i < 100; i += 2) {
		video_memory[i] = 'X';
		video_memory[i + 1] = 0xFF;
	}

	display.clear(&display);

	/* Check first few characters are spaces */
	for (int i = 0; i < 100; i += 2) {
		EXPECT_EQ(' ', video_memory[i]);
	}
}

TEST_F(DisplayTest, ClearSetsColorAttribute)
{
	display.clear(&display);

	/* Check color attributes are set correctly */
	for (int i = 1; i < 100; i += 2) {
		EXPECT_EQ(WHITE_ON_BLACK, video_memory[i]);
	}
}

TEST_F(DisplayTest, ClearFillsEntireDisplay)
{
	display.clear(&display);

	unsigned int total_size = display.width * display.height * display.char_size;
	
	/* Verify entire display is cleared */
	for (unsigned int i = 0; i < total_size; i += 2) {
		EXPECT_EQ(' ', video_memory[i]) << "Character at position " << i;
		EXPECT_EQ(WHITE_ON_BLACK, video_memory[i + 1]) << "Attribute at position " << (i + 1);
	}
}

TEST_F(DisplayTest, WriteStringSimpleText)
{
	const char *text = "Hello";

	display.write_string(&display, text);

	/* Verify characters are written */
	EXPECT_EQ('H', video_memory[0]);
	EXPECT_EQ('e', video_memory[2]);
	EXPECT_EQ('l', video_memory[4]);
	EXPECT_EQ('l', video_memory[6]);
	EXPECT_EQ('o', video_memory[8]);
}

TEST_F(DisplayTest, WriteStringSetColorAttributes)
{
	const char *text = "ABC";

	display.write_string(&display, text);

	/* Verify color attributes are set */
	EXPECT_EQ(WHITE_ON_BLACK, video_memory[1]);
	EXPECT_EQ(WHITE_ON_BLACK, video_memory[3]);
	EXPECT_EQ(WHITE_ON_BLACK, video_memory[5]);
}

TEST_F(DisplayTest, WriteStringEmptyString)
{
	/* Fill with known values */
	video_memory[0] = 'X';
	video_memory[1] = 0xFF;

	display.write_string(&display, "");

	/* Verify nothing was written */
	EXPECT_EQ('X', video_memory[0]);
	EXPECT_EQ((char)0xFF, video_memory[1]);
}

TEST_F(DisplayTest, WriteStringSingleCharacter)
{
	display.write_string(&display, "A");

	EXPECT_EQ('A', video_memory[0]);
	EXPECT_EQ(WHITE_ON_BLACK, video_memory[1]);
}

TEST_F(DisplayTest, WriteStringUsesDisplayColor)
{
	display.color = 0x4E; /* Yellow on red */
	
	display.write_string(&display, "Test");

	/* Verify custom color is used */
	EXPECT_EQ(0x4E, video_memory[1]);
	EXPECT_EQ(0x4E, video_memory[3]);
	EXPECT_EQ(0x4E, video_memory[5]);
	EXPECT_EQ(0x4E, video_memory[7]);
}

TEST_F(DisplayTest, WriteStringLongText)
{
	const char *text = "This is a longer test string";

	display.write_string(&display, text);

	/* Verify first and last characters */
	EXPECT_EQ('T', video_memory[0]);
	EXPECT_EQ('\0', video_memory[56]); /* Last char 'g' at position 28*2 */
	
	/* Verify specific word */
	EXPECT_EQ('l', video_memory[20]); /* 'l' in "longer" */
	EXPECT_EQ('o', video_memory[22]);
	EXPECT_EQ('n', video_memory[24]);
	EXPECT_EQ('g', video_memory[26]);
}

TEST_F(DisplayTest, ClearAfterWrite)
{
	display.write_string(&display, "Test");
	display.clear(&display);

	/* Verify written text is cleared */
	for (int i = 0; i < 8; i += 2) {
		EXPECT_EQ(' ', video_memory[i]);
	}
}
