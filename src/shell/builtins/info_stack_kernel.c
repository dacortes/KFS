// SPDX-License-Identifier: GPL-2.0

#include <builtins.h>
#include <stack_kernel.h>

/**
 * @brief Print stack kernel information.
 *
 * Wraps the `print_stack_info()` function to provide a convenient interface for printing
 */
int cmd_info_stack_kernel(shell_t *self)
{
	(void)self;
	print_multiboot_info(self->info);
	return 0;
}