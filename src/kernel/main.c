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

#define END	"\033[m"
#define RED	"\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW	"\033[1;33m"
#define BLUE	"\033[1;34m"
#define TUR	"\033[1;35m"
#define CYAN	"\033[1;36m"
#define ORANGE	"\033[38;5;208m"
#define PURPLE	"\033[38;5;128m"

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
	char *kernel_init_msg = "Kernel initialized successfully.\n%s";
	char *another_string = "This is a string that came from an argument.\n";

	init_system();
	printf("Welcome to KFS! Active terminal: %u\n", sys.active_terminal);
	kprintk(KERN_INFO, "Kernel initialized successfully.\n");
	sys.terminals[sys.active_terminal].write_string(&sys.terminals[sys.active_terminal],
		" \033[1;33m Kernel initialized successfully.\033[0m\n");
	sys.main_loop(&sys);
	return 0;
}
