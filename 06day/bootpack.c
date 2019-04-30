////////////////////////////////////////////////////////////////////////////////
// nasmfunc.asm
extern void io_hlt(void);
extern void io_cli(void);
extern void io_out8(int port, int data);
extern int  io_load_eflags(void);
extern void io_store_eflags(int eflags);
extern void load_gdtr(int limit, int addr); 
extern void load_idtr(int limit, int addr); 

////////////////////////////////////////////////////////////////////////////////
// mysprintf.c
void mysprintf (char *str, char *fmt, ...);

////////////////////////////////////////////////////////////////////////////////
// struct
// bootinfo from haribote.asm
struct BOOTINFO {
    unsigned char cyls, leds, vmode, reserve;
    unsigned short scrnx, scrny;
    unsigned char *vram;
};

// GDT
struct SEGMENT_DESCRIPTOR {
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

////////////////////////////////////////////////////////////////////////////////
// static function
static void init_palette(void);
static void set_palette(int start, int cnt, unsigned char *rgb);
static void init_screen(unsigned char *vram, int x, int y);
static void init_mouse_cursor8(unsigned char *mouse, char back_col_no);
static void init_gdtidt(void);
static void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
static void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
 

static void boxfill8(
        unsigned char *vram,
        int screen_width,
        unsigned char col_no,
        int x0, int y0, int x1, int y1);

static void putblock8_8(
        unsigned char *vram,
        int screen_width,
        int pxsize, int pysize,
        int px0, int py0,
        unsigned char *buf, int bxsize);

static void putfont8(
        unsigned char *vram,
        int screen_width,
        unsigned char col_no,
        const unsigned char *font,
        int x, int y);

static void putfonts8_asc(
        unsigned char *vram,
        int screen_width,
        int x, int y,
        unsigned char col_no,
        const char *str);


////////////////////////////////////////////////////////////////////////////////
// colors
#define COL_CNT (16)

#define COL8_BLACK       (0)
#define COL8_RED         (1)
#define COL8_GREEN       (2)
#define COL8_YELLOW      (3)
#define COL8_BLUE        (4)
#define COL8_PURPLE      (5)
#define COL8_CYAN        (6)
#define COL8_WHITE       (7)
#define COL8_GRAY        (8)
#define COL8_DARK_RED    (9)
#define COL8_DARK_GREEN  (10)
#define COL8_DARK_YELLOW (11)
#define COL8_DARK_BLUE   (12)
#define COL8_DARK_PURPLE (13)
#define COL8_DARK_CYAN   (14)
#define COL8_DARK_GRAY   (15)

////////////////////////////////////////////////////////////////////////////////
// main
void HariMain (void)
{
    /* read boot info */
    struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;

    /* message buffer */
    char s[128] = {};

    /* mouse cursor */
    unsigned char mcursor[256] = {};
    int mx = 0;
    int my = 0;

    /* パレット初期化 */
    init_palette();

    /* スクリーン初期化 */
    init_screen(
            binfo->vram,
            binfo->scrnx,
            binfo->scrny);
    /* 画面中央になるように座標計算 */
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_DARK_CYAN);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    mysprintf(s, "(%d, %d)", mx, my);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);


    /* hlt */
    for (;;)
    {
        io_hlt();
    }
}


static void init_palette(void)
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


static void set_palette(int start, int cnt, unsigned char *rgb)
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

static void init_mouse_cursor8(unsigned char *mouse, char back_col_no)
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

static void init_gtdidt(void)
{
    int i = 0;
    struct SEGMENT_DESCRIPTOR *gdt =
        (struct SEGMENT_DESCRIPTOR *) 0x00270000;
    struct GATE_DESCRIPTOR *idt =
        (struct GATE_DESCRIPTOR *) 0x0026f800;

    /* GDT の初期化 */
    for (i = 0; i < 8192; i++)
    {
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
    load_gdtr(0xffff, 0x00270000);

    /* IDT の初期化 */
    for (i = 0; i < 256; i++)
    {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(0x7ff, 0x0026f800);
    return;
}

static void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
    if (limit > 0xfffff)
    {
        ar |= 0x8000;  /* G_bit = 1 */
        limit /= 0x1000;
    }

    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff;
    return;
}

static void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}

static void boxfill8(
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

static void putfont8(
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

static void putfonts8_asc(
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

static void putblock8_8(
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

static void init_screen(unsigned char *vram, int x, int y)
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

