// SPDX-License-Identifier: GPL-2.0

/**
 * @file ft_strlen.c
 * @brief C wrapper for ft_strlen assembly implementation
 *
 * This wrapper calls the assembly implementation from ft_strlen.s.
 * Both kernel and tests compile in 32-bit mode and use the same
 * assembly implementation.
 */

#include <kernel/wrappers/helper.h>

/**
 * Calculate the length of a null-terminated string.
 *
 * @param s Pointer to the null-terminated string
 * @return Number of characters before the null terminator,
 *         or 0 if s is NULL
 */
unsigned int ft_strlen(const char *s)
{
	return ft_strlen_asm(s);
}
