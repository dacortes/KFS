// SPDX-License-Identifier: GPL-2.0

/**
 * @file pic.c
 * @brief 8259 PIC implementation
 */

#include <kernel/interrupts/pic.h>
#include <kernel/io/io.h>


/**
 * Initialize the 8259 PIC
 *
 * Performs initialization sequence to remap PIC interrupts
 * and configure cascade mode.
 */
void pic_init(void)
{
	outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
	io_wait();

	outb(PIC1_DATA, 0x20);
	io_wait();
	outb(PIC2_DATA, 0x28);
	io_wait();

	outb(PIC1_DATA, 0x04);
	io_wait();
	outb(PIC2_DATA, 0x02);
	io_wait();

	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	outb(PIC1_DATA, 0xFD);
	outb(PIC2_DATA, 0xFF);
}

/**
 * Send End of Interrupt signal to PIC
 *
 * @param irq IRQ number (0-15)
 */
void pic_send_eoi(unsigned char irq)
{
	if (irq >= 8)
		outb(PIC2_CMD, PIC_EOI);

	outb(PIC1_CMD, PIC_EOI);
}

/**
 * Mask (disable) a specific IRQ
 *
 * @param irq IRQ number to mask (0-15)
 */
void pic_mask_irq(unsigned char irq)
{
	unsigned short port;
	unsigned char value;

	if (irq < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		irq -= 8;
	}

	value = inb(port) | (1 << irq);
	outb(port, value);
}

/**
 * Unmask (enable) a specific IRQ
 *
 * @param irq IRQ number to unmask (0-15)
 */
void pic_unmask_irq(unsigned char irq)
{
	unsigned short port;
	unsigned char value;

	if (irq < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		irq -= 8;
	}

	value = inb(port) & ~(1 << irq);
	outb(port, value);
}
