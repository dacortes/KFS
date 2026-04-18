#include <gdt.h>

// Force the GDT to be in the .gdt section (the linker will put it in 0x800)

struct gdt_entry gdt[7] __attribute__((section(".gdt"), aligned(8)));

struct gdt_ptr gp;

struct s_gdt {
	uint32_t base;
	uint32_t limit;
	uint8_t access;
	uint8_t flags;
};

#define CREATE_SOURCE_GDT(base, limit, access, flags) \
{ \
	.base = base, \
	.limit = limit, \
	.access = access, \
	.flags = flags \
}

static void encodeGdtEntry(int num, struct s_gdt source)
{
	gdt[num].base_low = (source.base & 0xFFFF);
	gdt[num].base_middle = (source.base >> 16) & 0xFF;
	gdt[num].base_high = (source.base >> 24) & 0xFF;

	gdt[num].limit_low = (source.limit & 0xFFFF);
	gdt[num].flags_limit_high = ((source.limit >> 16) & 0x0F) | (source.flags << 4);
    gdt[num].access_byte = source.access;
}

void init_gdt(void)
{
	// Descriptor 1: Kernel Code (base=0, limit=4GB, 32 bits)
	encodeGdtEntry(1, CREATE_SOURCE_GDT(0, 0xFFFFF, 0x9A, 0xC));
	// Descriptor 2: Kernel Data
	encodeGdtEntry(2, CREATE_SOURCE_GDT(0, 0xFFFFF, 0x92, 0xC));
	// Descriptor 3: Kernel Stack (mismo que Data)
	encodeGdtEntry(3, CREATE_SOURCE_GDT(0, 0xFFFFF, 0x92, 0xC));
	// Descriptor 4: User Code (DPL=3)
	encodeGdtEntry(4, CREATE_SOURCE_GDT(0, 0xFFFFF, 0xFA, 0xC));
	// Descriptor 5: User Data (DPL=3)
	encodeGdtEntry(5, CREATE_SOURCE_GDT(0, 0xFFFFF, 0xF2, 0xC));
	// Descriptor 6: User Stack (DPL=3)
	encodeGdtEntry(6, CREATE_SOURCE_GDT(0, 0xFFFFF, 0xF2, 0xC));

	// Configure the GDTR pointer
	gp.limit = sizeof(gdt) - 1;
    gp.base = (uint32_t)&gdt;

	// Load the GDT using inline assembler
	gdt_flush((uint32_t)&gp);
}