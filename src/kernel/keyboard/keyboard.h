// SPDX-License-Identifier: GPL-2.0

/**
 * @file keyboard.h
 * @brief PS/2 keyboard driver
 *
 * Provides PS/2 keyboard support with scan code translation
 * and state tracking for modifier keys.
 */

#pragma once

#include <kernel/display/display.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KEYBOARD_DATA_PORT	0x60
#define KEYBOARD_STATUS_PORT	0x64

#define SCANCODE_MAX		0x58

#define KEY_LSHIFT_PRESSED	0x2A
#define KEY_RSHIFT_PRESSED	0x36
#define KEY_LSHIFT_RELEASED	0xAA
#define KEY_RSHIFT_RELEASED	0xB6
#define KEY_LCTRL_PRESSED	0x1D
#define KEY_LCTRL_RELEASED	0x9D
#define KEY_BACKSPACE		0x0E
#define KEY_ENTER			0x1C
#define KEY_UP_PRESSED      0x48
#define KEY_UP_RELEASED     0xC8
#define KEY_DOWN_PRESSED    0x50
#define KEY_DOWN_RELEASED   0xD0
#define KEY_LEFT_PRESSED    0x4B
#define KEY_LEFT_RELEASED   0xCB
#define KEY_RIGHT_PRESSED   0x4D
#define KEY_RIGHT_RELEASED  0xCD

#define SHORTCUT_BUFFER_MAX 10

typedef struct keyboard keyboard_t;
typedef void (*shortcut_handler_t)(const unsigned char *keys, int count);
/**
 * Keyboard state structure
 *
 * Tracks modifier key states and provides display output.
 */
struct keyboard {
	unsigned char	shift_pressed;
	unsigned char	ctrl_pressed;
	volatile char	input;
	display_t *display;
	unsigned char	shortcut_buffer[SHORTCUT_BUFFER_MAX];
	int		shortcut_count;
	shortcut_handler_t shortcut_handler;
	void (*process_scancode)(struct keyboard *self, unsigned char scancode);
	void (*set_shortcut_handler)(struct keyboard *self,
				     shortcut_handler_t handler);
};


/**
 * Initialize the keyboard driver
 *
 * @param kbd Keyboard structure to initialize
 * @param disp Display for output
 */
void keyboard_init(keyboard_t *self, display_t *disp);

/**
 * Keyboard interrupt handler
 *
 * Called by IRQ1 handler when keyboard generates an interrupt.
 * Reads scan code and processes the key press/release.
 */
void keyboard_interrupt(void);

/**
 * Set the active keyboard instance
 *
 * @param kbd Keyboard instance to use in interrupt handler
 */
void keyboard_set_instance(keyboard_t *self);

#ifdef __cplusplus
}
#endif
