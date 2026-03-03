// SPDX-License-Identifier: GPL-2.0

/**
 * @file main.c
 * @brief KFS kernel entry point.
 *
 * The kernel entry is responsible for bootstrapping core subsystems
 * and transferring control to the system runtime loop.
 */

#include <system.h>
#include <print.h>

/* Helper string builders are intentionally retained as commented-out
 * examples for future reference.
 */

/**
 * @brief Kernel entry point.
 *
 * Calls `init_system()` to initialize kernel subsystems and then
 * transfers control to the runtime main loop defined by
 * `sys.main_loop`. Under normal operation this function does not
 * return.
 *
 * @return Zero on unexpected return (function generally does not return).
 */
int kernel_main(void)
{
	init_system();
	printf("Welcome to KFS! Active terminal: %u\n", sys.active_terminal);
	kprintk(KERN_INFO, "Kernel initialized successfully.\n");
	sys.main_loop(&sys);
	return 0;
}
