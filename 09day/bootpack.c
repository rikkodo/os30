#include "bootpack.h"

////////////////////////////////////////////////////////////////////////////////
// main
void HariMain (void)
{
    /* read boot info */
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;

    /* message buffer */
    char s[128] = {};

    /* key buffer */
    unsigned char keybuf[KEYBUF_READ_MAX];
    unsigned char mousebuf[MOUSE_READ_MAX];

    struct MOUSE_DECODE mousedec = {};

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

    mysprintf(s, "(%03d, %03d)", mx, my);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);

    enable_mouse(&mousedec);

    /* hlt */
    for (;;)
    {
        io_cli();
        if (fifo8_status(&keyinfo) != 0)
        {
            int keydata = fifo8_get(&keyinfo);
            io_sti();
            mysprintf(s, "key: %02x", keydata);
            boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 0, 16, 32 * 8 - 1, 31);
            putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_WHITE, s);
        }
        else if (fifo8_status(&mouseinfo) != 0)
        {
            int mousedata = fifo8_get(&mouseinfo);
            io_sti();

            if (mouse_decode(&mousedec, mousedata) == 1)
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
                boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 0, 32, 32 * 8 - 1, 63);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_WHITE, s);

                /* move cursor */
                boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, mx, my, mx + 15, my + 15);

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
                if (mx > binfo->scrnx - 16)
                {
                    mx = binfo->scrnx - 16;
                }
                if (my > binfo->scrny - 16)
                {
                    my = binfo->scrny - 16;
                }
            }

            mysprintf(s, "(%03d, %03d)", mx, my);
            boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 0, 0, 79, 15);
            putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);
            putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
        }
        else
        {
            io_stihlt();
        }
    }
}
