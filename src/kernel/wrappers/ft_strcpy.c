// SPDX-License-Identifier: GPL-2.0

/**
 * @file ft_strcpy.c
 * @brief C wrapper for ft_strcpy assembly implementation
 *
 * This wrapper calls the assembly implementation from ft_strcpy.s.
 * Both kernel and tests compile in 32-bit mode and use the same
 * assembly implementation.
 */

#include <kernel/wrappers/helper.h>

/**
 * Copy a null-terminated string from src to dst.
 *
 * @param dst Destination buffer
 * @param src Source string
 * @return Pointer to dst, or NULL if either argument is NULL
 */
char *ft_strcpy(char *dst, const char *src)
{
	return ft_strcpy_asm(dst, src);
}
