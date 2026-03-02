// SPDX-License-Identifier: GPL-2.0

#include <system.h>

/**
 * @file write.c
 * @brief Low-level write helpers for the active terminal.
 *
 * Provides the `write()` wrapper used by the printing subsystem. The
 * function delegates to the active terminal implementation.
 */

int write(const char *text, unsigned int count)
{
	if (!text)
		return 0;

	terminal_t  *terminal = &sys.terminals[sys.active_terminal];

	if (count == 1) {
		terminal->write_char(terminal, *text);
		return 1;
	}
	terminal->write_char(terminal, *text);
	return terminal->write_string(terminal, text);
}
