#ifndef _STDINT_H
# define STDINT_H

/* Signed types */
typedef signed char	int8_t;
typedef signed short	int16_t;
typedef signed int	int32_t;
typedef signed long	int64_t;

/* Unsigned types */
typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned int	uint32_t;
typedef unsigned long	uint64_t;

/* Size types */
typedef uint32_t	size_t;

/* Boolean */
typedef enum {
	false = 0,
	true = 1
} bool;

#define NULL ((void*)0)
#endif
