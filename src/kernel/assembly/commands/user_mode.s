global switch_to_user_mode
global get_current_privilege_level
global kernel_return_esp

section .text
[BITS 32]

section .bss
kernel_return_esp resd 1

section .text

switch_to_user_mode:
	push ebp
	mov ebp, esp

	mov eax, [ebp + 8]  ; user code entry point (in eax)
	mov edx, [ebp + 12] ; user stack pointer (in edx)
	mov [kernel_return_esp], esp

	mov ecx, cs
	and ecx, 0x3        ; get current privilege level
	cmp ecx, 3          ; check if already in user mode
	je .already_user

	cli


	; Load ring-3 data selectors before entering user code.
	; iret changes CS/SS/ESP/EFLAGS, but DS/ES/FS/GS keep previous values.
	mov cx, 0x2B
	mov ds, cx
	mov es, cx
	mov fs, cx
	mov gs, cx

	; Push user mode context for iret:
	; SS (user data segment), ESP, EFLAGS, CS (user code segment), EIP
	push dword 0x2B     ; user data segment (SS)
	push edx            ; user stack pointer (ESP)
	pushfd              ; push EFLAGS
	or dword [esp], 0x200 ; set IF flag to enable interrupts
	push dword 0x23     ; user code segment (CS)
	push dword eax      ; user instruction pointer (EIP)

	iret                ; switch to user mode
.already_user:
	pop ebp
	ret

get_current_privilege_level:
	mov eax, cs
	and eax, 0x3        ; return the current privilege level (CPL)
	ret