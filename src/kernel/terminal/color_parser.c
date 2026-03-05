// SPDX-License-Identifier: GPL-2.0

/**
 * @file color_parser.c
 * @brief ANSI escape sequence color parser for VGA text mode
 */

#include <color_parser.h>

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

static void update_current(color_parser_t *self)
{
	self->current_color = VGA_COLOR(self->foreground, self->background);
}

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

static void apply_sgr_sequence(color_parser_t *self)
{
	int i;

	if (!self)
		return;
	if (self->buffer_pos == 0) {
		self->foreground = VGA_WHITE;
		self->background = VGA_BLACK;
		update_current(self);
		return;
	}

	i = 0;
	while (self->buffer[i]) {
		int code;

		code = parse_number(self->buffer, &i);
		if (code == 0) {
			self->foreground = VGA_WHITE;
			self->background = VGA_BLACK;
		} else if (code == 1) {
			if (self->foreground <= VGA_LIGHT_GRAY)
				self->foreground = (uint8_t)(self->foreground + 8);
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
	update_current(self);
}

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

void color_parser_init(color_parser_t *parser)
{
	if (!parser)
		return;

	parser->parser_process = parser_process;
	parser->parser_strip = parser_strip;
	parser->parser_reset = parser_reset;

	parser_reset(parser);
}
