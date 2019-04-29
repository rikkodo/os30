extern void io_hlt(void);
extern void io_cli(void);
extern void io_out8(int port, int data);
extern int  io_load_eflags(void);
extern void io_store_eflags(int eflags);

#define COL_CNT (16)

static void init_palette(void);
static void set_palette(int start, int cnt, unsigned char *rgb);

static void boxfill8(
        unsigned char *vram,
        int screen_width,
        unsigned char col_no,
        int x0, int y0, int x1, int y1);


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

#define SCREEN_WIDTH     (320)
#define SCREEN_HEIGHT    (200)

void HariMain (void)
{
    int i = 0;
    char *vram = (char *)0xa0000;  /* VRAM番地先頭 */

    /* パレット初期化 */
    init_palette();

    boxfill8(vram, SCREEN_WIDTH, COL8_DARK_CYAN,  0,                0,                  SCREEN_WIDTH -  1, SCREEN_HEIGHT - 29);
    boxfill8(vram, SCREEN_WIDTH, COL8_GRAY,       0,                SCREEN_HEIGHT - 28, SCREEN_WIDTH -  1, SCREEN_HEIGHT - 28);
    boxfill8(vram, SCREEN_WIDTH, COL8_WHITE,      0,                SCREEN_HEIGHT - 27, SCREEN_WIDTH -  1, SCREEN_HEIGHT - 27);
    boxfill8(vram, SCREEN_WIDTH, COL8_GRAY,       0,                SCREEN_HEIGHT - 26, SCREEN_WIDTH -  1, SCREEN_HEIGHT -  1);

    boxfill8(vram, SCREEN_WIDTH, COL8_WHITE,      3,                SCREEN_HEIGHT - 24, 59,                SCREEN_HEIGHT - 24);
    boxfill8(vram, SCREEN_WIDTH, COL8_WHITE,      2,                SCREEN_HEIGHT - 24,  2,                SCREEN_HEIGHT -  4);
    boxfill8(vram, SCREEN_WIDTH, COL8_DARK_GRAY,  3,                SCREEN_HEIGHT -  4, 59,                SCREEN_HEIGHT -  4);
    boxfill8(vram, SCREEN_WIDTH, COL8_DARK_GRAY, 59,                SCREEN_HEIGHT - 23, 59,                SCREEN_HEIGHT -  5);
    boxfill8(vram, SCREEN_WIDTH, COL8_BLACK,      2,                SCREEN_HEIGHT -  3, 59,                SCREEN_HEIGHT -  3);
    boxfill8(vram, SCREEN_WIDTH, COL8_BLACK,     60,                SCREEN_HEIGHT - 24, 60,                SCREEN_HEIGHT -  3);

    boxfill8(vram, SCREEN_WIDTH, COL8_DARK_GRAY, SCREEN_WIDTH - 47, SCREEN_HEIGHT - 24, SCREEN_WIDTH -  4, SCREEN_HEIGHT - 24);
    boxfill8(vram, SCREEN_WIDTH, COL8_DARK_GRAY, SCREEN_WIDTH - 47, SCREEN_HEIGHT - 23, SCREEN_WIDTH - 47, SCREEN_HEIGHT -  4);
    boxfill8(vram, SCREEN_WIDTH, COL8_WHITE,     SCREEN_WIDTH - 47, SCREEN_HEIGHT -  3, SCREEN_WIDTH -  4, SCREEN_HEIGHT -  3);
    boxfill8(vram, SCREEN_WIDTH, COL8_WHITE,     SCREEN_WIDTH -  3, SCREEN_HEIGHT - 24, SCREEN_WIDTH -  3, SCREEN_HEIGHT -  3);

    /* hlt */
    for (;;)
    {
        io_hlt();
    }
}


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

