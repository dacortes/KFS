// SPDX-License-Identifier: GPL-2.0

#include <builtins.h>
#include <kernel/memory/memory.h>

int cmd_memstat(shell_t *self)
{
	(void)self;
	memory_dump_state();
	return 0;
}
