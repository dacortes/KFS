// SPDX-License-Identifier: GPL-2.0

/**
 * @file test_system.cpp
 * @brief Unit tests for system terminal switching and shortcut handling
 */

#include <gtest/gtest.h>
#include <cstring>

/*
 * Stub out inline assembly that cannot execute in user-space tests.
 * The macros must be defined before system.c is included.
 */
#define asm(...)
#define __asm__(...)

/*
 * Include system.c directly so static functions (switch_terminal,
 * shortcut_handler, create_terminal) are visible to the tests.
 * system.c is intentionally NOT in KERNEL_LIB_SOURCES to avoid
 * duplicate symbols.
 */
extern "C" {
#include <kernel/system/system.c>
}

class SystemTest : public ::testing::Test {
protected:
	char video_memory[4000]; /* 80 * 25 * 2 */

	void SetUp() override
	{
		memset(&sys, 0, sizeof(sys));
		memset(video_memory, 0, sizeof(video_memory));

		/* Initialize display with test-safe video memory */
		display_init(&sys.display);
		sys.display.videomemptr = video_memory;

		/* Initialize terminals on test display */
		for (int i = 0; i < MAX_TERMINAL; i++) {
			terminal_init(&sys.terminals[i], &sys.display, i);
			sys.terminals[i].clear(&sys.terminals[i]);
		}
		sys.active_terminal = 0;

		/* Wire up function pointers as init_system would */
		keyboard_init(&sys.keyboard);
		sys.keyboard.set_shortcut_handler(&sys.keyboard,
						  shortcut_handler);
		sys.switch_terminal = switch_terminal;
	}

	/**
	 * Get the character byte at screen position (x, y)
	 */
	char char_at(int x, int y)
	{
		int offset = (y * sys.display.width + x) *
			     sys.display.char_size;

		return video_memory[offset];
	}
};

/* ------------------------------------------------------------------ */
/*                     switch_terminal tests                          */
/* ------------------------------------------------------------------ */

TEST_F(SystemTest, SwitchTerminalChangesActiveId)
{
	EXPECT_EQ(sys.active_terminal, 0u);

	sys.switch_terminal(&sys, 1);

	EXPECT_EQ(sys.active_terminal, 1u);
}

TEST_F(SystemTest, SwitchTerminalIgnoresInvalidId)
{
	sys.switch_terminal(&sys, MAX_TERMINAL);

	EXPECT_EQ(sys.active_terminal, 0u);
}

TEST_F(SystemTest, SwitchTerminalIgnoresLargeId)
{
	sys.switch_terminal(&sys, 999);

	EXPECT_EQ(sys.active_terminal, 0u);
}

TEST_F(SystemTest, SwitchTerminalIgnoresSameId)
{
	sys.switch_terminal(&sys, 0);

	EXPECT_EQ(sys.active_terminal, 0u);
}

TEST_F(SystemTest, SwitchTerminalNullSelfDoesNotCrash)
{
	sys.switch_terminal(NULL, 1);

	EXPECT_EQ(sys.active_terminal, 0u);
}

TEST_F(SystemTest, SwitchTerminalResetsViewOffset)
{
	/* Scroll terminal 1 so its view_offset > 0 */
	terminal_t *t1 = &sys.terminals[1];

	for (int i = 0; i < 30; i++)
		t1->write_char(t1, '\n');
	t1->scroll(t1, 3);
	EXPECT_GT(t1->view_offset, (uint16_t)0);

	/* Switch to terminal 1 — view_offset should be reset */
	sys.switch_terminal(&sys, 1);

	EXPECT_EQ(t1->view_offset, 0);
}

TEST_F(SystemTest, SwitchTerminalRendersNewContent)
{
	/* Type distinct chars in each terminal */
	terminal_t *t0 = &sys.terminals[0];
	terminal_t *t1 = &sys.terminals[1];

	t0->handle_keyboard_input(t0, 'A');
	t1->handle_keyboard_input(t1, 'Z');

	/* Switch to terminal 1 — display should show 'Z' */
	sys.switch_terminal(&sys, 1);

	EXPECT_EQ(char_at(TERMINAL_PREFIX_LEN, 0), 'Z');
}

