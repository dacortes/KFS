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

/* Globals for user stack capture in ring 3 demo */
unsigned int gdt_user_stack_esp;
unsigned int gdt_user_stack_ss;
unsigned int gdt_user_stack_ebp;

unsigned char gdt_user_demo_state;
unsigned char gdt_user_demo_buffer[16];


struct gdt_ptr gp;

struct s_gdt {
	unsigned int base;
	unsigned int limit;
	unsigned char access;
	unsigned char flags;
};

#define CREATE_SOURCE_GDT(dst, g_base, g_limit, g_access, g_flags) \
	(dst).base = (g_base); \
	(dst).limit = (g_limit); \
	(dst).access = (g_access); \
	(dst).flags = (g_flags); \

/**
 * @brief Configure a GDT entry.
 *
 * @param num Descriptor index.
 * @param base Segment base address.
 * @param limit Segment limit.
 * @param access Descriptor access byte.
 * @param gran Granularity flags.
 */
static void gdt_set_gate(int num, struct s_gdt source)
{
	gdt[num].base_low = (source.base & 0xFFFF);
	gdt[num].base_middle = (source.base >> 16) & 0xFF;
	gdt[num].base_high = (source.base >> 24) & 0xFF;

	gdt[num].limit_low = (source.limit & 0xFFFF);
	gdt[num].granularity = ((source.limit >> 16) & 0x0F) |
				   (source.flags << 4);
	gdt[num].access = source.access;
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
	struct s_gdt source;

	gdtp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
	gdtp.base = (unsigned int)&gdt;

	/* Descriptor 1: Kernel Code (base=0, limit=4GB, 32 bits) */
	CREATE_SOURCE_GDT(source, 0, 0xFFFFF, 0x9A, 0xC);

	gdt_set_gate(1, source);
	/* Descriptor 2: Kernel Data */
	CREATE_SOURCE_GDT(source, 0, 0xFFFFF, 0x92, 0xC);
	gdt_set_gate(2, source);
	/* Descriptor 3: Kernel Stack (same as data) */
	CREATE_SOURCE_GDT(source, 0, 0xFFFFF, 0x92, 0xC);
	gdt_set_gate(3, source);
	/* Descriptor 4: User Code (DPL=3) */
	CREATE_SOURCE_GDT(source, 0, 0xFFFFF, 0xFA, 0xC);
	gdt_set_gate(4, source);
	/* Descriptor 5: User Data (DPL=3) */
	CREATE_SOURCE_GDT(source, 0, 0xFFFFF, 0xF2, 0xC);
	gdt_set_gate(5, source);
	/* Descriptor 6: User Stack (DPL=3) */
	CREATE_SOURCE_GDT(source, 0, 0xFFFFF, 0xF2, 0xC);
	gdt_set_gate(6, source);

	/* Slot 7: Task State Segment */
	tss_base = (unsigned int)&tss;
	tss.esp0 = (unsigned int)&stack_top;
	tss.ss0 = GDT_KERNEL_DATA_SELECTOR;
	tss.iomap_base = sizeof(struct tss_entry);
	CREATE_SOURCE_GDT(source, tss_base,
			  sizeof(struct tss_entry) - 1,
			  0x89,
			  0x00);
	gdt_set_gate(7, source);

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
 * @brief Capture current stack pointer, segment, and frame pointer.
 *
 * Uses inline assembly to read ESP (stack pointer), SS (stack segment),
 * and EBP (frame pointer) registers.
 *
 * @param label Descriptive label for this stack capture.
 * @return struct gdt_stack_info with register values.
 */
struct gdt_stack_info gdt_get_stack_info(const char *label)
{
	struct gdt_stack_info info;
	unsigned int esp_val, ss_val, ebp_val;

	info.label = label;
	__asm__ volatile(
		"mov %%esp, %0\n\t"
		"mov %%ss, %1\n\t"
		"mov %%ebp, %2\n\t"
		: "=r"(esp_val), "=r"(ss_val), "=r"(ebp_val)
	);
	info.esp = esp_val;
	info.ss = ss_val;
	info.ebp = ebp_val;
	return info;
}

/**
 * @brief Print kernel stack information in human-friendly format.
 *
 * Displays kernel stack pointer from TSS, kernel stack segment selector,
 * and frame pointer.
 */
void gdt_print_kernel_stack(void)
{
	struct gdt_stack_info kernel_stack;

	kernel_stack = gdt_get_stack_info("Kernel Stack");
	printf("\n[STACK] === Kernel Stack Information ===");
	printf("\n[STACK] Label: %s\n", kernel_stack.label);
	printf("[STACK] ESP (Stack Pointer): 0x%x\n", kernel_stack.esp);
	printf("[STACK] SS  (Stack Segment): 0x%x (Kernel: 0x%x)\n",
	       kernel_stack.ss, GDT_KERNEL_DATA_SELECTOR);
	printf("[STACK] EBP (Frame Pointer): 0x%x\n", kernel_stack.ebp);
	printf("[STACK] TSS.esp0 (Ring 0 Stack): 0x%x\n",
	       (unsigned int)&stack_top);
	printf("[STACK] Stack Direction: decreasing (esp grows downward)\n");
}

/**
 * @brief Print user stack information in human-friendly format.
 *
 * Displays user stack pointer and user stack segment selector.
 */
void gdt_print_user_stack(void)
{
	struct gdt_stack_info user_stack;

	user_stack = gdt_get_stack_info("User Stack");
	printf("\n[STACK] === User Stack Information ===");
	printf("\n[STACK] Label: %s\n", user_stack.label);
	printf("[STACK] ESP (Stack Pointer): 0x%x\n", user_stack.esp);
	printf("[STACK] SS  (Stack Segment): 0x%x (User: 0x%x)\n",
	       user_stack.ss, GDT_USER_DATA_SELECTOR);
	printf("[STACK] EBP (Frame Pointer): 0x%x\n", user_stack.ebp);
	printf("[STACK] User demo stack top: 0x%x\n",
	       (unsigned int)&user_demo_stack[sizeof(user_demo_stack)]);
	printf("[STACK] Stack Direction: decreasing (esp grows downward)\n");
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

/**
 * @brief User-mode code that captures stack info and halts.
 *
 * This function (implemented in assembly) is called in ring 3.
 * Captures ESP, SS, EBP registers and stores them in globals.
 */
void gdt_user_stack_demo_entry(void);

/**
 * @brief Demonstrate kernel vs user stacks by capturing both contexts.
 *
 * Prints kernel stack before switching to ring 3, transitions to user
 * mode where stack info is captured to global variables, then displays
 * the captured user stack for comparison.
 */
void gdt_run_stack_demo(void)
{
	unsigned int user_stack_top;
	unsigned int kernel_esp_before;

	/* Capture kernel stack esp at this point */
	__asm__ volatile("mov %%esp, %0" : "=r"(kernel_esp_before));

	printf("\n\n========================================");
	printf("\n[STACK] KERNEL vs USER STACK DEMO\n");
	printf("========================================\n");

	/* Show kernel stack before privilege change */
	gdt_print_kernel_stack();

	/* Initialize capture variables */
	gdt_user_stack_esp = 0;
	gdt_user_stack_ss = 0;
	gdt_user_stack_ebp = 0;

	/* Prepare and enter user mode */
	user_stack_top = (unsigned int)&user_demo_stack[sizeof(user_demo_stack)];
	printf("\n[STACK] Transitioning from RING 0 to RING 3...\n");
	printf("[STACK] User stack will be initialized at: 0x%x\n",
	       user_stack_top);
	gdt_enter_user_mode((unsigned int)gdt_user_stack_demo_entry,
	                    user_stack_top);

	/* If we get here, display the captured user stack info */
	if (gdt_user_stack_esp != 0) {
		printf("\n[STACK] === User Stack Information (Captured) ===");
		printf("\n[STACK] ESP (Stack Pointer): 0x%x\n",
		       gdt_user_stack_esp);
		printf("[STACK] SS  (Stack Segment): 0x%x (User: 0x%x)\n",
		       gdt_user_stack_ss, GDT_USER_DATA_SELECTOR);
		printf("[STACK] EBP (Frame Pointer): 0x%x\n",
		       gdt_user_stack_ebp);
	}

	printf("\n[STACK] COMPARISON:\n");
	printf("[STACK] Kernel ESP was:  0x%x\n", kernel_esp_before);
	printf("[STACK] User   ESP was:  0x%x\n", gdt_user_stack_esp);
	printf("[STACK] Kernel SS:       0x%x vs User SS: 0x%x\n",
	       GDT_KERNEL_DATA_SELECTOR, GDT_USER_DATA_SELECTOR);
	printf("[STACK] -> DIFFERENT stacks AND segment selectors!\n");
	printf("\n[STACK] END DEMO\n");
	printf("==========================================\n\n");
}
