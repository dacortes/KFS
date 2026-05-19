global return_to_kernel_mode
global syscall_handler

extern printf
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
	call printf
	add esp, 4

	; We are done with the user->kernel return request.
	; Do not unwind the interrupt frame here: restore original kernel call stack directly.
	mov esp, [kernel_return_esp]
	pop ebp
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	sti			; re-enable IRQs: int gate cleared IF on syscall entry
	ret

section .rodata
	syscall_zero_msg db "[SYSCALL] Returning from user mode to kernel", 10, 0
