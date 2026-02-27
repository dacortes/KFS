// SPDX-License-Identifier: GPL-2.0

#include <kernel/wrappers/helper.h>

int	ft_atoi(const char *str)
{
	int				s;
	unsigned int	c;
	long int		r;

	s = 1;
	c = 0;
	r = 0;
	while (str[c] && (str[c] == ' ' || str[c] == '\n'
			|| str[c] == '\t' || str[c] == '\v' || str[c] == '\f'
			|| str[c] == '\r'))
		c++;
	if (str[c] == '+' || str[c] == '-')
		if (str[c++] == '-')
			s *= -1;
	while (ft_isdigit(str[c]))
	{
		r = (r * 10) + str[c] - 48;
		c++;
	}
	return (s * r);
}