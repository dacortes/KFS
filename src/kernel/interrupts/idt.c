// SPDX-License-Identifier: GPL-2.0

/**
 * @file idt.c
 * @brief Interrupt Descriptor Table implementation
 */

#include <kernel/interrupts/idt.h>
#include <kernel/interrupts/gdt.h>
#include <kernel/wrappers/commands.h>
#include <kernel/wrappers/helper.h>
#include <print.h>

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtp;
static kernel_signal_handler_t g_signal_handlers[KERNEL_SIGNAL_MAX_HANDLERS];
static void *g_signal_contexts[KERNEL_SIGNAL_MAX_HANDLERS];
static unsigned int g_signal_ids[KERNEL_SIGNAL_MAX_HANDLERS];
static unsigned int g_signal_handler_count;
static unsigned int g_pending_signals[KERNEL_SIGNAL_QUEUE_SIZE];
static void *g_pending_contexts[KERNEL_SIGNAL_QUEUE_SIZE];
static unsigned int g_pending_signal_count;
static unsigned int g_register_window[16];
static unsigned int g_register_window_count;

/**
 * Return the raw handler address stored for a given IDT vector.
 *
 * @param vector Interrupt vector number to inspect.
 * @return Encoded base address for the handler, or 0 when invalid.
 */
unsigned int idt_get_entry(unsigned int vector)
{
	if (vector >= IDT_ENTRIES)
		return 0;
	return (unsigned int)idt[vector].base_low |
		((unsigned int)idt[vector].base_high << 16);
}

/**
 * Dispatch a kernel exception or interrupt through the registered handler path.
 *
 * @param vector Exception or interrupt vector to trigger.
 * @return 0 on success, -1 when the vector does not exist.
 */
