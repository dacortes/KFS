#pragma once

#define LOG_BUFFER_SIZE     65536 // 64KB para logs del sistema

typedef struct system_log_s system_log_t;

struct system_log_s {
	char    buffer[LOG_BUFFER_SIZE];

	uint32_t	head;                        /* Punto de escritura */
	uint32_t	tail;                        /* Punto de lectura más antiguo */

	uint32_t	console_loglevel;              /* Qué niveles mostrar en pantalla */
	uint32_t	syslog_loglevel;               /* Qué niveles guardar en el log */
	
	/* Información adicional por entrada (opcional) */
	uint32_t	*entry_levels;                /* Nivel de cada entrada */
	uint32_t	*entry_timestamps;            /* Timestamp de cada entrada */
	uint32_t	entry_count;                  /* Contador de entradas */

	void (*write)(system_log_t *self, uint32_t level, const char* msg);
    void (*dump)(system_log_t *self, terminal_t* output);
    void (*set_loglevel)(system_log_t *self, uint32_t console_level, uint32_t syslog_level);
};