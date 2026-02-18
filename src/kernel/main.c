// SPDX-License-Identifier: GPL-2.0

/**
 * @file main.c
 * @brief KFS kernel entry point
 *
 * Main entry point for the KFS kernel. Initializes the kernel
 * and starts execution.
 */

#include <kernel/display/display.h>
#include <kernel/wrappers/helper.h>
#include <terminal.h>

/**
 * Append a string to buffer and return new pointer position
 *
 * @param dest Destination pointer in buffer
 * @param src Source string to append
 * @return Pointer to end of appended string (after last char)
 */
static char *append_string(char *dest, const char *src)
{
	ft_strcpy(dest, src);
	return dest + ft_strlen(src);
}

/**
 * Build demonstration message using string helper wrappers
 *
 * @param buffer Destination buffer for the message
 * @param msg1 First test string
 * @param msg2 Second test string
 */
static void build_demo_message(char *buffer, const char *msg1,
				const char *msg2)
{
	char *ptr;
	unsigned int len;

	ptr = buffer;

	/* Demonstrate ft_strcpy wrapper */
	ptr = append_string(ptr, "Copied: ");
	ptr = append_string(ptr, msg1);

	/* Demonstrate ft_strlen wrapper */
	ptr = append_string(ptr, " | Len: ");
	len = ft_strlen(msg2);
	/* Convert length to string manually */
	*ptr++ = '0' + len;

	/* Demonstrate ft_strcmp wrapper */
	ptr = append_string(ptr, " | Cmp: ");
	if (ft_strcmp(msg1, msg2) < 0)
		ptr = append_string(ptr, "KFS<Kernel");

	ptr = append_string(ptr, " | Hello World: ");
	if (ft_strcmp(msg1, msg2) < 0)
		ptr = append_string(ptr, "42");

	/* Null terminate */
	*ptr = '\0';
}

/**
 * Main entry point for the kernel.
 *
 * Initializes the kernel and demonstrates string helper wrappers
 *
 * @return Does not return
 */
int kernel_main(void)
{
	display_t display;
	terminal_t	term;
	char buffer[80];

	display_init(&display);
	terminal_init(&term, &display);
	term.clear(&term);
	build_demo_message(buffer, "KFS", "Kernel");
	term.write_string(&term, buffer);
	// display.clear(&display);

	// /* Build and display demonstration message */
	// build_demo_message(buffer, "KFS", "Kernel");
	// display.write_string(&display, buffer);



	while (1) /* Infinite loop to keep the kernel running */
		;
}
