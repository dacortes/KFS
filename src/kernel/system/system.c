#include <system.h>

system_t sys;

extern void irq1_handler(void);

static void create_terminal(void)
{
	for(int i = 0; i < MAX_TERMINAL; i++) {
		terminal_init(&sys.terminals[i], &sys.display, i);
		sys.terminals[i].clear(&sys.terminals[i]);
	}
	sys.active_terminal = 0;
}

void init_system(void)
{
	display_init(&sys.display);
	sys.display.clear(&sys.display);
	create_terminal();
	idt_init();
	pic_init();
	idt_set_gate(0x21, (unsigned int)irq1_handler, 0x10, 0x8E);
	keyboard_init(&sys.keyboard);
	__asm__ volatile("sti");
	sys.terminals[sys.active_terminal].clear(&sys.terminals[sys.active_terminal]);
	sys.terminals[0].write_string(&sys.terminals[0], "<42> : ");
}
