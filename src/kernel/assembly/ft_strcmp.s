; SPDX-License-Identifier: GPL-2.0

; ft_strcmp.s - Compare two null-terminated strings (i386)
;
; int ft_strcmp_asm(const char *s1, const char *s2);
;
; cdecl calling convention (32-bit):
;   [esp+4] = s1
;   [esp+8] = s2
; Returns:
;   eax: <0 if s1 < s2, 0 if equal, >0 if s1 > s2
;
; Note: Called by C wrapper in ft_strcmp.c

section .text
	global ft_strcmp_asm

ft_strcmp_asm:
	push	ebp
	mov	ebp, esp
	push	esi
	push	edi

	mov	edi, [ebp + 8]		; edi = s1
	mov	esi, [ebp + 12]		; esi = s2

.loop:
	movzx	eax, byte [edi]		; load byte from s1
	movzx	edx, byte [esi]		; load byte from s2

	cmp	eax, edx		; compare bytes
	jne	.diff			; jump if different

	test	al, al			; check for NUL terminator
	je	.equal			; both ended -> equal

	inc	edi
	inc	esi
	jmp	.loop

.diff:
	ja	.greater		; unsigned: s1 > s2
	mov	eax, -1			; s1 < s2
	jmp	.done

.greater:
	mov	eax, 1			; s1 > s2
	jmp	.done

.equal:
	xor	eax, eax		; return 0

.done:
	pop	edi
	pop	esi
	pop	ebp
	ret
