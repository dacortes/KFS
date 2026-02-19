// SPDX-License-Identifier: GPL-2.0

/**
 * @file keyboard.c
 * @brief PS/2 keyboard driver implementation
 */

#include <kernel/keyboard/keyboard.h>
#include <kernel/interrupts/pic.h>
#include <kernel/io/io.h>

static keyboard_t *active_keyboard;

static const char scancode_to_ascii[SCANCODE_MAX] = {
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
	0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
	'*', 0, ' '
};

static const char scancode_to_ascii_shift[SCANCODE_MAX] = {
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
	'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
	0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
	0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
	'*', 0, ' '
};

/**
 * Process a scan code and display the corresponding character
 *
 * @param kbd Keyboard state structure
 * @param scancode Raw scan code from keyboard
 */
static void process_scancode(keyboard_t *kbd, unsigned char scancode)
{
	char ascii;
	char str[2];

	if (scancode == KEY_LSHIFT_PRESSED || scancode == KEY_RSHIFT_PRESSED) {
		kbd->shift_pressed = 1;
		return;
	}

	if (scancode == KEY_LSHIFT_RELEASED || scancode == KEY_RSHIFT_RELEASED) {
		kbd->shift_pressed = 0;
		return;
	}

	if (scancode == KEY_LCTRL_PRESSED) {
		kbd->ctrl_pressed = 1;
		return;
	}

	if (scancode == KEY_LCTRL_RELEASED) {
		kbd->ctrl_pressed = 0;
		return;
	}

	if (scancode >= 0x80)
		return;

	if (scancode >= SCANCODE_MAX)
		return;

	if (kbd->shift_pressed)
		ascii = scancode_to_ascii_shift[scancode];
	else
		ascii = scancode_to_ascii[scancode];

	if (ascii == 0)
		return;

	str[0] = ascii;
	str[1] = '\0';
	kbd->display->write_string(kbd->display, str);
}

/**
 * Initialize the keyboard driver
 *
 * @param kbd Keyboard structure to initialize
 * @param disp Display for output
 */
void keyboard_init(keyboard_t *kbd, display_t *disp)
{
	kbd->shift_pressed = 0;
	kbd->ctrl_pressed = 0;
	kbd->display = disp;
	active_keyboard = kbd;
}

/**
 * Set the active keyboard instance
 *
 * @param kbd Keyboard instance to use in interrupt handler
 */
void keyboard_set_instance(keyboard_t *kbd)
{
	active_keyboard = kbd;
}

/**
 * Keyboard interrupt handler
 *
 * Called by IRQ1 handler when keyboard generates an interrupt.
 */
void keyboard_interrupt(void)
{
	unsigned char scancode;

	scancode = inb(KEYBOARD_DATA_PORT);

	if (active_keyboard)
		process_scancode(active_keyboard, scancode);

	pic_send_eoi(IRQ_KEYBOARD);
}
