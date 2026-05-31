// SPDX-License-Identifier: GPL-2.0

#include <builtins.h>

int cmd_echo(shell_t *self)
{
	for (size_t word = 1; word < self->num_tk; word++) {
		char blank = word == 1 ? '\0' : ' ';
		const char *str = self->token[word].word;

		if (printf("%c%s", blank, str) == -1)
			return -1;
	}

	if (printf("\n") == -1)
		return -1;

	return 0;
}
