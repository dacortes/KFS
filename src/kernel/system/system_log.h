#pragma once

#define LOG_BUFFER_SIZE     65536 // 64KB para logs del sistema

#include <stdint.h>
#include <kernel/terminal/terminal.h>
#include <stdarg.h>

/* Kernel log levels */
#define KERN_EMERG   0
#define KERN_ALERT   1
#define KERN_CRIT    2
#define KERN_ERR     3
#define KERN_WARNING 4
#define KERN_NOTICE  5
#define KERN_INFO    6
#define KERN_DEBUG   7

typedef struct system_log_s system_log_t;

struct system_log_s {
	char    buffer[LOG_BUFFER_SIZE];

	uint32_t	head;
	uint32_t	tail;

	uint32_t	console_loglevel;
	uint32_t	syslog_loglevel;
	
	uint32_t	*entry_levels;
	uint32_t	*entry_timestamps;
	uint32_t	entry_count;

	void (*write)(system_log_t *self, uint32_t level, const char* msg);
	void (*dump)(system_log_t *self, terminal_t* output);
	void (*set_loglevel)(system_log_t *self, uint32_t console_level, uint32_t syslog_level);
};

void system_log_init(system_log_t *log);

void kprintk(uint32_t level, const char *fmt, ...);
void kvprintk(uint32_t level, const char *fmt, va_list ap);