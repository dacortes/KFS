; boot/entry.s
[BITS 32]

KERNEL_BASE	equ 0xC0000000
PD_INDEX_HIGH	equ (KERNEL_BASE >> 22)

; =========================
; Multiboot v1 header
; =========================
section .multiboot
align 4
	dd 0x1BADB002			; magic number (Multiboot v1)
	dd 0x00000002			; flags (request extended memory map)
	dd -(0x1BADB002 + 0x00000002)	; checksum

; =========================
; Bootstrap page tables
; =========================
; These tables are linked at 0xC0... but loaded at 0x0010....
; Before paging is enabled we address them through their physical
; address (symbol - KERNEL_BASE).
section .data
align 4096
boot_page_directory:
	times 1024 dd 0

align 4096
boot_page_table0:
	times 1024 dd 0

; =========================
; Code section
; =========================
section .text
global _start
extern kernel_main
extern stack_top

_start:
	cli				; disable interrupts (safety)

	; Preserve Multiboot registers across page table setup.
	mov esi, eax			; esi = multiboot magic
	mov edi, ebx			; edi = multiboot_info pointer

	; ---------------------------------------------------------
	; Build a page table that maps the first 4 MiB of physical
	; memory 1:1. This covers the kernel image and the stack.
	; ---------------------------------------------------------
	mov edx, boot_page_table0 - KERNEL_BASE
	xor ecx, ecx			; page table index
	xor eax, eax			; physical address
.fill_table:
	mov ebx, eax
	or ebx, 0x003			; present | writable
	mov [edx + ecx * 4], ebx
	add eax, 0x1000
	inc ecx
	cmp ecx, 1024
	jb .fill_table

	; ---------------------------------------------------------
	; Install the same table at:
	;   PDE[0]   -> identity map 0x00000000 - 0x003FFFFF
	;   PDE[768] -> higher half 0xC0000000 - 0xC03FFFFF
	; ---------------------------------------------------------
	mov edx, boot_page_directory - KERNEL_BASE
	mov eax, boot_page_table0 - KERNEL_BASE
	or eax, 0x003			; present | writable
	mov [edx], eax			; PDE[0]
	mov [edx + PD_INDEX_HIGH * 4], eax ; PDE[768]

	; ---------------------------------------------------------
	; Load CR3 with the physical address of the page directory
	; and enable paging (CR0.PG).
	; ---------------------------------------------------------
	mov eax, boot_page_directory - KERNEL_BASE
	mov cr3, eax
	mov eax, cr0
	or eax, 0x80000000		; CR0.PG
	mov cr0, eax

	; ---------------------------------------------------------
	; Absolute jump into the higher half. A relative jump would
	; land at the physical address because both EIP values share
	; the same offset.
	; ---------------------------------------------------------
	mov eax, higher_half
	jmp eax

higher_half:
	; Keep the flat segments installed by GRUB. Reloading them
	; from a hard-coded selector is unsafe because GRUB's GDT
	; layout is not part of the Multiboot ABI. gdt_init() will
	; install the kernel GDT and reload every segment later.
	mov esp, stack_top		; high-half stack
	xor ebp, ebp
	cld

	; Multiboot convention:
	; eax = magic
	; ebx = multiboot_info pointer (physical address)
	push edi			; 2nd argument: multiboot_info
	push esi			; 1st argument: magic
	call kernel_main

.hang:
	hlt
	jmp .hang
