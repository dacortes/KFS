; SPDX-License-Identifier: GPL-2.0

global reboot_system

section .text
[BITS 32]

reboot_system:
	cli

.wait_input_clear:
	in al, 0x64
	test al, 0x02
	jnz .wait_input_clear

	mov al, 0xFE
	out 0x64, al

.hang:
	hlt
	jmp .hang
