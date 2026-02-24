// SPDX-License-Identifier: GPL-2.0

/**
 * @file terminal.c
 * @brief Virtual terminal implementation
 */

#include <terminal.h>

/**
 * clear_buffer - Zero out a buffer
 * @buff: Buffer to clear
 *
 * Fills the entire buffer with null terminators up to DEVICE_BUFFER_SIZE.
 */
static void clear_buffer(char *buff)
{
	if (!buff)
		return;
	for (int i = 0; i < DEVICE_BUFFER_SIZE; i++)
		buff[i] = '\0';
}

/**
 * clear_ter - Clear terminal display and line buffer
 * @self: Terminal instance
 *
 * Clears the display and resets the current line buffer.
 */
static void clear_ter(terminal_t *self)
{
	if (!self)
		return;
	self->display->clear(self->display);
	clear_buffer(self->line);
}

/**
 * save_history - Save a command to terminal history
 * @self: Terminal instance
 * @text: Text to save in history
 *
 * Saves text to the circular history buffer and updates history pointers.
 */
static void save_history(terminal_t *self, const char *text)
{
	if (!self || !text)
		return;

	ft_strncpy(self->history[self->his_head], text, TERMINAL_HISTORY_LEN - 1);
	self->history[self->his_head][TERMINAL_HISTORY_LEN - 1] = '\0';
	self->his_head = (self->his_head + 1) % TERMINAL_HISTORY_SIZE;
	if (self->his_size < TERMINAL_HISTORY_SIZE)
		self->his_size++;
}

/**
 * write_char - Write a single character to terminal
 * @self: Terminal instance
 * @c: Character to write
 *
 * Writes a character at the current cursor position and advances the cursor.
 * Handles newlines and wrapping at display boundaries.
 */
static void write_char(terminal_t *self, char c)
{
	if (!self)
		return;

	if (c == '\n') {
		self->cursor_x = 0;
		self->cursor_y++;
	} else {
		self->display->put_at(self->display, c, self->cursor_x, self->cursor_y);
		self->cursor_x++;
	}

	if (self->cursor_x >= self->display->width) {
		self->cursor_x = 0;
		self->cursor_y++;
	}

	if (self->cursor_y >= self->display->height)
		self->cursor_y = self->display->height - 1;
}

/**
 * write_string - Write a string to terminal
 * @self: Terminal instance
 * @str: Null-terminated string to write
 *
 * Writes each character in the string and saves it to history.
 */
static void write_string(terminal_t *self, const char *str)
{
	unsigned int i;

	if (!self)
		return;

	self->save_history(self, str);
	i = 0;
	while (str[i]) {
		self->write_char(self, str[i]);
		i++;
	}
}

/**
 * push_char - Handle character input from keyboard
 * @self: Terminal instance
 * @input: Input character
 *
 * Processes keyboard input including backspace, newline, and regular characters.
 * Updates the line buffer and displays the character.
 */
static void push_char(terminal_t *self, char input)
{
	if (!self || !input)
		return;

	if (input == '\b') {
		if (self->line_pos > 0) {
			if (self->cursor_x > 0) {
				self->cursor_x--;
			} else if (self->cursor_y > 0) {
				self->cursor_y--;
				self->cursor_x = self->display->width - 1;
			}
			self->display->put_at(self->display, ' ', self->cursor_x, self->cursor_y);
			self->line_pos--;
			self->line[self->line_pos] = '\0';
		}
		return;
	}
	if (input == '\n') {
		self->save_history(self, self->line);
		self->write_char(self, input);
		self->line_pos = 0;
		self->line_len = 0;
		clear_buffer(self->line);
		return;
	}
	if (self->line_pos >= DEVICE_BUFFER_SIZE - 1)
		return;
	self->line[self->line_pos] = input;
	self->line[self->line_pos + 1] = '\0';
	self->line_pos++;
	self->write_char(self, input);
}

/**
 * terminal_init - Initialize a terminal instance
 * @self: Terminal instance to initialize
 * @display: Display device for output
 *
 * Sets up the terminal state, clears history, initializes cursor position,
 * and assigns function pointers.
 */
void terminal_init(terminal_t *self, display_t *display, uint32_t id)
{
	if (!self)
		return;

	self->id = id;
	ft_strcpy(self->name, "virtual Terminal");
	for (int i = 0; i < TERMINAL_HISTORY_SIZE; i++)
		self->history[i][0] = '\0';
	self->his_size = 0;
	self->his_head = 0;
	self->cursor_x = 0;
	self->cursor_y = 0;

	self->curr_color = WHITE_ON_BLACK;
	self->display = display;

	clear_buffer(self->line);
	self->line_pos = 0;
	self->line_len = 0;

	self->clear = clear_ter;
	self->write_char = write_char;
	self->write_string = write_string;
	self->push_char = push_char;
	self->save_history = save_history;
}
