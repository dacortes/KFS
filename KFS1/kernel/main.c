/* kernel/main.c */

#define VGA_BUFFER ((volatile unsigned short*)0xB8000)
#define VGA_WIDTH 80

static void put_char(int x, int y, char c, unsigned char color)
{
    VGA_BUFFER[y * VGA_WIDTH + x] = (unsigned short)(c | color << 8);
}

void kernel_main(unsigned int magic, void *mb_info)
{
    (void)mb_info;

    if (magic != 0x2BADB002)
        while (1);

    put_char(0, 0, '4', 0x0F);
    put_char(1, 0, '2', 0x0F);

    while (1)
        ;
}

