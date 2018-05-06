; BOOT INFO

CYSL    EQU     0x0ff0      ; 読み込みシリンダ数 ブートセクタが設定
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
fin:
        HLT
        JMP     fin
