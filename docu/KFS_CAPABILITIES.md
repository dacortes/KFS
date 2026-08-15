# KFS Capabilities

This document will track the capabilities of the KFS project by milestone.
Each section will be filled in as the project evolves.

## KFS 0

- Capability: TBD
- Notes: TBD
- Relevant files: TBD

## KFS 1

- Capability: GRUB boot, ASM entry, linker, and VGA screen output.
- Notes: Documented in [kfs_1.md](kfs_1.md).
- Relevant files: [kfs_1.md](kfs_1.md), [kfs_1.pdf](kfs_1.pdf)

## KFS 2

- Capability: GDT, stack setup, TSS, and stack printing.
- Notes: Documented in [kfs_2.md](kfs_2.md).
- Relevant files: [kfs_2.md](kfs_2.md), [kfs_2.pdf](kfs_2.pdf)

## KFS 3

- Capability: Interrupt Descriptor Table, interrupt stubs, PIC remap, IRQ routing, and exception handling probes.
- Notes: The interrupt subsystem now includes a kernel exception signal queue, registration and dispatch logic, and a safe runtime probe for observing handlers without corrupting the VM. The full discussion is recorded in [kfs_3.md](kfs_3.md).
- Relevant files: [kfs_3.md](kfs_3.md), [src/kernel/interrupts/idt.c](../src/kernel/interrupts/idt.c), [src/kernel/interrupts/idt.h](../src/kernel/interrupts/idt.h), [src/kernel/assembly/idt.s](../src/kernel/assembly/idt.s), [src/kernel/assembly/isr.s](../src/kernel/assembly/isr.s), [src/shell/builtins/idt_probe.c](../src/shell/builtins/idt_probe.c)

## KFS 4

- Capability: TBD
- Notes: TBD
- Relevant files: TBD

## KFS 5

- Capability: TBD
- Notes: TBD
- Relevant files: TBD

## KFS 6

- Capability: TBD
- Notes: TBD
- Relevant files: TBD

## KFS 7

- Capability: TBD
- Notes: TBD
- Relevant files: TBD

## KFS 8

- Capability: TBD
- Notes: TBD
- Relevant files: TBD

## KFS 9

- Capability: TBD
- Notes: TBD
- Relevant files: TBD