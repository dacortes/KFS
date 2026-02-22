#include <terminal.h>

static void clear_buffer(char *buff) {
	if (!buff)
		return ;
	for(int i = 0; i < DEVICE_BUFFER_SIZE; i++)
		buff[i] = '\0';
}

static void clear_ter(terminal_t *self) {
	if (!self)
		return;
	self->display->clear(self->display);
	clear_buffer(self->line);
}

static void save_history(terminal_t *self, const char *text) {
	if (!self || !text)
		return;

	ft_strncpy(self->history[self->his_head], text, TERMINAL_HISTORY_LEN - 1);
	self->history[self->his_head][TERMINAL_HISTORY_LEN - 1] = '\0';
	self->his_head = (self->his_head + 1) % TERMINAL_HISTORY_SIZE;
	 if (self->his_size < TERMINAL_HISTORY_SIZE) {
        self->his_size++;
    }
}

static void write_char(terminal_t *self, char c) {
	if (!self)
		return;
	
	if (c == '\n') {
		self->cursor_x = 0;
		self->cursor_y++;
	} else {
		self->display->put_at(self->display, c, self->cursor_x, self->cursor_y);
		self->cursor_x++;
	}

	if (self->cursor_x >= self->display->width) {
		self->cursor_x = 0;
		self->cursor_y++;
	}

	if (self->cursor_y >= self->display->height) {
		self->cursor_y = self->display->height - 1;
	}
}

static void write_string(terminal_t *self, const char *str) {
	unsigned int i;

	if (!self)
		return;

	self->save_history(self, str);
	i = 0;
	while (str[i]) {
		self->write_char(self, str[i]);
		i++;
	}
}

static void push_char(terminal_t *self, char input) {
	if (!self || !input)
		return;

	if (input == '\b') {
			if (self->line_pos > 0) {
				if (self->cursor_x > 0) {
					self->cursor_x--;
				} else if (self->cursor_y > 0) {
					self->cursor_y--;
					self->cursor_x = self->display->width - 1;
				}
				self->display->put_at(self->display, ' ', self->cursor_x, self->cursor_y);
				self->line_pos--;
				self->line[self->line_pos] = '\0';
		}
		return;
	}
	if (input == '\n') {
		self->save_history(self, self->line);
		self->write_char(self, input);
		self->line_pos = 0;
		self->line_len = 0;
		clear_buffer(self->line);
		return;
	}
	if (self->line_pos >= DEVICE_BUFFER_SIZE - 1)
		return;
	self->line[self->line_pos] = input;
	self->line[self->line_pos + 1] = '\0';
	self->line_pos++;
	self->write_char(self, input);
}

void terminal_init(terminal_t *self, display_t *display) {
	if (!self)
		return;

	self->id = 1;
	ft_strcpy(self->name, "virtual Terminal");
	for (int i = 0; i < TERMINAL_HISTORY_SIZE; i++) {
		self->history[i][0] = '\0';
	}
	self->his_size = 0;
	self->his_head = 0;
	self->cursor_x = 0;
	self->cursor_y = 0;

	self->curr_color = WHITE_ON_BLACK;
	self->display = display;

	clear_buffer(self->line);
	self->line_pos = 0;
	self->line_len = 0;

	self->clear = clear_ter;
	self->write_char = write_char;
	self->write_string = write_string;
	self->push_char = push_char;
	self->save_history = save_history;
}
