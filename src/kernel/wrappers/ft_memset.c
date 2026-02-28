// SPDX-License-Identifier: GPL-2.0
/**
 * @file ft_memset.c
 * @brief Custom implementation of memset for kernel use
 *
 * Provides a simple byte-wise memory set function to initialize buffers.
 * This is used in place of the standard library's memset to avoid
 * dependencies on external libraries in the kernel.
 */
#include <kernel/wrappers/helper.h>

void	*ft_memset(void *b, int c, unsigned int len)
{
	unsigned int	i;

	i = 0;
	while (i < len) {
		((unsigned char *)b)[i] = c;
		i++;
	}
	return (b);
}
