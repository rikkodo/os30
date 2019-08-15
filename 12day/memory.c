#include "bootpack.h"

#define EFLAGS_AC_BIT        0x00040000
#define CR0_CACHE_DISABLE    0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
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

unsigned int memman_alloc_4k(struct MEMMAN *memman, unsigned int reqsize)
{
    unsigned int ret = 0;
    reqsize = (reqsize + 0xfff)  & 0xfffff000;
    ret = memman_alloc(memman, reqsize);
    return ret;
}
int memman_free_4k(struct MEMMAN *memman, unsigned int addr, unsigned int size)
{
    int ret = 0;
    size = (size + 0xfff) & 0xfffff000;
    ret = memman_free(memman, addr, size);
    return ret;
}

