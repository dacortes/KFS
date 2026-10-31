ENTRY(_start)

KERNEL_BASE = 0xC0000000;

SECTIONS
{
	/* Kernel virtual address: 3GB + 1MB. Physical load address: 1MB. */
	. = KERNEL_BASE + 1M;
	kernel_start = .;

	.multiboot ALIGN(4) : AT(ADDR(.multiboot) - KERNEL_BASE) {
		*(.multiboot)
	}

	.text ALIGN(4K) : AT(ADDR(.text) - KERNEL_BASE) {
		text_start = .;
		*(.text*)
	}

	.rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_BASE) {
		rodata_start = .;
		*(.rodata*)
	}

	.data ALIGN(4K) : AT(ADDR(.data) - KERNEL_BASE) {
		data_start = .;
		*(.data*)
	}

	.bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_BASE) {
		bss_start = .;
		*(COMMON)
		*(.bss*)
		bss_end = .;
	}

	/* ========== Stack (16 KB) ========== */
	. = ALIGN(16);
	stack_bottom = .;
	. += 16K;
	stack_top = .;

	endkernel = .;
	kernel_end = endkernel;
	kernel_size = endkernel - kernel_start;
}
