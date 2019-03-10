BOTPAK  EQU     0x00280000  ; bootpackのロード先
DSKCAC  EQU     0x00100000  ; ディスクキャッシュの場所
DSKCAC0 EQU     0x00008000  ; ディスクキャッシュの場所（リアルモード）


; BOOT INFO

CYLS    EQU     0x0ff0      ; 読み込みシリンダ数 ブートセクタが設定
LEDS    EQU     0x0ff1      ; キーボードの LED 状態を記録
VMODE   EQU     0x0ff2      ; 色数 何ビットカラーか
SCRNX   EQU     0x0ff4      ; 解像度 X
SCRNY   EQU     0x0ff6      ; 解像度 Y
VRAM    EQU     0x0ff8      ; グラフィックバッファ開始位置

; 読み込み地点
        ORG     0xc200

; グラフィックス指定
        MOV     AL,0x13     ; VGA グラッフィクス 320x200x8bit Color
        MOV     AH,0x00
        INT     0x10
        MOV     BYTE [VMODE],8     ; 画面モードを記録
        MOV     WORD [SCRNX],320
        MOV     WORD [SCRNY],200
        MOV     DWORD [VRAM],0x000a0000

; キーボードの状態を BIOS から取得
        MOV     AH,0x02
        INT     0x16        ; keyboard BIOS
        MOV     [LEDS],AL

; 割り込みブロック
;   AT互換機の仕様では、PICの初期化をするなら、
;   こいつをCLI前にやっておかないと、たまにハングアップする
;   PICの初期化はあとでやる
        MOV     AL,0xff
        OUT     0x21,AL
        NOP                 ; OUT 命令を連続させるとうまくいかない？
        OUT     0xa1,AL

        CLI                 ; さらに CPU レベルでも割り込み禁止

; CPUから1MB以上のメモリにアクセスできるように、A20GATEを設定
        CALL    waitkbdout
        MOV     AL,0xd1
        OUT     0x64,AL
        CALL    waitkbdout
        MOV     AL,0xdf     ; enable A20
        OUT     0x60,AL
        CALL    waitkbdout

; プロテクトモードへ移行
; [INSTRSET "i486p"]
        LGDT    [GDTR0]     ; 暫定GDTを設定
        MOV     EAX,CR0
        AND     EAX,0x7fffffff  ; bit31を0にする（ページング禁止のため）
        OR      EAX,0x00000001  ; bit0を1にする（プロテクトモード移行のため）
        MOV     CR0,EAX
        JMP     pipelineflush
pipelineflush:
        MOV     AX,1*8      ;  読み書き可能セグメント32bit
        MOV     DS,AX
        MOV     ES,AX
        MOV     FS,AX
        MOV     GS,AX
        MOV     SS,AX

; bootpack の転送
        MOV     ESI,bootpack    ; from
        MOV     EDI,BOTPAK      ; to
        MOV     ECX,512*1024/4
        CALL    memcpy

; ディスクデータ
; ブートセクタ
        MOV     ESI,0x7c00      ; from
        MOV     EDI,DSKCAC      ; to
        MOV     ECX,512/4
        CALL    memcpy

; 残り
        MOV     ESI,DSKCAC0+512 ; from
        MOV     EDI,DSKCAC+512  ; to
        MOV     ECX,0
        MOV     CL,BYTE [CYLS]
        IMUL    ECX,512*18*2/4  ; CYLS to bytes/4
        SUB     ECX,512/4       ; DEC IPL
        CALL    memcpy


; start bootpack
        MOV     EBX,BOTPAK
        MOV     ECX,[EBX+16]
        ADD     ECX,3           ; ECX += 3
        SHR     ECX,2           ; ECX /= 4
        JZ      skip            ; 転送するものがない
        MOV     ESI,[EBX+20]    ; from
        ADD     ESI,EBX
        MOV     EDI,[EBX+12]    ; to
        CALL    memcpy
skip:
        MOV     ESP,[EBX+12]    ; stack の初期値
        JMP     DWORD 2*8:0x0000001b

waitkbdout:
        IN      AL,0x64
        AND     AL,0x02
        JNZ     waitkbdout      ; ANDの結果が0でなければwaitkbdoutへ
        RET

memcpy:
        MOV     EAX,[ESI]
        ADD     ESI,4
        MOV     [EDI],EAX
        ADD     EDI,4
        SUB     ECX,1
        JNZ     memcpy          ; 引き算した結果が0でなければmemcpy
        RET
; memcpyはアドレスサイズプリフィクスを入れ忘れなければ、ストリング命令でも書ける


GDT0:
        TIMES   8 DB 0          ; ヌルセレクタ
        DW      0xffff,0x0000,0x9200,0x00cf ; 読み書き可能セグメント32bit
        DW      0xffff,0x0000,0x9a28,0x0047 ; 実行可能セグメント32bit（bootpack用)

        DW      0

GDTR0:
        DW      8*3-1
        DD      GDT0

        ALIGNB  16, DB 0

bootpack:


; おしまい．
fin:
        HLT
        JMP     fin
