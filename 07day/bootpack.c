#include "bootpack.h"

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

    /* mouse cursor */
    unsigned char mcursor[256] = {};
    int mx = 0;
    int my = 0;

    init_gdtidt();
    init_pic();
    io_sti();  /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */

    fifo8_init(&keyinfo, KEYBUF_READ_MAX, keybuf);
    io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */
    io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

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

    /* hlt */
    for (;;)
    {
        io_cli();
        if (fifo8_status(&keyinfo) == 0)
        {
            io_stihlt();
        }
        else
        {
            int keydata = fifo8_get(&keyinfo);
            io_sti();
            mysprintf(s, "key: %x", keydata);
            boxfill8(binfo->vram, binfo->scrnx, COL8_BLACK, 0, 0, 32 * 8 - 1, 15);
            putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);
        }
    }
}
