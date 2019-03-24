extern void io_hlt(void);
extern void write_mem8(void*, int);

void HariMain (void)
{
    int i = 0;
    for (i = 0xa0000; i< 0xaffff; i++)
    {
        write_mem8((void*)i, 15);
    }
    /* hlt */
    for (;;)
    {
        io_hlt();
    }
}
