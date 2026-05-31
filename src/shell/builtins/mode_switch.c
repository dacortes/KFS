// SPDX-License-Identifier: GPL-2.0

#include <builtins.h>

static uint8_t user_stack[4096];

/**
 * User mode test function - runs in Ring 3.
 * Avoid calling kernel C helpers (printf, etc.) directly from here.
 */
static void user_mode(void)
{
	asm volatile("xorl %%eax, %%eax\n\t"
		"int $0x80\n\t"
		: : : "eax", "memory");

	/* Must never continue in ring 3 without a valid return frame. */
	for (;;)
		asm volatile("hlt");
}

int cmd_user_mode(shell_t *self)
{
	(void)self;

	int current_mode = get_current_privilege_level();

	printf("[USER MODE] Current privilege level: %d\n", current_mode);

	if (current_mode == 0) {
		void *stack_top = (void *)(user_stack + sizeof(user_stack));

		printf("[USER MODE] Switching to user mode (Ring 3)\n");

		/* Switch to user mode and run user_mode() function */
		switch_to_user_mode(user_mode, stack_top);

		//printf("[USER MODE] Returned from user mode\n");
	} else {
		printf("[USER MODE] Already in user mode\n");
	}

	return 0;
}

int cmd_show_mode(shell_t *self)
{
	(void)self;

	int mode = get_current_privilege_level();

	printf("[SHOW MODE] Current privilege level: %d\n", mode);
	if (mode == 0)
		printf("[SHOW MODE] Running in kernel mode (Ring 0)\n");
	else if (mode == 3)
		printf("[SHOW MODE] Running in user mode (Ring 3)\n");
	else
		printf("[SHOW MODE] Unknown privilege level: %d\n", mode);
	return 0;
}
