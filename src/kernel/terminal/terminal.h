// SPDX-License-Identifier: GPL-2.0

/**
 * @file terminal.h
 * @brief Virtual terminal interface for user input/output
 *
 * Provides a terminal abstraction with command history, cursor management,
 * and character input handling using the display subsystem.
 */

#pragma once

#include <kernel/display/display.h>
#include <stdint.h>
#include <kernel/wrappers/helper.h>
#include <kernel/keyboard/keyboard.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifndef SCROLL_BUFFER_ROWS
#define SCROLL_BUFFER_ROWS 200
#endif

#ifndef BLACK_ON_WHITE
#define BLACK_ON_WHITE 0x70
#endif

#ifndef CURSOR_LEFT
#define CURSOR_LEFT -1
#endif

#ifndef CURSOR_RIGHT
#define CURSOR_RIGHT 1
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
 * @cursor_char: Character currently under the cursor
 * @scroll_buf: Circular buffer storing scrollback row content
 * @scroll_first: Index of oldest row in circular scrollback buffer
 * @scroll_count: Number of valid rows in scrollback buffer
 * @view_offset: Lines scrolled up from the bottom (0 = latest)
 * @write_char: Function to write a single character
 * @write_string: Function to write a string
 * @clear: Function to clear the terminal
 * @scroll: Function to scroll view (positive = up, negative = down)
 * @set_color: Function to set text color (not yet implemented)
 * @handle_keyboard_input: Function to handle keyboard input
 * @save_history: Function to save text to history
 * @set_cursor_color: Function to set cursor cell color
 * @move_cursor: Function to move cursor left or right
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
	char			cursor_char;

	display_t		*display;

	char			scroll_buf[SCROLL_BUFFER_ROWS][DISPLAY_W];
	uint16_t		scroll_first;
	uint16_t		scroll_count;
	uint16_t		view_offset;

	char			line[DEVICE_BUFFER_SIZE];
	uint32_t		line_pos;
	uint32_t		line_len;

	void (*write_char)(terminal_t *self, char c);
	void (*write_string)(terminal_t *self, const char *string);
	void (*clear)(terminal_t *self);

	void (*scroll)(terminal_t *self, int lines);
	void (*set_color)(terminal_t *self, uint8_t color);

	void (*handle_keyboard_input)(terminal_t *self, unsigned char input);

	void (*save_history)(terminal_t *self, const char *text);

	void (*set_cursor_color)(terminal_t *self, uint8_t color);
	void (*move_cursor)(terminal_t *self, int direction);
};

/**
 * Initialize a terminal instance with a display device
 *
 * @param terminal Pointer to the terminal struct to initialize
 * @param display Pointer to the display device to use for output
 */
void terminal_init(terminal_t	*terminal, display_t *display);

#ifdef __cplusplus
}
#endif