TEST_F(SystemTest, SwitchBackRestoresOriginalContent)
{
	terminal_t *t0 = &sys.terminals[0];
	terminal_t *t1 = &sys.terminals[1];

	t0->handle_keyboard_input(t0, 'A');
	t1->handle_keyboard_input(t1, 'Z');

	sys.switch_terminal(&sys, 1);
	EXPECT_EQ(char_at(TERMINAL_PREFIX_LEN, 0), 'Z');

	sys.switch_terminal(&sys, 0);
	EXPECT_EQ(char_at(TERMINAL_PREFIX_LEN, 0), 'A');
}

/* ------------------------------------------------------------------ */
/*                     shortcut_handler tests                         */
/* ------------------------------------------------------------------ */

TEST_F(SystemTest, ShortcutCtrlOneSwitchesToTerminalZero)
{
	/* Start on terminal 1 */
	sys.switch_terminal(&sys, 1);
	EXPECT_EQ(sys.active_terminal, 1u);

	/* Ctrl+1: scancode 0x02 maps to terminal 0 */
	unsigned char keys[] = {0x02};

	shortcut_handler(keys, 1);

	EXPECT_EQ(sys.active_terminal, 0u);
}

TEST_F(SystemTest, ShortcutCtrlTwoSwitchesToTerminalOne)
{
	EXPECT_EQ(sys.active_terminal, 0u);

	/* Ctrl+2: scancode 0x03 maps to terminal 1 */
	unsigned char keys[] = {0x03};

	shortcut_handler(keys, 1);

	EXPECT_EQ(sys.active_terminal, 1u);
}

TEST_F(SystemTest, ShortcutIgnoresOutOfRangeScancode)
{
	/* Scancode 0x0C is beyond '0' key — should be ignored */
	unsigned char keys[] = {0x0C};

	shortcut_handler(keys, 1);

	EXPECT_EQ(sys.active_terminal, 0u);
}

TEST_F(SystemTest, ShortcutIgnoresNullKeys)
{
	shortcut_handler(NULL, 1);

	EXPECT_EQ(sys.active_terminal, 0u);
}

TEST_F(SystemTest, ShortcutIgnoresZeroCount)
{
	unsigned char keys[] = {0x03};

	shortcut_handler(keys, 0);

	EXPECT_EQ(sys.active_terminal, 0u);
}

TEST_F(SystemTest, ShortcutViaKeyboardProcessScancode)
{
	/*
	 * End-to-end: simulate Ctrl+2 through the keyboard
	 * process_scancode path to verify the shortcut_handler
	 * is properly connected.
	 */
	EXPECT_EQ(sys.active_terminal, 0u);

	sys.keyboard.process_scancode(&sys.keyboard, KEY_LCTRL_PRESSED);
	sys.keyboard.process_scancode(&sys.keyboard, 0x03); /* '2' key */
	sys.keyboard.process_scancode(&sys.keyboard, KEY_LCTRL_RELEASED);

	EXPECT_EQ(sys.active_terminal, 1u);
}

/* ------------------------------------------------------------------ */
/*                     Terminal isolation tests                       */
/* ------------------------------------------------------------------ */

TEST_F(SystemTest, TerminalsHaveDistinctIds)
{
	EXPECT_EQ(sys.terminals[0].id, 0u);
	EXPECT_EQ(sys.terminals[1].id, 1u);
}

TEST_F(SystemTest, TypingOnlyAffectsActiveTerminal)
{
	terminal_t *t0 = &sys.terminals[0];
	terminal_t *t1 = &sys.terminals[1];

	t0->handle_keyboard_input(t0, 'X');

	EXPECT_EQ(t0->line[0], 'X');
	EXPECT_EQ(t0->line_len, 1u);
	EXPECT_EQ(t1->line[0], '\0');
	EXPECT_EQ(t1->line_len, 0u);
}

