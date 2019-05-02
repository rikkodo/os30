#include "bootpack.h"

void init_palette(void)
{
    /* 色テーブル */
    static unsigned char table_rgb[COL_CNT * 3] =  /* 3: r, g, b */
    {
        0x00, 0x00, 0x00,  /*  0:黒         */
        0xff, 0x00, 0x00,  /*  1:明るい赤   */
        0x00, 0xff, 0x00,  /*  2:明るい緑   */
        0xff, 0xff, 0x00,  /*  3:明るい黄色 */
        0x00, 0x00, 0xff,  /*  4:明るい青   */
        0xff, 0x00, 0xff,  /*  5:明るい紫   */
        0x00, 0xff, 0xff,  /*  6:明るい水色 */
        0xff, 0xff, 0xff,  /*  7:白         */
        0xc6, 0xc6, 0xc6,  /*  8:明るい灰色 */
        0x84, 0x00, 0x00,  /*  9:暗い赤     */
        0x00, 0x84, 0x00,  /* 10:暗い緑     */
        0x84, 0x84, 0x00,  /* 11:暗い黄色   */
        0x00, 0x00, 0x84,  /* 12:暗い青     */
        0x84, 0x00, 0x84,  /* 13:暗い紫     */
        0x00, 0x84, 0x84,  /* 14:暗い水色   */
        0x84, 0x84, 0x84,  /* 15:暗い灰色   */
    };

    set_palette(0, COL_CNT, table_rgb);

    return;
}

void set_palette(int start, int cnt, unsigned char *rgb)
{
    int i = 0;
    int eflags = 0;

    /* 割り込みフラグ取得 */
    eflags = io_load_eflags();
    /* 割り込みの無効化 */
    io_cli();

    /* 色の格納 */
    io_out8(0x03c8, start);
    for (i = start; i < cnt; i++)
    {
        io_out8(0x03c9, rgb[i * 3 + 0] / 4);
        io_out8(0x03c9, rgb[i * 3 + 1] / 4);
        io_out8(0x03c9, rgb[i * 3 + 2] / 4);
    }

    /* 割り込みフラグを戻す */
    io_store_eflags(eflags);

    return;
}

void init_mouse_cursor8(unsigned char *mouse, char back_col_no)
    /* マウスカーソルを準備（16x16） */
{
    static unsigned char cursor[16][16] =
    {
        "**************..",
        "*OOOOOOOOOOO*...",
        "*OOOOOOOOOO*....",
        "*OOOOOOOOO*.....",
        "*OOOOOOOO*......",
        "*OOOOOOO*.......",
        "*OOOOOOO*.......",
        "*OOOOOOOO*......",
        "*OOOO**OOO*.....",
        "*OOO*..*OOO*....",
        "*OO*....*OOO*...",
        "*O*......*OOO*..",
        "**........*OOO*.",
        "*..........*OOO*",
        "............*OO*",
        ".............***"
    };
    int x = 0;
    int y = 0;

    for (y = 0; y < 16; y++)
    {
        for (x = 0; x < 16; x++)
        {
            if (cursor[y][x] == '*')
            {
                mouse[y * 16 + x] = COL8_BLACK;
            }
            else if (cursor[y][x] == 'O')
            {
                mouse[y * 16 + x] = COL8_WHITE;
            }
            else if (cursor[y][x] == '.')
            {
                mouse[y * 16 + x] = back_col_no;
            }
        }
    }
    return;
}

void boxfill8(
        unsigned char *vram,
        int screen_width,
        unsigned char col_no,
        int x0, int y0, int x1, int y1)
{
    int xx = 0;
    int yy = 0;

    for (yy = y0; yy <= y1; yy++)
    {
        for (xx = x0; xx <= x1; xx++)
        {
            vram[yy * screen_width + xx] = col_no;
        }
    }

    return;
}

void putfont8(
        unsigned char *vram,
        int screen_width,
        unsigned char col_no,
        const unsigned char *font,
        int x, int y)
{
    int i = 0;

    for (i = 0; i < 16; i++)
    {
        unsigned char *p = vram + (y + i) * screen_width + x;
        unsigned char d = font[i];
        if ((d & 0x80) != 0) {p[0] = col_no;}
        if ((d & 0x40) != 0) {p[1] = col_no;}
        if ((d & 0x20) != 0) {p[2] = col_no;}
        if ((d & 0x10) != 0) {p[3] = col_no;}
        if ((d & 0x08) != 0) {p[4] = col_no;}
        if ((d & 0x04) != 0) {p[5] = col_no;}
        if ((d & 0x02) != 0) {p[6] = col_no;}
        if ((d & 0x01) != 0) {p[7] = col_no;}
    }
    return;
}

void putfonts8_asc(
        unsigned char *vram,
        int screen_width,
        int x, int y,
        unsigned char col_no,
        const char *str)
{
    extern unsigned const char hankaku[4096];

    const char *c = 0;
    for (c = str; *c != 0x00; c++)
    {
        putfont8(vram, screen_width, col_no,
                hankaku + *c * 16, x, y);
        x += 8;
    }

    return;
}

void putblock8_8(
        unsigned char *vram,
        int screen_width,
        int pxsize, int pysize,
        int px0, int py0,
        unsigned char *buf, int bxsize)
{
    int x = 0;
    int y = 0;

    for (y = 0; y < pysize; y++)
    {
        for (x = 0; x < pxsize; x++)
        {
            vram[(py0 + y) * screen_width + (px0 + x)] = buf[y * bxsize + x];
        }
    }
    return;
}

void init_screen8(unsigned char *vram, int x, int y)
{
    boxfill8(vram, x, COL8_DARK_CYAN,  0,     0,      x -  1, y - 29);
    boxfill8(vram, x, COL8_GRAY,       0,     y - 28, x -  1, y - 28);
    boxfill8(vram, x, COL8_WHITE,      0,     y - 27, x -  1, y - 27);
    boxfill8(vram, x, COL8_GRAY,       0,     y - 26, x -  1, y -  1);

    boxfill8(vram, x, COL8_WHITE,      3,     y - 24, 59,     y - 24);
    boxfill8(vram, x, COL8_WHITE,      2,     y - 24,  2,     y -  4);
    boxfill8(vram, x, COL8_DARK_GRAY,  3,     y -  4, 59,     y -  4);
    boxfill8(vram, x, COL8_DARK_GRAY, 59,     y - 23, 59,     y -  5);
    boxfill8(vram, x, COL8_BLACK,      2,     y -  3, 59,     y -  3);
    boxfill8(vram, x, COL8_BLACK,     60,     y - 24, 60,     y -  3);

    boxfill8(vram, x, COL8_DARK_GRAY, x - 47, y - 24, x -  4, y - 24);
    boxfill8(vram, x, COL8_DARK_GRAY, x - 47, y - 23, x - 47, y -  4);
    boxfill8(vram, x, COL8_WHITE,     x - 47, y -  3, x -  4, y -  3);
    boxfill8(vram, x, COL8_WHITE,     x -  3, y - 24, x -  3, y -  3);

    return;
}

