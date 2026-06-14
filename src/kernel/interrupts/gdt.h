// SPDX-License-Identifier: GPL-2.0

/**
 * @file gdt.h
 * @brief Global Descriptor Table (GDT) management for i386.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define GDT_ENTRIES 8

#define GDT_KERNEL_CODE_SELECTOR 0x08
#define GDT_KERNEL_DATA_SELECTOR 0x10
#define GDT_KERNEL_STACK_SELECTOR 0x18
#define GDT_USER_CODE_SELECTOR 0x23
#define GDT_USER_DATA_SELECTOR 0x2B
#define GDT_USER_STACK_SELECTOR 0x33
#define GDT_TSS_SELECTOR 0x38

/**
 * @brief Single GDT descriptor (8 bytes).
 */
struct gdt_entry {
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_middle;
	unsigned char access;
	unsigned char granularity;
	unsigned char base_high;
} __attribute__((packed));

/**
 * @brief GDTR format consumed by the lgdt instruction.
 */
struct gdt_ptr {
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));

/**
 * @brief Task State Segment used for privilege transitions.
 */
struct tss_entry {
	unsigned int prev_tss;
	unsigned int esp0;
	unsigned int ss0;
	unsigned int esp1;
	unsigned int ss1;
	unsigned int esp2;
	unsigned int ss2;
	unsigned int cr3;
	unsigned int eip;
	unsigned int eflags;
	unsigned int eax;
	unsigned int ecx;
	unsigned int edx;
	unsigned int ebx;
	unsigned int esp;
	unsigned int ebp;
	unsigned int esi;
	unsigned int edi;
	unsigned int es;
	unsigned int cs;
	unsigned int ss;
	unsigned int ds;
	unsigned int fs;
	unsigned int gs;
	unsigned int ldt;
	unsigned short trap;
	unsigned short iomap_base;
} __attribute__((packed));

/**
 * @brief Top of the kernel stack exported by the boot linker script.
 */
extern char stack_top;

/**
 * @brief Initialize and load a flat kernel GDT.
 *
 * Installs eight entries:
 *  - null descriptor
 *  - kernel code segment (selector 0x08, DPL=0)
 *  - kernel data segment (selector 0x10, DPL=0)
 *  - kernel stack segment (selector 0x18, DPL=0)
 *  - user code segment (selector 0x23, DPL=3)
 *  - user data segment (selector 0x2B, DPL=3)
 *  - user stack segment (selector 0x33, DPL=3)
 *  - task state segment (selector 0x38, DPL=0)
 */
void gdt_init(void);

/**
 * @brief Load the GDT and reload CPU segment registers.
 *
 * @param gdt_ptr_addr Address of struct gdt_ptr.
 */
void gdt_load(unsigned int gdt_ptr_addr);

/**
 * @brief Retrieve the current GDTR contents via sgdt instruction.
 *
 * @return struct gdt_ptr with limit and base of loaded GDT.
 */
struct gdt_ptr gdt_get_gdtr(void);

/**
 * @brief Verify that the GDT was correctly loaded into the CPU.
 *
 * @return Nonzero if GDTR matches expected GDT address and size, 0 otherwise.
 */
int gdt_verify(void);

/**
 * @brief Start a small user-mode demo that intentionally triggers a GP fault.
 *
 * The demo transitions to ring 3 and tries to load a kernel data selector,
 * which should fault and prove that privilege checks are enforced.
 */
void gdt_run_privilege_demo(void);

/**
 * @brief Flag written from ring 3 to prove user data access works.
 */
extern unsigned char gdt_user_demo_state;

/**
 * @brief Small buffer written from ring 3 before triggering the fault.
 */
extern unsigned char gdt_user_demo_buffer[16];

/**
 * @brief Kernel handler for the demo's general protection fault.
 *
 * Prints a diagnostic message and halts.
 */
void gdt_handle_gp_fault(void);

/**
 * @brief Structure to hold stack pointer and segment information.
 */
struct gdt_stack_info {
	unsigned int esp;
	unsigned int ss;
	unsigned int ebp;
	const char *label;
};

/**
 * @brief Capture current stack pointer, segment, and frame pointer.
 *
 * Reads ESP, SS, and EBP registers via inline assembly.
 *
 * @return struct gdt_stack_info with current stack state.
 */
struct gdt_stack_info gdt_get_stack_info(const char *label);

/**
 * @brief Print kernel stack information in human-friendly format.
 *
 * Displays kernel stack pointer (ESP0 from TSS), kernel stack segment,
 * and frame pointer to show the kernel stack state.
 */
void gdt_print_kernel_stack(void);

/**
 * @brief Print user stack information in human-friendly format.
 *
 * Displays user stack pointer and stack segment for comparison.
 */
void gdt_print_user_stack(void);

/**
 * @brief Demonstrate kernel vs user stacks by capturing both contexts.
 *
 * Prints kernel stack info, transitions to ring 3 where stack information
 * is captured to globals, then displays both kernel and user stack info
 * for comparison to show they use different stacks and segment selectors.
 */
void gdt_run_stack_demo(void);

/**
 * @brief Globals storing captured user stack state from ring 3.
 *
 * Populated by gdt_user_stack_demo_entry when running in ring 3.
 */
extern unsigned int gdt_user_stack_esp;
extern unsigned int gdt_user_stack_ss;
extern unsigned int gdt_user_stack_ebp;

/**
 * @brief Enter ring 3 using the supplied entry point and user stack top.
 *
 * @param entry User-mode entry address.
 * @param stack_top User-mode stack top address.
 */
void gdt_enter_user_mode(unsigned int entry, unsigned int stack_top);

/**
 * @brief Ring 3 demo entry point used by the privilege proof.
 */
void gdt_user_mode_entry(void);

#ifdef __cplusplus
}
#endif
