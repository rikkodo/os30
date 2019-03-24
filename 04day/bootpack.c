extern void io_hlt(void);
extern void io_cli(void);
extern void io_out8(int port, int data);
extern int  io_load_eflags(void);
extern void io_store_eflags(int eflags);

#define COL_CNT (16)

static void init_palette(void);
static void set_palette(int start, int cnt, unsigned char *rgb);


void HariMain (void)
{
    int i = 0;
    char *p = (char *)0;

    /* パレット初期化 */
    init_palette();

    /* 色ぬり */
    for (i = 0xa0000; i <= 0xaffff; i++)
    {
        char* p = (char *)i;
        *p = i & 0x0f;
    }

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
