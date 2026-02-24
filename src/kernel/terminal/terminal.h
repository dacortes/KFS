// SPDX-License-Identifier: GPL-2.0

/**
 * @file terminal.h
 * @brief Virtual terminal interface for user input/output
 *
 * Provides a terminal abstraction with command history, cursor management,
 * and character input handling using the display subsystem.
 */

#pragma once

#include <display.h>
#include <stdint.h>
#include <helper.h>
#include <keyboard.h>

#ifndef MAX_NAME
#define MAX_NAME 32
#endif

#ifndef DEVICE_BUFFER_SIZE
#define DEVICE_BUFFER_SIZE 256
#endif

#ifndef TERMINAL_HISTORY_SIZE
#define TERMINAL_HISTORY_SIZE 10
#endif

#ifndef TERMINAL_HISTORY_LEN
#define TERMINAL_HISTORY_LEN 80 
#endif

typedef struct terminal_s terminal_t;

/**
 * struct terminal_s - Virtual terminal state and methods
 * @id: Terminal identifier
 * @name: Terminal name string
 * @history: Circular buffer of command history
 * @his_size: Current number of items in history
 * @his_head: Current position in circular history buffer
 * @cursor_x: Horizontal cursor position
 * @cursor_y: Vertical cursor position
 * @curr_color: Current color attribute for text
 * @display: Pointer to associated display device
 * @line: Current input line buffer
 * @line_pos: Current position in line buffer
 * @line_len: Current length of line buffer
 * @write_char: Function to write a single character
 * @write_string: Function to write a string
 * @clear: Function to clear the terminal
 * @scroll_up: Function to scroll up (not yet implemented)
 * @scroll_down: Function to scroll down (not yet implemented)
 * @set_color: Function to set text color (not yet implemented)
 * @push_char: Function to handle keyboard input
 * @save_history: Function to save text to history
 */
struct terminal_s {
	uint32_t		id;
	char			name[MAX_NAME];

	char			history[TERMINAL_HISTORY_SIZE][TERMINAL_HISTORY_LEN];
	uint32_t		his_size;	/* history size */
	uint32_t		his_head;	/* hsitory index */

	uint16_t		cursor_x;
	uint16_t		cursor_y;
	uint8_t			curr_color;

	display_t		*display;

	char			line[DEVICE_BUFFER_SIZE];
	uint32_t		line_pos;
	uint32_t		line_len;

	void (*write_char)(terminal_t *self, char c);
	void (*write_string)(terminal_t *self, const char *string);
	void (*clear)(terminal_t *self);

	void (*scroll_up)(terminal_t *self, uint32_t lines);
	void (*scroll_down)(terminal_t *self, uint32_t lines);
	void (*set_color)(terminal_t *self, uint8_t color);

	void (*push_char)(terminal_t *self, char input);

	void (*save_history)(terminal_t *self, const char *text);
};

/**
 * Initialize a terminal instance with a display device
 *
 * @param terminal Pointer to the terminal struct to initialize
 * @param display Pointer to the display device to use for output
 */
void terminal_init(terminal_t	*terminal, display_t *display, uint32_t id);