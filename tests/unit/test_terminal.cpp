// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_terminal.cpp
 * @brief Unit tests for virtual terminal subsystem
 */

#include <gtest/gtest.h>
#include <terminal.h>
#include <kernel/keyboard/keyboard.h>
#include <cstring>

class TerminalTest : public ::testing::Test {
protected:
	terminal_t term;
	display_t display;
	char video_memory[4000]; /* 80 * 25 * 2 */

	void SetUp() override
	{
		memset(video_memory, 0, sizeof(video_memory));
		display_init(&display);
		display.videomemptr = video_memory;
		terminal_init(&term, &display);
	}

	/**
	 * Get the VGA attribute byte at screen position (x, y)
	 */
	uint8_t attr_at(int x, int y)
	{
		int offset = (y * display.width + x) * display.char_size;

		return (uint8_t)video_memory[offset + 1];
	}

	/**
	 * Get the character byte at screen position (x, y)
	 */
	char char_at(int x, int y)
	{
		int offset = (y * display.width + x) * display.char_size;

		return video_memory[offset];
	}
};

/* ------------------------------------------------------------------ */
/*                     Initialization tests                           */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, InitSetsDefaults)
{
	EXPECT_EQ(term.cursor_x, 0);
	EXPECT_EQ(term.cursor_y, 0);
	EXPECT_EQ(term.line_pos, 0u);
	EXPECT_EQ(term.line_len, 0u);
	EXPECT_EQ(term.his_size, 0u);
	EXPECT_EQ(term.his_head, 0u);
	EXPECT_EQ(term.curr_color, WHITE_ON_BLACK);
}

TEST_F(TerminalTest, InitAssignsFunctionPointers)
{
	EXPECT_NE(term.write_char, nullptr);
	EXPECT_NE(term.write_string, nullptr);
	EXPECT_NE(term.clear, nullptr);
	EXPECT_NE(term.handle_keyboard_input, nullptr);
	EXPECT_NE(term.save_history, nullptr);
	EXPECT_NE(term.set_cursor_color, nullptr);
	EXPECT_NE(term.move_cursor, nullptr);
}

/* ------------------------------------------------------------------ */
/*                     Cursor color tests                             */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, ClearShowsCursorAtOrigin)
{
	term.clear(&term);

	EXPECT_EQ(attr_at(0, 0), BLACK_ON_WHITE);
}

TEST_F(TerminalTest, SetCursorColorChangesAttribute)
{
	term.set_cursor_color(&term, BLACK_ON_WHITE);
	EXPECT_EQ(attr_at(0, 0), BLACK_ON_WHITE);

	term.set_cursor_color(&term, WHITE_ON_BLACK);
	EXPECT_EQ(attr_at(0, 0), WHITE_ON_BLACK);
}

TEST_F(TerminalTest, CursorAppearsAfterInit)
{
	/* clear is called after init in kernel_main, cursor must show */
	term.clear(&term);
	EXPECT_EQ(attr_at(0, 0), BLACK_ON_WHITE);
}

/* ------------------------------------------------------------------ */
/*                     Character input tests                          */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, PrintableCharWritesToDisplay)
{
	term.handle_keyboard_input(&term, 'A');

	EXPECT_EQ(char_at(0, 0), 'A');
	EXPECT_EQ(term.line[0], 'A');
	EXPECT_EQ(term.line_pos, 1u);
	EXPECT_EQ(term.line_len, 1u);
}

TEST_F(TerminalTest, CursorAdvancesAfterChar)
{
	term.handle_keyboard_input(&term, 'A');

	EXPECT_EQ(term.cursor_x, 1);
	EXPECT_EQ(term.cursor_y, 0);
	EXPECT_EQ(attr_at(1, 0), BLACK_ON_WHITE);
}

TEST_F(TerminalTest, PreviousCellRestoredAfterChar)
{
	term.handle_keyboard_input(&term, 'A');

	/* Position (0,0) should be normal text color, not cursor */
	EXPECT_EQ(attr_at(0, 0), WHITE_ON_BLACK);
}

TEST_F(TerminalTest, MultipleCharsAdvanceCursor)
{
	term.handle_keyboard_input(&term, 'H');
	term.handle_keyboard_input(&term, 'i');

	EXPECT_EQ(char_at(0, 0), 'H');
	EXPECT_EQ(char_at(1, 0), 'i');
	EXPECT_EQ(term.cursor_x, 2);
	EXPECT_EQ(term.line_len, 2u);
	EXPECT_EQ(attr_at(2, 0), BLACK_ON_WHITE);
}

