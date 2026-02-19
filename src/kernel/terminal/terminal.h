#pragma once

#include <display.h>
#include <stdint.h>
#include <helper.h>
#include <keyboard.h>

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

typedef struct terminal_s terminal_t;

struct terminal_s {
	uint32_t		id;
	char			name[MAX_NAME];

	char			history[TERMINAL_HISTORY_SIZE][TERMINAL_HISTORY_LEN];
	uint32_t		his_size;	/* history size */
	uint32_t		his_head;	/* hsitory index */

	uint16_t		cursor_x;
	uint16_t		cursor_y;
	uint8_t			curr_color;

	display_t		*display;

	char			line[DEVICE_BUFFER_SIZE];
	uint32_t		line_pos;
	uint32_t		line_len;

	void (*write_char)(terminal_t *self, char c);
	void (*write_string)(terminal_t *self, const char *string);
	void (*clear)(terminal_t *self);

	void (*scroll_up)(terminal_t *self, uint32_t lines);
    void (*scroll_down)(terminal_t *self, uint32_t lines);
    void (*set_color)(terminal_t *self, uint8_t color);

	void (*push_char)(terminal_t *self, char input);
    char *(*read_line)(terminal_t *self);

	void (*save_history)(terminal_t *self);
};

void terminal_init(terminal_t	*terminal, display_t *display);