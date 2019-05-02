#include "bootpack.h"

#define KEYCMD_SENDTO_MOUSE  0xd4
#define MOUSECMD_ENABLE      0xf4

void enable_mouse(struct MOUSE_DECODE *mousedec)
{
    /* enable mouse */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);

    mousedec->phase = 0;
    return;
}

int mouse_decode(struct MOUSE_DECODE *mousedec, int mousedata)
{
    switch (mousedec->phase)
    {
    case 0: /* wait for 0xfa */
        if (mousedata == 0xfa)
        {
            mousedec->phase = 1;
        }
        return 0;
    case 1: /* wait for mouse 1 */
        if ((mousedata & 0xc8) == 0x08)
        {
            /* valid bite 1 */
            mousedec->buf[0] = mousedata;
            mousedec->phase = 2;
        }
        return 0;
    case 2: /* wait for mouse 2 */
        mousedec->buf[1] = mousedata;
        mousedec->phase = 3;
        return 0;
    case 3: /* wait for mouse 3 */
        mousedec->buf[2] = mousedata;
        mousedec->phase = 1;

        /* read data */
        mousedec->btn = mousedec->buf[0] & 0x07;
        mousedec->x = mousedec->buf[1];
        mousedec->y = mousedec->buf[2];

        if ((mousedec->buf[0] & 0x10) != 0)
        {
            mousedec->x |= 0xffffff00;
        }
        if ((mousedec->buf[0] & 0x20) != 0)
        {
            mousedec->y |= 0xffffff00;
        }

        /* upside down */
        mousedec->y *= -1;

        return 1;
    }

    /* unreach */
    return -1;
}