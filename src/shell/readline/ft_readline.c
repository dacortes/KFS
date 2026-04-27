// SPDX-License-Identifier: GPL-2.0

#include <ft_readline.h>

void set_prompt(const char *prompt)
{
	if (!prompt) {
		printf("%s", ERROR_NULL_PROMPT);
		return ;
	}
	
	uint32_t active = sys.active_terminal;
	terminal_t *term = &sys.terminals[active];

	ft_strlcpy(term->prefix, prompt, ft_strlen(prompt) + 1);
}

char *readline(char *line)
{
	uint32_t active = sys.active_terminal;
	terminal_t *term = &sys.terminals[active];
	unsigned int	len = ft_strlen(term->line);

	if (!term->line_ready)
		return NULL;
	ft_strlcpy(line, term->line, len + 1);
	term->line_ready = 0;
	term->clear_line(term);
	return line;
}