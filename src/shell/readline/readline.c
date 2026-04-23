#include <readline.h>

void set_prompt(const char *prompt)
{
	uint32_t active = self->active_terminal;
	terminal_t *term = &sys.terminals[active];

	if (!prompt) {
		printf("%s", ERROR_NULL_PROMPT);
		return ;
	}
	ft_strncpy(strterm->prefix, prompt, ft_strlen(prompt));
}

char *readline(void)
{
	uint32_t active = self->active_terminal;
	terminal_t *term = &sys.terminals[active];

	if (!term->line)
		return ;

	unsigned int	len = ft_strlen(term->line);
	char	line[len + 1];
	ft_strncpy(line, term->line, len);
	line[len] = '\0';
	return line;
}