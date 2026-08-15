// SPDX-License-Identifier: GPL-2.0

#include <builtins.h>
#include <kernel/interrupts/idt.h>

static void idt_probe_handler(unsigned int signal, void *context)
{
	unsigned int *counter = (unsigned int *)context;

	if (counter != NULL)
		(*counter)++;
	printf("  [IDT] handler fired: signal=0x%x count=%u\n",
		signal, counter != NULL ? *counter : 0u);
}

static void idt_fault_handler(unsigned int signal, void *context)
{
	unsigned int *counter = (unsigned int *)context;

	if (counter != NULL)
		(*counter)++;
	printf("  [IDT] real handler fired: signal=0x%x count=%u\n",
		signal, counter != NULL ? *counter : 0u);
}

int cmd_idt_probe(shell_t *self)
{
	unsigned int divide_count = 0;
	unsigned int gp_count = 0;
	unsigned int page_count = 0;

	(void)self;

	/* The real CPU exception stubs in exceptions.s call hlt after dispatch,
	 * so triggering a true divide-by-zero would halt the kernel immediately.
	 * For the terminal demo we intentionally show the signal queue and
	 * handler dispatch path without invoking the real fault stub.
	 */
	printf("[IDT] signal queue demonstration\n");
	printf("  [IDT] registering handlers for divide_error, general_protection, page_fault\n");
	if (kernel_register_signal_handler(KERNEL_SIG_DIVIDE_ERROR,
					idt_probe_handler, &divide_count) != 0)
		return -1;
	if (kernel_register_signal_handler(KERNEL_SIG_GENERAL_PROTECTION,
					idt_probe_handler, &gp_count) != 0)
		return -1;
	if (kernel_register_signal_handler(KERNEL_SIG_PAGE_FAULT,
					idt_probe_handler, &page_count) != 0)
		return -1;
	if (kernel_schedule_signal(KERNEL_SIG_DIVIDE_ERROR, &divide_count) != 0)
		return -1;
	if (kernel_schedule_signal(KERNEL_SIG_GENERAL_PROTECTION, &gp_count) != 0)
		return -1;
	if (kernel_schedule_signal(KERNEL_SIG_PAGE_FAULT, &page_count) != 0)
		return -1;

	printf("  [IDT] queue: divide_error, general_protection, page_fault\n");
	kernel_exception_dispatch(KERNEL_SIG_DIVIDE_ERROR);
	kernel_exception_dispatch(KERNEL_SIG_GENERAL_PROTECTION);
	kernel_exception_dispatch(KERNEL_SIG_PAGE_FAULT);
	printf("  [IDT] final counts: divide_error=%u general_protection=%u page_fault=%u\n",
		divide_count, gp_count, page_count);
	printf("[IDT] demonstration complete\n");
	return 0;
}

int cmd_idt_fault(shell_t *self)
{
	unsigned int divide_count = 0;

	(void)self;

	printf("[IDT] WARNING: this command intentionally triggers a divide-by-zero fault.\n");
	printf("[IDT] registering the real divide-by-zero handler before the fault.\n");
	if (kernel_register_signal_handler(KERNEL_SIG_DIVIDE_ERROR,
					idt_fault_handler, &divide_count) != 0) {
		printf("[IDT] failed to register divide-by-zero handler\n");
		return -1;
	}
	printf("[IDT] executing real divide-by-zero now...\n");

	__asm__ volatile (
		"xor %%ecx, %%ecx\n\t"
		"xor %%edx, %%edx\n\t"
		"mov $1, %%eax\n\t"
		"cdq\n\t"
		"idiv %%ecx\n\t"
		:
		:
		: "eax", "ecx", "edx");

	printf("[IDT] divide-by-zero returned unexpectedly\n");
	return 0;
}
