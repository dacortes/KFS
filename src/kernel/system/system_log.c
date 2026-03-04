// SPDX-License-Identifier: GPL-2.0

#include <system.h>
#include <system_log.h>
#include <kernel/print/print.h>


static void system_log_write_impl(system_log_t *self, uint32_t level, const char *msg)
{
	if (!self || !msg)
		return;

	if (level > self->syslog_loglevel)
		return;

	const char *p = msg;

	while (*p) {
		self->buffer[self->head++] = *p++;
		if (self->head >= LOG_BUFFER_SIZE)
			self->head = 0;
		if (self->head == self->tail) {
			self->tail++;
			if (self->tail >= LOG_BUFFER_SIZE)
				self->tail = 0;
		}
	}

	self->buffer[self->head++] = '\n';
	if (self->head >= LOG_BUFFER_SIZE)
		self->head = 0;
	if (self->head == self->tail) {
		self->tail++;
		if (self->tail >= LOG_BUFFER_SIZE)
			self->tail = 0;
	}

	self->entry_count++;
}

static void system_log_dump_impl(system_log_t *self, terminal_t *output)
{
	if (!self || !output)
		return;

	char tmp[1024];
	int tmp_pos = 0;
	uint32_t pos = self->tail;

	while (pos != self->head && tmp_pos < (int)sizeof(tmp) - 1) {
		tmp[tmp_pos++] = self->buffer[pos++];
		if (pos >= LOG_BUFFER_SIZE)
			pos = 0;
	}
	tmp[tmp_pos] = '\0';
	output->write_string(output, tmp);
}

static void system_log_set_loglevel_impl(system_log_t *self, uint32_t console_level, uint32_t syslog_level)
{
	if (!self)
		return;
	self->console_loglevel = console_level;
	self->syslog_loglevel = syslog_level;
}

void system_log_init(system_log_t *log)
{
	if (!log)
		return;

	log->head = 0;
	log->tail = 0;
	log->console_loglevel = KERN_INFO;
	log->syslog_loglevel = KERN_DEBUG;
	log->entry_levels = 0;
	log->entry_timestamps = 0;
	log->entry_count = 0;

	log->write = system_log_write_impl;
	log->dump = system_log_dump_impl;
	log->set_loglevel = system_log_set_loglevel_impl;
}

