#include "bootpack.h"


static void HariMain_in(void);
static void make_window8(unsigned char *buf, int xsize, int ysize, const char * title);


////////////////////////////////////////////////////////////////////////////////
// main
void HariMain (void)
{
    init_gdtidt();
    init_pic();
    io_sti();  /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */

    HariMain_in();
    return;
}

static void HariMain_in (void)
{
    /* read boot info */
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;

    /* key buffer */
    unsigned char keybuf[KEYBUF_READ_MAX] = {};
    unsigned char mousebuf[MOUSE_READ_MAX] = {};

    struct MOUSE_DECODE mousedec = {};

    /* mouse cursor */
    int mx = 0;
    int my = 0;
    int i = 0;

    /* memory map */
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    unsigned int memtotal = 0;

    /* sheet */
    struct SHEET_CTL *shtctl = 0;
    struct SHEET *sht_back = 0;
    struct SHEET *sht_mouse = 0;
    unsigned char *buf_back = 0;
    unsigned char buf_mouse[256] = {};

    struct SHEET *sht_win = 0;
    unsigned char *buf_win = 0;

    /* message buffer */
    char s[128] = {};

    fifo8_init(&keyinfo, KEYBUF_READ_MAX, keybuf);
    fifo8_init(&mouseinfo, MOUSE_READ_MAX, mousebuf);
    io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */
    io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

    init_keyboard();
    enable_mouse(&mousedec);

    /* メモリ初期化 */
    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x009e000);  /* 0x00001000 -- 0x0009eff */
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    /* パレット初期化 */
    init_palette();

    /* シート作成 */
    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);

    /* スクリーン初期化 */
    sht_back = sheet_alock(shtctl);
    buf_back = (unsigned char*) memman_alloc_4k(memman, binfo->scrny * binfo->scrnx);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);  // -1 -> 透明色なし
    init_screen8(buf_back, binfo->scrnx, binfo->scrny);
    // init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
    sheet_slide(sht_back, 0, 0);
    sheet_updown(sht_back, 0);

    /* テストウインドウ作成 */
    sht_win = sheet_alock(shtctl);
    buf_win = (unsigned char*) memman_alloc_4k(memman, 160 * 68);
    sheet_setbuf(sht_win, buf_win, 160, 68, -1);
    make_window8(buf_win, 160, 68, "window");
    putfonts8_asc(buf_win, 160, 24, 28, COL8_BLACK, "Welcome to");
    putfonts8_asc(buf_win, 160, 24, 44, COL8_BLACK, "  Haribote-OS!");
    sheet_slide(sht_win, 80, 72);
    sheet_updown(sht_win, 1);

    /* マウスカーソル初期化 */
    sht_mouse = sheet_alock(shtctl);
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
    /* マウスカーソルが画面中央になるように座標計算 */
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(buf_mouse, 99);
    sheet_slide(sht_mouse, mx, my);
    sheet_updown(sht_mouse, 2);

    mysprintf(s, "memory %dMB    free %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc(buf_back, binfo->scrnx, 0, 48, COL8_WHITE, s);
    sheet_refresh(sht_back, 0, 48, binfo->scrnx, 64);

    /* hlt */
    for (;;)
    {
        io_cli();
        if (fifo8_status(&keyinfo) != 0)
        {
            i = fifo8_get(&keyinfo);
            io_sti();
            mysprintf(s, "key: %02x", i);
            boxfill8(buf_back, binfo->scrnx, COL8_DARK_CYAN, 0, 16, 32 * 8 - 1, 32 - 1);
            putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_WHITE, s);
            sheet_refresh(sht_back, 0, 16, 32 * 8, 32);
        }
        else if (fifo8_status(&mouseinfo) != 0)
        {
            i = fifo8_get(&mouseinfo);
            io_sti();

            if (mouse_decode(&mousedec, i) == 1)
            {
                /* output */
                mysprintf(s, "[lcr] %3d %3d", mousedec.x, mousedec.y);
                if ((mousedec.btn & 0x01) != 0)
                {
                    s[1] = 'L';
                }
                if ((mousedec.btn & 0x02) != 0)
                {
                    s[3] = 'R';
                }
                if ((mousedec.btn & 0x04) != 0)
                {
                    s[2] = 'C';
                }
                boxfill8(buf_back, binfo->scrnx, COL8_DARK_CYAN, 0, 32, 32 * 13 - 1, 48 - 1);
                putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_WHITE, s);
                sheet_refresh(sht_back, 0, 32, 32 * 13, 48);

                mx += mousedec.x;
                my += mousedec.y;
                if (mx < 0)
                {
                    mx = 0;
                }
                if (my < 0)
                {
                    my = 0;
                }
                if (mx > binfo->scrnx - 1)
                {
                    mx = binfo->scrnx - 1;
                }
                if (my > binfo->scrny - 1)
                {
                    my = binfo->scrny - 1;
                }
            }

            mysprintf(s, "(%03d, %03d)", mx, my);
            boxfill8(buf_back, binfo->scrnx, COL8_DARK_CYAN, 0, 0, 79, 15);
            putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_WHITE, s);
            sheet_refresh(sht_back, 0, 0, 80, 16);
            sheet_slide(sht_mouse, mx, my);  // -> 再描画は関数内部で実施
        }
        else
        {
            io_stihlt();
        }
    }
}

/* WINDOW */

void make_window8(unsigned char *buf, int xsize, int ysize, const char * title)
{
    static char closebtn[14][16] = {
        "OOOOOOOOOOOOOOO@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQQQ@@QQQQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "0$$$$$$$$$$$$$$@",
        "@@@@@@@@@@@@@@@@",
    };

    int x = 0;
    int y = 0;

    char col = 0;
    //                                       ///////////,///////////,///////////,///////////,
    boxfill8(buf, xsize, COL8_GRAY,          0,          0,          xsize - 1,  0           );
    boxfill8(buf, xsize, COL8_WHITE,         1,          1,          xsize - 2,  1           );
    boxfill8(buf, xsize, COL8_GRAY,          0,          0,          0,          ysize - 1   );
    boxfill8(buf, xsize, COL8_WHITE,         1,          1,          1,          ysize - 2   );
    boxfill8(buf, xsize, COL8_DARK_GRAY,     xsize - 2,  1,          xsize - 2,  ysize - 2   );
    boxfill8(buf, xsize, COL8_BLACK,         xsize - 1,  0,          xsize - 1,  ysize - 1   );

    boxfill8(buf, xsize, COL8_GRAY,          2,          2,          xsize - 3,  ysize - 3   );
    boxfill8(buf, xsize, COL8_DARK_BLUE,     3,          3,          xsize - 4,  20          );

    boxfill8(buf, xsize, COL8_DARK_GRAY,     1,          ysize - 2,  xsize - 2,  ysize - 2   );
    boxfill8(buf, xsize, COL8_BLACK,         0,          ysize - 1,  xsize - 1,  ysize - 1   );

    putfonts8_asc(buf, xsize, 24, 4, COL8_WHITE, title);

    for (y = 0; y < 14; y++)
    {
        for (x = 0; x < 16; x++)
        {
            switch(closebtn[y][x])
            {
                case '@':
                    col = COL8_BLACK;
                    break;

                case '$':
                    col = COL8_DARK_GRAY;
                    break;

                case 'Q':
                    col = COL8_GRAY;
                    break;

                default:
                    col = COL8_WHITE;
                    break;
            }
            buf[(5+y) * xsize + (xsize - 21 + x)] = col;
        }
    }
}