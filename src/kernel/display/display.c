// SPDX-License-Identifier: GPL-2.0

/**
 * @file display.c
 * @brief Display management implementation
 */

#include <kernel/display/display.h>

/**
 * clear - Clears the entire display
 * @display: Pointer to the display object
 *
 * Fills the display with space characters using the current color attribute.
 * Preserves the display's color setting.
 */
static void clear(display_t *display)
{
	unsigned int total_size;

	total_size = display->width * display->height * display->char_size;
	for (unsigned int i = 0; i < total_size; i += display->char_size) {
		display->videomemptr[i] = ' ';
		display->videomemptr[i + 1] = display->color;
	}
}

/**
 * write_string - Writes a null-terminated string to the display
 * @display: Pointer to the display object
 * @string: Null-terminated string to write
 *
 * Writes the string starting at the beginning of video memory,
 * using the current color attribute.
 *
 * Note: Does not handle line wrapping or scrolling
 */
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
