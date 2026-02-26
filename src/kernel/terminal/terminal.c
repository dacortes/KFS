// SPDX-License-Identifier: GPL-2.0

/**
 * @file terminal.c
 * @brief Virtual terminal implementation
 */

#include <terminal.h>
#include <kernel/keyboard/keyboard.h>

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

	ft_strncpy(self->history[self->his_head], text,
		   TERMINAL_HISTORY_LEN - 1);
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
		self->display->put_at(self->display, c, self->cursor_x,
				      self->cursor_y);
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
 * handle_keyboard_input - Handle character input from keyboard
 * @self: Terminal instance
 * @input: Input character
 *
 * Processes keyboard input including backspace, newline, and regular characters.
 * Updates the line buffer and displays the character.
 */
static void handle_keyboard_input(terminal_t *self, unsigned char input)
{
	if (!self || !input)
		return;

	if (input == KEY_LEFT_PRESSED) {
		self->move_cursor(self, CURSOR_LEFT);
		return;
	}

	if (input == KEY_RIGHT_PRESSED) {
		self->move_cursor(self, CURSOR_RIGHT);
		return;
	}

	if (input == '\b') {
		if (self->line_pos > 0) {
			if (self->cursor_x > 0) {
				self->cursor_x--;
			} else if (self->cursor_y > 0) {
				self->cursor_y--;
				self->cursor_x = self->display->width - 1;
			}
			self->display->put_at(self->display, ' ',
					      self->cursor_x, self->cursor_y);
			self->line_pos--;
			self->line[self->line_pos] = '\0';
		}
		self->update_cursor(self);
		return;
	}
	if (input == '\n') {
		self->save_history(self, self->line);
		self->write_char(self, input);
		self->line_pos = 0;
		self->line_len = 0;
		clear_buffer(self->line);
		self->update_cursor(self);
		return;
	}
	if (self->line_pos >= DEVICE_BUFFER_SIZE - 1)
		return;
	self->line[self->line_pos] = input;
	self->line[self->line_pos + 1] = '\0';
	self->line_pos++;
	self->write_char(self, input);
	self->update_cursor(self);
}

/**
 * update_cursor - Update cursor visual state
 * @self: Terminal instance
 *
 * Draws the cursor at current position with inverted colors.
 */
static void update_cursor(terminal_t *self)
{
	uint16_t offset;
	char *video;
	char display_char;

	if (!self || !self->display)
		return;

	offset = (self->cursor_y * self->display->width + self->cursor_x) *
		 self->display->char_size;
	video = self->display->videomemptr + offset;

	display_char = self->cursor_char;
	if (display_char == '\0')
		display_char = ' ';

	video[0] = display_char;
	video[1] = BLACK_ON_WHITE;
}

/**
 * move_cursor - Move cursor left or right
 * @self: Terminal instance
 * @direction: Direction to move (CURSOR_LEFT or CURSOR_RIGHT)
 *
 * Moves the cursor position within the current line boundaries.
 * Updates cursor position tracking and visual state.
 */
static void move_cursor(terminal_t *self, int direction)
{
	uint16_t line_start_x;
	uint16_t line_start_y;
	uint16_t old_x;
	uint16_t old_y;
	uint16_t new_pos;
	uint16_t old_pos;

	if (!self)
		return;

	old_x = self->cursor_x;
	old_y = self->cursor_y;

	line_start_x = old_x - (self->line_pos % self->display->width);
	line_start_y = old_y - (self->line_pos / self->display->width);

	old_pos = (old_y - line_start_y) * self->display->width +
		  (old_x - line_start_x);

	if (direction == CURSOR_LEFT && old_pos > 0) {
		new_pos = old_pos - 1;
	} else if (direction == CURSOR_RIGHT && old_pos < self->line_pos - 1) {
		new_pos = old_pos + 1;
	} else {
		return;
	}

	self->cursor_y = line_start_y + (new_pos / self->display->width);
	self->cursor_x = line_start_x + (new_pos % self->display->width);

	self->display->put_at(self->display,
			      self->cursor_char ? self->cursor_char : ' ',
			      old_x, old_y);

	self->cursor_char = self->line[new_pos];
	if (!self->cursor_char)
		self->cursor_char = ' ';

	self->update_cursor(self);
}

/**
 * terminal_init - Initialize a terminal instance
 * @self: Terminal instance to initialize
 * @display: Display device for output
 *
 * Sets up the terminal state, clears history, initializes cursor position,
 * and assigns function pointers.
 */
void terminal_init(terminal_t *self, display_t *display)
{
	if (!self)
		return;

	self->id = 1;
	ft_strcpy(self->name, "virtual Terminal");
	for (int i = 0; i < TERMINAL_HISTORY_SIZE; i++)
		self->history[i][0] = '\0';
	self->his_size = 0;
	self->his_head = 0;
	self->cursor_x = 0;
	self->cursor_y = 0;

	self->curr_color = WHITE_ON_BLACK;
	self->cursor_char = ' ';
	self->display = display;

	clear_buffer(self->line);
	self->line_pos = 0;
	self->line_len = 0;

	self->clear = clear_ter;
	self->write_char = write_char;
	self->write_string = write_string;
	self->handle_keyboard_input = handle_keyboard_input;
	self->save_history = save_history;
	self->update_cursor = update_cursor;
	self->move_cursor = move_cursor;
}
