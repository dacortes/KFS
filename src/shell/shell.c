// SPDX-License-Identifier: GPL-2.0
#include <shell.h>
#include <print.h>

static uint32_t word_size(char *line)
{
	if (!line || !*line)
		return false;

	int size = 0;

	while(line[size] && !ft_isblank(line[size]))
		size++;
	return size;
}

static void cut_blanks(uint32_t *i, char *line)
{
	if (!line || !*line)
		return ;

	while(line[*i] && ft_isblank(line[*i]))
		(*i)++;
	return ;
}

static void add_token(t_token *token, char *word, uint32_t *tk, uint32_t word_size)
{
	if (word_size > 0 && word_size < MAX_WORD) {
		ft_strlcpy(token->word, word, word_size + 1);
		(*tk)++;
	}
}

uint16_t create_tokens(t_shell *self, char *line)
{
	uint32_t i = 0;
	uint32_t size;

	if (!line || !*line)
		return false;
	self->num_tk = 0;
	while(line[i]) {
		cut_blanks(&i, line);
		
		if (!line[i])
			break ;

		size = word_size(&line[i]);

		add_token(&self->token[self->num_tk], &line[i], &self->num_tk, size);
		i += size;
	}
	return (self->num_tk > 0);
}

void	print_tokens(t_shell *self)
{
	uint32_t nill;

	for(uint32_t i = 0; i < self->num_tk; i++) {
		nill = i == 0 && printf("\n[%sCOMMAND%s] %s\n", BLUE, END, self->token[i].word);
		nill = i != 0 && printf("  [%d] arg: %s\n", i, self->token[i].word);
	}
	nill = true;
}

void shell_clear(t_shell *self)
{
	// cuando se tenga malloc remplazar por clears con free
	self->num_tk = 0;
	ft_memchr(self->line, '\0', sizeof(self->line));
	ft_memchr(self->token, 0, sizeof(self->token));
	ft_memchr(self->commands, '\0', sizeof(self->commands));
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

	self->num_tk = 0;
	self->lv = 0;
	//Verificar el historial, si esta apuntando correctamnete
	self->history = (char ***)&term->history;
	shell_clear(self);
	init_commands(self, def_commands);
	self->create_tokens = create_tokens;
	self->clear = shell_clear; 
	self->print = print_tokens;
}