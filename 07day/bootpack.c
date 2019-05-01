#include "bootpack.h"

static void wait_KBC_sendready(void);
static void init_keyboard(void);
static void enable_mouse(void);

////////////////////////////////////////////////////////////////////////////////
// main
void HariMain (void)
{
    /* read boot info */
    struct BOOTINFO *binfo = (struct BOOTINFO *) 0x00000ff0;

    /* message buffer */
    char s[128] = {};

    /* key buffer */
    unsigned char keybuf[KEYBUF_READ_MAX];
    unsigned char mousebuf[MOUSE_READ_MAX];

    /* mouse cursor */
    unsigned char mcursor[256] = {};
    int mx = 0;
    int my = 0;

    init_gdtidt();
    init_pic();
    io_sti();  /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */

    fifo8_init(&keyinfo, KEYBUF_READ_MAX, keybuf);
    fifo8_init(&mouseinfo, MOUSE_READ_MAX, mousebuf);
    io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */
    io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

    init_keyboard();

    /* パレット初期化 */
    init_palette();

    /* スクリーン初期化 */
    init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);

    /* マウスカーソルが画面中央になるように座標計算 */
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_DARK_CYAN);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    mysprintf(s, "(%d, %d)", mx, my);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);

    enable_mouse();

    /* hlt */
    for (;;)
    {
        io_cli();
        if (fifo8_status(&keyinfo) != 0)
        {
            int keydata = fifo8_get(&keyinfo);
            io_sti();
            mysprintf(s, "key: %x", keydata);
            boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 0, 16, 32 * 8 - 1, 31);
            putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_WHITE, s);
        }
        else if (fifo8_status(&mouseinfo) != 0)
        {
            int mousedata = fifo8_get(&mouseinfo);
            io_sti();
            mysprintf(s, "mou: %x", mousedata);
            boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 0, 32, 32 * 8 - 1, 63);
            putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_WHITE, s);
        }
        else
        {
            io_stihlt();
        }
    }
}

static void wait_KBC_sendready(void)
{
    for (;;)
    {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0)
        {
            break;
        }
    }
    return;
}

static void init_keyboard(void)
{
    /* init keyboard controler */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
    return;
}

static void enable_mouse(void)
{
    /* enable mouse */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    return;
}