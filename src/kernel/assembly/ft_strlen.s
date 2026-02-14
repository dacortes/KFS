; SPDX-License-Identifier: GPL-2.0

; ft_strlen.s - Calculate length of a null-terminated string (i386)
;
; size_t ft_strlen_asm(const char *s);
;
; cdecl calling convention (32-bit):
;   [esp+4] = s (pointer to string)
; Returns:
;   eax = length of string (0 if NULL)
;
; Note: Called by C wrapper in ft_strlen.c

section .text
	global ft_strlen_asm

ft_strlen_asm:
	push	ebp
	mov	ebp, esp
	push	edi

	mov	edi, [ebp + 8]		; edi = s
	test	edi, edi		; check for NULL
	jz	.null

	xor	eax, eax		; eax = 0 (counter)

.loop:
	cmp	byte [edi + eax], 0	; compare current byte to NUL
	je	.done			; if zero, end of string
	inc	eax			; increment length
	jmp	.loop			; repeat

.done:
	pop	edi
	pop	ebp
	ret

.null:
	xor	eax, eax		; return 0
	pop	edi
	pop	ebp
	ret
