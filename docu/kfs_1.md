# KFS 1 - Grub, Boot and Screen

This document maps the KFS 1 subject to the repository state at the
time this milestone was being built.
It records only the boot, linker, build, and screen-output pieces that
belong to KFS 1.

## PDF Scope

The KFS 1 subject requires:

- a GRUB-bootable kernel using Multiboot v1
- an ASM boot entry and a custom linker script
- a freestanding kernel build with ASM and C sources
- screen output from kernel code
- a Makefile that produces the final bootable image
- documentation and a small image size limit


### Boot Chain

Flow:

`grub.cfg` -> `src/boot/entry.s` -> `src/kernel/main.c`

- GRUB loads `myos42.bin` from `/boot/myos42.bin`.
- `src/boot/entry.s` contains the Multiboot v1 header and the `_start`
  entry point.
- `_start` sets up the stack and calls `kernel_main`.
- `src/boot/linker.ld` places the Multiboot header early, loads the
  kernel at 1 MB, and reserves a 16 KB stack.

Key references:

- [grub.cfg](../grub.cfg)
- [entry.s](../src/boot/entry.s#L7)
- [linker.ld](../src/boot/linker.ld#L2)

### Kernel Entry

The kernel entry point is `kernel_main()` in
[src/kernel/main.c](../src/kernel/main.c#L28).

For KFS 1, the important part is the control handoff from the ASM boot
stub into C code, where the kernel can write directly to VGA memory.

### Screen Output

Screen output is implemented as a small OOP-style display object.

- [display.h](../src/kernel/display/display.h) defines `display_t`.
- [display_init()](../src/kernel/display/display.c#L58) initializes VGA
  text mode defaults at `0xb8000`.
- The internal `clear()` method fills the full screen with spaces.
- The internal `put_at()` method writes a character at a given cell.

This satisfies the KFS 1 requirement for basic screen output.

The original milestone expects a very small demo: boot, reach C code,
and print a visible marker on the screen.

## Build System

The Makefile already matches the KFS 1 requirements for a freestanding
kernel build.

- [Makefile](../Makefile#L38) uses `gcc`, `ld`, and `nasm`.
- It builds with `-m32`, `-ffreestanding`, `-fno-builtin`, and
  `-nostdlib`.
- It links the kernel with the custom linker script.
- It produces the final bootable image `myos42.iso`.

Key references:

- [compiler and linker flags](../Makefile#L38)
- [kernel sources](../Makefile#L74)
- [default build target](../Makefile#L213)