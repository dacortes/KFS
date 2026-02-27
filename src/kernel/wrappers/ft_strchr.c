// SPDX-License-Identifier: GPL-2.0

#include <kernel/wrappers/helper.h>

char	*ft_strchr(const char *s, int c)
{
	return (ft_memchr(s, c, ft_strlen(s) + 1));
}