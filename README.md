# KFS — Kernel From Scratch

Educational x86 (i686) 32-bit kernel. Boots via GRUB (Multiboot v1), runs in VGA text mode.

## What it can do

### Boot & Hardware
- Multiboot v1 boot with GRUB, kernel loaded at 1 MB
- 16 KB kernel stack, flat memory model (GRUB's GDT)
- IDT (256 entries) + 8259A PIC (remapped, IRQ1 unmasked)
- PS/2 keyboard driver (IRQ1): US QWERTY, shift, ctrl, extended arrow keys
- Ctrl+key shortcut system (buffered, callback-driven)

### Display
- VGA text mode, 80×25, 16-colour palette
- OOP display struct with `clear()` and `put_at()` methods
- ~40 predefined colour macros (`WHITE_ON_BLACK`, `LIGHT_RED_ON_BLACK`, etc.)

### Terminals
- 2 virtual terminals (`tty 0`, `tty 1`) with title-bar tabs
- Terminal switching via Ctrl+1 / Ctrl+2
- 200-line scrollback buffer (characters + colour attributes)
- Scroll up/down with arrow keys while viewing history
- Line editing: insert-mode typing, backspace, left/right cursor movement
- 10-entry command history (80 chars each)
- ANSI SGR colour sequences: reset, bold, basic/bright fg (30–37, 90–97), bg (40–47, 100–107), default fg/bg (39/49), 256-colour fg (`38;5;N`)
- Prompt prefix: `<42> : `

### Printf & Logging
- `printf()`: `%c`, `%s`, `%d`/`%i`, `%u`, `%x`, `%X`, `%p`, `%%`
- Redirectable writer for output capture
- Kernel log system: 64 KB ring buffer, 8 log levels (`KERN_EMERG`–`KERN_DEBUG`), `kprintk()`, dump to terminal

### Libc Subset
| Function | Notes |
|---|---|
| `ft_strlen`, `ft_strcmp`, `ft_strcpy` | x86 assembly (NASM) + C wrappers |
| `ft_strncpy`, `ft_memset`, `ft_memchr`, `ft_strchr` | Pure C |
| `ft_atoi`, `ft_isdigit` | Pure C |

### Build & Testing
- Makefile: freestanding i686 ELF32, `grub-mkrescue` ISO, QEMU targets
- 14 GoogleTest suites (225 tests), lcov coverage, cppcheck, checkpatch.pl
- CI pipeline: style → static analysis → build → coverage (≥80%)

## Quick Start

```bash
make                # Build kernel binary
make iso            # Create bootable ISO
make run            # Run in QEMU
make test           # Build and run all tests
make debug          # QEMU with GDB on port 1234
./scripts/run_all_checks.sh   # Full CI validation
```

## References

- https://computers-art.medium.com/writing-a-basic-kernel-6479a495b713
- http://www.math.utah.edu/docs/info/ld_3.html#SEC4
- http://www.osdever.net/bkerndev/Docs/keyboard.htm

