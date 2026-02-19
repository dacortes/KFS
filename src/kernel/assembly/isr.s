; SPDX-License-Identifier: GPL-2.0

; isr.s - Interrupt Service Routine stubs (i386)
;
; Provides assembly wrappers for hardware interrupt handlers.
; These stubs save/restore CPU state and call C interrupt handlers.

[BITS 32]

section .text
	global irq1_handler
	extern keyboard_interrupt

;**
; * IRQ1 handler stub (Keyboard interrupt)
; *
; * Saves all registers, calls keyboard_interrupt() in C,
; * then restores state and returns from interrupt.
; * The C handler sends EOI to the PIC.
; *
; * @clobber none (all registers preserved)
;**/
irq1_handler:
	pusha				; Save all general-purpose registers
	call keyboard_interrupt		; Call C handler
	popa				; Restore all registers
	iret				; Return from interrupt
