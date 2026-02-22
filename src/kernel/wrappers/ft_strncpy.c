#include <kernel/wrappers/helper.h>
#include <stdint.h>

char	*ft_strncpy(char *dst, const char *src, unsigned int len)
{
	unsigned int    i;

	if (!dst || !src)
		return NULL;	
	i = 0;
	while (src[i] && i < len)
	{
		dst[i] = src[i];
		i++;
	}
	if (i < len && !src[i])
	{
		while (dst[i])
			dst[i++] = '\0';
	}
	return (dst);
}
