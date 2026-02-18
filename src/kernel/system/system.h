#pragma once

typedef struct system_s system_t;

struct system_s {
	/* Display global (singleton) */
	display_t	*display;
	
	/* Terminales del sistema */
	terminal_t	**terminals;
	uint32_t	terminal_count;
	uint32_t	active_terminal;
	
	/* Log del sistema */
	system_log_t	*syslog;
    
};