#pragma once

#include <stdint.h>

/**
 * @brief Selectors (index * 8)
 * All *_SELs refer to selector and only to these.
 */
#define NULL_SEL			0x00
#define KERNEL_CODE_SEL		0x08
#define KERNEL_DATA_SEL		0x10
#define KERNEL_STACK_SEL	0x18   

#define USER_CODE_SEL		0x20
#define USER_DATA_SEL		0x28
#define USER_STACK_SEL		0x30


/**
 * @brief structure of an 8-byte descriptor

 * Intel x86 segment descriptor format (8 bytes):
 * 
 * Byte:    0      1      2      3      4      5      6      7
 *        +------+------+-------+------+-------+-------+------+------+
 *        |limit | base | base  |access|flags  | base  |      |      |
 *        |low   | low  |middle | byte |+limit | high  | (unused?)   |
 *        |(0-15)|(0-15)|(16-23)|      |(16-19)|(24-31)|             |
 *        +------+------+-------+------+-------+-------+------+------+


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
	uint8_t		flags_limit_high;
    uint8_t		base_high;

} __attribute__((packed));

/**
 * @brief GDTR pointer structure (needs packing to be 6 bytes)
 * 
 * The LGDT instruction expects a 6-byte structure:
 * - 2 bytes: limit (size of GDT - 1)
 * - 4 bytes: linear base address of GDT
 */
struct gdtr_ptr {
	uint16_t	limit;
	uint32_t	base;
} __attribute__((packed));

/**
 * @brief Function to load the GDT (implemented in entry.s)
 * 
 * @param gdt_ptr Pointer to the gdt_ptr structure
 */
extern void gdt_flush(uint32_t gdt_ptr);

/**
 * @brief Function to initialize the GDT
 */
void init_gdt(void);
