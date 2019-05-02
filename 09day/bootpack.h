////////////////////////////////////////////////////////////////////////////////
// haribote.asm
struct BOOTINFO
{                            /* 0x0ff0-0x0fff */
        unsigned char cyls;  /* ブートセクタはどこまでディスクを読んだのか */
        unsigned char leds;  /* ブート時のキーボードのLEDの状態 */
        unsigned char vmode; /* ビデオモード  何ビットカラーか */
        unsigned char reserve;
        unsigned short scrnx, scrny; /* 画面解像度 */
        unsigned char *vram;
};
#define ADR_BOOTINFO 0x00000ff0

////////////////////////////////////////////////////////////////////////////////
// nasmfunc.asm
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
int load_cr0(void);
void store_cr0(int cr0);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

////////////////////////////////////////////////////////////////////////////////
// fifo.c
struct FIFO8
{
        unsigned char *buf;
        int p, q, size, free, flags;
};
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int  fifo8_put(struct FIFO8 *fifo, unsigned char data);
int  fifo8_get(struct FIFO8 *fifo);
int  fifo8_status(struct FIFO8 *fifo);

////////////////////////////////////////////////////////////////////////////////
// graphic.c
void init_palette(void);
void set_palette(int start, int cnt, unsigned char *rgb);
void init_screen8(unsigned char *vram, int x, int y);
void init_mouse_cursor8(unsigned char *mouse, char back_col_no);

void boxfill8(
    unsigned char *vram,
    int screen_width,
    unsigned char col_no,
    int x0, int y0, int x1, int y1);

void putblock8_8(
    unsigned char *vram,
    int screen_width,
    int pxsize, int pysize,
    int px0, int py0,
    unsigned char *buf, int bxsize);

void putfont8(
    unsigned char *vram,
    int screen_width,
    unsigned char col_no,
    const unsigned char *font,
    int x, int y);

void putfonts8_asc(
    unsigned char *vram,
    int screen_width,
    int x, int y,
    unsigned char col_no,
    const char *str);

#define COL_CNT (16)

#define COL8_BLACK (0)
#define COL8_RED (1)
#define COL8_GREEN (2)
#define COL8_YELLOW (3)
#define COL8_BLUE (4)
#define COL8_PURPLE (5)
#define COL8_CYAN (6)
#define COL8_WHITE (7)
#define COL8_GRAY (8)
#define COL8_DARK_RED (9)
#define COL8_DARK_GREEN (10)
#define COL8_DARK_YELLOW (11)
#define COL8_DARK_BLUE (12)
#define COL8_DARK_PURPLE (13)
#define COL8_DARK_CYAN (14)
#define COL8_DARK_GRAY (15)

////////////////////////////////////////////////////////////////////////////////
// dsctbl.c
struct SEGMENT_DESCRIPTOR
{
        short limit_low, base_low;
        char base_mid, access_right;
        char limit_high, base_high;
};
struct GATE_DESCRIPTOR
{
        short offset_low, selector;
        char dw_count, access_right;
        short offset_high;
};
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
#define ADR_IDT 0x0026f800
#define LIMIT_IDT 0x000007ff
#define ADR_GDT 0x00270000
#define LIMIT_GDT 0x0000ffff
#define ADR_BOTPAK 0x00280000
#define LIMIT_BOTPAK 0x0007ffff
#define AR_DATA32_RW 0x4092
#define AR_CODE32_ER 0x409a
#define AR_INTGATE32 0x008e

////////////////////////////////////////////////////////////////////////////////
// mysprintf.c
void mysprintf(char *str, char *fmt, ...);

////////////////////////////////////////////////////////////////////////////////
// kyeboard.c
#define KEYBUF_READ_MAX 32
extern struct FIFO8 keyinfo;
#define PORT_KEYDAT          0x0060    /* キーボードポート */
#define PORT_KEYCMD          0x0064
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(void);

////////////////////////////////////////////////////////////////////////////////
// mouse.c
#define MOUSE_READ_MAX 128
extern struct FIFO8 mouseinfo;
struct MOUSE_DECODE
{
    unsigned char buf[3], phase;
    int x, y, btn;
};
void inthandler2c(int *esp);
void enable_mouse(struct MOUSE_DECODE *mousedec);
int mouse_decode(struct MOUSE_DECODE *mousedec, int mousedata);

////////////////////////////////////////////////////////////////////////////////
// interrupt.c
void init_pic(void);
void inthandler27(int *esp);
#define PIC0_ICW1 0x0020
#define PIC0_OCW2 0x0020
#define PIC0_IMR 0x0021
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC1_ICW1 0x00a0
#define PIC1_OCW2 0x00a0
#define PIC1_IMR 0x00a1
#define PIC1_ICW2 0x00a1
#define PIC1_ICW3 0x00a1
#define PIC1_ICW4 0x00a1
