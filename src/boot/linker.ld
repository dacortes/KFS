/* linker.ld */
ENTRY(_start)

SECTIONS
{

    /* Init section GDT*/
    . = 0x800;
    
    /* section GDT */
    .gdt : {
        *(.gdt)
    }
    /* Kernel loaded at 1MB */
    . = 1M;

    /* Multiboot header MUST be early */
    .multiboot :
    {
        *(.multiboot)
    }

    .text : ALIGN(4K)
    {
        *(.text*)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata*)
    }

    .data : ALIGN(4K)
    {
        *(.data*)
    }

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss*)
    }

    /* Stack (16 KB) */
    . = ALIGN(16);
    stack_bottom = .;
    . += 16K;
    stack_top = .;
}

