// SPDX-License-Identifier: GPL-2.0

#include <builtins.h>

int cmd_reboot(shell_t *self)
{
	(void)self;
	reboot_system();
	return 0;
}
