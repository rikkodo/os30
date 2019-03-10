extern void io_hlt(void);


void HariMain (void)
{
    /* hlt */
fin:
    io_hlt();  // call nasm func
    goto fin;
}
