#pragma once

#include <terminal.h>
#include <display.h>
#include <keyboard.h>
#include <pic.h>
#include <idt.h>
#include <helper.h>

#ifndef MAX_TERMINAL
#define MAX_TERMINAL 2
#endif

typedef struct system_s system_t;

struct system_s {
	/* Display global (singleton) */
	display_t	display;
	keyboard_t	keyboard;
	

	terminal_t	terminals[MAX_TERMINAL];
	uint32_t	active_terminal;
	
	/* Log del sistema */
	// system_log_t	*syslog;

    void (*init)(system_t *self);
	void (*main_loop)(system_t *self);
};

extern system_t sys;

void init_system(void);