// SPDX-License-Identifier: GPL-2.0

/**
 * @file ft_strncpy.c
 * @brief Bounded string copy function
 */

#include <kernel/wrappers/helper.h>
#include <stdint.h>

/**
 * ft_strncpy - Copy at most len bytes from src to dst
 * @dst: Destination buffer
 * @src: Source string
 * @len: Maximum number of bytes to copy
 *
 * Copies up to len characters from src to dst. If src is shorter than len,
 * the remainder of dst is filled with null bytes. Unlike standard strncpy,
 * returns empty string if either pointer is NULL.
 *
 * Return: Pointer to dst, or empty string "" if either argument is NULL
 */
char	*ft_strncpy(char *dst, const char *src, unsigned int len)
{
	unsigned int    i;

	if (!dst || !src)
		return "";
	i = 0;
	while (i < len && src[i]) {
		dst[i] = src[i];
		i++;
	}
	if (i < len && !src[i]) {
		while (dst[i])
			dst[i++] = '\0';
	}
	return (dst);
}
