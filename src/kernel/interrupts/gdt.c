// SPDX-License-Identifier: GPL-2.0

/**
 * @file gdt.c
 * @brief Global Descriptor Table (GDT) implementation.
 */

#include <kernel/interrupts/gdt.h>
#include <kernel/print/print.h>

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gdtp;
static struct tss_entry tss;
static unsigned char user_demo_stack[4096];

unsigned char gdt_user_demo_state;
unsigned char gdt_user_demo_buffer[16];

/**
 * @brief Configure a GDT entry.
 *
 * @param num Descriptor index.
 * @param base Segment base address.
 * @param limit Segment limit.
 * @param access Descriptor access byte.
 * @param gran Granularity flags.
 */
static void gdt_set_gate(unsigned int num, unsigned int base,
			 unsigned int limit, unsigned char access,
			 unsigned char gran)
{
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = (limit >> 16) & 0x0F;
	gdt[num].granularity |= gran & 0xF0;
	gdt[num].access = access;
}

/**
 * @brief Load the task register with the GDT TSS selector.
 */
static void gdt_load_tss(void)
{
	unsigned short tss_selector;

	tss_selector = GDT_TSS_SELECTOR;
	__asm__ volatile("ltr %0" : : "m"(tss_selector));
}

/**
 * @brief Initialize and load a flat kernel GDT.
 */
void gdt_init(void)
{
	unsigned int tss_base;

	gdtp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
	gdtp.base = (unsigned int)&gdt;

	gdt_set_gate(0, 0, 0, 0, 0);

	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

	tss_base = (unsigned int)&tss;
	tss.esp0 = (unsigned int)&stack_top;
	tss.ss0 = GDT_KERNEL_DATA_SELECTOR;
	tss.iomap_base = sizeof(struct tss_entry);
	gdt_set_gate(5, tss_base, sizeof(struct tss_entry) - 1, 0x89, 0x00);

	gdt_load((unsigned int)&gdtp);
	gdt_load_tss();
}

/**
 * @brief Read the current GDTR from the CPU via sgdt.
 *
 * @return Struct with limit and base of the currently loaded GDT.
 */
struct gdt_ptr gdt_get_gdtr(void)
{
	struct gdt_ptr result;

	__asm__ volatile("sgdt %0" : "=m"(result));
	return result;
}

/**
 * @brief Verify the GDT was loaded correctly into the CPU.
 *
 * Reads GDTR and confirms it matches the GDT we installed.
 *
 * @return Nonzero (true) if GDTR matches, 0 (false) otherwise.
 */
int gdt_verify(void)
{
	struct gdt_ptr current;

	current = gdt_get_gdtr();

	if (current.base != (unsigned int)&gdt)
		return 0;
	if (current.limit != gdtp.limit)
		return 0;
	return 1;
}

/**
 * @brief Extract the DPL (Descriptor Privilege Level) from an access byte.
 *
 * @param access Descriptor access value.
 * @return DPL value (0-3, higher = less privileged).
 */
static unsigned char gdt_get_dpl(unsigned char access)
{
	return (access >> 5) & 0x3;
}

/**
 * @brief Extract the P (Present) flag from an access byte.
 *
 * @param access Descriptor access value.
 * @return Nonzero if descriptor is present, 0 otherwise.
 */
static unsigned char gdt_get_present(unsigned char access)
{
	return (access >> 7) & 0x1;
}

/**
 * @brief Extract the S (Descriptor type) field from an access byte.
 *
 * @param access Descriptor access value.
 * @return Nonzero if code/data segment, 0 if system segment.
 */
static unsigned char gdt_get_system(unsigned char access)
{
	return (access >> 4) & 0x1;
}

/**
 * @brief Log GDT diagnostic info showing each descriptor's key properties.
 *
 * Reads the GDT directly from the CPU via GDTR to verify actual loaded state.
 * Useful for verifying GDT structure at boot time.
 */
void gdt_log_descriptors(void)
{
	struct gdt_ptr current_gdtr;
	struct gdt_entry *cpu_gdt;
	unsigned int i;
	unsigned int max_entries;

	current_gdtr = gdt_get_gdtr();
	cpu_gdt = (struct gdt_entry *)current_gdtr.base;

	max_entries = (current_gdtr.limit + 1) / sizeof(struct gdt_entry);

	printf("[GDT] Reading from CPU GDTR: base=0x%x limit=0x%x (%d entries)\n",
	       current_gdtr.base, current_gdtr.limit, (int)max_entries);

	for (i = 0; i < max_entries && i < GDT_ENTRIES; i++) {
		unsigned char access;
		unsigned char dpl;

		access = cpu_gdt[i].access;
		dpl = gdt_get_dpl(access);

		if (i == 0) {
			printf("[GDT] Entry %d: null descriptor\n", (int)i);
		} else {
			printf("[GDT] Entry %d: access=0x%02x P=%d DPL=%d S=%d\n",
			       (int)i, access,
			       (int)gdt_get_present(access),
			       (int)dpl,
			       (int)gdt_get_system(access));
		}
	}
}

/**
 * @brief Handle the demo's general protection fault.
 */
void gdt_handle_gp_fault(void)
{
	printf("[GDT] User data access succeeded: state=%u buffer=%c\n",
	       gdt_user_demo_state, gdt_user_demo_buffer[0]);
	printf("[GDT] Fault occurred at gdt_kernel_data_access_attempt\n");
	printf("[GDT] Instruction: mov ds, 0x10 in ring 3\n");
	for (;;)
		__asm__ volatile("hlt");
}

/**
 * @brief Start a ring-3 demo that intentionally triggers a GP fault.
 */
void gdt_run_privilege_demo(void)
{
	unsigned int user_stack_top;

	gdt_user_demo_state = 0;
	gdt_user_demo_buffer[0] = '?';
	gdt_user_demo_buffer[1] = '\0';

	user_stack_top = (unsigned int)&user_demo_stack[sizeof(user_demo_stack)];
	printf("[GDT] Entering ring 3 demo; user data should work first\n");
	gdt_enter_user_mode((unsigned int)gdt_user_mode_entry, user_stack_top);
}
