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
	extern kernel_return_esp

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
; * For faults coming from ring 3, recover to the saved kernel frame used by
; * the user-mode switch demo. For kernel faults, halt to avoid undefined state.
;**
gp_fault_handler:
	; #GP pushes an error code; saved CS is at [esp + 8]
	mov eax, [esp + 8]
	and eax, 0x3
	cmp eax, 0x3
	jne .kernel_fault

	; User-mode fault path: resume kernel caller saved by switch_to_user_mode.
	mov esp, [kernel_return_esp]
	pop ebp
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	cld
	sti
	ret

.kernel_fault:
	cli
	hlt
	jmp .kernel_fault
