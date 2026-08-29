// SPDX-License-Identifier: GPL-2.0

#include <ft_readline.h>

void set_prompt(const char *prompt)
{
	if (!prompt) {
		printf("%s", ERROR_NULL_PROMPT);
		return;
	}
	uint32_t active = sys.active_terminal;
	terminal_t *term = &sys.terminals[active];
	ft_strlcpy(term->prefix, prompt, sizeof(term->prefix));
	term->prefix_len = ft_strlen(term->prefix);
}

char *readline(const char *prompt)
{
	uint32_t active = sys.active_terminal;
	terminal_t *term = &sys.terminals[active];
	unsigned int	len = ft_strlen(term->line);

	(void)prompt;
	set_prompt(prompt);
	if (!term->line_ready)
		return NULL;

	char *line = ft_strndup(term->line, len);

	if (!line)
		return NULL;

	term->line_ready = 0;
	term->clear_line(term);
	return line;
}