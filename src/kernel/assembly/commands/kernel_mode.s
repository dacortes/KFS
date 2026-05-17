global return_to_kernel_mode
global syscall_handler

extern print
extern kernel_return_eip
extern kernel_return_esp

section .text
[BITS 32]

return_to_kernel_mode:
	int 0x80 ; trigger a software interrupt to return to kernel mode
	ret

syscall_handler:
	; CPU automatically pushed: EIP, CS, EFLAGS, ESP, SS
	; at [esp], [esp+4], [esp+8], [esp+12], [esp+16]
	
	; Save all general-purpose registers
	pushad
	push ds
	push es
	push fs
	push gs

	; Check syscall number before loading segment selectors.
	cmp eax, 0
	je .exit_user_mode

	; Set kernel data segments
	mov ax, 0x10 ; kernel data segment selector
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; For non-zero syscalls, just return to user mode
	pop gs
	pop fs
	pop es
	pop ds
	popad
	iret

.exit_user_mode:
	push dword syscall_zero_msg
	call print
	add esp, 4
	
	; Restore all registers (reverse order of push)
	pop gs
	pop fs
	pop es
	pop ds
	popad

	; Restore the original kernel stack and resume the caller directly.
	mov esp, [kernel_return_esp]
	mov ebp, [esp]
	ret

section .rodata
	syscall_zero_msg db "[SYSCALL] cmp eax, 0 -> zero branch", 10, 0
	syscall_nonzero_msg db "[SYSCALL] cmp eax, 0 -> nonzero branch", 10, 0

