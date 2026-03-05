#pragma once
#include <color.h>
#include <stdint.h>

#ifndef COLOR_BUFFER_SIZE
#define COLOR_BUFFER_SIZE 32
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

	uint8_t	foreground;
	uint8_t	background;
	uint8_t	current_color;

	int (*parser_process)(color_parser_t *self, char c);
	const char *(*parser_strip)(color_parser_t *self, const char *input, char *output, int max_len);
	void (*parser_reset)(color_parser_t *self);
};

void color_parser_init(color_parser_t *parser);
