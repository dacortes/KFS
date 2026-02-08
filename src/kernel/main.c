// SPDX-License-Identifier: GPL-2.0

/**
 * @file main.c
 * @brief KFS kernel entry point
 *
 * Main entry point for the KFS kernel. Initializes the kernel
 * and starts execution.
 */

#include <kernel/display/display.h>

/**
 * Main entry point for the kernel.
 *
 * Initializes the kernel
 *
 * @return Does not return
 */
int kernel_main(void)
{
	display_t display;

	display_init(&display);
	display.clear(&display);
	display.write_string(&display, "42");

	while (1) // Infinite loop to keep the kernel running
		;
}