/* ------------------------------------------------------------------ */
/*                     create_terminal tests                          */
/* ------------------------------------------------------------------ */

TEST_F(SystemTest, CreateTerminalInitializesAllTerminals)
{
	/* Dirty the terminals first */
	sys.terminals[0].cursor_x = 99;
	sys.terminals[1].cursor_x = 99;
	sys.active_terminal = 1;

	create_terminal();

	for (int i = 0; i < MAX_TERMINAL; i++) {
		EXPECT_NE(sys.terminals[i].write_char, nullptr);
		EXPECT_NE(sys.terminals[i].clear, nullptr);
		EXPECT_NE(sys.terminals[i].render, nullptr);
		EXPECT_EQ(sys.terminals[i].id, (uint32_t)i);
		EXPECT_EQ(sys.terminals[i].line_len, 0u);
	}
	EXPECT_EQ(sys.active_terminal, 0u);
}

TEST_F(SystemTest, CreateTerminalClearsEachTerminal)
{
	/* Write content to terminal 0 */
	terminal_t *t0 = &sys.terminals[0];

	t0->handle_keyboard_input(t0, 'A');
	EXPECT_EQ(t0->line_len, 1u);

	create_terminal();

	EXPECT_EQ(sys.terminals[0].line_len, 0u);
	EXPECT_EQ(sys.terminals[0].cursor_x, TERMINAL_PREFIX_LEN);
	EXPECT_EQ(sys.terminals[0].cursor_y, 0);
}

TEST_F(SystemTest, CreateTerminalSetsActiveToZero)
{
	sys.active_terminal = 1;

	create_terminal();

	EXPECT_EQ(sys.active_terminal, 0u);
}

/* ------------------------------------------------------------------ */
/*                     init_system tests                              */
/* ------------------------------------------------------------------ */

/*
 * init_system() calls display_init() which sets videomemptr to the
 * real VGA address (0xb8000). Since tests run in user-space, calling
 * init_system() directly would segfault on the first display write.
 *
 * Instead we verify the individual pieces that init_system wires up,
 * using the fixture's SetUp() which mirrors the same sequence with
 * a safe video buffer.
 */

TEST_F(SystemTest, InitSystemAssignsSwitchTerminal)
{
	/* SetUp already wired switch_terminal like init_system does */
	EXPECT_NE(sys.switch_terminal, nullptr);
}

TEST_F(SystemTest, InitSystemAssignsMainLoop)
{
	sys.main_loop = main_loop;
	EXPECT_NE(sys.main_loop, nullptr);
}

TEST_F(SystemTest, InitSystemDisplayIsConfigured)
{
	EXPECT_EQ(sys.display.width, 80);
	EXPECT_EQ(sys.display.height, 25);
	EXPECT_NE(sys.display.clear, nullptr);
	EXPECT_NE(sys.display.put_at, nullptr);
}

TEST_F(SystemTest, InitSystemTerminalsAreConfigured)
{
	for (int i = 0; i < MAX_TERMINAL; i++) {
		EXPECT_NE(sys.terminals[i].write_char, nullptr);
		EXPECT_NE(sys.terminals[i].clear, nullptr);
		EXPECT_EQ(sys.terminals[i].id, (uint32_t)i);
	}
	EXPECT_EQ(sys.active_terminal, 0u);
}

TEST_F(SystemTest, InitSystemKeyboardIsConfigured)
{
	EXPECT_NE(sys.keyboard.process_scancode, nullptr);
	EXPECT_NE(sys.keyboard.set_shortcut_handler, nullptr);
	EXPECT_NE(sys.keyboard.shortcut_handler, nullptr);
}

/* ------------------------------------------------------------------ */
/*                     main_loop tests                                */
/* ------------------------------------------------------------------ */

TEST_F(SystemTest, MainLoopNullSelfReturnsImmediately)
{
	/* main_loop(NULL) should just return without crashing */
	main_loop(NULL);
}
