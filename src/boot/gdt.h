#pragma once

#include <stdint.h>

/**
 * @brief Selectors (index * 8)
 * All *_SELs refer to selector and only to these.
 */
#define NULL_SEL		0x00
#define KERNEL_CODE_SEL	0x08
#define KERNEL_DATA_SEL	0x10
#define KERNEL_CODE_SEL	0x18

#define USER_CODE_SEL	0x20
#define USER_DATA_SEL	0x28
#define USER_STACK_SEL	0x30


/**
 * @brief structure of an 8-byte descriptor
 * @param limit_low		limit	0	-	15
 * @param base_low		base	0	-	15
 * @param base_middle	base	16	-	23
 * @param access_byte	access	0	-	7
 * access rights (P, DPL, S, Type)
 * @param flags_limit	Granularity flag	16	-	19
 * @param base_high		base				24	-	31
 */
struct segment_selector {
	uint16_t	limit_low;
	uint16_t    base_low;
	uint8_t		base_middle;
	uint8_t		access_byte;

} __attribute__((packed));

