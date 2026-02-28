// SPDX-License-Identifier: GPL-2.0

/**
 * @file terminal.c
 * @brief Virtual terminal implementation
 */

#include <kernel/terminal/terminal.h>
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
	self->set_cursor_color(self, BLACK_ON_WHITE);
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
 * set_cursor_color - Set the attribute byte at the cursor position
 * @self: Terminal instance
 * @color: VGA color attribute to apply
 *
 * Changes the attribute byte at the current cursor position to @color.
 * Use WHITE_ON_BLACK to restore normal text, BLACK_ON_WHITE to show
 * the cursor block.
 */
static void set_cursor_color(terminal_t *self, uint8_t color)
{
	uint16_t offset;
	char *video;

	if (!self || !self->display)
		return;

	offset = (self->cursor_y * self->display->width + self->cursor_x) *
		 self->display->char_size;
	video = self->display->videomemptr + offset;
	video[1] = color;
}

/**
 * redraw_line_from - Redraw line buffer on display from a position
 * @self: Terminal instance
 * @from: Line buffer index to start redraw from
 * @sx: Screen X at the from position
 * @sy: Screen Y at the from position
 * @count: Number of characters to redraw
 *
 * Redraws characters from the line buffer starting at index @from,
 * placing them on screen starting at (@sx, @sy).
 */
static void redraw_line_from(terminal_t *self, uint16_t from,
			     uint16_t sx, uint16_t sy, uint16_t count)
{
	uint16_t i;

	for (i = 0; i < count; i++) {
		char c = self->line[from + i];

		if (!c)
			c = ' ';
		self->display->put_at(self->display, c, sx, sy);
		sx++;
		if (sx >= self->display->width) {
			sx = 0;
			sy++;
		}
	}
}

/**
 * handle_backspace - Delete character before cursor
 * @self: Terminal instance
 *
 * Removes the character before the cursor, shifts remaining characters
 * left, and redraws the affected portion of the line.
 */
static void handle_backspace(terminal_t *self)
{
	uint16_t sx;
	uint16_t sy;
	int i;

	if (self->line_pos == 0)
		return;

	for (i = self->line_pos - 1; i < (int)self->line_len - 1; i++)
		self->line[i] = self->line[i + 1];
	self->line_len--;
	self->line[self->line_len] = '\0';
	self->line_pos--;

	if (self->cursor_x > 0) {
		self->cursor_x--;
	} else if (self->cursor_y > 0) {
		self->cursor_y--;
		self->cursor_x = self->display->width - 1;
	}

	sx = self->cursor_x;
	sy = self->cursor_y;
	redraw_line_from(self, self->line_pos, sx, sy,
			 self->line_len - self->line_pos + 1);
}

/**
 * handle_newline - Process enter key press
 * @self: Terminal instance
 *
 * Echoes the current line buffer as "GOT: [line]", saves it
 * to history, then resets the line state for new input.
 */
static void handle_newline(terminal_t *self)
{
	const char *prefix = "GOT: ";

	self->save_history(self, self->line);
	self->write_char(self, '\n');

	while (*prefix) {
		self->write_char(self, *prefix);
		prefix++;
	}
	for (uint32_t j = 0; j < self->line_len; j++)
		self->write_char(self, self->line[j]);
	self->write_char(self, '\n');

	self->line_pos = 0;
	self->line_len = 0;
	clear_buffer(self->line);
}

/**
 * handle_printable - Insert a printable character at cursor position
 * @self: Terminal instance
 * @input: Character to insert
 *
 * Inserts @input into the line buffer at line_pos, shifts trailing
 * characters right, redraws the affected portion, and advances
 * the cursor.
 */
static void handle_printable(terminal_t *self, unsigned char input)
{
	uint16_t sx;
	uint16_t sy;
	int i;

	if (self->line_len >= DEVICE_BUFFER_SIZE - 1)
		return;

	for (i = self->line_len; i > (int)self->line_pos; i--)
		self->line[i] = self->line[i - 1];
	self->line[self->line_pos] = input;
	self->line_len++;
	self->line[self->line_len] = '\0';

	sx = self->cursor_x;
	sy = self->cursor_y;
	redraw_line_from(self, self->line_pos, sx, sy,
			 self->line_len - self->line_pos);

	self->line_pos++;
	self->cursor_x++;
	if (self->cursor_x >= self->display->width) {
		self->cursor_x = 0;
		self->cursor_y++;
	}
	if (self->cursor_y >= self->display->height)
		self->cursor_y = self->display->height - 1;
}

/**
 * handle_keyboard_input - Handle character input from keyboard
 * @self: Terminal instance
 * @input: Input character
 *
 * Dispatches keyboard input to the appropriate handler for arrow
 * keys, backspace, newline, or printable characters.
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

	self->set_cursor_color(self, WHITE_ON_BLACK);

	if (input == '\b')
		handle_backspace(self);
	else if (input == '\n')
		handle_newline(self);
	else
		handle_printable(self, input);

	self->set_cursor_color(self, BLACK_ON_WHITE);
}

/**
 * move_cursor - Move cursor left or right within the line
 * @self: Terminal instance
 * @direction: Direction to move (CURSOR_LEFT or CURSOR_RIGHT)
 *
 * Moves the cursor position within the current line boundaries.
 * Updates both the line buffer position (line_pos) and the screen
 * coordinates (cursor_x, cursor_y), then redraws the cursor.
 */
static void move_cursor(terminal_t *self, int direction)
{
	if (!self)
		return;

	if (direction == CURSOR_LEFT && self->line_pos > 0) {
		self->set_cursor_color(self, WHITE_ON_BLACK);
		self->line_pos--;
		if (self->cursor_x > 0) {
			self->cursor_x--;
		} else if (self->cursor_y > 0) {
			self->cursor_y--;
			self->cursor_x = self->display->width - 1;
		}
		self->set_cursor_color(self, BLACK_ON_WHITE);
	} else if (direction == CURSOR_RIGHT &&
		   self->line_pos < self->line_len) {
		self->set_cursor_color(self, WHITE_ON_BLACK);
		self->line_pos++;
		self->cursor_x++;
		if (self->cursor_x >= self->display->width) {
			self->cursor_x = 0;
			self->cursor_y++;
		}
		self->set_cursor_color(self, BLACK_ON_WHITE);
	}
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
	self->set_cursor_color = set_cursor_color;
	self->move_cursor = move_cursor;
}
