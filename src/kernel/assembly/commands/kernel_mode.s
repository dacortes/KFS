global return_to_kernel_mode
global syscall_handler

section .text
[BITS 32]

return_to_kernel_mode:
	int 0x80 ; trigger a software interrupt to return to kernel mode
	ret

syscall_handler:
	pushad ; save all general-purpose registers
	push ds ; save data segment register
	push es ; save extra segment register
	push fs ; save fs segment register
	push gs ; save gs segment register

	; Handle the system call here (e.g., read syscall number from eax, arguments from other registers)
	mov ax, 0x10 ; kernel data segment selector
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	