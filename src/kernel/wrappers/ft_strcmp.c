// SPDX-License-Identifier: GPL-2.0

/**
 * @file ft_strcmp.c
 * @brief C wrapper for ft_strcmp assembly implementation
 *
 * This wrapper calls the assembly implementation from ft_strcmp.s.
 * Both kernel and tests compile in 32-bit mode and use the same
 * assembly implementation.
 */

#include <kernel/wrappers/helper.h>

/**
 * Compare two null-terminated strings lexicographically.
 *
 * @param s1 Pointer to the first string
 * @param s2 Pointer to the second string
 * @return Negative value if s1 < s2, 0 if equal,
 *         positive value if s1 > s2
 */
int ft_strcmp(const char *s1, const char *s2)
{
	return ft_strcmp_asm(s1, s2);
}
