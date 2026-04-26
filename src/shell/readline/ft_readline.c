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

	ft_strncpy(term->prefix, prompt, ft_strlen(prompt));
}

char *readline(void)
{
	// remplazar por un puntero
	static char	line[MAX_LINE_LEN];
	uint32_t active = sys.active_terminal;
	terminal_t *term = &sys.terminals[active];
	unsigned int	len = ft_strlen(term->line);
	
	if (len >= MAX_LINE_LEN)
        return NULL;

	// Remplazar por memoria dinamica cuando se tenga
	ft_strncpy(line, term->line, len);
	line[len] = '\0';

	return line;
}