// SPDX-License-Identifier: GPL-2.0

#include <print.h>

/**
 * @file printf.c
 * @brief Minimal printf implementation for kernel output.
 *
 * The implementation provides a small subset of printf functionality
 * suitable for kernel debugging and early boot messages. It keeps
 * formatting simple and synchronous.
 */

/**
 * @brief Parse the format string and dispatch format handlers.
 *
 * This internal helper walks `str` and calls `formats()` for each
 * encountered '%' specifier. It accepts a pointer to `va_list` so that
 * `va_arg` advances are preserved across calls.
 *
 * @param args Pointer to the active `va_list` of variadic arguments.
 * @param str Format string to parse.
 * @param count Pointer to running output count; updated by writes.
 * @return Current count value, or -1 on error.
 */
static int	checking(va_list *args, char const *str, int *count)
{
	int	i;

	i = 0;
	while (str[i]) {
		if (str[i] == '%') {
			if (formats(args, str[i + 1], count) == -1)
				return (-1);
			i++;
		} else {
			if (char_format(str[i], count) == -1)
				return (-1);
		}
		if (str[i])
			i++;
	}
	return (*count);
}

int printf(const char *str, ...)
{
	int	count;
	va_list	args;

	count = 0;
	va_start(args, str);
	if (checking(&args, str, &count) == -1) {
		va_end(args);
		return -1;
	}
	va_end(args);
	return count;
}
