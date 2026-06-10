; boot/entry.s
[BITS 32]

; =========================
; Multiboot v1 header
; =========================
section .multiboot
align 4
    dd 0x1BADB002              ; magic number (Multiboot v1)
    dd 0x2                     ; flags (request extended memory map)
    dd -(0x1BADB002 + 0x2)     ; checksum

; =========================
; Code section
; =========================
section .text
global _start
extern kernel_main
extern stack_top

_start:
    cli                        ; disable interrupts (safety)

    mov esp, stack_top         ; set stack pointer

    ; Multiboot convention:
    ; eax = magic
    ; ebx = multiboot_info pointer
    push ebx                   ; 2nd argument
    push eax                   ; 1st argument
    call kernel_main

.hang:
    hlt
    jmp .hang

