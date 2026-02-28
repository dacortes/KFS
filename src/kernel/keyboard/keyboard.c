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
 * Process a scan code
 *
 * @param self Keyboard state structure
 * @param scancode Raw scan code from keyboard
 */
static void process_scancode(keyboard_t *self, unsigned char scancode)
{
	char input;

	if (scancode == 0xE0) {
		self->extended_code = 1;
		return;
	}
	if (self->extended_code) {
		self->extended_code = 0;
		if (scancode == 0x48)
			self->input = KEY_UP_PRESSED;
		else if (scancode == 0x50)
			self->input = KEY_DOWN_PRESSED;
		else if (scancode == 0x4B)
			self->input = KEY_LEFT_PRESSED;
		else if (scancode == 0x4D)
			self->input = KEY_RIGHT_PRESSED;
		return;
	}

	if (scancode == KEY_LSHIFT_PRESSED || scancode == KEY_RSHIFT_PRESSED) {
		self->shift_pressed = 1;
		return;
	}

	if (scancode == KEY_LSHIFT_RELEASED || scancode == KEY_RSHIFT_RELEASED) {
		self->shift_pressed = 0;
		return;
	}

	if (scancode == KEY_LCTRL_PRESSED) {
		self->ctrl_pressed = 1;
		self->shortcut_count = 0;
		return;
	}

	if (scancode == KEY_LCTRL_RELEASED) {
		self->ctrl_pressed = 0;
		if (self->shortcut_count > 0 && self->shortcut_handler)
			self->shortcut_handler(self->shortcut_buffer,
					       self->shortcut_count);
		self->shortcut_count = 0;
		return;
	}

	if (scancode >= 0x80)
		return;

	if (scancode >= SCANCODE_MAX)
		return;

	if (self->ctrl_pressed) {
		if (self->shortcut_count < SHORTCUT_BUFFER_MAX) {
			self->shortcut_buffer[self->shortcut_count] = scancode;
			self->shortcut_count++;
		}
		return;
	}

	if (self->shift_pressed)
		input = scancode_to_ascii_shift[scancode];
	else
		input = scancode_to_ascii[scancode];

	if (input == 0)
		return;

	self->input = input;
}

/**
 * Register a shortcut handler
 *
 * @param self Keyboard instance
 * @param handler Function to call when shortcut is triggered
 */
static void set_shortcut_handler(keyboard_t *self,
				 shortcut_handler_t handler)
{
	self->shortcut_handler = handler;
}


/**
 * Set the active keyboard instance
 *
 * @param self Keyboard instance to use in interrupt handler
 */
void keyboard_set_instance(keyboard_t *self)
{
	active_keyboard = self;
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
		active_keyboard->process_scancode(active_keyboard, scancode);

	pic_send_eoi(IRQ_KEYBOARD);
}

/**
 * Initialize the keyboard driver
 *
 * @param self Keyboard structure to initialize
 */
void keyboard_init(keyboard_t *self)
{
	int i;

	self->shift_pressed = 0;
	self->ctrl_pressed = 0;
	self->extended_code = 0;
	self->input = 0;
	self->shortcut_count = 0;
	self->shortcut_handler = 0;
	self->process_scancode = process_scancode;
	self->set_shortcut_handler = set_shortcut_handler;
	for (i = 0; i < SHORTCUT_BUFFER_MAX; i++)
		self->shortcut_buffer[i] = 0;
	active_keyboard = self;
}
