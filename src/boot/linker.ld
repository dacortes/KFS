/* linker.ld */
ENTRY(_start)

SECTIONS
{

    /* Init section GDT*/
    . = 0x800;
    
    /* section GDT */
    .gdt : {
		gdt_phys_start = .;
		*(.gdt)
		. = ALIGN(8);
		gdt_phys_end = .;
    }
    /* Kernel loaded at 1MB */
    . = 1M;

    /* Multiboot header MUST be early */
    .multiboot : ALIGN(4)
    {
        *(.multiboot)
    }

    .text : ALIGN(4K)
    {
		text_start = .;
        *(.text*)
    }

    .rodata : ALIGN(4K)
    {
		rodata_start = .;
        *(.rodata*)
    }

    .data : ALIGN(4K)
    {
		data_start = .;
        *(.data*)
    }

    .bss : ALIGN(4K)
    {
		bss_start = .;
        *(COMMON)
        *(.bss*)
		bss_end = .;
    }

	endkernel = .;

    /* Stack (16 KB) */
    . = ALIGN(16);
    stack_bottom = .;
    . += 16K;
    stack_top = .;

	kernel_start = 1M;
    kernel_end = endkernel;
    kernel_size = endkernel - 1M;
}

