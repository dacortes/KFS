// SPDX-License-Identifier: GPL-2.0

/**
 * @file main.c
 * @brief KFS kernel entry point
 *
 * Main entry point for the KFS kernel. Initializes the kernel
 * subsystems including interrupts and keyboard, then enters
 * the main loop.
 */

#include <kernel/display/display.h>
#include <kernel/wrappers/helper.h>
#include <kernel/interrupts/idt.h>
#include <kernel/interrupts/pic.h>
#include <kernel/keyboard/keyboard.h>

extern void irq1_handler(void);

/**
 * Append a string to buffer and return new pointer position
 *
 * @param dest Destination pointer in buffer
 * @param src Source string to append
 * @return Pointer to end of appended string (after last char)
 */
/* static char *append_string(char *dest, const char *src)
{
	ft_strcpy(dest, src);
	return dest + ft_strlen(src);
}
 */
/**
 * Build demonstration message using string helper wrappers
 *
 * @param buffer Destination buffer for the message
 * @param msg1 First test string
 * @param msg2 Second test string
 */
/* static void build_demo_message(char *buffer, const char *msg1,
				const char *msg2)
{
	char *ptr;
	unsigned int len;

	ptr = buffer;

	ptr = append_string(ptr, "Copied: ");
	ptr = append_string(ptr, msg1);

	ptr = append_string(ptr, " | Len: ");
	len = ft_strlen(msg2);
	*ptr++ = '0' + len;

	ptr = append_string(ptr, " | Cmp: ");
	if (ft_strcmp(msg1, msg2) < 0)
		ptr = append_string(ptr, "KFS<Kernel");

	ptr = append_string(ptr, " | Hello World: ");
	if (ft_strcmp(msg1, msg2) < 0)
		ptr = append_string(ptr, "42");

	*ptr = '\0';
}*/

/**
 * Main entry point for the kernel.
 *
 * Initializes display, interrupts, PIC, and keyboard.
 * Enters main loop waiting for keyboard input.
 *
 * @return Does not return
 */
int kernel_main(void)
{
	display_t display;
	keyboard_t keyboard;

	display_init(&display);
	display.clear(&display);

	display.write_string(&display, "KFS Kernel v0.1 - Initializing...\n");

	idt_init();
	display.write_string(&display, "[OK] IDT initialized\n");

	pic_init();
	display.write_string(&display, "[OK] PIC initialized\n");

	idt_set_gate(0x21, (unsigned int)irq1_handler, 0x10, 0x8E);
	display.write_string(&display, "[OK] Keyboard IRQ registered\n");

	keyboard_init(&keyboard, &display);
	display.write_string(&display, "[OK] Keyboard initialized\n");

	__asm__ volatile("sti");
	display.write_string(&display, "[OK] Interrupts enabled\n\n");

	display.clear(&display);
	display.write_string(&display, "Ready. Type something:\n");

	while (1)
		__asm__ volatile("hlt");
}
