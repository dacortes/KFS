# KFS 3 - Interrupt Descriptor Table

This document maps the KFS 3 subject to the current repository state.
It records how the IDT, interrupt stubs, PIC remapping, and IRQ setup
are implemented today.

## PDF Scope

The KFS 3 subject requires:

- an Interrupt Descriptor Table (IDT) with 256 entries
- one descriptor per interrupt vector and a pointer structure for `lidt`
- interrupt handler stubs for exceptions and IRQs
- a remapped PIC so hardware IRQs use vectors 0x20..0x2F
- at least one real ISR (keyboard IRQ1) wired into the IDT
- the IDT to be loaded into the CPU at boot or runtime initialization

## Current Implementation

The repo already contains the KFS 3 logic in the interrupt subsystem.
The IDT definition lives in [../src/kernel/interrupts/idt.h](../src/kernel/interrupts/idt.h).

Key pieces:

- `IDT_ENTRIES` is set to 256.
- `struct idt_entry` models the x86 interrupt gate layout.
- `struct idt_ptr` matches the format consumed by `lidt`.
- `idt_init()` installs the table and loads it into the CPU.
- `idt_set_gate()` fills an entry with the handler base address,
  the kernel code selector, and the gate flags.

Relevant symbols:

- [idt.h](../src/kernel/interrupts/idt.h#L10)
- [struct idt_entry / idt_ptr](../src/kernel/interrupts/idt.h#L17)
- [idt_init()](../src/kernel/interrupts/idt.c#L40)
- [idt_set_gate()](../src/kernel/interrupts/idt.c#L17)
- [idt_load()](../src/kernel/assembly/idt.s#L13)

## IDT Layout

The IDT is a flat array of 256 descriptors:

- [src/kernel/interrupts/idt.c](../src/kernel/interrupts/idt.c#L12)
  declares `static struct idt_entry idt[IDT_ENTRIES];`
- [src/kernel/interrupts/idt.c](../src/kernel/interrupts/idt.c#L13)
  declares the `struct idt_ptr idtp;` used by `lidt`

Each table entry stores the following fields:

- `base_low`
- `selector`
- `zero`
- `flags`
- `base_high`

This matches the standard x86 interrupt-gate format and is packed to
avoid padding bytes between fields.

## Initialization Flow

The actual setup is implemented in [src/kernel/interrupts/idt.c](../src/kernel/interrupts/idt.c#L40).

- `idtp.limit` is set to `(sizeof(struct idt_entry) * IDT_ENTRIES) - 1`
- `idtp.base` is set to the address of the table itself
- every entry is cleared first with `idt_set_gate(i, 0, 0, 0)`
- the general-protection fault vector 13 is hooked to `gp_fault_handler`
- the software interrupt vector `0x80` is hooked to `syscall_handler`
- `idt_load((unsigned int)&idtp)` executes `lidt` and loads the table

Relevant references:

- [idt_init()](../src/kernel/interrupts/idt.c#L40)
- [GP fault gate](../src/kernel/interrupts/idt.c#L51)
- [syscall gate](../src/kernel/interrupts/idt.c#L57)
- [assembly loader](../src/kernel/assembly/idt.s#L21)

## PIC and IRQ Wiring

The IDT is not used in isolation: the PIC must be remapped before IRQs
are enabled.

- [src/kernel/interrupts/pic.c](../src/kernel/interrupts/pic.c#L13) remaps PIC1
  and PIC2 to vectors `0x20` and `0x28`.
- [src/kernel/system/system.c](../src/kernel/system/system.c#L172) installs the
  keyboard interrupt gate at `0x21`.
- The startup sequence calls `gdt_init()`, `idt_init()`, `pic_init()`, then
  registers the keyboard handler before enabling `sti`.

This is the important runtime sequence for a working keyboard interrupt:

`gdt_init()` -> `idt_init()` -> `pic_init()` -> `idt_set_gate(0x21, ...)`
-> `sti`

## Interrupt Stubs

The assembly stubs are implemented in [src/kernel/assembly/isr.s](../src/kernel/assembly/isr.s).

- `irq1_handler` saves registers, calls `keyboard_interrupt`, restores the
  register frame, and executes `iret`.
- `gp_fault_handler` handles general protection faults and resumes the saved
  user-mode context when applicable; otherwise it halts the CPU.

The syscall path is implemented in [src/kernel/assembly/commands/kernel_mode.s](../src/kernel/assembly/commands/kernel_mode.s):

- `syscall_handler` saves state and restores the kernel data segments
- it checks the syscall number
- it can return to user mode using the saved ring-3 frame

## Exception Signal Dispatch and Debug Visibility

The project has evolved beyond the basic KFS 3 tutorial flow and now includes
an explicit kernel exception signal model in [src/kernel/interrupts/idt.c](../src/kernel/interrupts/idt.c).

Key runtime concepts:

- `kernel_register_signal_handler()` installs a callback for a given exception vector.
- `kernel_schedule_signal()` enqueues a pending signal for later dispatch.
- `kernel_exception_dispatch()` walks the queue and invokes all matching handlers.
- `kernel_signal_dispatch()` is the handler dispatch layer that invokes the actual
  callback for the vector.

This is a useful abstraction for debugging and for observing the exact signal path
without depending only on the raw assembly ISR entry points.

The shell exposes this through the builtins in [src/shell/builtins/idt_probe.c](../src/shell/builtins/idt_probe.c):

- `idt_probe` is the safe demo: it registers handlers, enqueues signals, and calls
  `kernel_exception_dispatch()` so the queue and handler flow can be observed on the
  kernel console without triggering a fatal hardware fault.
- `idt_fault` intentionally executes a real divide-by-zero instruction. This is the
  proof path for the actual CPU exception vector and is intentionally fatal: after the
  exception handler runs, the kernel halts.

This distinction matters because a true x86 exception cannot be meaningfully "tested"
inside the host build. The host-safe test path validates the logic, while the real
interrupt path is only demonstrable in a booted VM or bare-metal kernel session.

## Notes for This Milestone

- This repo satisfies the KFS 3 subject with a working IDT,
  remapped PIC, keyboard interrupt path, and a structured exception-dispatch layer.
- The implementation is a practical, working version of the tutorial,
  not a minimal toy example.
- The project goes beyond the baseline by adding a syscall pathway, signal
  registration/dispatch, and a real fault demonstration that intentionally halts
  the kernel after the exception is triggered.
