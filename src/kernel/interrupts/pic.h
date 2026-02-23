// SPDX-License-Identifier: GPL-2.0

/**
 * @file pic.h
 * @brief 8259 Programmable Interrupt Controller (PIC) management
 *
 * Provides functions for initializing and managing the 8259A PIC chip,
 * which handles hardware interrupts (IRQs) on x86 systems.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define PIC1_CMD	0x20
#define PIC1_DATA	0x21
#define PIC2_CMD	0xA0
#define PIC2_DATA	0xA1

#define PIC_EOI		0x20

#define IRQ_KEYBOARD	1

#define ICW1_INIT	0x10
#define ICW1_ICW4	0x01
#define ICW4_8086	0x01


/**
 * Initialize the 8259 PIC
 *
 * Remaps PIC interrupts to avoid conflicts with CPU exceptions.
 * After remapping:
 * - Master PIC (IRQ 0-7)  -> interrupts 0x20-0x27
 * - Slave PIC  (IRQ 8-15) -> interrupts 0x28-0x2F
 *
 * Unmasks only the keyboard interrupt (IRQ1) by default.
 */
void pic_init(void);

/**
 * Send End of Interrupt signal to PIC
 *
 * Must be called at the end of every hardware interrupt handler
 * to notify the PIC that interrupt processing is complete.
 *
 * @param irq IRQ number (0-15)
 */
void pic_send_eoi(unsigned char irq);

/**
 * Mask (disable) a specific IRQ
 *
 * @param irq IRQ number to mask (0-15)
 */
void pic_mask_irq(unsigned char irq);

/**
 * Unmask (enable) a specific IRQ
 *
 * @param irq IRQ number to unmask (0-15)
 */
void pic_unmask_irq(unsigned char irq);

#ifdef __cplusplus
}
#endif
