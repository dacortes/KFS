// SPDX-License-Identifier: GPL-2.0

#include <system.h>
#include <kernel/print/print.h>

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
 * @brief Switch the active terminal and redraw the display.
 *
 * Changes `sys.active_terminal` to the given index and redraws
 * that terminal's scrollback content to video memory.
 *
 * @param self System instance
 * @param id Terminal index to switch to (must be < MAX_TERMINAL)
 */
static void switch_terminal(system_t *self, uint32_t id)
{
	terminal_t *term;
	uint32_t prev;

	if (!self || id >= MAX_TERMINAL || id == self->active_terminal)
		return;

	/* Update title windows: mark previous as inactive and new as active */
	prev = self->active_terminal;
	if (prev < MAX_TERMINAL)
		terminal_draw_title(&self->terminals[prev], 0);

	self->active_terminal = id;
	term = &self->terminals[id];
	/* Draw active title */
	terminal_draw_title(term, 1);

	term->set_offset(term, 0);
	term->render(term);
}

/**
 * @brief Keyboard shortcut handler for terminal switching.
 *
 * Interprets Ctrl+<number-key> shortcuts to switch terminals.
 * Scancode 0x02 = '1' (terminal 0), 0x03 = '2' (terminal 1), etc.
 *
 * @param keys Array of scancodes pressed during Ctrl hold
 * @param count Number of scancodes
 */
static void shortcut_handler(const unsigned char *keys, int count)
{
	if (!keys || count < 1)
		return;

	/* Scancodes 0x02..0x0B correspond to keys '1'..'0' */
	if (keys[0] >= 0x02 && keys[0] <= 0x0B) {
		uint32_t id = keys[0] - 0x02;

		sys.switch_terminal(&sys, id);
	}
}

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

	/* Draw title windows for all terminals (inactive), then select 0 */
	for (int i = 0; i < MAX_TERMINAL; i++)
		terminal_draw_title(&sys.terminals[i], 0);

	sys.active_terminal = 0;
	/* highlight active */
	if (MAX_TERMINAL > 0)
		terminal_draw_title(&sys.terminals[0], 1);
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

	system_log_init(&sys.syslog);

	create_terminal();

	idt_init();
	pic_init();

	/* Register keyboard IRQ handler (IRQ1) */
	idt_set_gate(0x21, (unsigned int)irq1_handler, 0x10, 0x8E);

	keyboard_init(&sys.keyboard);
	sys.keyboard.set_shortcut_handler(&sys.keyboard, shortcut_handler);

	/* Enable interrupts */
	__asm__ volatile("sti");

	sys.switch_terminal = switch_terminal;
	sys.main_loop = main_loop;
}
