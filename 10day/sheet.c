#include "bootpack.h"

/* FLAGS */
#define SHEET_USE      1 << 0;

static void sheet_refreshsub(const struct SHEET_CTL * ctl, int vx0, int vy0, int vx1, int vy1);

struct SHEET_CTL *shtctl_init(struct MEMMAN * memman, unsigned char *vram, int xsize, int ysize)
{
    struct SHEET_CTL *ctl = 0;
    int i = 0;
    ctl = (struct SHEET_CTL *)memman_alloc_4k(memman, sizeof(struct SHEET_CTL));
    if (ctl == 0)
    {
        return ctl;
    }

    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = -1;  // no sheet.
    for (i = 0; i < MAX_SEEHTS; i++)
    {
        ctl->sheets0[i].flags = 0;  // unused;
    }

    return ctl;
}

struct SHEET *sheet_alock(struct SHEET_CTL * const ctl)
{
    struct SHEET *sheet = 0;
    int i = 0;
    for (i = 0; i < MAX_SEEHTS; i++)
    {
        if (ctl->sheets0[i].flags == 0)
        {
            sheet = &(ctl->sheets0[i]);
            sheet->flags = SHEET_USE;
            sheet->height = -1;  // not shown;
            return sheet;
        }
    }

    return 0;  // all sheet in use.
}

void sheet_setbuf(struct SHEET * const sheet, unsigned char *buf, int xsize, int ysize, int col_inv)
{
    sheet->buf = buf;
    sheet->bxsize = xsize;
    sheet->bysize = ysize;
    sheet->col_inv = col_inv;
    return;
}

void sheet_updown(struct SHEET_CTL * const ctl, struct SHEET * const sheet, int height)
{
    int h = 0;
    int old = sheet->height;


    // 範囲外の補正
    if (height > ctl->top + 1)
    {
        height = ctl->top + 1;
    }
    if (height < -1)
    {
        height = -1;
    }

    sheet->height = height;

    //  並べ替え
    if (old > height)
    {
        // 以前より奥に行く
        if (height >= 0)
        {
            // 間にあるシートを引き上げる
            for (h = old; h > height; h--)
            {
                ctl->sheets[h] = ctl->sheets[h - 1];
                ctl->sheets[h]-> height = h;
            }
            ctl->sheets[height] = sheet;
        }
        else
        {
            // 非表示
            if (ctl->top > old)
            {
                for (h = old; h < ctl->top; h++)
                {
                    ctl->sheets[h] = ctl->sheets[h + 1];
                    ctl->sheets[h]->height = h;
                }
            }
            ctl->top--;
        }
        sheet_refreshsub(ctl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize);  // 再描画
    }
    else if (old < height)
    {
        // 以前より前に行く
        if (old > 0)
        {
            // 間のものを下げる
            for (h = old; h < height; h++)
            {
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sheet;
        }
        else
        {
            // 非表示から表示
            for (h = ctl->top; h >= height; h--)
            {
                ctl->sheets[h + 1] = ctl->sheets[h];
                ctl->sheets[h + 1]->height = h + 1;
            }
            ctl->sheets[height] = sheet;
            ctl->top++;
        }
        sheet_refreshsub(ctl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize);  // 再描画
    }
    return;
}

void sheet_refresh(const struct SHEET_CTL *ctl, struct SHEET * const sht, int bx0, int by0, int bx1, int by1)
{
    if (sht->height >= 0)
    {
        sheet_refreshsub(ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1);
    }
    return;
}

static void sheet_refreshsub(const struct SHEET_CTL * ctl, int vx0, int vy0, int vx1, int vy1)
{
    int h = 0;
    int bx = 0;
    int by = 0;
    int bx0 = 0;
    int by0 = 0;
    int bx1 = 0;
    int by1 = 0;
    int vx = 0;
    int vy = 0;

    unsigned char *buf = 0;
    unsigned char col = 0;
    unsigned char *vram = ctl->vram;

    struct SHEET * sht = 0;

    for (h = 0; h <= ctl->top; h++)
    {
        sht = ctl->sheets[h];
        buf = sht->buf;
        bx0 = vx0 - sht->vx0;
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        if (bx0 < 0) {bx0 = 0;}
        if (by0 < 0) {by0 = 0;}
        if (bx1 > sht->bxsize) {bx1 = sht->bxsize;}
        if (by1 > sht->bysize) {by1 = sht->bysize;}
        for (by = by0; by < by1; by++)
        {
            vy = sht->vy0 + by;
            for (bx = bx0; bx < bx1; bx++)
            {
                vx = sht->vx0 + bx;
                col = buf[by * sht->bxsize + bx];
                if (col != sht->col_inv)
                {
                    vram[vy * ctl->xsize + vx] = col;
                }
            }
        }
    }

    return;
}


void sheet_slide(struct SHEET_CTL * const ctl, struct SHEET * const sht, int vx0, int vy0)
{
    int pre_vx0 = sht->vx0;
    int pre_vy0 = sht->vy0;
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    if (sht->height >= 0)
    {
        // 表示中ならば再描画
        sheet_refreshsub(ctl, pre_vx0, pre_vy0, pre_vx0 + sht->bxsize, pre_vy0 + sht->bysize);
        sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize);
    }
    return;
}

void sheet_free(struct SHEET_CTL * const ctl, struct SHEET * const sht)
{
    if (sht->height >= 0)
    {
        // 表示中の場合消す。
        sheet_updown(ctl, sht, -1);
    }

    sht->flags = 0;
    return;
}
