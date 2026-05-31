// SPDX-License-Identifier: GPL-2.0

/**
 * @file idt.c
 * @brief Interrupt Descriptor Table implementation
 */

#include <kernel/interrupts/idt.h>
#include <kernel/interrupts/gdt.h>
#include <kernel/wrappers/commands.h>
#include <print.h>

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtp;

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

	/* General protection fault handler (#GP, vector 13). */
	idt_set_gate(13, (unsigned int)gp_fault_handler,
		     GDT_KERNEL_CODE_SELECTOR, 0x8E);

	/* Syscall handler - interrupt 0x80, callable from Ring 3 (user mode)
	 * 0xEE = Present | Ring 3 | 32-bit interrupt gate
	 */
	idt_set_gate(0x80, (unsigned int)syscall_handler,
		     GDT_KERNEL_CODE_SELECTOR, 0xEE);

	idt_load((unsigned int)&idtp);
}

/**
 * Load the IDT into the CPU
 *
 * Uses the lidt instruction to load the IDT pointer.
 *
 * @param idt_ptr Address of the idt_ptr structure containing base and limit
 */
int handle_syscall(void)
{
	printf("[KERNEL] Syscall called from Ring %d\n", get_current_privilege_level());
	return 0;
}
