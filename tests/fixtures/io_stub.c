// SPDX-License-Identifier: GPL-2.0

/**
 * @file io_stub.c
 * @brief Test stubs for I/O and PIC functions
 */

#include <stdint.h>
#include <kernel/interrupts/gdt.h>

/**
 * Stub for inb - not used in keyboard unit tests
 */
uint8_t inb(uint16_t port)
{
	(void)port;
	return 0;
}

/**
 * Stub for outb - not used in keyboard unit tests
 */
void outb(uint16_t port, uint8_t val)
{
	(void)port;
	(void)val;
}

/**
 * Stub for io_wait - not used in keyboard unit tests
 */
void io_wait(void)
{
}

/**
 * Stub for pic_send_eoi - not used in keyboard unit tests
 */
void pic_send_eoi(unsigned char irq)
{
	(void)irq;
}

/**
 * Stub for idt_init - not used in unit tests
 */
void idt_init(void)
{
}

/**
 * Stub for gdt_init - not used in unit tests
 */
void gdt_init(void)
{
}

/**
 * Stub for gdt_get_gdtr - not used in unit tests
 */
struct gdt_ptr gdt_get_gdtr(void)
{
	struct gdt_ptr result = {0, 0};
	return result;
}

/**
 * Stub for gdt_verify - not used in unit tests
 */
int gdt_verify(void)
{
	return 0;
}

/**
 * Stub for gdt_log_descriptors - not used in unit tests
 */
void gdt_log_descriptors(void)
{
}

/**
 * Stub for gdt_run_privilege_demo - not used in unit tests
 */
void gdt_run_privilege_demo(void)
{
}

/**
 * Stub for pic_init - not used in unit tests
 */
void pic_init(void)
{
}

/**
 * Stub for idt_set_gate - not used in unit tests
 */
void idt_set_gate(unsigned char num, unsigned int base,
		  unsigned short selector, unsigned char flags)
{
	(void)num;
	(void)base;
	(void)selector;
	(void)flags;
}

/**
 * Stub for irq1_handler - not used in unit tests
 */
void irq1_handler(void)
{
}
