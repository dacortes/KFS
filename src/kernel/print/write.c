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

	/* Ensure ANSI sequences are stripped by always using write_string.
	 * For single-character writes create a temporary nul-terminated buffer
	 * so callers that pass non-terminated chars (common in formatting)
	 * are handled correctly. For multi-char writes assume the provided
	 * string is nul-terminated (common caller behaviour) and pass it
	 * through the terminal string path which runs the color parser.
	 */
	if (count == 1) {
		char tmp[2];
		tmp[0] = *text;
		tmp[1] = '\0';
		return terminal->write_string(terminal, tmp);
	}

	return terminal->write_string(terminal, text);
}

/* Redirectable writer support: allows printf/format routines to
 * temporarily capture output into a buffer by setting a custom
 * writer function. By default the writer is NULL and the behavior
 * falls back to the terminal write above.
 */
typedef int (*write_fn_t)(const char *text, unsigned int count);

static write_fn_t global_writer;

int set_global_writer(write_fn_t fn)
{
	global_writer = fn;
	return 0;
}

int write_redirectable(const char *text, unsigned int count)
{
	if (global_writer)
		return global_writer(text, count);
	return write(text, count);
}
