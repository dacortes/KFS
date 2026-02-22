// SPDX-License-Identifier: GPL-2.0

/**
 * @file idt.h
 * @brief Interrupt Descriptor Table management
 *
 * Provides structures and functions for setting up and managing the
 * x86 Interrupt Descriptor Table (IDT), which maps interrupt numbers
 * to handler functions.
 */

#pragma once

#define IDT_ENTRIES 256

/**
 * IDT entry structure (8 bytes per entry)
 *
 * Describes a single interrupt gate in the IDT. The handler address
 * is split into base_low and base_high to match the x86 IDT format.
 */
struct idt_entry {
	unsigned short base_low;   /* Lower 16 bits of handler address */
	unsigned short selector;   /* Kernel code segment (usually 0x08) */
	unsigned char zero;        /* Always 0 */
	unsigned char flags;       /* Type and attributes (0x8E for int) */
	unsigned short base_high;  /* Upper 16 bits of handler address */
} __attribute__((packed));

/**
 * IDT pointer structure for lidt instruction
 *
 * 6-byte structure containing the size and address of the IDT.
 * This is what the lidt instruction actually loads.
 */
struct idt_ptr {
	unsigned short limit;      /* Size of IDT - 1 */
	unsigned int base;         /* Address of first IDT entry */
} __attribute__((packed));

/**
 * Initialize the Interrupt Descriptor Table
 *
 * Sets up the IDT pointer and loads it into the CPU using the lidt
 * instruction. Clears all IDT entries to prepare for handler registration.
 */
void idt_init(void);

/**
 * Set an entry in the IDT
 *
 * @param num Interrupt number (0-255)
 * @param base Address of the interrupt handler function
 * @param selector Code segment selector (usually 0x08 for kernel code)
 * @param flags Type and attributes (0x8E = 32-bit interrupt gate, ring 0)
 */
void idt_set_gate(unsigned char num, unsigned int base,
		  unsigned short selector, unsigned char flags);

/**
 * Load the IDT (assembly implementation)
 *
 * @param idt_ptr_addr Address of the idt_ptr structure
 */
void idt_load(unsigned int idt_ptr_addr);
