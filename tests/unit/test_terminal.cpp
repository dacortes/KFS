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
		terminal_init(&term, &display, 0);
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
	EXPECT_EQ(term.cursor_x, TERMINAL_PREFIX_LEN);
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
	EXPECT_NE(term.write_prefix, nullptr);
}

/* ------------------------------------------------------------------ */
/*                     Cursor color tests                             */
/* ------------------------------------------------------------------ */


TEST_F(TerminalTest, SetCursorColorChangesAttribute)
{
	term.set_cursor_color(&term, BLACK_ON_WHITE);
	EXPECT_EQ(attr_at(TERMINAL_PREFIX_LEN, 0), BLACK_ON_WHITE);

	term.set_cursor_color(&term, WHITE_ON_BLACK);
	EXPECT_EQ(attr_at(TERMINAL_PREFIX_LEN, 0), WHITE_ON_BLACK);
}

TEST_F(TerminalTest, WriteStringWithEscapeSetsColor)
{
	/* verify that an ANSI foreground sequence is stripped and color applied */
	term.clear(&term);
	term.curr_color = WHITE_ON_BLACK;
	term.write_string(&term, "\033[31mA");
	EXPECT_EQ(term.curr_color, RED_ON_BLACK);
	EXPECT_EQ(attr_at(TERMINAL_PREFIX_LEN, 0), RED_ON_BLACK);
}

/* ------------------------------------------------------------------ */
/*                     Character input tests                          */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, PrintableCharWritesToDisplay)
{
	term.handle_keyboard_input(&term, 'A');

	EXPECT_EQ(char_at(TERMINAL_PREFIX_LEN, 0), 'A');
	EXPECT_EQ(term.line[0], 'A');
	EXPECT_EQ(term.line_pos, 1u);
	EXPECT_EQ(term.line_len, 1u);
}

TEST_F(TerminalTest, CursorAdvancesAfterChar)
{
	term.handle_keyboard_input(&term, 'A');

	EXPECT_EQ(term.cursor_x, TERMINAL_PREFIX_LEN + 1);
	EXPECT_EQ(term.cursor_y, 0);
	EXPECT_EQ(attr_at(TERMINAL_PREFIX_LEN + 1, 0), BLACK_ON_WHITE);
}

TEST_F(TerminalTest, PreviousCellRestoredAfterChar)
{
	term.handle_keyboard_input(&term, 'A');

	/* Position where A was typed should be normal text color */
	EXPECT_EQ(attr_at(TERMINAL_PREFIX_LEN, 0), WHITE_ON_BLACK);
}

TEST_F(TerminalTest, MultipleCharsAdvanceCursor)
{
	term.handle_keyboard_input(&term, 'H');
	term.handle_keyboard_input(&term, 'i');

	EXPECT_EQ(char_at(TERMINAL_PREFIX_LEN, 0), 'H');
	EXPECT_EQ(char_at(TERMINAL_PREFIX_LEN + 1, 0), 'i');
	EXPECT_EQ(term.cursor_x, TERMINAL_PREFIX_LEN + 2);
	EXPECT_EQ(term.line_len, 2u);
	EXPECT_EQ(attr_at(TERMINAL_PREFIX_LEN + 2, 0), BLACK_ON_WHITE);
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
	EXPECT_EQ(term.cursor_x, TERMINAL_PREFIX_LEN);
	EXPECT_EQ(term.line[0], '\0');
}

