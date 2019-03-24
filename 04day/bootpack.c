extern void io_hlt(void);
extern void write_mem8(void*, int);

void HariMain (void)
{
    int i = 0;

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
