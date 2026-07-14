// SPDX-License-Identifier: GPL-2.0

/**
 * @file io_stub.c
 * @brief Test stubs for I/O and PIC functions
 */

#include <stdint.h>
#include <stddef.h>
#include <kernel/interrupts/gdt.h>
#include <setjmp.h>

int g_halt_system_calls;
int g_reboot_system_calls;
int g_switch_to_user_mode_calls;
int g_return_to_kernel_mode_calls;
int g_get_current_privilege_level_calls;
int g_current_privilege_level_value;
void (*g_last_user_mode_function)(void);
void *g_last_user_mode_stack_top;
int g_write_redirectable_calls;
static jmp_buf g_halt_jmp;
static jmp_buf g_reboot_jmp;

void reset_builtin_stub_state(void)
{
	g_halt_system_calls = 0;
	g_reboot_system_calls = 0;
	g_switch_to_user_mode_calls = 0;
	g_return_to_kernel_mode_calls = 0;
	g_get_current_privilege_level_calls = 0;
	g_current_privilege_level_value = 0;
	g_last_user_mode_function = NULL;
	g_last_user_mode_stack_top = NULL;
	g_write_redirectable_calls = 0;
}

void halt_system(void)
{
	g_halt_system_calls++;
	longjmp(g_halt_jmp, 1);
}

void reboot_system(void)
{
	g_reboot_system_calls++;
	longjmp(g_reboot_jmp, 1);
}

void switch_to_user_mode(void (*function)(void), void *stack_top)
{
	g_switch_to_user_mode_calls++;
	g_last_user_mode_function = function;
	g_last_user_mode_stack_top = stack_top;
}

void return_to_kernel_mode(void)
{
	g_return_to_kernel_mode_calls++;
}

int get_current_privilege_level(void)
{
	g_get_current_privilege_level_calls++;
	return g_current_privilege_level_value;
}

int __attribute__((weak)) write_redirectable(const char *text, unsigned int count)
{
	(void)text;
	g_write_redirectable_calls++;
	return (int)count;
}

/* Install a jmp handler for halt_system so tests can continue. Returns
 * 0 when installing; returns non-zero when returned via longjmp. */
int install_halt_jmp(void)
{
	return setjmp(g_halt_jmp);
}

int install_reboot_jmp(void)
{
	return setjmp(g_reboot_jmp);
}

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

uint32_t endkernel = 0x100000; 
