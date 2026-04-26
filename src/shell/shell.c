// SPDX-License-Identifier: GPL-2.0
#include <shell.h>

uint16_t create_tokens(t_shell *self, char *line)
{
	int i = 0;
	if (!line || !*line)
		return false;
	while(line[i]) {
		
	}
	return true;
}

uint16_t shell_clear(t_shell *self)
{
	// cuando se tenga malloc remplazar por clears con free
	ft_memset(self->line, '\0', sizeof(self->line));
	ft_memset(self->tokens, 0, sizeof(self->tokens));
	ft_memset(self->commands, '\0', sizeof(self->commands));
}

static void init_commands(t_shell *self, const char **def_commands)
{
	for (int i = 0; i < NUM_COMMANDS; i++)
		ft_strlcpy(self->commands[i], def_commands[i], MAX_WORD);
}

void	shell_init(t_shell *self)
{
	uint32_t active = sys.active_terminal;
	terminal_t *term = &sys.terminals[active];
	const char *def_commands[NUM_COMMANDS] = {"reboot", "half", "printf", "", ""};

	self->lv = 0;
	//Verificar el historial, si esta apuntando correctamnete
	self->history = &term->history;
	ft_memset(self->line, '\0', sizeof(self->line));
	ft_memset(self->tokens, 0, sizeof(self->tokens));
	ft_memset(self->commands, '\0', sizeof(self->commands));
	init_commands(self, def_commands);
	self->create_tokens = create_tokens;
	self->shell_clear = shell_clear; 
}