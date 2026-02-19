; SPDX-License-Identifier: GPL-2.0

; idt.s - Load the Interrupt Descriptor Table (i386)
;
; void idt_load(unsigned int idt_ptr_addr);
;
; Executes the lidt instruction to load the IDT register
; with the descriptor pointed to by idt_ptr_addr.

[BITS 32]

section .text
    global idt_load

;**
; * Load the IDT register
; *
; * @input [esp+4] - Address of idt_ptr structure (6 bytes)
; * @clobber none
;**/
idt_load:
    push    ebp
    mov     ebp, esp
    
    mov     eax, [ebp + 8]      ; eax = address of idt_ptr
    lidt    [eax]               ; Load IDT register
    
    pop     ebp
    ret