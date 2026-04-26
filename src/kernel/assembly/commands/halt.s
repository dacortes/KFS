; SPDX-License-Identifier: GPL-2.0

global halt_system

section .text
[BITS 32]

halt_system:
	cli
	hlt
	ret