// SPDX-License-Identifier: GPL-2.0

#include <builtins.h>

int cmd_reboot(shell_t *self)
{
	(void)self;
	printf("Rebooting...\n");
	reboot_system();
	printf("If you see this message, reboot failed!\n");
	return 0;
}
