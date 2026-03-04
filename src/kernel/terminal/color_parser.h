#pragma once
#include <color.h>
#include <stdint.h>

#ifndef COLOR_BUFFER_SIZE
#define COLOR_BUFFER_SIZE 16
#endif

#define COLOR_STATE_NORMAL      0
#define COLOR_STATE_ESC         1
#define COLOR_STATE_BRACKET     2
#define COLOR_STATE_NUMBER      3

typedef struct color_parser_s color_parser_t;

struct color_parser_s
{
	uint8_t	state;
	char	buffer[COLOR_BUFFER_SIZE];
	uint8_t buffer_pos;

	/* the most recent decoded components; callers combine them */
	uint8_t	last_foreground;
	uint8_t	last_background;

	int (*parser_process)(color_parser_t *self, char c);
	const char *(*parser_strip)(color_parser_t *self, const char *input, char *output, int max_len);
	void (*parser_reset)(color_parser_t *self);
};

/* helper to compute full VGA attribute from the parser state */
static inline uint8_t color_parser_get_color(const color_parser_t *self)
{
	if (!self)
		return WHITE_ON_BLACK;
	/* foreground lives in low nibble, background in high */
	return (self->last_foreground & 0x0F) | (self->last_background & 0xF0);
}

void color_parser_init(color_parser_t *parser);

