# KFS 2 - GDT and Stack

This document maps the KFS 2 subject to the current repository.
It records how the GDT, kernel stack, and privilege-transition demos
are implemented today.

## PDF Scope

The KFS 2 subject requires:

- a Global Descriptor Table
- kernel code, kernel data, kernel stack, user code, user data, and
  user stack descriptors
- the GDT to be placed at address `0x00000800`
- the GDT to be declared to the BIOS and loaded by the kernel
- a human-friendly kernel stack printout
- a basic shell for extra debugging actions as a bonus

## Current Implementation

The repo already implements the full GDT stack for the subject and then
goes further with privilege demos, stack capture, and shell commands.

### GDT Layout

The GDT definition lives in [src/kernel/interrupts/gdt.h](../src/kernel/interrupts/gdt.h).

Key pieces:

- `GDT_ENTRIES` is set to 8.
- The kernel selector constants are defined for code, data, stack, and
  TSS use.
- `struct gdt_entry`, `struct gdt_ptr`, and `struct tss_entry` model
  the descriptor table and the CPU GDTR/TSS layouts.
- `stack_top` is imported from the boot linker script and reused as the
  ring 0 stack top in the TSS.

Relevant symbols:

- [gdt.h](../src/kernel/interrupts/gdt.h#L10)
- [struct gdt_entry / gdt_ptr / tss_entry](../src/kernel/interrupts/gdt.h#L20)
- [gdt_init()](../src/kernel/interrupts/gdt.h#L75)
- [gdt_print_kernel_stack()](../src/kernel/interrupts/gdt.h#L176)
- [gdt_run_stack_demo()](../src/kernel/interrupts/gdt.h#L192)
- [gdt_enter_user_mode()](../src/kernel/interrupts/gdt.h#L209)

### GDT Initialization

The actual setup is implemented in [src/kernel/interrupts/gdt.c](../src/kernel/interrupts/gdt.c).

- `gdt_init()` fills the descriptor table.
- Entries 1 to 6 cover kernel/user code, data, and stack.
- Entry 7 is the TSS descriptor.
- `tss.esp0` is initialized from `stack_top` so ring 0 transitions use
  the kernel stack.
- `gdt_load()` is called to load GDTR, and `gdt_load_tss()` loads the
  task register.
- `gdt_verify()` checks that the loaded GDTR matches the expected table.
- `gdt_log_descriptors()` prints the active descriptor state.

Relevant symbols:

- [gdt_init()](../src/kernel/interrupts/gdt.c#L75)
- [gdt_verify()](../src/kernel/interrupts/gdt.c#L135)
- [gdt_log_descriptors()](../src/kernel/interrupts/gdt.c#L190)
- [gdt_print_kernel_stack()](../src/kernel/interrupts/gdt.c#L269)
- [gdt_run_privilege_demo()](../src/kernel/interrupts/gdt.c#L311)
- [gdt_run_stack_demo()](../src/kernel/interrupts/gdt.c#L339)

### Boot and Stack Linkage

The GDT subject also depends on the boot-time stack layout.

- [src/boot/linker.ld](../src/boot/linker.ld) declares the kernel at 1 MB.
- It places the `.gdt` section at `0x800`.
- It exports `stack_bottom` and `stack_top` for the TSS stack setup.
- [src/boot/entry.s](../src/boot/entry.s) loads `stack_top` into `esp`
  before calling `kernel_main()`.

Relevant references:

- [linker.ld](../src/boot/linker.ld#L2)
- [entry.s](../src/boot/entry.s#L7)

### Human-Friendly Stack Output

The subject asks for a readable kernel stack printout. In the current
repo this is implemented by the GDT diagnostics code instead of a tiny
standalone helper.

- `gdt_get_stack_info()` captures `esp`, `ss`, and `ebp`.
- `gdt_print_kernel_stack()` prints the kernel stack state.
- `gdt_print_user_stack()` prints the captured ring 3 stack state.
- `gdt_run_stack_demo()` prints both contexts and compares them.

This gives a clearer result than a raw register dump and matches the
subject goal of making the stack visible in a human-friendly way.

Relevant symbols:

- [gdt_get_stack_info()](../src/kernel/interrupts/gdt.c#L250)
- [gdt_print_kernel_stack()](../src/kernel/interrupts/gdt.c#L269)
- [gdt_print_user_stack()](../src/kernel/interrupts/gdt.c#L289)
- [gdt_run_stack_demo()](../src/kernel/interrupts/gdt.c#L339)

## Runtime Flow

The main runtime sequence is:

`grub.cfg` -> `src/boot/entry.s` -> `kernel_main()` -> `init_system()`
-> `gdt_init()` -> `gdt_log_descriptors()` / stack demo / shell

- `kernel_main()` calls `init_system()`.
- `init_system()` initializes display, logging, terminals, GDT, IDT,
  PIC, and keyboard input.
- It also installs the keyboard shortcut handler used to reach the
  privilege demo.

Relevant references:

- [kernel_main()](../src/kernel/main.c#L28)
- [init_system()](../src/kernel/system/system.c#L156)
- [shortcut_handler()](../src/kernel/system/system.c#L59)
- [switch_terminal()](../src/kernel/system/system.c#L28)

## Bonus Shell Flow

The KFS 2 bonus asks for a minimal shell. This repository already has a
shell and several debugging commands.

- [src/shell/shell.c](../src/shell/shell.c) registers the builtins.
- `reboot`, `half`, `user_mode`, and `show_mode` are available.
- `cmd_user_mode()` switches to ring 3 using `switch_to_user_mode()`.
- `cmd_show_mode()` prints the current privilege level.
- The shell is backed by the readline/tokenization flow in
  [src/shell/readline/ft_readline.c](../src/shell/readline/ft_readline.c).

Relevant symbols:

- [shell_init()](../src/shell/shell.c#L109)
- [cmd_user_mode()](../src/shell/builtins/mode_switch.c#L22)
- [cmd_show_mode()](../src/shell/builtins/mode_switch.c#L46)
- [user_mode()](../src/shell/builtins/mode_switch.c#L11)

## Notes For This Milestone

- The PDF’s mandatory part is satisfied by the GDT setup plus the stack
  printing path.
- The repo’s implementation is richer than the subject baseline because
  it includes privilege demos and shell commands for debugging.
- When you send KFS 3, this file can be extended with the next subject’s
  requirements and implementation map in the same style.