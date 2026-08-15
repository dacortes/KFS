// SPDX-License-Identifier: GPL-2.0

#include <gtest/gtest.h>
#include <cstring>
#include <kernel/interrupts/idt.h>

extern "C" void irq1_handler(void);

static int g_signal_called;
static unsigned int g_last_signal;
static void *g_last_context;

static void signal_stub(unsigned int signal, void *context)
{
	unsigned int *counter = (unsigned int *)context;

	g_signal_called++;
	g_last_signal = signal;
	g_last_context = context;
	if (counter != NULL)
		(*counter)++;
}

TEST(InterruptTableTest, SignalRegistrationAndScheduling)
{
	struct kernel_signal_context signal_context;
	struct kernel_signal_snapshot snapshot;

	memset(&signal_context, 0, sizeof(signal_context));
	memset(&snapshot, 0, sizeof(snapshot));
	g_signal_called = 0;
	g_last_signal = 0;
	g_last_context = NULL;

	EXPECT_EQ(kernel_register_signal_handler(KERNEL_SIG_DIVIDE_ERROR,
						 signal_stub, &signal_context), 0);
	EXPECT_EQ(kernel_schedule_signal(KERNEL_SIG_DIVIDE_ERROR,
					 &signal_context), 0);
	EXPECT_EQ(kernel_signal_dispatch(KERNEL_SIG_DIVIDE_ERROR, &snapshot), 0);
	EXPECT_EQ(g_signal_called, 1);
	EXPECT_EQ(g_last_signal, KERNEL_SIG_DIVIDE_ERROR);
	EXPECT_EQ(g_last_context, &signal_context);
}

TEST(InterruptTableTest, PanicCleanupAndStackSnapshot)
{
	struct kernel_stack_snapshot snapshot;
	unsigned int registers[8] = {0x12345678, 0x9abcdef0, 0x0, 0x1,
						 0x2, 0x3, 0x4, 0x5};

	memset(&snapshot, 0, sizeof(snapshot));
	kernel_set_registers(registers, 8);
	kernel_clear_registers();
	kernel_save_stack(&snapshot);

	EXPECT_GT(snapshot.depth, 0u);
	EXPECT_GT(snapshot.bytes_saved, 0u);
}
TEST(InterruptTableTest, IDTEntryAndSignalQueueSmokeTest)
{
	unsigned int divide_count = 0;
	unsigned int gp_count = 0;
	unsigned int page_count = 0;
	unsigned int dispatch_count = 0;

	g_signal_called = 0;
	g_last_signal = 0;
	g_last_context = NULL;

	idt_init();
	idt_set_gate(0x21, (unsigned int)irq1_handler, 0x08, 0x8E);
	EXPECT_NE(idt_get_entry(0x21), 0u);
	EXPECT_NE(idt_get_entry(0x80), 0u);
	EXPECT_NE(idt_get_entry(KERNEL_SIG_GENERAL_PROTECTION), 0u);
	EXPECT_NE(idt_get_entry(KERNEL_SIG_PAGE_FAULT), 0u);

	EXPECT_EQ(kernel_register_signal_handler(KERNEL_SIG_DIVIDE_ERROR,
							 signal_stub, &divide_count), 0);
	EXPECT_EQ(kernel_register_signal_handler(KERNEL_SIG_GENERAL_PROTECTION,
							 signal_stub, &gp_count), 0);
	EXPECT_EQ(kernel_register_signal_handler(KERNEL_SIG_PAGE_FAULT,
							 signal_stub, &page_count), 0);
	EXPECT_EQ(kernel_schedule_signal(KERNEL_SIG_DIVIDE_ERROR, &divide_count), 0);
	EXPECT_EQ(kernel_schedule_signal(KERNEL_SIG_GENERAL_PROTECTION, &gp_count), 0);
	EXPECT_EQ(kernel_schedule_signal(KERNEL_SIG_PAGE_FAULT, &page_count), 0);

	kernel_exception_dispatch(KERNEL_SIG_DIVIDE_ERROR);
	kernel_exception_dispatch(KERNEL_SIG_GENERAL_PROTECTION);
	kernel_exception_dispatch(KERNEL_SIG_PAGE_FAULT);

	dispatch_count = g_signal_called;
	EXPECT_GE(dispatch_count, 3u);
	EXPECT_GE(divide_count, 1u);
	EXPECT_GE(gp_count, 1u);
	EXPECT_GE(page_count, 1u);
}