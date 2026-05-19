global return_to_kernel_mode
global syscall_handler

extern print
extern kernel_return_esp

section .text
[BITS 32]

return_to_kernel_mode:
	int 0x80 ; trigger a software interrupt to return to kernel mode
	ret

syscall_handler:
	; CPU automatically pushed: EIP, CS, EFLAGS, ESP, SS
	pushad
	push ds
	push es
	push fs
	push gs

	; Always switch data segments to kernel selectors before touching kernel code/data.
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	cmp eax, 0
	je .exit_user_mode

	; Non-zero syscall: return to user mode through iret path.
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

	pop gs
	pop fs
	pop es
	pop ds
	popad

	; Restore original kernel stack saved before iret-to-user and resume caller.
	mov esp, [kernel_return_esp]
	pop ebp
	ret

section .rodata
	syscall_zero_msg db "[SYSCALL] Returning from user mode to kernel", 10, 0
