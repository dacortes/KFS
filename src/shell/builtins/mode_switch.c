#include <builtins.h>

static uint8_t user_stack[4096] __attribute__((aligned(16)));

static uint32_t get_cs(void)
{
	uint32_t cs;
	asm volatile("mov %%cs, %0" : "=r"(cs));
	return cs;
}

static void user_mode(void)
{
	printf("[USER MODE] Running in Ring 3\n");
	printf("[USER MODE] CS: 0x%x\n", get_cs());

	printf("[USER MODE] Hello from user space\n");
	printf("[USER MODE] Returning to kernel mode\n");
	return_to_kernel_mode();
}

int cmd_user_mode(shell_t *self)
{
	(void)self;

	int current_mode = get_current_privilege_level();
	printf("[USER MODE] Current privilege level: %d\n", current_mode);

	if (current_mode == 0) {
		printf("[USER MODE] Switching to user mode\n");

		void *stack_top = (void *)((uint32_t *)user_stack + sizeof(user_stack));

		switch_to_user_mode(user_mode, stack_top);

		switch_to_user_mode(user_mode, user_stack + sizeof(user_stack));
		printf("[USER MODE] Back in kernel mode\n");
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
	if (mode == 0) {
		printf("[SHOW MODE] Running in kernel mode (Ring 0)\n");
	} else if (mode == 3) {
		printf("[SHOW MODE] Running in user mode (Ring 3)\n");
	} else {
		printf("[SHOW MODE] Unknown privilege level: %d\n", mode);
	}
	return 0;
}
