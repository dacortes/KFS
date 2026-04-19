; SPDX-License-Identifier: GPL-2.0

; gdt.s - Load the Global Descriptor Table (i386)
;
; void gdt_load(unsigned int gdt_ptr_addr);
;
; Loads GDTR from the pointer, then reloads segment registers for the
; new kernel code/data segments.

[BITS 32]

section .text
	global gdt_load
	global gdt_enter_user_mode
	global gdt_user_mode_entry
	global gdt_kernel_data_access_attempt
	extern gdt_user_demo_state
	extern gdt_user_demo_buffer

gdt_load:
	push ebp
	mov ebp, esp

	mov eax, [ebp + 8]
	lgdt [eax] ; Load the GDT register with the address of the GDT pointer

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	jmp 0x08:reload_cs

reload_cs:
	pop ebp
	ret

gdt_enter_user_mode:
	push ebp
	mov ebp, esp

	mov eax, [ebp + 8]
	mov edx, [ebp + 12]

	push dword 0x23
	push dword edx
	pushfd
	or dword [esp], 0x200
	push dword 0x1B
	push dword eax
	iretd

gdt_user_mode_entry:
	; Ring 3: use user data segment first.
	mov ax, 0x23
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; This proves ring 3 can access its own data segment.
	mov byte [gdt_user_demo_state], 1
	mov byte [gdt_user_demo_buffer], 'U'

	gdt_kernel_data_access_attempt:
	; Ring 3: this is the exact instruction that should fault.
	mov ax, 0x10
	mov ds, ax
	hlt
	jmp gdt_user_mode_entry
