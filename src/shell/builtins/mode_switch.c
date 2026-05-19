// SPDX-License-Identifier: GPL-2.0

#include <builtins.h>

static uint8_t user_stack[4096];

static uint32_t get_cs(void)
{
	uint32_t cs;

	asm volatile("mov %%cs, %0" : "=r"(cs));
	return cs;
}

void print()
{
	printf("hello\n");
}

/**
 * User mode test function - runs in Ring 3.
 * Avoid calling kernel C helpers (printf, etc.) directly from here.
 */
static void user_mode(void)
{
	asm volatile("movl $0, %%eax\n\t"
	             "int $0x80"
	             : : : "eax");
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
