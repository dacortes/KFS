// SPDX-License-Identifier: GPL-2.0

/**
 * @file display.c
 * @brief Display management implementation
 */

#include <display/display.h>

/* Forward declarations */
static void clear(display_t *display);
static void write_string(display_t *display, const char *string);

static void clear(display_t *display)
{
	unsigned int total_size;

	total_size = display->width * display->height * display->char_size;
	for (unsigned int i = 0; i < total_size; i += display->char_size) {
		display->videomemptr[i] = ' ';
		display->videomemptr[i + 1] = display->color;
	}
}

static void write_string(display_t *display, const char *string)
{
	unsigned int i;
	unsigned int j;

	i = 0;
	j = 0;
	while (string[j] != '\0') {
		display->videomemptr[i] = string[j];
		display->videomemptr[i + 1] = display->color;
		++j;
		i += display->char_size;
	}
}

void display_init(display_t *display)
{
	display->videomemptr = (char *)VIDEO_MEM_ADDR;
	display->width = DISPLAY_W;
	display->height = DISPLAY_H;
	display->char_size = CHAR_SIZE;
	display->color = WHITE_ON_BLACK;
	display->clear = clear;
	display->write_string = write_string;

}
