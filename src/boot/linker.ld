ENTRY(_start)

SECTIONS
{
	/* Init section GDT */
	. = 0x800;
	.gdt : {
		gdt_phys_start = .;
		*(.gdt)
		. = ALIGN(8);
		gdt_phys_end = .;
	}

	/* Kernel loaded at 1MB */
	. = 1M;

	.multiboot : ALIGN(4) {
		*(.multiboot)
	}

	.text : ALIGN(4K) {
		text_start = .;
		*(.text*)
	}

	.rodata : ALIGN(4K) {
		rodata_start = .;
		*(.rodata*)
	}

	.data : ALIGN(4K) {
		data_start = .;
		*(.data*)
	}

	.bss : ALIGN(4K) {
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

	kernel_start = 1M;
	kernel_size = endkernel - kernel_start;
}
