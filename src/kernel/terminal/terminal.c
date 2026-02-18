#include <terminal.h>


static void clear_ter(terminal_t *self) {
	self->display->clear(self->display);
}

static void write_string_ter(terminal_t *self, const char *string) {
	self->display->write_string(self->display, string);
}

void terminal_init(terminal_t *self, display_t *display) {
	self->id = 1;
	ft_strcpy(self->name, "virtual Terminal");
	self->history = NULL;
	self->his_size = 0;
	self->his_head = 0;
	self->cursor_x = 0;
	self->cursor_y = 0;
	self->curr_color = WHITE_ON_BLACK;
	self->display = display;
	self->clear = clear_ter;
	self->write_string = write_string_ter;
}
