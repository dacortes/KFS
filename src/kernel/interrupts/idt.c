// SPDX-License-Identifier: GPL-2.0

/**
 * @file idt.c
 * @brief Interrupt Descriptor Table implementation
 */

#include <kernel/interrupts/idt.h>

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtp;

extern void idt_load(unsigned int idt_ptr_addr);

/**
 * Set an entry in the IDT
 *
 * @param num Interrupt number (0-255)
 * @param base Address of the interrupt handler function
 * @param selector Code segment selector (usually 0x08 for kernel code)
 * @param flags Type and attributes (0x8E = 32-bit interrupt gate, ring 0)
 */
void idt_set_gate(unsigned char num, unsigned int base,
		  unsigned short selector, unsigned char flags)
{
	idt[num].base_low = base & 0xFFFF;
	idt[num].base_high = (base >> 16) & 0xFFFF;
	idt[num].selector = selector;
	idt[num].zero = 0;
	idt[num].flags = flags;
}

/**
 * Initialize the Interrupt Descriptor Table
 *
 * Sets up the IDT pointer and loads it into the CPU using the lidt
 * instruction. Clears all IDT entries to prepare for handler registration.
 */
void idt_init(void)
{
	unsigned int i;

	idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
	idtp.base = (unsigned int)&idt;

	for (i = 0; i < IDT_ENTRIES; i++)
		idt_set_gate(i, 0, 0, 0);

	idt_load((unsigned int)&idtp);
}