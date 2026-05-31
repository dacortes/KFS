// SPDX-License-Identifier: GPL-2.0

#include "builtins.h"

int cmd_half(shell_t *self)
{
	(void)self;
	halt_system();
	return 0;
}
