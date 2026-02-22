// SPDX-License-Identifier: GPL-2.0

/**
 * @file helper.h
 * @brief String helper functions implemented in assembly (i386)
 *
 * Provides basic string manipulation functions for the kernel,
 * implemented in x86 32-bit assembly for direct use in a
 * freestanding environment without libc.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Calculate the length of a null-terminated string.
 *
 * @param s Pointer to the null-terminated string
 * @return Number of characters before the null terminator,
 *         or 0 if s is NULL
 */
unsigned int ft_strlen(const char *s);

/**
 * Compare two null-terminated strings lexicographically.
 *
 * Compares strings byte-by-byte using unsigned char values.
 *
 * @param s1 Pointer to the first string
 * @param s2 Pointer to the second string
 * @return Negative value if s1 < s2, 0 if equal,
 *         positive value if s1 > s2
 */
int ft_strcmp(const char *s1, const char *s2);

/**
 * Copy a null-terminated string from src to dst.
 *
 * Copies the string pointed to by src, including the null terminator,
 * to the buffer pointed to by dst. The caller must ensure dst has
 * enough space.
 *
 * @param dst Destination buffer
 * @param src Source string
 * @return Pointer to dst, or NULL if either argument is NULL
 */
char *ft_strcpy(char *dst, const char *src);

/*
 * Internal assembly implementations
 * These symbols are exported from the .s files and should not be
 * called directly - use the wrapper functions above instead.
 */
unsigned int ft_strlen_asm(const char *s);
int ft_strcmp_asm(const char *s1, const char *s2);
char *ft_strcpy_asm(char *dst, const char *src);

char	*ft_strncpy(char *dst, const char *src, unsigned int len);

#ifdef __cplusplus
}
#endif
