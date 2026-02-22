// SPDX-License-Identifier: GPL-2.0

#include <gtest/gtest.h>
#include <kernel/display/display.h>
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

TEST_F(DisplayTest, PutAtTopLeft)
{
	display.put_at(&display, 'A', 0, 0);

	/* Verify character written at position (0,0) */
	EXPECT_EQ('A', video_memory[0]);
	EXPECT_EQ(WHITE_ON_BLACK, video_memory[1]);
}

TEST_F(DisplayTest, PutAtCustomPosition)
{
	/* Write 'X' at column 5, row 3 */
	display.put_at(&display, 'X', 5, 3);

	/* Calculate expected offset: (3 * 80 + 5) * 2 = 490 */
	unsigned int offset = (3 * 80 + 5) * 2;
	EXPECT_EQ('X', video_memory[offset]);
	EXPECT_EQ(WHITE_ON_BLACK, video_memory[offset + 1]);
}

TEST_F(DisplayTest, PutAtUsesDisplayColor)
{
	display.color = 0x4E; /* Yellow on red */
	
	display.put_at(&display, 'B', 10, 2);

	/* Calculate offset: (2 * 80 + 10) * 2 = 340 */
	unsigned int offset = (2 * 80 + 10) * 2;
	EXPECT_EQ('B', video_memory[offset]);
	EXPECT_EQ(0x4E, video_memory[offset + 1]);
}

TEST_F(DisplayTest, PutAtMultipleCharacters)
{
	display.put_at(&display, 'H', 0, 0);
	display.put_at(&display, 'i', 1, 0);
	display.put_at(&display, '!', 2, 0);

	EXPECT_EQ('H', video_memory[0]);
	EXPECT_EQ('i', video_memory[2]);
	EXPECT_EQ('!', video_memory[4]);
}

TEST_F(DisplayTest, PutAtOutOfBoundsX)
{
	/* Fill with known value */
	memset(video_memory, 'Z', sizeof(video_memory));
	
	/* Try to write beyond width */
	display.put_at(&display, 'A', 80, 0);

	/* Verify nothing was written - memory should still be 'Z' */
	int all_z = 1;
	for (int i = 0; i < 100; i++) {
		if (video_memory[i] != 'Z') {
			all_z = 0;
			break;
		}
	}
	EXPECT_EQ(1, all_z);
}

TEST_F(DisplayTest, PutAtOutOfBoundsY)
{
	/* Fill with known value */
	memset(video_memory, 'Z', sizeof(video_memory));
	
	/* Try to write beyond height */
	display.put_at(&display, 'A', 0, 25);

	/* Verify nothing was written - memory should still be 'Z' */
	int all_z = 1;
	for (int i = 0; i < 100; i++) {
		if (video_memory[i] != 'Z') {
			all_z = 0;
			break;
		}
	}
	EXPECT_EQ(1, all_z);
}

TEST_F(DisplayTest, PutAtBottomRight)
{
	/* Write to last valid position (79, 24) */
	display.put_at(&display, '$', 79, 24);

	/* Calculate offset: (24 * 80 + 79) * 2 = 3998 */
	unsigned int offset = (24 * 80 + 79) * 2;
	EXPECT_EQ('$', video_memory[offset]);
	EXPECT_EQ(WHITE_ON_BLACK, video_memory[offset + 1]);
}

TEST_F(DisplayTest, ClearAfterPutAt)
{
	display.put_at(&display, 'T', 0, 0);
	display.put_at(&display, 'e', 1, 0);
	display.put_at(&display, 's', 2, 0);
	display.put_at(&display, 't', 3, 0);
	display.clear(&display);

	/* Verify written text is cleared */
	for (int i = 0; i < 8; i += 2) {
		EXPECT_EQ(' ', video_memory[i]);
	}
}
