// SPDX-License-Identifier: GPL-2.0

#pragma once

#include <stdarg.h>
#include <helper.h>

#define END   "\033[0m"
#define BLACK   "\033[0;30m"
#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define BLUE    "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN    "\033[0;36m"
#define WHITE   "\033[0;37m"

#define BRIGHT_BLACK   "\033[1;30m"
#define BRIGHT_RED     "\033[1;31m"
#define BRIGHT_GREEN   "\033[1;32m"
#define BRIGHT_YELLOW  "\033[1;33m"
#define BRIGHT_BLUE    "\033[1;34m"
#define BRIGHT_MAGENTA "\033[1;35m"
#define BRIGHT_CYAN    "\033[1;36m"
#define BRIGHT_WHITE   "\033[1;37m"

#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"

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
