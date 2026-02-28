// SPDX-License-Identifier: GPL-2.0

#include <kernel/wrappers/helper.h>

void	*ft_memchr(const void *s, int c, unsigned int n)
{
	unsigned int	i;

	i = 0;
	while (i < n) {
		if (((unsigned char *)s)[i] == (unsigned char)c)
			return (((unsigned char *)s) + i);
		i++;
	}
	return (NULL);
}
