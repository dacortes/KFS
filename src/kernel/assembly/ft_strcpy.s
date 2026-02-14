; SPDX-License-Identifier: GPL-2.0

; ft_strcpy.s - Copy a null-terminated string (i386)
;
; char *ft_strcpy_asm(char *dst, const char *src);
;
; cdecl calling convention (32-bit):
;   [esp+4] = dst
;   [esp+8] = src
; Returns:
;   eax: pointer to dst, or NULL if either argument is NULL
;
; Note: Called by C wrapper in ft_strcpy.c

section .text
	global ft_strcpy_asm

ft_strcpy_asm:
	push	ebp
	mov	ebp, esp
	push	esi
	push	edi

	mov	edi, [ebp + 8]		; edi = dst
	mov	esi, [ebp + 12]		; esi = src

	test	edi, edi		; check dst for NULL
	jz	.null
	test	esi, esi		; check src for NULL
	jz	.null

	mov	eax, edi		; save dst as return value

.copy_loop:
	mov	dl, [esi]		; load byte from src
	mov	[edi], dl		; store byte into dst
	inc	esi
	inc	edi
	test	dl, dl			; was it NUL?
	jnz	.copy_loop

	pop	edi
	pop	esi
	pop	ebp
	ret

.null:
	xor	eax, eax		; return NULL
	pop	edi
	pop	esi
	pop	ebp
	ret
