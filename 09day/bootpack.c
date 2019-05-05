#include "bootpack.h"

#define EFLAGS_AC_BIT        0x00040000
#define CR0_CACHE_DISABLE    0x60000000
static unsigned int memtest(unsigned int start, unsigned int end);

#define MEMMAN_MAX          4090

struct FREEINFO {
    unsigned int addr, size;
};

struct MEMMAN {
    unsigned int infocnt;              /* 空き情報個数 */
    unsigned int infocntmax;           /* 空き情報個数最大値: log用 */
    unsigned int lostsum;              /* 解放に失敗した合計サイズ */
    unsigned int lostcnt;              /* 解放に失敗した回数 */
    struct FREEINFO info[MEMMAN_MAX];  /*  */
};

void memman_init(struct MEMMAN *memman);
unsigned int memman_total(struct MEMMAN *memman);
unsigned int memman_alloc(struct MEMMAN *memman, unsigned int reqsize);
int memman_free(struct MEMMAN *memman, unsigned int addr, unsigned int size);

#define MEMMAN_ADDR         0x003c0000

static void HariMain_in(void);

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
    unsigned char mcursor[256] = {};
    int mx = 0;
    int my = 0;
    int i = 0;

    /* memory map */
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    unsigned int memtotal = 0;

    /* message buffer */
    char s[128] = {};

    fifo8_init(&keyinfo, KEYBUF_READ_MAX, keybuf);
    fifo8_init(&mouseinfo, MOUSE_READ_MAX, mousebuf);
    io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */
    io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

    init_keyboard();
    enable_mouse(&mousedec);


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

    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x009e000);  /* 0x00001000 -- 0x0009eff */
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    mysprintf(s, "memory %dMB    free %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 48, COL8_WHITE, s);

    /* hlt */
    for (;;)
    {
        io_cli();
        if (fifo8_status(&keyinfo) != 0)
        {
            i = fifo8_get(&keyinfo);
            io_sti();
            mysprintf(s, "key: %02x", i);
            boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 0, 16, 32 * 8 - 1, 31);
            putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_WHITE, s);
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
                boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 0, 32, 32 * 8 - 1, 47);
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

static unsigned int memtest(unsigned int start, unsigned int end)
{
    char flag486 = 0;
    unsigned int i = 0;
    unsigned int cr0 = 0;
    unsigned int eflags = 0;

    /* 386? over 486? */
    /* 386 does not have AC Bit -> write 1 has no effect. keep 0 */
    eflags = io_load_eflags();
    eflags |= EFLAGS_AC_BIT;  /* AC-Bit -> 1 */
    io_store_eflags(eflags);
    eflags = io_load_eflags();
    if ((eflags & EFLAGS_AC_BIT) != 0)
    {
        /* 486 */
        flag486 = 1;
    }
    /* reset AC FLAG */
    eflags &= ~EFLAGS_AC_BIT;  /* AC-Bit -> 0 */
    io_store_eflags(eflags);

    if (flag486 != 0)
    {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;  /* disable cache */
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if (flag486 != 0)
    {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;  /* enable cache */
        store_cr0(cr0);
    }

    return i;
}

void memman_init(struct MEMMAN *memman)
{
    memman->infocnt = 0;
    memman->infocntmax = 0;
    memman->lostsum = 0;
    memman->lostcnt = 0;
    return;
}

unsigned int memman_total(struct MEMMAN *memman)
{
    unsigned int i = 0;
    unsigned int t = 0;
    for (i = 0; i < memman->infocnt; i++)
    {
        t += memman->info[i].size;
    }
    return t;
}

unsigned int memman_alloc(struct MEMMAN *memman, unsigned int reqsize)
{
    unsigned int i = 0;
    unsigned int adr = 0;

    for (i = 0; i < memman->infocnt; i++)
    {
        if (memman->info[i].size >= reqsize)
        {
            /* 利用可能な空き */
            adr = memman->info[i].addr;
            memman->info[i].addr += reqsize;
            memman->info[i].size -= reqsize;
            if (memman->info[i].size == 0)
            {
                /* info[i]に空きがなくなったので前詰め */
                memman->infocnt--;
                for (/* NOP */; i< memman->infocnt; i++)
                {
                    memman->info[i] = memman->info[i + 1];
                }
            }
            return adr;
        }
    }
    return 0;  /* 空きが無い */
}

int memman_free(struct MEMMAN *memman, unsigned int addr, unsigned int size)
{
    unsigned int i = 0;
    unsigned int j = 0;

    /* 挿入箇所の探索 */
    for (i = 0; i < memman->infocnt; i++)
    {
        if (memman->info[i].addr > addr)
        {
            break;
        }
    }

    /* 前に情報あり */
    if (i > 0)
    {
        if (memman->info[i - 1].addr + memman->info[i - 1].size == addr)
        {
            /* 単純に接続可能 */
            memman->info[i - 1].size += size;
            if (i < memman->infocnt)
            {
                if (addr + size == memman->info[i].addr)
                {
                    /* 後ろとも接続可能 */
                    memman->info[i - 1].size += memman->info[i].size;
                    /* info[i]に空きがなくなったので前詰め */
                    memman->infocnt--;
                    for (/* NOP */; i < memman->infocnt; i++)
                    {
                        memman->info[i] = memman->info[i + 1];
                    }
                }
            }
            return 0;
        }
    }

    /* 前との接合はできなかった． */
    if (i < memman->infocnt)
    {
        if (addr + size == memman->info[i].addr)
        {
            /* 後ろとならば接続可能 */
            memman->info[i].addr = addr;
            memman->info[i].size += size;
            return 0;
        }
    }

    /* 前とも後ろとも接合できず */
    if (memman->infocnt < MEMMAN_MAX)
    {
        /* ずらす */
        for (j = memman->infocnt; j > i; j--)
        {
            memman->info[j] = memman->info[j - 1];
        }
        memman->infocnt++;
        if (memman->infocntmax < memman->infocnt)
        {
            memman->infocntmax = memman->infocnt;
        }

        memman->info[i].addr = addr;
        memman->info[j].size = size;
        return 0;
    }

    /* 格納不可能 */
    memman->lostcnt++;
    memman->lostsum += size;
    return -1;
}