/* ------------------------------------------------------------------ */
/*                     Backspace tests                                */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, BackspaceDeletesLastChar)
{
	term.handle_keyboard_input(&term, 'A');
	term.handle_keyboard_input(&term, '\b');

	EXPECT_EQ(term.line_pos, 0u);
	EXPECT_EQ(term.line_len, 0u);
	EXPECT_EQ(term.cursor_x, 0);
	EXPECT_EQ(term.line[0], '\0');
}

TEST_F(TerminalTest, BackspaceAtStartDoesNothing)
{
	term.handle_keyboard_input(&term, '\b');

	EXPECT_EQ(term.cursor_x, 0);
	EXPECT_EQ(term.cursor_y, 0);
	EXPECT_EQ(term.line_pos, 0u);
}

TEST_F(TerminalTest, BackspaceMidLineShiftsChars)
{
	term.handle_keyboard_input(&term, 'A');
	term.handle_keyboard_input(&term, 'B');
	term.handle_keyboard_input(&term, 'C');

	/* Move cursor left to sit on 'C', then left again to 'B' */
	term.move_cursor(&term, CURSOR_LEFT);
	term.move_cursor(&term, CURSOR_LEFT);

	/* Cursor is now at position 1 (on 'B'), backspace deletes 'A' */
	term.handle_keyboard_input(&term, '\b');

	EXPECT_EQ(term.line_len, 2u);
	EXPECT_EQ(term.line[0], 'B');
	EXPECT_EQ(term.line[1], 'C');
}

/* ------------------------------------------------------------------ */
/*                     Newline / enter tests                          */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, NewlineResetsLineBuffer)
{
	term.handle_keyboard_input(&term, 'H');
	term.handle_keyboard_input(&term, 'i');
	term.handle_keyboard_input(&term, '\n');

	EXPECT_EQ(term.line_pos, 0u);
	EXPECT_EQ(term.line_len, 0u);
	EXPECT_EQ(term.line[0], '\0');
}

TEST_F(TerminalTest, NewlineEchoesGotPrefix)
{
	term.handle_keyboard_input(&term, 'A');
	term.handle_keyboard_input(&term, 'B');
	term.handle_keyboard_input(&term, '\n');

	/* After newline, cursor should be on row 2+
	 * Row 1 should contain "GOT: AB"
	 */
	EXPECT_EQ(char_at(0, 1), 'G');
	EXPECT_EQ(char_at(1, 1), 'O');
	EXPECT_EQ(char_at(2, 1), 'T');
	EXPECT_EQ(char_at(3, 1), ':');
	EXPECT_EQ(char_at(4, 1), ' ');
	EXPECT_EQ(char_at(5, 1), 'A');
	EXPECT_EQ(char_at(6, 1), 'B');
}

TEST_F(TerminalTest, NewlineSavesHistory)
{
	term.handle_keyboard_input(&term, 'X');
	term.handle_keyboard_input(&term, '\n');

	/* History entry 0 should contain "X" */
	EXPECT_EQ(term.history[0][0], 'X');
	EXPECT_EQ(term.history[0][1], '\0');
	EXPECT_EQ(term.his_size, 1u);
}

/* ------------------------------------------------------------------ */
/*                     Cursor movement tests                          */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, MoveLeftDecrementsPosition)
{
	term.handle_keyboard_input(&term, 'A');
	term.handle_keyboard_input(&term, 'B');

	term.move_cursor(&term, CURSOR_LEFT);

	EXPECT_EQ(term.line_pos, 1u);
	EXPECT_EQ(term.cursor_x, 1);
}

TEST_F(TerminalTest, MoveLeftAtStartDoesNothing)
{
	term.move_cursor(&term, CURSOR_LEFT);

	EXPECT_EQ(term.line_pos, 0u);
	EXPECT_EQ(term.cursor_x, 0);
}

TEST_F(TerminalTest, MoveRightIncrementsPosition)
{
	term.handle_keyboard_input(&term, 'A');
	term.handle_keyboard_input(&term, 'B');

	term.move_cursor(&term, CURSOR_LEFT);
	term.move_cursor(&term, CURSOR_LEFT);
	term.move_cursor(&term, CURSOR_RIGHT);

	EXPECT_EQ(term.line_pos, 1u);
	EXPECT_EQ(term.cursor_x, 1);
}

