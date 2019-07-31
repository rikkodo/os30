#include "bootpack.h"

/* FIFO関連 */

#define FLAGS_OVERRUN       0x0001;

void fifo8_init(struct FIFO8 * const fifo, int size, unsigned char * const buf)
{
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->p = 0;
    fifo->q = 0;
    return;
}

int fifo8_put(struct FIFO8 * const fifo, unsigned char data)
{
    if (fifo->free == 0)
    {
        fifo->flags |= FLAGS_OVERRUN;
        return -1;
    }
    fifo->buf[fifo->p] = data;
    fifo->p++;
    if (fifo->p >= fifo->size)
    {
        fifo->p = 0;
    }
    fifo->free--;
    return 0;
}

int fifo8_get(struct FIFO8 * const fifo)
{
    int data = 0;
    if (fifo->free == fifo->size)
    {
        /* バッファがからの時は -1 を返すようにする． */
        return -1;
    }

    data = fifo->buf[fifo->q];
    fifo->q++;
    if (fifo->q >= fifo->size)
    {
        fifo->q = 0;
    }
    fifo->free++;
    return data;
}

int  fifo8_status(const struct FIFO8 * const fifo)
{
    return fifo->size - fifo->free;
}