int idt_call_handler(unsigned int vector)
{
	unsigned int entry = idt_get_entry(vector);

	printf("[IDT] idt_call_handler: vector=0x%x entry=0x%x\n",
		vector, entry);
	if (entry == 0)
		return -1;
	if (vector < KERNEL_EXCEPTION_COUNT)
		kernel_exception_dispatch(vector);
	return 0;
}

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

	idt_set_gate(0, (unsigned int)isr0, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(1, (unsigned int)isr1, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(2, (unsigned int)isr2, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(3, (unsigned int)isr3, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(4, (unsigned int)isr4, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(5, (unsigned int)isr5, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(6, (unsigned int)isr6, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(7, (unsigned int)isr7, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(8, (unsigned int)isr8, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(9, (unsigned int)isr9, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(10, (unsigned int)isr10, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(11, (unsigned int)isr11, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(12, (unsigned int)isr12, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(13, (unsigned int)isr13, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(14, (unsigned int)isr14, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(15, (unsigned int)isr15, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(16, (unsigned int)isr16, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(17, (unsigned int)isr17, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(18, (unsigned int)isr18, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(19, (unsigned int)isr19, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(20, (unsigned int)isr20, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(21, (unsigned int)isr21, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(22, (unsigned int)isr22, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(23, (unsigned int)isr23, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(24, (unsigned int)isr24, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(25, (unsigned int)isr25, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(26, (unsigned int)isr26, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(27, (unsigned int)isr27, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(28, (unsigned int)isr28, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(29, (unsigned int)isr29, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(30, (unsigned int)isr30, GDT_KERNEL_CODE_SELECTOR, 0x8E);
	idt_set_gate(31, (unsigned int)isr31, GDT_KERNEL_CODE_SELECTOR, 0x8E);

	/* Syscall handler - interrupt 0x80, callable from Ring 3 (user mode)
	 * 0xEE = Present | Ring 3 | 32-bit interrupt gate
	 */
	idt_set_gate(0x80, (unsigned int)syscall_handler,
		     GDT_KERNEL_CODE_SELECTOR, 0xEE);

	printf("[IDT] init: base=0x%x limit=%u entries=%u\n",
		(unsigned int)idtp.base, idtp.limit, IDT_ENTRIES);
	idt_load((unsigned int)&idtp);
	printf("[IDT] loaded into CPU\n");
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

/**
 * Register a handler for a kernel exception signal.
 *
 * @param signal Exception signal to observe.
 * @param handler Callback invoked when the signal is dispatched.
 * @param context User-provided state passed to the callback.
 * @return 0 on success, -1 on invalid or full registration state.
 */
int kernel_register_signal_handler(unsigned int signal,
					kernel_signal_handler_t handler,
					void *context)
{
	unsigned int i;

	if (handler == 0 || signal >= KERNEL_EXCEPTION_COUNT)
		return -1;

	for (i = 0; i < g_signal_handler_count; i++) {
		if (g_signal_ids[i] == signal) {
			g_signal_handlers[i] = handler;
			g_signal_contexts[i] = context;
			return 0;
		}
	}

	if (g_signal_handler_count >= KERNEL_SIGNAL_MAX_HANDLERS)
		return -1;

	g_signal_ids[g_signal_handler_count] = signal;
	g_signal_handlers[g_signal_handler_count] = handler;
	g_signal_contexts[g_signal_handler_count] = context;
	g_signal_handler_count++;
	printf("[IDT] registered handler: signal=0x%x slot=%u\n",
		signal, g_signal_handler_count - 1);
	return 0;
}

/**
 * Queue a pending signal for later dispatch.
 *
 * @param signal Exception vector to enqueue.
 * @param context Optional context associated with the queued signal.
 * @return 0 on success, -1 when the queue is full or vector is invalid.
 */
int kernel_schedule_signal(unsigned int signal, void *context)
{
	if (g_pending_signal_count >= KERNEL_SIGNAL_QUEUE_SIZE)
		return -1;
	if (signal >= KERNEL_EXCEPTION_COUNT)
		return -1;

	g_pending_signals[g_pending_signal_count] = signal;
	g_pending_contexts[g_pending_signal_count] = context;
	g_pending_signal_count++;
	printf("[IDT] queued signal: signal=0x%x queue_size=%u context=0x%x\n",
		signal, g_pending_signal_count, (unsigned int)context);
	return 0;
}

/**
 * Invoke every registered handler for a specific signal.
 *
 * @param signal Exception number to dispatch.
 * @param snapshot Optional snapshot container to populate.
 * @return 0 on success.
 */
int kernel_signal_dispatch(unsigned int signal,
				 struct kernel_signal_snapshot *snapshot)
{
	unsigned int i;

	if (snapshot != 0)
		ft_memset(snapshot, 0, sizeof(*snapshot));
	for (i = 0; i < g_signal_handler_count; i++) {
		if (g_signal_ids[i] == signal && g_signal_handlers[i] != 0) {
			printf("[IDT] dispatching handler: signal=0x%x handler_slot=%u context=0x%x\n",
				signal, i, (unsigned int)g_signal_contexts[i]);
			g_signal_handlers[i](signal, g_signal_contexts[i]);
			if (snapshot != 0)
				snapshot->signal = signal;
		}
	}
	return 0;
}

/**
 * Save a register window for later exception analysis.
 *
 * @param regs Register array to copy into the kernel snapshot window.
 * @param count Number of registers to copy.
 */
void kernel_set_registers(const unsigned int *regs, unsigned int count)
{
	unsigned int i;

	g_register_window_count = count < 16 ? count : 16;
	for (i = 0; i < g_register_window_count; i++)
		g_register_window[i] = (regs != 0) ? regs[i] : 0;
}

/**
 * Clear the saved register window used by exception tracing.
 */
void kernel_clear_registers(void)
{
	ft_memset(g_register_window, 0, sizeof(g_register_window));
	g_register_window_count = 0;
}

/**
 * Capture a snapshot of the processor stack for debugging and panic analysis.
 *
 * @param snapshot Output structure to receive stack state.
 */
void kernel_save_stack(struct kernel_stack_snapshot *snapshot)
{
	unsigned int *stack_ptr;
	unsigned int x;

	if (snapshot == 0)
		return;

	ft_memset(snapshot, 0, sizeof(*snapshot));
	__asm__ volatile("mov %%esp, %0" : "=r"(stack_ptr));
	snapshot->stack_pointer = (unsigned int)stack_ptr;
	snapshot->stack_top = snapshot->stack_pointer + 256;
	snapshot->depth = 0;
	for (x = 0; x < 16; x++) {
		snapshot->words[x] = stack_ptr[x];
		snapshot->depth++;
		if (snapshot->words[x] == 0)
			break;
	}
	snapshot->bytes_saved = snapshot->depth * sizeof(unsigned int);
}

/**
 * Dispatch all queued and registered handlers matching a vector.
 *
 * @param vector Exception vector to process.
 */
void kernel_exception_dispatch(unsigned int vector)
{
	struct kernel_signal_snapshot snapshot;
	unsigned int i;

	printf("[IDT] exception_dispatch: vector=0x%x pending=%u handlers=%u\n",
		vector, g_pending_signal_count, g_signal_handler_count);
	ft_memset(&snapshot, 0, sizeof(snapshot));
	snapshot.signal = vector;
	for (i = 0; i < g_pending_signal_count; i++) {
		if (g_pending_signals[i] == vector) {
			printf("[IDT] matching queued signal: signal=0x%x at index=%u\n",
				vector, i);
			kernel_signal_dispatch(g_pending_signals[i], &snapshot);
			g_pending_signals[i] = 0;
			g_pending_contexts[i] = 0;
		}
	}
	if (g_signal_handler_count == 0)
		return;
	for (i = 0; i < g_signal_handler_count; i++) {
		if (g_signal_ids[i] == vector && g_signal_handlers[i] != 0) {
			printf("[IDT] invoking registered handler: signal=0x%x slot=%u\n",
				vector, i);
			g_signal_handlers[i](vector, g_signal_contexts[i]);
		}
	}
}
