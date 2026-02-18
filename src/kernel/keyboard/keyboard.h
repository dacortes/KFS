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
#define KEY_ENTER		0x1C

/**
 * Keyboard state structure
 *
 * Tracks modifier key states and provides display output.
 */
struct keyboard {
	unsigned char shift_pressed;
	unsigned char ctrl_pressed;
	display_t *display;
};

typedef struct keyboard keyboard_t;

/**
 * Initialize the keyboard driver
 *
 * @param kbd Keyboard structure to initialize
 * @param disp Display for output
 */
void keyboard_init(keyboard_t *kbd, display_t *disp);

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
void keyboard_set_instance(keyboard_t *kbd);
