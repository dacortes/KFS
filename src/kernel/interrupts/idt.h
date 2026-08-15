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

#ifdef __cplusplus
extern "C" {
#endif

#define IDT_ENTRIES 256
#define KERNEL_EXCEPTION_COUNT 32
#define KERNEL_SIGNAL_MAX_HANDLERS 16
#define KERNEL_SIGNAL_QUEUE_SIZE 32

#define KERNEL_SIG_DIVIDE_ERROR 0x00
#define KERNEL_SIG_DEBUG 0x01
#define KERNEL_SIG_NMI 0x02
#define KERNEL_SIG_BREAKPOINT 0x03
#define KERNEL_SIG_OVERFLOW 0x04
#define KERNEL_SIG_BOUND_RANGE 0x05
#define KERNEL_SIG_INVALID_OPCODE 0x06
#define KERNEL_SIG_DEVICE_NOT_AVAILABLE 0x07
#define KERNEL_SIG_DOUBLE_FAULT 0x08
#define KERNEL_SIG_COPROCESSOR_SEGMENT_OVERRUN 0x09
#define KERNEL_SIG_INVALID_TSS 0x0A
#define KERNEL_SIG_SEGMENT_NOT_PRESENT 0x0B
#define KERNEL_SIG_STACK_SEGMENT_FAULT 0x0C
#define KERNEL_SIG_GENERAL_PROTECTION 0x0D
#define KERNEL_SIG_PAGE_FAULT 0x0E
#define KERNEL_SIG_RESERVED_15 0x0F
#define KERNEL_SIG_MATH_FAULT 0x10
#define KERNEL_SIG_ALIGNMENT_CHECK 0x11
#define KERNEL_SIG_MACHINE_CHECK 0x12
#define KERNEL_SIG_SIMD_FLOATING_POINT 0x13
#define KERNEL_SIG_VIRTUALIZATION 0x14
#define KERNEL_SIG_CONTROL_PROTECTION 0x15
#define KERNEL_SIG_RESERVED_22 0x16
#define KERNEL_SIG_RESERVED_23 0x17
#define KERNEL_SIG_RESERVED_24 0x18
#define KERNEL_SIG_RESERVED_25 0x19
#define KERNEL_SIG_RESERVED_26 0x1A
#define KERNEL_SIG_RESERVED_27 0x1B
#define KERNEL_SIG_RESERVED_28 0x1C
#define KERNEL_SIG_RESERVED_29 0x1D
#define KERNEL_SIG_RESERVED_30 0x1E
#define KERNEL_SIG_RESERVED_31 0x1F

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
unsigned int idt_get_entry(unsigned int vector);
int idt_call_handler(unsigned int vector);

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

/**
 * Handle a system call
 *
 * This function is called when a system call is received from user mode.
 */
int handle_syscall(void);

struct kernel_signal_context {
	unsigned int signal;
	unsigned int error_code;
	unsigned int eip;
	unsigned int cs;
	unsigned int eflags;
	unsigned int esp;
	unsigned int ss;
	unsigned int registers[16];
	unsigned int register_count;
};

struct kernel_signal_snapshot {
	unsigned int signal;
	unsigned int error_code;
	unsigned int eip;
	unsigned int cs;
	unsigned int eflags;
	unsigned int esp;
	unsigned int ss;
};

struct kernel_stack_snapshot {
	unsigned int depth;
	unsigned int bytes_saved;
	unsigned int stack_pointer;
	unsigned int stack_top;
	unsigned int words[16];
};

typedef void (*kernel_signal_handler_t)(unsigned int signal, void *context);

int kernel_register_signal_handler(unsigned int signal,
					kernel_signal_handler_t handler,
					void *context);
int kernel_schedule_signal(unsigned int signal, void *context);
int kernel_signal_dispatch(unsigned int signal,
				 struct kernel_signal_snapshot *snapshot);
void kernel_set_registers(const unsigned int *regs, unsigned int count);
void kernel_clear_registers(void);
void kernel_save_stack(struct kernel_stack_snapshot *snapshot);
void kernel_exception_dispatch(unsigned int vector);

extern void gp_fault_handler(void);
extern void syscall_handler(void);
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

#ifdef __cplusplus
}
#endif
