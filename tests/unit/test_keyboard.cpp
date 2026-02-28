// SPDX-License-Identifier: GPL-2.0

#include <gtest/gtest.h>
#include <kernel/keyboard/keyboard.h>
#include <kernel/display/display.h>

class KeyboardTest : public ::testing::Test {
protected:
	keyboard_t keyboard;
	display_t display;

	void SetUp() override
	{
		display_init(&display);
		keyboard_init(&keyboard);
	}
};

static unsigned char captured_keys[SHORTCUT_BUFFER_MAX];
static int captured_count;

static void test_shortcut_handler(const unsigned char *keys, int count)
{
	int i;

	captured_count = count;
	for (i = 0; i < count && i < SHORTCUT_BUFFER_MAX; i++)
		captured_keys[i] = keys[i];
}

TEST_F(KeyboardTest, InitializesWithNoShortcut)
{
	EXPECT_EQ(keyboard.shortcut_count, 0);
	EXPECT_EQ(keyboard.shortcut_handler, nullptr);
}

TEST_F(KeyboardTest, RegistersShortcutHandler)
{
	keyboard.set_shortcut_handler(&keyboard, test_shortcut_handler);
	EXPECT_EQ(keyboard.shortcut_handler, test_shortcut_handler);
}

TEST_F(KeyboardTest, CapturesSingleKeyShortcut)
{
	captured_count = 0;
	keyboard.set_shortcut_handler(&keyboard, test_shortcut_handler);

	keyboard.process_scancode(&keyboard, KEY_LCTRL_PRESSED);
	keyboard.process_scancode(&keyboard, 0x1E);
	keyboard.process_scancode(&keyboard, KEY_LCTRL_RELEASED);

	EXPECT_EQ(captured_count, 1);
	EXPECT_EQ(captured_keys[0], 0x1E);
}

TEST_F(KeyboardTest, CapturesMultipleKeyShortcut)
{
	captured_count = 0;
	keyboard.set_shortcut_handler(&keyboard, test_shortcut_handler);

	keyboard.process_scancode(&keyboard, KEY_LCTRL_PRESSED);
	keyboard.process_scancode(&keyboard, 0x1E);
	keyboard.process_scancode(&keyboard, 0x2E);
	keyboard.process_scancode(&keyboard, 0x14);
	keyboard.process_scancode(&keyboard, KEY_LCTRL_RELEASED);

	EXPECT_EQ(captured_count, 3);
	EXPECT_EQ(captured_keys[0], 0x1E);
	EXPECT_EQ(captured_keys[1], 0x2E);
	EXPECT_EQ(captured_keys[2], 0x14);
}

TEST_F(KeyboardTest, ResetsBufferOnCtrlPress)
{
	keyboard.shortcut_buffer[0] = 0x1E;
	keyboard.shortcut_count = 1;

	keyboard.process_scancode(&keyboard, KEY_LCTRL_PRESSED);

	EXPECT_EQ(keyboard.shortcut_count, 0);
}

TEST_F(KeyboardTest, DoesNotExceedBufferMax)
{
	int i;

	keyboard.process_scancode(&keyboard, KEY_LCTRL_PRESSED);

	for (i = 0; i < SHORTCUT_BUFFER_MAX + 5; i++)
		keyboard.process_scancode(&keyboard, 0x1E);

	EXPECT_EQ(keyboard.shortcut_count, SHORTCUT_BUFFER_MAX);

	for (i = 0; i < SHORTCUT_BUFFER_MAX; i++)
		EXPECT_EQ(keyboard.shortcut_buffer[i], 0x1E);
}

TEST_F(KeyboardTest, NoHandlerCallWithoutKeys)
{
	captured_count = -1;
	keyboard.set_shortcut_handler(&keyboard, test_shortcut_handler);

	keyboard.process_scancode(&keyboard, KEY_LCTRL_PRESSED);
	keyboard.process_scancode(&keyboard, KEY_LCTRL_RELEASED);

	EXPECT_EQ(captured_count, -1);
}

TEST_F(KeyboardTest, LeftShiftPressAndRelease)
{
	EXPECT_EQ(keyboard.shift_pressed, 0);

	keyboard.process_scancode(&keyboard, KEY_LSHIFT_PRESSED);
	EXPECT_EQ(keyboard.shift_pressed, 1);

	keyboard.process_scancode(&keyboard, KEY_LSHIFT_RELEASED);
	EXPECT_EQ(keyboard.shift_pressed, 0);
}

TEST_F(KeyboardTest, RightShiftPressAndRelease)
{
	EXPECT_EQ(keyboard.shift_pressed, 0);

	keyboard.process_scancode(&keyboard, KEY_RSHIFT_PRESSED);
	EXPECT_EQ(keyboard.shift_pressed, 1);

	keyboard.process_scancode(&keyboard, KEY_RSHIFT_RELEASED);
	EXPECT_EQ(keyboard.shift_pressed, 0);
}

TEST_F(KeyboardTest, ProcessesAsciiWithoutShift)
{
	keyboard.process_scancode(&keyboard, 0x1E);
	EXPECT_EQ(keyboard.input, 'a');

	keyboard.process_scancode(&keyboard, 0x02);
	EXPECT_EQ(keyboard.input, '1');
}

TEST_F(KeyboardTest, ProcessesAsciiWithShift)
{
	keyboard.process_scancode(&keyboard, KEY_LSHIFT_PRESSED);

	keyboard.process_scancode(&keyboard, 0x1E);
	EXPECT_EQ(keyboard.input, 'A');

	keyboard.process_scancode(&keyboard, 0x02);
	EXPECT_EQ(keyboard.input, '!');
}

TEST_F(KeyboardTest, IgnoresInvalidScancode)
{
	keyboard.input = 'X';

	keyboard.process_scancode(&keyboard, 0x00);
	EXPECT_EQ(keyboard.input, 'X');
}

TEST_F(KeyboardTest, SetInstanceChangesActiveKeyboard)
{
	keyboard_t keyboard2;
	display_t display2;

	display_init(&display2);
	keyboard_init(&keyboard2);

	keyboard2.set_shortcut_handler(&keyboard2, test_shortcut_handler);
	keyboard_set_instance(&keyboard2);

	captured_count = 0;
	keyboard2.process_scancode(&keyboard2, KEY_LCTRL_PRESSED);
	keyboard2.process_scancode(&keyboard2, 0x1E);
	keyboard2.process_scancode(&keyboard2, KEY_LCTRL_RELEASED);

	EXPECT_EQ(captured_count, 1);
}
