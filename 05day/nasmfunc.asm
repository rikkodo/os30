; nasmfunc.asm


section .text
    GLOBAL  io_hlt, io_cli, io_sti, io_stihlt
    GLOBAL  io_in8,  io_in16,  io_in32
    GLOBAL  io_out8, io_out16, io_out32
    GLOBAL  io_load_eflags, io_store_eflags


io_hlt:                     ; void io_hlt(void);
    HLT
    RET

io_cli:                     ; void io_cli(void);
    CLI
    RET

io_sti:                     ; void io_sti(void);
    STI
    RET

io_stihlt:                  ; void io_stihlt(void);
    STI
    HLT
    RET


io_in8:                     ; int io_in8(int port);
    MOV     EDX,[ESP+4]     ; EDX <- port
    MOV     EAX,0
    IN      AL,DX           ; AL(= ret(7..0)) <- DX
    RET

io_in16:                    ; int io_in16(int port);
    MOV     EDX,[ESP+4]     ; EDX <- port
    MOV     EAX,0
    IN      AX,DX           ; AX(= ret(f..0)) <- DX
    RET

io_in32:                    ; int io_in32(int port);
    MOV     EDX,[ESP+4]     ; EDX <- port
    IN      EAX,DX          ; EAX(= ret) <- DX
    RET


io_out8:                    ; int io_out8(int port, int data);
    MOV     EDX,[ESP+4]     ; EDX <- port
    MOV     AL,[ESP+8]      ; AL <- data(7..0)
    OUT     DX,AL
    RET

io_out16:                   ; int io_out16(int port, int data);
    MOV     EDX,[ESP+4]     ; EDX <- port
    MOV     AX,[ESP+8]      ; AX <- data(f..0)
    OUT     DX,AX
    RET

io_out32:                   ; int io_out32(int port, int data);
    MOV     EDX,[ESP+4]     ; EDX <- port
    MOV     EAX,[ESP+8]     ; EAX <- data
    OUT     DX,EAX
    RET


io_load_eflags:             ; int io_load_eflags(void);
    PUSHFD                  ; PUSH EFLAGS
    POP     EAX             ; EAX <- EFLAGS
    RET

io_store_eflags:            ; void io_store_eflags(int eflags);
    MOV     EAX,[ESP+4]     ; EDX <- eflags
    PUSH    EAX             ; STACKTOP <- EAX
    POPFD                   ; EFLAGS <- STACKTOP
    RET
