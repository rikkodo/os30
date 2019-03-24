; nasmfunc.asm


section .text
    GLOBAL  io_hlt
    GLOBAL  write_mem8


io_hlt:         ; void io_hlt(void);
    HLT
    RET

write_mem8:     ; void write_mem8(void* addr, int data)
    MOV     ECX,[ESP+4]     ; ECX <- addr
    MOV     AL,[ESP+8]      ; AL  <- data
    MOV     [ECX],AL        ; *[ECX] <- AL = *[addr] <- AL
    RET