TEST_F(TerminalTest, BackspaceAtStartDoesNothing)
{
	term.handle_keyboard_input(&term, '\b');

	EXPECT_EQ(term.cursor_x, TERMINAL_PREFIX_LEN);
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

TEST_F(TerminalTest, NewlineMovesToNextLineWithPrefix)
{
	const char *pfx = TERMINAL_PREFIX;

	term.handle_keyboard_input(&term, 'A');
	term.handle_keyboard_input(&term, 'B');
	term.handle_keyboard_input(&term, '\n');

	/* After newline, row 1 should have the prefix */
	for (int i = 0; pfx[i]; i++)
		EXPECT_EQ(char_at(i, 1), pfx[i]);
	EXPECT_EQ(term.cursor_x, TERMINAL_PREFIX_LEN);
	EXPECT_EQ(term.cursor_y, 1);
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
	EXPECT_EQ(term.cursor_x, TERMINAL_PREFIX_LEN + 1);
}

TEST_F(TerminalTest, MoveLeftAtStartDoesNothing)
{
	term.move_cursor(&term, CURSOR_LEFT);

	EXPECT_EQ(term.line_pos, 0u);
	EXPECT_EQ(term.cursor_x, TERMINAL_PREFIX_LEN);
}

TEST_F(TerminalTest, MoveRightIncrementsPosition)
{
	term.handle_keyboard_input(&term, 'A');
	term.handle_keyboard_input(&term, 'B');

	term.move_cursor(&term, CURSOR_LEFT);
	term.move_cursor(&term, CURSOR_LEFT);
	term.move_cursor(&term, CURSOR_RIGHT);

	EXPECT_EQ(term.line_pos, 1u);
	EXPECT_EQ(term.cursor_x, TERMINAL_PREFIX_LEN + 1);
}

TEST_F(TerminalTest, MoveRightBeyondLineLenDoesNothing)
{
	term.handle_keyboard_input(&term, 'A');

	/* Cursor is already at line_len (1), move right should not go further */
	term.move_cursor(&term, CURSOR_RIGHT);

	EXPECT_EQ(term.line_pos, 1u);
	EXPECT_EQ(term.cursor_x, TERMINAL_PREFIX_LEN + 1);
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

	/* Cursor should be at (PFX+2,0) with BLACK_ON_WHITE */
	EXPECT_EQ(attr_at(TERMINAL_PREFIX_LEN + 2, 0), BLACK_ON_WHITE);

	term.move_cursor(&term, CURSOR_LEFT);

	/* Old position restored, new position highlighted */
	EXPECT_EQ(attr_at(TERMINAL_PREFIX_LEN + 2, 0), WHITE_ON_BLACK);
	EXPECT_EQ(attr_at(TERMINAL_PREFIX_LEN + 1, 0), BLACK_ON_WHITE);
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

	EXPECT_EQ(char_at(TERMINAL_PREFIX_LEN, 0), 'A');
	EXPECT_EQ(char_at(TERMINAL_PREFIX_LEN + 1, 0), 'B');
	EXPECT_EQ(char_at(TERMINAL_PREFIX_LEN + 2, 0), 'C');
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

	EXPECT_EQ(char_at(TERMINAL_PREFIX_LEN, 0), 'H');
	EXPECT_EQ(char_at(TERMINAL_PREFIX_LEN + 1, 0), 'i');
	EXPECT_EQ(term.cursor_x, TERMINAL_PREFIX_LEN + 2);
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

	/* 7 prefix + 80 chars = 87, wraps at 80: 87 % 80 = 7 */
	EXPECT_EQ(term.cursor_x, TERMINAL_PREFIX_LEN);
	EXPECT_EQ(term.cursor_y, 1);
}

/* ------------------------------------------------------------------ */
/*                     Scroll initialization tests                    */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, InitAssignsScrollPointer)
{
	EXPECT_NE(term.scroll, nullptr);
}


/* ------------------------------------------------------------------ */
/*                     Scroll no-op tests                             */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, ScrollUpDoesNothingWhenScreenNotFull)
{
	/* Write some chars but don't fill the screen */
	term.handle_keyboard_input(&term, 'A');

	term.scroll(&term, 1);

	EXPECT_EQ(term.view_offset, 0);
}

TEST_F(TerminalTest, ScrollDownDoesNothingAtBottom)
{
	term.scroll(&term, -1);

	EXPECT_EQ(term.view_offset, 0);
}

TEST_F(TerminalTest, ScrollNullSelfDoesNotCrash)
{
	term.scroll(NULL, 1);
	term.scroll(NULL, -1);
}

/* ------------------------------------------------------------------ */
/*                     Scroll content tests                           */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, ScrollUpShowsOlderContent)
{
	/* Write 'Z' on the first visible row after prefix */
	term.write_char(&term, 'Z');

	/* Push it off screen */
	for (int i = 0; i < 25; i++)
		term.write_char(&term, '\n');

	/* Scroll up to see the old row */
	term.scroll(&term, 1);

	EXPECT_EQ(term.view_offset, 1);
	/* The old row with 'Z' should be visible at prefix column */
	EXPECT_EQ(char_at(TERMINAL_PREFIX_LEN, 0), 'Z');
}

TEST_F(TerminalTest, ScrollDownReturnsToLatest)
{
	term.write_char(&term, 'Z');
	for (int i = 0; i < 25; i++)
		term.write_char(&term, '\n');

	term.scroll(&term, 1);
	EXPECT_EQ(term.view_offset, 1);

	term.scroll(&term, -1);
	EXPECT_EQ(term.view_offset, 0);
}

TEST_F(TerminalTest, ScrollDownClampsAtZero)
{
	/* Create scrollback and scroll up first */
	for (int i = 0; i < 30; i++)
		term.write_char(&term, '\n');
	term.scroll(&term, 2);
	EXPECT_GT(term.view_offset, (uint16_t)0);

	/* Scroll down more than view_offset */
	term.scroll(&term, -100);

	EXPECT_EQ(term.view_offset, 0);
}

TEST_F(TerminalTest, ScrollDownRestoresCursorAtBottom)
{
	for (int i = 0; i < 30; i++)
		term.write_char(&term, '\n');

	term.scroll(&term, 1);
	term.scroll(&term, -1);

	/* Cursor highlight should be restored */
	EXPECT_EQ(attr_at(term.cursor_x, term.cursor_y), BLACK_ON_WHITE);
}

/* ------------------------------------------------------------------ */
/*                     Typing snaps scroll to bottom                  */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, TypingSnapsViewToBottom)
{
	/* Create scrollback */
	for (int i = 0; i < 30; i++)
		term.write_char(&term, '\n');

	term.scroll(&term, 2);
	EXPECT_GT(term.view_offset, (uint16_t)0);

	/* Type a character - should snap back */
	term.handle_keyboard_input(&term, 'A');

	EXPECT_EQ(term.view_offset, 0);
}

