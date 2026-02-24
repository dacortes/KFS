#pragma once

#include <terminal.h>
#include <display.h>
#include <keyboard.h>
#include <pic.h>
#include <idt.h>
#include <helper.h>

#ifndef MAX_TERMINAL
#define MAX_TERMINAL 2
#endif
// SPDX-License-Identifier: GPL-2.0

/**
 * @file system.h
 * @brief Global kernel system context and public system API.
 *
 * Declares the `system_t` singleton which aggregates core subsystems
 * (display, keyboard, terminals) and exposes lifecycle entry points.
 */

#pragma once

#include <terminal.h>
#include <display.h>
#include <keyboard.h>
#include <pic.h>
#include <idt.h>
#include <helper.h>

#ifndef MAX_TERMINAL
#define MAX_TERMINAL 2
#endif

typedef struct system_s system_t;

/**
 * @struct system_s
 * @brief Kernel global system context.
 *
 * This structure collects the kernel-wide subsystems that are used
 * across the runtime and provides hooks for initialization and the
 * main event loop.
 */
struct system_s {
	/** Display interface (global singleton). */
	display_t display;

	/** Keyboard input state. */
	keyboard_t keyboard;

	/** Array of virtual terminals. */
	terminal_t terminals[MAX_TERMINAL];

	/** Index of the currently active terminal. */
	uint32_t active_terminal;

	/* Pointer reserved for a system-wide logger (unused). */
	// system_log_t *syslog;

    void (*init)(system_t *self);
	void (*main_loop)(system_t *self);
};

/** Global singleton instance of the kernel system context. */
extern system_t sys;

/**
 * @brief Initialize the global system state and subsystems.
 *
 * Sets up the display, virtual terminals, IDT, PIC and keyboard, and
 * prepares `sys.main_loop` for runtime execution.
 */
void init_system(void);


/* Keyboard IRQ1 handler is defined in the interrupt assembly/handler code. */
void irq1_handler(void);