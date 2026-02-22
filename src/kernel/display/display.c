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
 * put_at - Write a character at specific coordinates
 * @display: Pointer to the display object
 * @c: Character to write
 * @x: X coordinate (column)
 * @y: Y coordinate (row)
 *
 * Writes a character at the specified position using the current color.
 * Does nothing if coordinates are out of bounds.
 */
static void put_at(display_t *display, char c, unsigned int x, unsigned int y)
{
	unsigned int offset;

	if (x >= display->width || y >= display->height)
		return;

	offset = (y * display->width + x) * display->char_size;

	display->videomemptr[offset] = c;
	display->videomemptr[offset + 1] = display->color;
}

/**
 * display_init - Initialize a display instance
 * @display: Pointer to the display struct to initialize
 *
 * Sets up the display with VGA text mode defaults (80x25 at 0xb8000)
 * and assigns function pointers for clear and put_at operations.
 */
void display_init(display_t *display)
{
	display->videomemptr = (char *)VIDEO_MEM_ADDR;
	display->width = DISPLAY_W;
	display->height = DISPLAY_H;
	display->char_size = CHAR_SIZE;
	display->color = WHITE_ON_BLACK;
	display->clear = clear;
	display->put_at = put_at;
}