/* ------------------------------------------------------------------ */
/*                     Up/Down key dispatch tests                     */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, UpKeyTriggersScrollUp)
{
	/* Create scrollback */
	for (int i = 0; i < 30; i++)
		term.write_char(&term, '\n');

	term.handle_keyboard_input(&term, KEY_UP_PRESSED);

	EXPECT_EQ(term.view_offset, 1);
}

TEST_F(TerminalTest, DownKeyTriggersScrollDown)
{
	for (int i = 0; i < 30; i++)
		term.write_char(&term, '\n');

	term.handle_keyboard_input(&term, KEY_UP_PRESSED);
	EXPECT_EQ(term.view_offset, 1);

	term.handle_keyboard_input(&term, KEY_DOWN_PRESSED);
	EXPECT_EQ(term.view_offset, 0);
}

/* ------------------------------------------------------------------ */
/*                     Scrollback buffer wrapping test                */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, ScrollbackWrapsCircularBuffer)
{
	/* Push more than SCROLL_BUFFER_ROWS lines to force wrap */
	for (int i = 0; i < SCROLL_BUFFER_ROWS + 10; i++)
		term.write_char(&term, '\n');

	/* scroll_count should be clamped at SCROLL_BUFFER_ROWS */
	EXPECT_EQ(term.scroll_count, SCROLL_BUFFER_ROWS);

	/* Should still be able to scroll without crash */
	uint16_t max = term.scroll_count - display.height;

	term.scroll(&term, (int)max);
	EXPECT_EQ(term.view_offset, max);

	term.scroll(&term, -(int)max);
	EXPECT_EQ(term.view_offset, 0);
}

/* ------------------------------------------------------------------ */
/*                     Prefix tests                                   */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, PrefixAppearsAfterNewline)
{
	const char *pfx = TERMINAL_PREFIX;

	term.handle_keyboard_input(&term, 'A');
	term.handle_keyboard_input(&term, '\n');

	/* Newline moves to row 1, prefix should appear there */
	for (int i = 0; pfx[i]; i++)
		EXPECT_EQ(char_at(i, 1), pfx[i]);
	EXPECT_EQ(term.cursor_x, TERMINAL_PREFIX_LEN);
}

TEST_F(TerminalTest, PrefixSetDuringInit)
{
	EXPECT_EQ(term.prefix_len, TERMINAL_PREFIX_LEN);
	EXPECT_STREQ(term.prefix, TERMINAL_PREFIX);
}

/* ------------------------------------------------------------------ */
/*                     Render tests                                   */
/* ------------------------------------------------------------------ */


TEST_F(TerminalTest, RenderRestoresCursorAtBottom)
{
	term.handle_keyboard_input(&term, 'A');

	/* Wipe video memory */
	memset(video_memory, 0, sizeof(video_memory));

	term.render(&term);

	/* Cursor should be highlighted at current position */
	EXPECT_EQ(attr_at(term.cursor_x, term.cursor_y), BLACK_ON_WHITE);
}

TEST_F(TerminalTest, RenderDoesNotCrashWithNull)
{
	term.render(NULL);
}

TEST_F(TerminalTest, RenderShowsOlderContentWhenScrolled)
{
	/* Write 'Q' then push it off screen */
	term.write_char(&term, 'Q');
	for (int i = 0; i < 25; i++)
		term.write_char(&term, '\n');

	/* Scroll up to see 'Q' */
	term.set_offset(&term, 1);
	term.render(&term);

	EXPECT_EQ(char_at(TERMINAL_PREFIX_LEN, 0), 'Q');
}

TEST_F(TerminalTest, RenderDoesNotHighlightCursorWhenScrolled)
{
	/* Write content and push off screen */
	term.handle_keyboard_input(&term, 'A');
	for (int i = 0; i < 25; i++)
		term.write_char(&term, '\n');

	/* Scroll up — cursor highlight should NOT appear */
	term.set_offset(&term, 1);
	term.render(&term);

	/* Current cursor position should not be BLACK_ON_WHITE
	 * since we're viewing old content, not the latest
	 */
	EXPECT_NE(attr_at(term.cursor_x, term.cursor_y), BLACK_ON_WHITE);
}

/* ------------------------------------------------------------------ */
/*                     set_offset tests                               */
/* ------------------------------------------------------------------ */

TEST_F(TerminalTest, SetOffsetChangesViewOffset)
{
	term.set_offset(&term, 5);
	EXPECT_EQ(term.view_offset, 5);
}

TEST_F(TerminalTest, SetOffsetToZeroResetsView)
{
	term.set_offset(&term, 10);
	EXPECT_EQ(term.view_offset, 10);

	term.set_offset(&term, 0);
	EXPECT_EQ(term.view_offset, 0);
}

TEST_F(TerminalTest, InitAssignsRenderPointer)
{
	EXPECT_NE(term.render, nullptr);
}

TEST_F(TerminalTest, InitAssignsSetOffsetPointer)
{
	EXPECT_NE(term.set_offset, nullptr);
}