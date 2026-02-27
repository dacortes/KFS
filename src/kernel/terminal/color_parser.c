#include <color_parser.h>
#include <helper.h>

static uint8_t ansi_to_display_color(int ansi_number)
{
	switch(ansi_number) {
		// Foreground normales (30-37) - texto sobre fondo negro
		case 30: return BLACK_ON_BLACK;      // Negro
		case 31: return RED_ON_BLACK;        // Rojo
		case 32: return GREEN_ON_BLACK;      // Verde
		case 33: return BROWN_ON_BLACK;      // Marrón/Amarillo oscuro
		case 34: return BLUE_ON_BLACK;       // Azul
		case 35: return MAGENTA_ON_BLACK;    // Magenta
		case 36: return CYAN_ON_BLACK;       // Cian
		case 37: return LIGHT_GRAY_ON_BLACK; // Gris claro
		case 39: return WHITE_ON_BLACK;      // Default foreground
		
		// Foreground brillantes (90-97)
		case 90: return DARK_GRAY_ON_BLACK;      // Gris oscuro
		case 91: return LIGHT_RED_ON_BLACK;      // Rojo brillante
		case 92: return LIGHT_GREEN_ON_BLACK;    // Verde brillante
		case 93: return YELLOW_ON_BLACK;         // Amarillo brillante
		case 94: return LIGHT_BLUE_ON_BLACK;     // Azul brillante
		case 95: return LIGHT_MAGENTA_ON_BLACK;  // Magenta brillante
		case 96: return LIGHT_CYAN_ON_BLACK;     // Cian brillante
		case 97: return WHITE_ON_BLACK;          // Blanco brillante
		
		// Background normales (40-47) - texto blanco sobre fondo
		case 40: return BLACK_ON_BLACK;      // Fondo negro (texto negro)
		case 41: return WHITE_ON_RED;        // Fondo rojo
		case 42: return WHITE_ON_GREEN;      // Fondo verde
		case 43: return WHITE_ON_BROWN;      // Fondo marrón
		case 44: return WHITE_ON_BLUE;       // Fondo azul
		case 45: return WHITE_ON_MAGENTA;    // Fondo magenta
		case 46: return WHITE_ON_CYAN;       // Fondo cian
		case 47: return WHITE_ON_LIGHT_GRAY; // Fondo gris claro
		case 49: return WHITE_ON_BLACK;      // Default background
		
		// Background brillantes (100-107)
		case 100: return BLACK_ON_LIGHT_GRAY;    // Fondo gris oscuro? (ajusta según necesites)
		case 101: return WHITE_ON_BRIGHT_RED;    // Fondo rojo brillante
		case 102: return WHITE_ON_BRIGHT_GREEN;  // Fondo verde brillante
		case 103: return WHITE_ON_BRIGHT_YELLOW; // Fondo amarillo brillante
		case 104: return WHITE_ON_BRIGHT_BLUE;   // Fondo azul brillante
		case 105: return WHITE_ON_BRIGHT_MAGENTA;// Fondo magenta brillante
		case 106: return WHITE_ON_BRIGHT_CYAN;   // Fondo cian brillante
		
		default: return WHITE_ON_BLACK;
	}
}

static void parser_finish(color_parser_t *self)
{
	if (!self || self->buffer_pos == 0)
		return;

	char *buffer = self->buffer;
	int fg_num = 39;
	int bg_num = 49;

	char *sep = ft_strchr(buffer, ';');
	if (sep) {
		*sep = '\0';
		fg_num = ft_atoi(buffer);
		bg_num = ft_atoi(sep + 1);

		self->last_foreground = ansi_to_vga_color(fg_num);
		self->last_background = ansi_to_vga_color(bg_num);
	} else {
		fg_num = ft_atoi(buffer);

		if (fg_num >= 40 && fg_num <= 49)
			self->last_background = ansi_to_vga_color(fg_num);
		else
			self->last_foreground = ansi_to_vga_color(fg_num);
	}
	self->state = COLOR_STATE_NORMAL;
	self->buffer_pos = 0;
	self->buffer[0] = '\0';
}

static int parser_process(color_parser_t *self, char c)
{
	if (!self)
		return 0;
	
	switch (self->state) {
		case COLOR_STATE_NORMAL:
			if (c == '\033') {
				self->state = COLOR_STATE_ESC;
				return 1;
			}
			return 0;
		case COLOR_STATE_ESC:
			if (c == '\033') {
				self->state = COLOR_STATE_NORMAL;
				return 2;
			}
			if (c == '[') {
				self->state = COLOR_STATE_BRACKET;
				self->buffer_pos = 0;
				self->buffer[0] = '\0';
				return 1;
			}
			self->state = COLOR_STATE_NORMAL;
			return 2;
		case COLOR_STATE_BRACKET:
			if (c >= '0' && c <= '9') {
				self->buffer[self->buffer_pos++] = c;
				self->buffer[self->buffer_pos] = '\0';
				self->state = COLOR_STATE_NUMBER;
				return 1;
			}
			self->state = COLOR_STATE_NORMAL;
			return 2;
		case COLOR_STATE_NUMBER:
			if (c >= '0' && c <= '9') {
				if (self->buffer_pos < COLOR_BUFFER_SIZE - 1) {
					self->buffer[self->buffer_pos++] = c;
					self->buffer[self->buffer_pos] = '\0';
				}
				return 1;
			}
			else if (c == ';') {
				if (self->buffer_pos < COLOR_BUFFER_SIZE - 1) {
					self->buffer[self->buffer_pos++] = c;
					self->buffer[self->buffer_pos] = '\0';
				}
				return 1;
			}
			else if (c == 'm') {
				parser_finish(self);
				return 1;
			}
			else {
				self->state = COLOR_STATE_NORMAL;
				return 2;
			}
	}
	return 0;
}

static const char *parser_strip(color_parser_t *self, const char *input, char *output, int max_len)
{
	if (!self || !input || !output || max_len <= 0) 
		return input;
	
	int in_pos = 0;
	int out_pos = 0;
	
	self->parser_reset(self);
	
	while (input[in_pos] && out_pos < max_len - 1) {
		int result = self->parser_process(self, input[in_pos]);

		if (result == 0) {
			output[out_pos++] = input[in_pos];
			in_pos++;
		}
		else if (result == 1)
			in_pos++;
		else if (result == 2)
			output[out_pos++] = '\033';
	}
	output[out_pos] = '\0';
	return output;
}

static void parser_reset(color_parser_t *self)
{
    if (!self)
		return;

	self->state = COLOR_STATE_NORMAL;
	self->buffer_pos = 0;
	self->buffer[0] = '\0';
	self->last_foreground = WHITE_ON_BLACK;
	self->last_background = BLACK_ON_BLACK;
}

void color_parser_init(color_parser_t *parser)
{
    if (!parser)
		return;
    
    parser->parser_process = parser_process;
    parser->parser_strip = parser_strip;
    parser->parser_reset = parser_reset;
    
    parser_reset(parser);
}