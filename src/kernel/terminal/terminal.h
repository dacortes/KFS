#pragma once

#include <display.h>
#include <stdint.h>
#include <helper.h>

#ifndef MAX_NAME
#define MAX_NAME 32
#endif

typedef struct terminal_s terminal_t;

struct terminal_s {
	uint32_t		id;
	char			name[MAX_NAME];

	uint16_t		**history;
	uint32_t		his_size;	/* history size */
	uint32_t		his_head;	/* hsitory index */

	uint16_t		cursor_x;
	uint16_t		cursor_y;
	uint8_t			curr_color;

	display_t		*display;

	void (*write_string)(terminal_t *self, const char *string);
	void (*clear)(terminal_t *self);

	void (*scroll_up)(terminal_t *self, uint32_t lines);
    void (*scroll_down)(terminal_t *self, uint32_t lines);
    void (*set_color)(terminal_t *self, uint8_t color);

	/* funcion para activar esta terminal (cambiar buffer del display)*/
	void (*activate)(terminal_t* self);
};

void terminal_init(terminal_t	*terminal, display_t *display);