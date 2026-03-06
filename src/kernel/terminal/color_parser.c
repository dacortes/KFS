// SPDX-License-Identifier: GPL-2.0

/**
 * @file color_parser.c
 * @brief ANSI escape sequence color parser for VGA text mode
 */

#include <color_parser.h>

/**
 * ansi_basic_to_vga - Map an ANSI basic color code to a VGA color index
 * @ansi_number: ANSI SGR color code (30-37, 90-97)
 *
 * Returns the VGA_* constant that best matches the given ANSI SGR
 * foreground color number. Unrecognised codes return VGA_WHITE.
 */
static uint8_t ansi_basic_to_vga(int ansi_number)
{
	switch (ansi_number) {
	case 30: return VGA_BLACK;
	case 31: return VGA_RED;
	case 32: return VGA_GREEN;
	case 33: return VGA_BROWN;
	case 34: return VGA_BLUE;
	case 35: return VGA_MAGENTA;
	case 36: return VGA_CYAN;
	case 37: return VGA_LIGHT_GRAY;
	case 90: return VGA_DARK_GRAY;
	case 91: return VGA_LIGHT_RED;
	case 92: return VGA_LIGHT_GREEN;
	case 93: return VGA_YELLOW;
	case 94: return VGA_LIGHT_BLUE;
	case 95: return VGA_LIGHT_MAGENTA;
	case 96: return VGA_LIGHT_CYAN;
	case 97: return VGA_WHITE;
	default: return VGA_WHITE;
	}
}

/**
 * ansi_256_to_vga - Map a 256-colour ANSI index to a VGA color index
 * @color_id: 256-colour palette index (0-255)
 *
 * Performs a best-effort mapping of the 256-colour ANSI palette to the
 * 16 VGA text-mode colours. Notable mappings: 208 → VGA_BROWN (orange
 * approximation), 128 → VGA_MAGENTA. Indices below 16 are mapped
 * directly. All others fall back to VGA_WHITE.
 */
static uint8_t ansi_256_to_vga(int color_id)
{
	if (color_id == 208)
		return VGA_BROWN; /* ORANGE approximation */
	if (color_id == 128)
		return VGA_MAGENTA; /* PURPLE approximation */
	if (color_id < 8)
		return (uint8_t)color_id;
	if (color_id < 16)
		return (uint8_t)(color_id - 8 + VGA_DARK_GRAY);
	return VGA_WHITE;
}

/**
 * update_current - Recompute the combined VGA color attribute
 * @self: Color parser instance
 *
 * Recalculates current_color from the stored foreground and background
 * indices using VGA_COLOR() and stores the result in self->current_color.
 */
static void update_current(color_parser_t *self)
{
	self->current_color = VGA_COLOR(self->foreground, self->background);
}

/**
 * parse_number - Read a decimal integer from a character buffer
 * @buffer: Null-terminated buffer to read from
 * @idx: Pointer to current read position; advanced past consumed digits
 *
 * Reads consecutive ASCII digit characters starting at buffer[*idx]
 * and returns their decimal value. @idx is updated to point to the
 * first non-digit character after the number.
 */
static int parse_number(const char *buffer, int *idx)
{
	int value;

	value = 0;
	while (buffer[*idx] >= '0' && buffer[*idx] <= '9') {
		value = (value * 10) + (buffer[*idx] - '0');
		(*idx)++;
	}
	return value;
}

/**
 * apply_sgr_sequence - Apply buffered SGR parameters to parser state
 * @self: Color parser instance
 *
 * Parses the semicolon-delimited SGR parameter buffer accumulated
 * since the last ESC[ and updates foreground, background, and
 * current_color accordingly. An empty buffer (bare ESC[m) resets
 * all attributes to their defaults (VGA_WHITE on VGA_BLACK).
 * Bold (code 1) is deferred and applied after all color codes so
 * that sequences like ESC[1;31m yield the bright variant correctly.
 */
