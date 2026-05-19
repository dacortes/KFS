; SPDX-License-Identifier: GPL-2.0

; isr.s - Interrupt Service Routine stubs (i386)
;
; Provides assembly wrappers for hardware interrupt handlers.
; These stubs save/restore CPU state and call C interrupt handlers.

[BITS 32]

section .text
	global irq1_handler
	global gp_fault_handler
	extern keyboard_interrupt
	extern gdt_handle_gp_fault

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

;**
; * General protection fault handler stub
; *
; * Dispatches to the C-level GDT proof handler. The handler does not
; * return.
;**
gp_fault_handler:
	pusha
	call gdt_handle_gp_fault

.hang:
	cli
	hlt
	jmp .hang
