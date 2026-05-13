// SPDX-License-Identifier: GPL-2.0
#include <shell.h>
#include <print.h>
#include <helper.h>

static uint32_t word_size(char *line)
{
	if (!line || !*line)
		return false;

	int size = 0;

	while (line[size] && !ft_isblank(line[size]))
		size++;
	return size;
}

static void cut_blanks(uint32_t *i, char *line)
{
	if (!line || !*line)
		return;

	while (line[*i] && ft_isblank(line[*i]))
		(*i)++;
}

static void add_token(token_t *token, char *word, uint32_t *tk, uint32_t word_size)
{
	if (word_size > 0 && word_size < MAX_WORD) {
		ft_strlcpy(token->word, word, word_size + 1);
		(*tk)++;
	}
}

uint16_t create_tokens(shell_t *self, char *line)
{
	uint32_t i = 0;

	if (!line || !*line)
		return false;
	self->num_tk = 0;
	while (line[i]) {
		uint32_t size;

		cut_blanks(&i, line);
		if (!line[i])
			break;

		size = word_size(&line[i]);

		add_token(&self->token[self->num_tk], &line[i], &self->num_tk, size);
		i += size;
	}
	return (self->num_tk > 0);
}

void	printoken_ts(shell_t *self)
{
	for (uint32_t i = 0; i < self->num_tk; i++) {
		if (i == 0)
			printf("\n[%sCOMMAND%s] %s\n", BLUE, END, self->token[i].word);
		else
			printf("  [%u] arg: %s\n", i, self->token[i].word);
	}
}

void token_clear(token_t *self)
{
	if (!self)
		return;
	if (self->word[0] == '\0')
		return;
	ft_memset(self->word, '\0', sizeof(self->word));
	self->type = 0;
}

static void initoken_t(token_t *token)
{
	token->clear = token_clear;
	token->clear(token);
}

static void shell_clear_tokens(shell_t *self)
{
	for (int i = 0; i < MAX_WORD; i++)
		self->token[i].clear(&self->token[i]);
}

void shell_clear(shell_t *self)
{
	// cuando se tenga malloc remplazar por clears con free
	self->num_tk = 0;
	ft_memset(self->line, '\0', sizeof(self->line));
	shell_clear_tokens(self);
}

static int cmd_reboot(shell_t *self)
{
	(void)self;
	reboot_system();
	return 0;
}

static int cmd_half(shell_t *self)
{
	(void)self;
	halt_system();
	return 0;
}

static int execute(shell_t *self)
{
	char *cmd = self->token[0].word;

	for(size_t num = 0; num < NUM_COMMANDS; num++) {
		if (!ft_strcmp(cmd, self->builtins[num].name))
			return self->builtins[num].func(self);
	}
	printf("%s[ERROR]%s: %s: command not found\n", RED, END, cmd);
	return 127;
}

void	shell_init(shell_t *self)
{
	uint32_t active = sys.active_terminal;
	terminal_t *term = &sys.terminals[active];
	const builtin_t builtins[] = {
		{"reboot", cmd_reboot, "Reboot the system"},
		{"half",   cmd_half,   "Halt the CPU"},
		// {"printf", cmd_printf, "Print arguments"},
		{NULL, NULL, NULL}
	};
	self->num_tk = 0;
	self->lv = 0;
	self->builtins = builtins;
	for (int i = 0; i < MAX_WORD; i++)
		initoken_t(&self->token[i]);
	//Verificar el historial, si esta apuntando correctamnete
	self->history = (char ***)&term->history;
	shell_clear(self);
	self->create_tokens = create_tokens;
	self->clear = shell_clear;
	self->print = printoken_ts;
}
