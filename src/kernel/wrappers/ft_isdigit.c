// SPDX-License-Identifier: GPL-2.0

#include <kernel/wrappers/helper.h>

int	ft_isdigit(int c)
{
	if (c >= '0' && c <= '9')
		return (1);
	else
		return (0);
}
