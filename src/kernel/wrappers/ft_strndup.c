// SPDX-License-Identifier: GPL-2.0

#include <helper.h>
#include <print.h>

/**
 * @brief Duplicates a string up to a specified number of characters.
 *
 * This function allocates memory for a new string and copies at most `n`
 * characters from the source string `src`. The new string is always
 * null‑terminated. The allocated memory must be freed by the caller when
 * no longer needed.
 *
 * @param src Pointer to the source string to duplicate. Must be a valid
 *            null‑terminated string. If NULL, the function returns NULL.
 * @param n   Maximum number of characters to copy from `src` (not including
 *            the terminating null byte). The resulting string will have
 *            a length of at most `n`.
 *
 * @return    On success, a pointer to the newly allocated string containing
 *            the duplicated content. On failure (memory exhaustion), NULL is
 *            returned. If `src` is NULL, NULL is returned.
 *
 * @note      This function uses ft_calloc to allocate memory, so the
 *            allocated block is zero‑initialised. If `n` is greater than
 *            or equal to the length of `src`, the entire source string is
 *            duplicated (including its null terminator, since the allocation
 *            always reserves space for it). If `src` is shorter than `n`,
 *            the duplication stops at the null terminator.
 *
 * @warning   The caller is responsible for freeing the allocated memory
 *            using vfree or the appropriate deallocator. Not freeing the
 *            memory will result in a memory leak.
 */

char	*ft_strndup(const char *src, size_t n)
{
	if (!src)
		return NULL;

	size_t	len = ft_strlen(src);

	if (n < len)
		len = n;

	char *dst = (char *)ft_calloc(len + 1, sizeof(char));

	if (!dst)
		return NULL;

	ft_memcpy(dst, src, len);
	printf("size dst %u\n", sizeof(dst));
	// for (uint32_t i = 0; i < len; i++)
	// 	dst[i] = src[i];

	printf("=======> src = *%s* dst = *%s*  %x size = %u len = %u\n", src, dst, dst, n, len);
	return dst;
}
