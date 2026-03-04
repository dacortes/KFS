// SPDX-License-Identifier: GPL-2.0

/**
 * @file color_parser.c
 * @brief ANSI escape sequence color parser for VGA text mode
 */

#include <color_parser.h>
#include <helper.h>

/**
 * ansi_to_display_color - Map ANSI color number to VGA attribute
 * @ansi_number: ANSI SGR color code
 *
 * Return: VGA attribute byte for the given ANSI color code.
 */
/**
 * ansi_to_vga_color - Map ANSI color number to VGA attribute
 * @ansi_number: ANSI SGR color code
 *
 * Return: VGA attribute byte for the given ANSI color code.
 *
 * Historically the local name was ansi_to_display_color; the parser
 * API uses "vga" terminology so we keep this name everywhere to
 * prevent confusion.  The mapping attempts to isolate the foreground
 * or background component depending on the code so that callers can
 * combine results if both are specified.
 */
static uint8_t ansi_to_vga_color(int ansi_number)
{
	switch (ansi_number) {
	/* Foreground (30-37) */
	case 30: return BLACK_ON_BLACK;
	case 31: return RED_ON_BLACK;
	case 32: return GREEN_ON_BLACK;
	case 33: return BROWN_ON_BLACK;
	case 34: return BLUE_ON_BLACK;
	case 35: return MAGENTA_ON_BLACK;
	case 36: return CYAN_ON_BLACK;
	case 37: return LIGHT_GRAY_ON_BLACK;
	case 39: return WHITE_ON_BLACK;

	/* Bright foreground (90-97) */
	case 90: return DARK_GRAY_ON_BLACK;
	case 91: return LIGHT_RED_ON_BLACK;
	case 92: return LIGHT_GREEN_ON_BLACK;
	case 93: return YELLOW_ON_BLACK;
	case 94: return LIGHT_BLUE_ON_BLACK;
	case 95: return LIGHT_MAGENTA_ON_BLACK;
	case 96: return LIGHT_CYAN_ON_BLACK;
	case 97: return WHITE_ON_BLACK;

	/* Background (40-47) */
	case 40: return BLACK_ON_BLACK;
	case 41: return WHITE_ON_RED;
	case 42: return WHITE_ON_GREEN;
	case 43: return WHITE_ON_BROWN;
	case 44: return WHITE_ON_BLUE;
	case 45: return WHITE_ON_MAGENTA;
	case 46: return WHITE_ON_CYAN;
	case 47: return WHITE_ON_LIGHT_GRAY;
	case 49: return WHITE_ON_BLACK;

	/* Bright background (100-107) */
	case 100: return BLACK_ON_LIGHT_GRAY;
	case 101: return WHITE_ON_BRIGHT_RED;
	case 102: return WHITE_ON_BRIGHT_GREEN;
	case 103: return WHITE_ON_BRIGHT_YELLOW;
	case 104: return WHITE_ON_BRIGHT_BLUE;
	case 105: return WHITE_ON_BRIGHT_MAGENTA;
	case 106: return WHITE_ON_BRIGHT_CYAN;

	default: return WHITE_ON_BLACK;
	}
}

/**
 * parser_finish - Finalise a parsed ANSI sequence
 * @self: Parser instance
 *
 * Interprets the accumulated numeric buffer as foreground and/or
 * background ANSI codes separated by ';' and stores the resulting
 * VGA attributes.
 */
static void parser_finish(color_parser_t *self)
{
	char *buffer;
	char *sep;
	int fg_num;

	if (!self || self->buffer_pos == 0)
		return;

	buffer = self->buffer;

	sep = ft_strchr(buffer, ';');
	if (sep) {
		*sep = '\0';
		fg_num = ft_atoi(buffer);
		int bg_num = ft_atoi(sep + 1);

		self->last_foreground = ansi_to_vga_color(fg_num);
		self->last_background = ansi_to_vga_color(bg_num);
	} else {
		fg_num = ft_atoi(buffer);

		if (fg_num >= 40 && fg_num <= 49)
			self->last_background = ansi_to_vga_color(fg_num);
		else
			self->last_foreground = ansi_to_vga_color(fg_num);
	}
	self->state = COLOR_STATE_NORMAL;
	self->buffer_pos = 0;
	self->buffer[0] = '\0';
}

static int parser_process(color_parser_t *self, char c)
{
	if (!self)
		return 0;

	switch (self->state) {
	case COLOR_STATE_NORMAL:
		if (c == '\033') {
			self->state = COLOR_STATE_ESC;
			return 1;
		}
		return 0;
	case COLOR_STATE_ESC:
		if (c == '\033') {
			self->state = COLOR_STATE_NORMAL;
			return 2;
		}
		if (c == '[') {
			self->state = COLOR_STATE_BRACKET;
			self->buffer_pos = 0;
			self->buffer[0] = '\0';
			return 1;
		}
		self->state = COLOR_STATE_NORMAL;
		return 2;
	case COLOR_STATE_BRACKET:
		if (c >= '0' && c <= '9') {
			self->buffer[self->buffer_pos++] = c;
			self->buffer[self->buffer_pos] = '\0';
			self->state = COLOR_STATE_NUMBER;
			return 1;
		}
		self->state = COLOR_STATE_NORMAL;
		return 2;
	case COLOR_STATE_NUMBER:
		if (c >= '0' && c <= '9') {
			if (self->buffer_pos < COLOR_BUFFER_SIZE - 1) {
				self->buffer[self->buffer_pos++] = c;
				self->buffer[self->buffer_pos] = '\0';
			}
			return 1;
		} else if (c == ';') {
			if (self->buffer_pos < COLOR_BUFFER_SIZE - 1) {
				self->buffer[self->buffer_pos++] = c;
				self->buffer[self->buffer_pos] = '\0';
			}
			return 1;
		} else if (c == 'm') {
			parser_finish(self);
			return 1;
		}
		self->state = COLOR_STATE_NORMAL;
		return 2;
	}
	return 0;
}

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
	/* Do not reset parser state here: preserve partial escape
	 * sequences across multiple write() calls so that printf-style
	 * code which emits characters one-by-one correctly completes
	 * sequences and updates colors when the final 'm' arrives.
	 */

	while (input[in_pos] && out_pos < max_len - 1) {
		int result = self->parser_process(self, input[in_pos]);

		if (result == 0) {
			output[out_pos++] = input[in_pos];
			in_pos++;
		} else if (result == 1) {
			in_pos++;
		} else if (result == 2) {
			output[out_pos++] = '\033';
		}
	}
	output[out_pos] = '\0';
	return output;
}

static void parser_reset(color_parser_t *self)
{
	if (!self)
		return;

	self->state = COLOR_STATE_NORMAL;
	self->buffer_pos = 0;
	self->buffer[0] = '\0';
	self->last_foreground = WHITE_ON_BLACK;
	self->last_background = BLACK_ON_BLACK;
}

void color_parser_init(color_parser_t *parser)
{
	if (!parser)
		return;

	parser->parser_process = parser_process;
	parser->parser_strip = parser_strip;
	parser->parser_reset = parser_reset;

	parser_reset(parser);
}
