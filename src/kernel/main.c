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
#include <terminal.h>
#include <kernel/interrupts/idt.h>
#include <kernel/interrupts/pic.h>
#include <kernel/keyboard/keyboard.h>

extern void irq1_handler(void);

static display_t *g_display;

static const char g_scancode_to_ascii[58] = {
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
	0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
	'*', 0, ' '
};

/**
 * Demo keyboard shortcut handler
 *
 * Displays captured shortcut keys when Ctrl is released.
 * Converts scancodes to ASCII and writes them to display.
 *
 * @param scancodes Buffer containing captured scancodes
 * @param count Number of scancodes in buffer
 */
static void shortcut_demo_handler(const unsigned char *scancodes, int count)
{
	int i;
	char ascii;
	const char *prefix = "Ctrl+";
	unsigned int x;
	unsigned int y;

	if (!g_display || count == 0)
		return;

	x = 0;
	y = 10;

	while (*prefix) {
		g_display->put_at(g_display, *prefix, x++, y);
		prefix++;
	}

	for (i = 0; i < count; i++) {
		if (scancodes[i] < 58) {
			ascii = g_scancode_to_ascii[scancodes[i]];
			if (ascii != 0)
				g_display->put_at(g_display, ascii, x++, y);
		}
	}
}

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
	terminal_t	term;
	keyboard_t keyboard;

	display_init(&display);
	g_display = &display;
	terminal_init(&term, &display);
	term.clear(&term);

	idt_init();
	pic_init();
	idt_set_gate(0x21, (unsigned int)irq1_handler, 0x10, 0x8E);
	keyboard_init(&keyboard, &display);
	keyboard.set_shortcut_handler(&keyboard, shortcut_demo_handler);
	__asm__ volatile("sti");
	term.clear(&term);

	while (1) {
		if (keyboard.input) {
			term.handle_keyboard_input(&term, keyboard.input);
			keyboard.input = 0;
		}
		__asm__ volatile("hlt");
	}
}
