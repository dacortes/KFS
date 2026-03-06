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

/**
 * struct color_parser_s - Stateful ANSI SGR escape-sequence parser
 * @state: Current FSM state (COLOR_STATE_*)
 * @buffer: Accumulated SGR parameter bytes (digits and semicolons)
 * @buffer_pos: Write index into @buffer
 * @foreground: Current VGA foreground color index (VGA_* constant)
 * @background: Current VGA background color index (VGA_* constant)
 * @current_color: Combined VGA attribute byte (foreground | background << 4)
 * @parser_process: Feed one character into the FSM; returns 1 if consumed
 * @parser_strip: Copy @input to @output with all escape sequences removed
 * @parser_reset: Reset FSM state and restore default colors
 */
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

/**
 * color_parser_init - Initialize a color_parser_t instance
 * @parser: Pointer to the struct to initialize
 *
 * Wires up all function pointers and calls parser_reset() to set the
 * default state and colors. Must be called before using any method.
 */
void color_parser_init(color_parser_t *parser);
