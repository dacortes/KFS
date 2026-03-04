// SPDX-License-Identifier: GPL-2.0

#pragma once

#include <stdarg.h>
#include <helper.h>

/**
 * @file print.h
 * @brief Public API for kernel printing helpers and printf-like formatting.
 *
 * This header exposes simple printing helpers used by the kernel console
 * subsystem. Functions are small, synchronous helpers intended for use
 * from kernel code; callers should check return values for error
 * propagation.
 */

/**
 * @brief Write raw bytes to the active terminal.
 *
 * @param text Pointer to bytes to write. If NULL the function returns 0.
 * @param count Number of bytes to write.
 * @return Number of bytes written, or -1 on error.
 */
int write(const char *text, unsigned int count);

/**
 * @brief Write a single character to the active terminal and update count.
 *
 * @param c Character to write.
 * @param count Pointer to running output count; incremented on success.
 * @return New count value, or -1 on error.
 */
int char_format(char c, int *count);

/**
 * @brief Write a NUL-terminated string to the active terminal.
 *
 * If `str` is NULL the literal "(nill)" is printed instead.
 *
 * @param str NUL-terminated string to print or NULL.
 * @param count Pointer to running output count; incremented on success.
 * @return New count value, or -1 on error.
 */
int str_format(const char *str, int *count);

/**
 * @brief Handle a single printf-style format specifier.
 *
 * The function accepts a pointer to a `va_list` so that `va_arg`
 * advances persist across multiple calls.
 *
 * @param args Pointer to the active `va_list`.
 * @param type Format specifier character (e.g. 's', 'c', '%').
 * @param count Pointer to running output count; updated by the formatter.
 * @return New count value, or -1 on error.
 */
int	formats(va_list *args, char const type, int *count);

/**
 * @brief Simple printf-like function for kernel output.
 *
 * Supported format specifiers are a reduced set: `%`, `c`, `s`, `d`, `i`,
 * `u`, `x`, `X`, `p`. The implementation is intentionally small and
 * synchronous.
 *
 * @param str Format string.
 * @return Number of characters printed, or -1 on error.
 */
int printf(const char *str, ...);

/* Runtime hook to redirect write() output temporarily. Used by
 * `printk` to capture formatted output into a buffer. */
typedef int (*write_fn_t)(const char *text, unsigned int count);
int set_global_writer(write_fn_t fn);
int write_redirectable(const char *text, unsigned int count);
