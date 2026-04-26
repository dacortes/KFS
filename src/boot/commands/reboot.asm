global reboot_system

section .text
[BITS 32]

reboot_system:
	cli

.wait:
	in al, 0x64
	test al, 0x02
	jnz .wait

	mov al, 0xFE
	out 0x64, al
	jmp $ 