TEST_F(TerminalTest, MoveRightBeyondLineLenDoesNothing)
{
	term.handle_keyboard_input(&term, 'A');

	/* Cursor is already at line_len (1), move right should not go further */
	term.move_cursor(&term, CURSOR_RIGHT);

	EXPECT_EQ(term.line_pos, 1u);
	EXPECT_EQ(term.cursor_x, 1);
}

TEST_F(TerminalTest, ArrowKeysDispatchMoveCursor)
{
	term.handle_keyboard_input(&term, 'A');
	term.handle_keyboard_input(&term, 'B');

	term.handle_keyboard_input(&term, KEY_LEFT_PRESSED);
	EXPECT_EQ(term.line_pos, 1u);

	term.handle_keyboard_input(&term, KEY_RIGHT_PRESSED);
	EXPECT_EQ(term.line_pos, 2u);
}

TEST_F(TerminalTest, CursorColorFollowsMovement)
{
	term.handle_keyboard_input(&term, 'A');
	term.handle_keyboard_input(&term, 'B');

	/* Cursor should be at (2,0) with BLACK_ON_WHITE */
	EXPECT_EQ(attr_at(2, 0), BLACK_ON_WHITE);

	term.move_cursor(&term, CURSOR_LEFT);

	/* Old position (2,0) restored, new position (1,0) highlighted */
	EXPECT_EQ(attr_at(2, 0), WHITE_ON_BLACK);
	EXPECT_EQ(attr_at(1, 0), BLACK_ON_WHITE);
}

/* ------------------------------------------------------------------ */
/*                     Insert at cursor tests                         */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, InsertMidLineShiftsBuffer)
{
	term.handle_keyboard_input(&term, 'A');
	term.handle_keyboard_input(&term, 'C');

	/* Move left so cursor is between A and C */
	term.move_cursor(&term, CURSOR_LEFT);

	/* Insert B */
	term.handle_keyboard_input(&term, 'B');

	EXPECT_EQ(term.line[0], 'A');
	EXPECT_EQ(term.line[1], 'B');
	EXPECT_EQ(term.line[2], 'C');
	EXPECT_EQ(term.line_len, 3u);
}

TEST_F(TerminalTest, InsertMidLineUpdatesDisplay)
{
	term.handle_keyboard_input(&term, 'A');
	term.handle_keyboard_input(&term, 'C');
	term.move_cursor(&term, CURSOR_LEFT);
	term.handle_keyboard_input(&term, 'B');

	EXPECT_EQ(char_at(0, 0), 'A');
	EXPECT_EQ(char_at(1, 0), 'B');
	EXPECT_EQ(char_at(2, 0), 'C');
}

/* ------------------------------------------------------------------ */
/*                     History tests                                  */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, HistoryWrapsAround)
{
	for (int i = 0; i < TERMINAL_HISTORY_SIZE + 2; i++) {
		char c = 'A' + (i % 26);

		term.handle_keyboard_input(&term, c);
		term.handle_keyboard_input(&term, '\n');
	}

	EXPECT_EQ(term.his_size, (uint32_t)TERMINAL_HISTORY_SIZE);
}

/* ------------------------------------------------------------------ */
/*                     Write string tests                             */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, WriteStringOutputsChars)
{
	term.write_string(&term, "Hi");

	EXPECT_EQ(char_at(0, 0), 'H');
	EXPECT_EQ(char_at(1, 0), 'i');
	EXPECT_EQ(term.cursor_x, 2);
}

/* ------------------------------------------------------------------ */
/*                     Null pointer safety                            */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, NullSelfDoesNotCrash)
{
	/* Call via function pointers with NULL - should not crash */
	term.set_cursor_color(NULL, BLACK_ON_WHITE);
	term.move_cursor(NULL, CURSOR_LEFT);
	term.handle_keyboard_input(NULL, 'A');
}

/* ------------------------------------------------------------------ */
/*                     Line wrap tests                                */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, CursorWrapsAtDisplayWidth)
{
	for (int i = 0; i < 80; i++)
		term.handle_keyboard_input(&term, 'X');

	EXPECT_EQ(term.cursor_x, 0);
	EXPECT_EQ(term.cursor_y, 1);
}
