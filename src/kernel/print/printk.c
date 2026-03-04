// SPDX-License-Identifier: GPL-2.0

#include <system.h>
#include <system_log.h>
#include <stdarg.h>
#include <print.h>

/* Build the formatted string by reusing the printf-format dispatch
 * implemented in `formats()`; to capture the output we temporarily
 * redirect `write()` via `set_global_writer()`/`write_redirectable()`.
 */
static char *vbuf_ptr;
static int vbuf_pos;
static int vbuf_len;

static int buffer_writer_impl(const char *text, unsigned int cnt)
{
	for (unsigned int i = 0; i < cnt && vbuf_pos < vbuf_len - 1; i++)
		vbuf_ptr[vbuf_pos++] = text[i];
	return (int)cnt;
}

void kvprintk(uint32_t level, const char *fmt, va_list ap)
{
	char buf[512];
	int count = 0;

	if (!fmt)
		return;

	/* prepare capture buffer */
	vbuf_ptr = buf;
	vbuf_pos = 0;
	vbuf_len = (int)sizeof(buf);

	/* redirect writes into our buffer */
	set_global_writer(buffer_writer_impl);

	for (int i = 0; fmt[i]; i++) {
		if (fmt[i] == '%') {
			if (formats(&ap, fmt[i + 1], &count) == -1)
				break;
			i++; /* skip specifier */
		} else {
			/* write single char via redirectable writer */
			if (write_redirectable(&fmt[i], 1) == -1)
				break;
			count++;
		}
	}

	/* stop redirecting */
	set_global_writer(NULL);

	/* terminate buffer */
	buf[vbuf_pos] = '\0';

	/* store in system log */
	sys.syslog.write(&sys.syslog, level, buf);

	/* optionally print to console */
	if (level <= sys.syslog.console_loglevel) {
		terminal_t *term = &sys.terminals[sys.active_terminal];

		term->write_prefix(term);
		term->write_string(term, buf);
		term->set_cursor_color(term, BLACK_ON_WHITE);
	}
}

void kprintk(uint32_t level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	kvprintk(level, fmt, ap);
	va_end(ap);
}
