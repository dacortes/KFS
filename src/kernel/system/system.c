// SPDX-License-Identifier: GPL-2.0

#include <system.h>

/**
 * @file system.c
 * @brief System-level implementation: terminals, interrupts, and main loop.
 *
 * Implements the initialization sequence for the kernel-wide `sys`
 * singleton and the primary runtime loop responsible for consuming
 * keyboard input and routing it to the active terminal.
 */

system_t sys;

/**
 * @brief Initialize all configured virtual terminals.
 *
 * Calls `terminal_init()` for each terminal and clears its state.
 * Sets `sys.active_terminal` to zero.
 */
static void create_terminal(void)
{
	for (int i = 0; i < MAX_TERMINAL; i++) {
		terminal_init(&sys.terminals[i], &sys.display, i);
		sys.terminals[i].clear(&sys.terminals[i]);
	}
	sys.active_terminal = 0;
}

/**
 * @brief Primary kernel main loop.
 *
 * Continuously reads a single-byte keyboard input from
 * `self->keyboard.input`, forwards the character to the active
 * terminal via `term->push_char()` and then halts the CPU until the
 * next interrupt using the `hlt` instruction.
 *
 * @param self Pointer to the kernel `system_t` instance. Function
 *             returns immediately if `self` is NULL.
 */
static void main_loop(system_t *self)
{

	if (!self)
		return;

	while (1) {
		unsigned char *ascii = &self->keyboard.input;
		uint32_t active = self->active_terminal;
		terminal_t *term = &sys.terminals[active];

		if (*ascii)
			term->handle_keyboard_input(term, *ascii);
		self->keyboard.input = 0;
		__asm__ volatile("hlt");
	}
}

/**
 * @brief Initialize kernel subsystems and prepare runtime.
 *
 * Sequence:
 *  - Initialize and clear the display
 *  - Create and clear virtual terminals
 *  - Initialize IDT and PIC
 *  - Register the keyboard IRQ handler (IRQ1)
 *  - Initialize keyboard subsystem
 *  - Enable CPU interrupts
 *  - Print a prompt to the primary terminal and set the main loop
 *
 * After this call `sys` is ready and `sys.main_loop` points to the
 * runtime loop implemented in `main_loop()`.
 */
void init_system(void)
{
	display_init(&sys.display);
	sys.display.clear(&sys.display);

	create_terminal();

	idt_init();
	pic_init();

	/* Register keyboard IRQ handler (IRQ1) */
	idt_set_gate(0x21, (unsigned int)irq1_handler, 0x10, 0x8E);

	keyboard_init(&sys.keyboard);

	/* Enable interrupts */
	__asm__ volatile("sti");

	sys.main_loop = main_loop;
}
