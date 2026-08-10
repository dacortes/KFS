// SPDX-License-Identifier: GPL-2.0

#include <helper.h>

/**
 * @brief Copies a block of memory from source to destination.
 *
 * This function copies `n` bytes from the memory area pointed to by `src`
 * to the memory area pointed to by `dst`. The memory areas must not overlap.
 * If they do, the behaviour is undefined. The function returns a pointer to
 * the destination buffer.
 *
 * @param dst Pointer to the destination memory area. May be NULL; if so,
 *            the function returns NULL without performing any copy.
 * @param src Pointer to the source memory area. May be NULL; if so,
 *            the function returns NULL without performing any copy.
 * @param n   Number of bytes to copy.
 *
 * @return    Pointer to `dst` on success. If either `dst` or `src` is NULL,
 *            NULL is returned.
 *
 * @note      This implementation does not handle overlapping memory regions.
 *            For copying overlapping areas, use ft_memmove instead.
 *
 * @warning   If both pointers are non‑NULL, they must point to valid memory
 *            regions of at least `n` bytes. The function does not perform
 *            any additional validation beyond the NULL checks.
 */
void *ft_memcpy(void *dst, const void *src, size_t n)
{
	size_t i = 0;

	if (!dst || !src)
		return NULL;
	while (i < n)
	{
		((unsigned char *)dst)[i] = ((unsigned char *)src)[i];
		i++;
	}
	return dst;
}
