// SPDX-License-Identifier: GPL-2.0

#include "print.h"

/**
 * @file formats.c
 * @brief Format helper implementations for `printf`-like output.
 *
 * Contains format handlers used by the lightweight `printf` above.
 */

int char_format(char c, int *count)
{
	if (write_redirectable(&c, 1) == -1)
		return (-1);
	*count = (*count + 1);
	return *count;
}

int str_format(const char *str, int *count)
{
	int i;
	const char *s;

	i = 0;
	s = str ? str : "(nill)";
	while (s[i]) {
		if (char_format(s[i], count) == -1)
			return -1;
		i++;
	}
	return *count;
}

int	base_number_format(unsigned int n, int *count, int opt, unsigned int bas)
{
	char	*base;

	if (opt == 0)
		base = "0123456789";
	if (opt == 1)
		base = "0123456789abcdef";
	if (opt == 2)
		base = "0123456789ABCDEF";
	if (n >= bas) {
		if (base_number_format(n / bas, count, opt, bas) == -1)
			return -1;
	}
	if (char_format(base[n % bas], count) == -1)
		return -1;
	return *count;
}

int int_format(int n, int *count)
{
	unsigned int	nbr;

	if (n == -2147483648) {
		if (str_format("-2147483648", count) == -1)
			return -1;
		return *count;
	} else if (n < 0) {
		if (char_format('-', count) == -1)
			return -1;
		nbr = -n;
	} else
		nbr = n;
	return base_number_format(nbr, count, 0, 10);
}

int	ptr_format(unsigned long n, int *count)
{
	if (str_format("0x", count) == -1)
		return -1;
	if (base_number_format(n, count, 1, 16) == -1)
		return -1;
	return *count;
}

int	formats(va_list *args, char const type, int *count)
{
	/**
	 * @brief Dispatch a single format specifier.
	 *
	 * This function consumes the appropriate argument from `*args` and
	 * forwards it to the corresponding formatter.
	 */
	if (type == '%')
		*count = char_format('%', count);
	if (type == 'c')
		*count = char_format(va_arg(*args, int), count);
	if (type == 's')
		*count = str_format(va_arg(*args, char *), count);
	if (type == 'd' || type == 'i')
		*count = int_format(va_arg(*args, int), count);
	if (type == 'u')
		*count = base_number_format(va_arg(*args, unsigned int), count, 0, 10);
	if (type == 'x')
		*count = base_number_format(va_arg(*args, unsigned int), count, 1, 16);
	if (type == 'X')
		*count = base_number_format(va_arg(*args, unsigned int), count, 2, 16);
	if (type == 'p')
		*count = ptr_format(va_arg(*args, unsigned long), count);
	return *count;
}
