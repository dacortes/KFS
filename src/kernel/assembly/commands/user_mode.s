global switch_to_user_mode
global get_current_privilege_level

section .text
[BITS 32]

switch_to_user_mode:
	push ebp;
	mov ebp, esp;

	mov eax, [ebp + 8] ; user code segment selector
	mov edx, [ebp + 12] ; user stack pointer

	mov ecx, cs
	and ecx, 0x3 ; get current privilege level
	cmp ecx, 3 ; check if already in user mode
	je .already_user

	cli

	push dword 0x23 ; user data segment (SS)
	push edx ; user stack pointer (ESP)
	pushfd ; push EFLAGS
	or dword [esp], 0x200 ; set IF flag to enable interrupts
	push dword 0x1B ; user code segment (CS)
	push dword eax ; user instruction pointer (EIP)

	iret ; return to user mode

	.already_user:
		pop ebp
		ret

get_current_privilege_level:
	mov eax, cs
	and eax, 0x3 ; return the current privilege level (CPL)
	ret