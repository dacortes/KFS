// SPDX-License-Identifier: GPL-2.0

#include <kernel/wrappers/helper.h>

uint16_t ft_isblank(char c)
{
	return (c == ' ' || c == '\t');
}