static void apply_sgr_sequence(color_parser_t *self)
{
	int i;
	int bold;

	if (!self)
		return;
	if (self->buffer_pos == 0) {
		self->foreground = VGA_WHITE;
		self->background = VGA_BLACK;
		update_current(self);
		return;
	}

	bold = 0;
	i = 0;
	while (self->buffer[i]) {
		int code;

		code = parse_number(self->buffer, &i);
		if (code == 0) {
			self->foreground = VGA_WHITE;
			self->background = VGA_BLACK;
			bold = 0;
		} else if (code == 1) {
			bold = 1;
		} else if (code >= 30 && code <= 37) {
			self->foreground = ansi_basic_to_vga(code);
		} else if (code >= 40 && code <= 47) {
			self->background = (uint8_t)(code - 40);
		} else if (code >= 90 && code <= 97) {
			self->foreground = ansi_basic_to_vga(code);
		} else if (code >= 100 && code <= 107) {
			self->background = (uint8_t)(code - 100 + 8);
		} else if (code == 39) {
			self->foreground = VGA_WHITE;
		} else if (code == 49) {
			self->background = VGA_BLACK;
		} else if (code == 38 && self->buffer[i] == ';') {
			i++;
			if (self->buffer[i] == '5' && self->buffer[i + 1] == ';') {
				int color_id;

				i += 2;
				color_id = parse_number(self->buffer, &i);
				self->foreground = ansi_256_to_vga(color_id);
			}
		}

		if (self->buffer[i] == ';')
			i++;
	}
	if (bold && self->foreground <= VGA_LIGHT_GRAY)
		self->foreground = (uint8_t)(self->foreground + 8);
	update_current(self);
}

/**
 * parser_process - Feed one character into the ANSI escape-sequence FSM
 * @self: Color parser instance
 * @c: Next input character
 *
 * Drives the finite-state machine that recognises ANSI SGR sequences
 * (ESC [ ... m). Returns 1 if the character was consumed as part of an
 * escape sequence (and should not be displayed), or 0 when the character
 * is ordinary printable text. On sequence completion, apply_sgr_sequence()
 * is called to update the color state.
 */
static int parser_process(color_parser_t *self, char c)
{
	if (!self)
		return 0;

	if (self->state == COLOR_STATE_NORMAL) {
		if (c == '\033') {
			self->state = COLOR_STATE_ESC;
			return 1;
		}
		return 0;
	}

	if (self->state == COLOR_STATE_ESC) {
		if (c == '[') {
			self->state = COLOR_STATE_BRACKET;
			self->buffer_pos = 0;
			self->buffer[0] = '\0';
			return 1;
		}
		self->state = COLOR_STATE_NORMAL;
		return 0;
	}

	if (self->state == COLOR_STATE_BRACKET) {
		if (c == 'm') {
			apply_sgr_sequence(self);
			self->state = COLOR_STATE_NORMAL;
			return 1;
		}
		if ((c >= '0' && c <= '9') || c == ';') {
			if (self->buffer_pos < COLOR_BUFFER_SIZE - 1) {
				self->buffer[self->buffer_pos++] = c;
				self->buffer[self->buffer_pos] = '\0';
			}
			return 1;
		}
		self->state = COLOR_STATE_NORMAL;
		return 0;
	}

	self->state = COLOR_STATE_NORMAL;
	return 0;
}

/**
 * parser_strip - Strip ANSI escape sequences from a string
 * @self: Color parser instance (state is updated as sequences are consumed)
 * @input: Null-terminated input string that may contain escape sequences
 * @output: Output buffer to receive the stripped text
 * @max_len: Capacity of @output including the null terminator
 *
 * Feeds every character of @input through parser_process(). Characters
 * that are not part of an escape sequence are copied to @output. The
 * result is always null-terminated. Returns @output on success, or
 * @input unchanged when any pointer argument is NULL or max_len <= 0.
 */
static const char *parser_strip(color_parser_t *self,
				const char *input, char *output,
				int max_len)
{
	int in_pos;
	int out_pos;

	if (!self || !input || !output || max_len <= 0)
		return input;

	in_pos = 0;
	out_pos = 0;
	while (input[in_pos] && out_pos < max_len - 1) {
		if (self->parser_process(self, input[in_pos]) == 0)
			output[out_pos++] = input[in_pos];
		in_pos++;
	}
	output[out_pos] = '\0';
	return output;
}

/**
 * parser_reset - Reset parser to its default state
 * @self: Color parser instance
 *
 * Clears the FSM state, empties the parameter buffer, and restores
 * foreground/background to their defaults (VGA_WHITE / VGA_BLACK).
 * current_color is recalculated via update_current().
 */
static void parser_reset(color_parser_t *self)
{
	if (!self)
		return;

	self->state = COLOR_STATE_NORMAL;
	self->buffer_pos = 0;
	self->buffer[0] = '\0';
	self->foreground = VGA_WHITE;
	self->background = VGA_BLACK;
	update_current(self);
}

/**
 * color_parser_init - Initialize a color_parser instance
 * @parser: Pointer to the color_parser_t struct to initialize
 *
 * Assigns the parser_process, parser_strip, and parser_reset function
 * pointers, then calls parser_reset() to set the initial FSM state and
 * default foreground/background colors. Must be called before using
 * any method on the struct.
 */
void color_parser_init(color_parser_t *parser)
{
	if (!parser)
		return;

	parser->parser_process = parser_process;
	parser->parser_strip = parser_strip;
	parser->parser_reset = parser_reset;

	parser_reset(parser);
}
